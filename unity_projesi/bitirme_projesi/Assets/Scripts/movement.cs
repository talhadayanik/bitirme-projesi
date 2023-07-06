using UnityEngine;
using UnityEngine.SceneManagement;

public class movement : MonoBehaviour
{
    public UDPReceive udpReceive;
    public string strReceived;
    public GameObject board;
    public int bolme;
    public float speed = 5;
    public string[] strData = new string[5];
    public float qx, qy, qz;
    public float dx = 0;
    public float dy = 0;
    public float dz = 0;
    public int butonCalState = 1;
    public int butonResState = 1;

    void FixedUpdate()
    {
        strReceived = udpReceive.data;
        strData = strReceived.Split(',');
        if (strData[0] != "" && strData[1] != "" && strData[2] != "" && strData[3] != "" && strData[4] != "")
        {
            
            qy = float.Parse(strData[0]) / bolme;
            qx = float.Parse(strData[1]) / bolme;
            qz = float.Parse(strData[2]) / bolme;
            butonCalState = int.Parse(strData[3]);
            butonResState = int.Parse(strData[4]);
            
            if (butonCalState == 0)
            {
                dx = qx;
                dy = qy;
                dz = qz;
            }

            if (butonResState == 0)
            {
                udpReceive.client.Close();
                udpReceive.receiveThread.Abort();
                SceneManager.LoadScene("MainScene");
            }
            
            board.transform.rotation =
               Quaternion.Lerp(board.transform.rotation, Quaternion.Euler(-qx + dx, 0, qz - dz), 0.1f);
        }
    }
}