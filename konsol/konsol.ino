#include <WebServer.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include "I2Cdev.h"
#include "MPU6050_6Axis_MotionApps20.h"

#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
#include "Wire.h"
#endif

//Kalibrasyon butonu
#define butonCal 4
//Reset butonu
#define butonRes 5

// the IP of the machine to which you send msgs - this should be the correct IP in most cases (see note in python code)
#define CONSOLE_IP "192.168.4.2"
#define CONSOLE_PORT 3333
const char* ssid = "ESP32-Access-Point";
const char* password = "123456789";
WiFiUDP udp;
WebServer server(80);

MPU6050 mpu(0x68);

#define OUTPUT_READABLE_EULER

#define INTERRUPT_PIN 15  // use pin 15 on ESP32, With ESP32, we can configure all the GPIO pins as hardware interrupt sources.
#define LED_PIN 2 // (Arduino is 13, Teensy is 11, Teensy++ is 6, ESP32 is )
bool blinkState = false;

//Are we currently connected?
boolean connected = false;

// MPU control/status vars
bool dmpReady = false;  // set true if DMP init was successful
uint8_t mpuIntStatus;   // holds actual interrupt status byte from MPU
uint8_t devStatus;      // return status after each device operation (0 = success, !0 = error)
uint16_t packetSize;    // expected DMP packet size (default is 42 bytes)
uint16_t fifoCount;     // count of all bytes currently in FIFO
uint8_t fifoBuffer[64]; // FIFO storage buffer

// orientation/motion vars
Quaternion q;           // [w, x, y, z]         quaternion container
VectorInt16 aa;         // [x, y, z]            accel sensor measurements
VectorInt16 aaReal;     // [x, y, z]            gravity-free accel sensor measurements
VectorInt16 aaWorld;    // [x, y, z]            world-frame accel sensor measurements
VectorFloat gravity;    // [x, y, z]            gravity vector
float euler[3];         // [psi, theta, phi]    Euler angle container
float ypr[3];           // [yaw, pitch, roll]   yaw/pitch/roll container and gravity vector

// packet structure for InvenSense teapot demo
uint8_t teapotPacket[14] = { '$', 0x02, 0, 0, 0, 0, 0, 0, 0, 0, 0x00, 0x00, '\r', '\n' };


// ================================================================
// ===               INTERRUPT DETECTION ROUTINE                ===
// ================================================================

volatile bool mpuInterrupt = false;     // indicates whether MPU interrupt pin has gone high
void dmpDataReady() {
  mpuInterrupt = true;
}

// ================================================================
// ===                      INITIAL SETUP                       ===
// ================================================================

void setup() {

  pinMode(butonCal, INPUT);
  pinMode(butonRes, INPUT);
  
  // I2C iletişim ayarlanıyor
  Wire.begin();
  Wire.setClock(400000); // 400kHz I2C clock.

  // seri iletişim başlatılıyor
  Serial.begin(115200);
  while (!Serial);

  // mpu6050 başlatılıyor
  Serial.println(F("Initializing I2C devices..."));
  mpu.initialize();
  pinMode(INTERRUPT_PIN, INPUT);

  // mpu bağlantısı test ediliyor
  Serial.println(F("Testing device connections..."));
  Serial.println(mpu.testConnection() ? F("MPU6050 connection successful") : F("MPU6050 connection failed"));

  // DMP (Digital Motion Processor) başlatılıyor
  Serial.println(F("Initializing DMP..."));
  devStatus = mpu.dmpInitialize();

  // minimum hassasiyet için ölçeklenmiş kendi gyro ofset değerleri set ediliyor
  mpu.setXGyroOffset(-132);
  mpu.setYGyroOffset(103);
  mpu.setZGyroOffset(0);
  mpu.setZAccelOffset(2014); // 1688 factory default for my test chip

  // DMP'nin başlatıldığından emin isek (0 döndürmüş ise) mpu'yu kalibre edip etkinleştiriyoruz.
  if (devStatus == 0) {
    // Calibration Time: generate offsets and calibrate our MPU6050
    mpu.CalibrateAccel(6);
    mpu.CalibrateGyro(6);
    mpu.PrintActiveOffsets();
    // turn on the DMP, now that it's ready
    Serial.println(F("Enabling DMP..."));
    mpu.setDMPEnabled(true);

    // enable ESP32 interrupt detection
    Serial.print(F("Enabling interrupt detection (ESP32 external interrupt "));
    Serial.print(digitalPinToInterrupt(INTERRUPT_PIN));
    Serial.println(F(")..."));
    attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN), dmpDataReady, RISING);
    mpuIntStatus = mpu.getIntStatus();

    // set our DMP Ready flag so the main loop() function knows it's okay to use it
    Serial.println(F("DMP ready! Waiting for first interrupt..."));
    dmpReady = true;

    // get expected DMP packet size for later comparison
    packetSize = mpu.dmpGetFIFOPacketSize();
  } else {
    // ERROR!
    // 1 = initial memory load failed
    // 2 = DMP configuration updates failed
    // (if it's going to break, usually the code will be 1)
    Serial.print(F("DMP Initialization failed (code "));
    Serial.print(devStatus);
    Serial.println(F(")"));
  }

  // configure LED for output
  pinMode(LED_PIN, OUTPUT);

  // Connect to Wi-Fi network with SSID and password
  Serial.println("Setting AP (Access Point)…");
  WiFi.softAP(ssid, password);
  server.begin();
}

void initMpu(){
  mpu.initialize();
  devStatus = mpu.dmpInitialize();

  // minimum hassasiyet için ölçeklenmiş kendi gyro offset değerleri set ediliyor
  mpu.setXGyroOffset(-132);
  mpu.setYGyroOffset(103);
  mpu.setZGyroOffset(0);
  mpu.setZAccelOffset(2014); // 1688 factory default for my test chip

  // DMP'nin başlatıldığından emin isek (0 döndürmüş ise) mpu'yu kalibre edip etkinleştiriyoruz.
  if (devStatus == 0) {
    // Calibration Time: generate offsets and calibrate our MPU6050
    mpu.CalibrateAccel(6);
    mpu.CalibrateGyro(6);
    mpu.PrintActiveOffsets();
    // turn on the DMP, now that it's ready
    Serial.println(F("Enabling DMP..."));
    mpu.setDMPEnabled(true);

    // enable ESP32 interrupt detection
    Serial.print(F("Enabling interrupt detection (ESP32 external interrupt "));
    Serial.print(digitalPinToInterrupt(INTERRUPT_PIN));
    Serial.println(F(")..."));
    attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN), dmpDataReady, RISING);
    mpuIntStatus = mpu.getIntStatus();

    // set our DMP Ready flag so the main loop() function knows it's okay to use it
    Serial.println(F("DMP ready! Waiting for first interrupt..."));
    dmpReady = true;

    // get expected DMP packet size for later comparison
    packetSize = mpu.dmpGetFIFOPacketSize();
  } else {
    // ERROR!
    // 1 = initial memory load failed
    // 2 = DMP configuration updates failed
    // (if it's going to break, usually the code will be 1)
    Serial.print(F("DMP Initialization failed (code "));
    Serial.print(devStatus);
    Serial.println(F(")"));
  }
}

// ================================================================
// ===                    MAIN PROGRAM LOOP                     ===
// ================================================================

void loop() {
  // if programming failed, don't try to do anything
  if (!dmpReady) return;
  // read a packet from FIFO
  if (mpu.dmpGetCurrentFIFOPacket(fifoBuffer)) { // Get the Latest packet
  #ifdef OUTPUT_READABLE_EULER
            int butonCalState = digitalRead(butonCal);
            int butonResState = digitalRead(butonRes);
            // display Euler angles in degrees
            mpu.dmpGetQuaternion(&q, fifoBuffer);
            mpu.dmpGetEuler(euler, &q);
            //Serial.print("euler\t");
            Serial.print(euler[0] * 180/M_PI);
            Serial.print(",");
            Serial.print(euler[1] * 180/M_PI);
            Serial.print(",");
            Serial.print(euler[2] * 180/M_PI);
            Serial.print(",");
            Serial.print(butonCalState);
            Serial.print(",");
            Serial.println(butonResState);

            if(butonCalState == 0){
              // Mpu'nun offsetlerinin mevcut konuma göre tekrar ayarlanabilmesi için kullanılır.
              initMpu();
            }
        
            udp.beginPacket(CONSOLE_IP, CONSOLE_PORT);
            udp.printf("%.2f,%.2f,%.2f,%i,%i\r\n", euler[0] * 180/M_PI, euler[1] * 180/M_PI, euler[2] * 180/M_PI, butonCalState, butonResState);
            udp.endPacket();

        #endif

    // blink LED to indicate activity
    blinkState = !blinkState;
    digitalWrite(LED_PIN, blinkState);

  }
}
