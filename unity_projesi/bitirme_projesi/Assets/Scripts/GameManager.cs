using System.Collections.Generic;
using UnityEngine;
using Random = UnityEngine.Random;
using TMPro;

public class GameManager : MonoBehaviour
{
    public bool gameOver = false;
    public int circleCount = 10;
    public GameObject circle;
    public GameObject ballPrefab;
    public List<GameObject> circles = new List<GameObject>();
    public float offsetMultiplier;
    public float rotationSpeed = 0.05f;
    public GameObject ball;

    public GameObject gameOverPopup;

    // Timer
    [Header("Timer")]
    public TextMeshProUGUI timerText;

    [Header("Timer Setting")]
    public float currentTime;
    
    private float[,] centers =
    {
        {4.46f,-34.26f},{5.94f,-34.26f},{7.91f,-34.26f},{9.44f,-34.26f},
        {4.46f,-35.9f},{5.94f,-35.9f},{7.91f,-35.9f},{9.44f,-35.9f},
        {4.46f,-38.06f},{5.94f,-38.06f},{7.91f,-38.06f},{9.44f,-38.06f},
        {4.46f,-39.66f},{5.94f,-39.66f},{7.91f,-39.66f},{9.44f,-39.66f}
    };
    //private float offset = 0.915f;
    private float offset = 0f;

    void Awake()
    {
        Screen.sleepTimeout = SleepTimeout.NeverSleep;
    }

    void Start()
    {
        CreateLevel();
    }
    
    void Update()
    {
        float h = Input.GetAxisRaw("Horizontal");
        float v = Input.GetAxisRaw("Vertical");
        
        gameObject.transform.Rotate(v * rotationSpeed, 0,
            -h * rotationSpeed, Space.Self);

        if (!gameOver)
        {
            currentTime += Time.deltaTime;
            timerText.text = currentTime.ToString("0.00");
        }
        
    }

    private void CreateLevel()
    {
        SpawnCirclesAndBall();
    }

    private void SpawnCirclesAndBall()
    {
        int circleIndex = 0;
        List<int> usedIndex = new List<int>();
        
        int bspawnIndex = Random.Range(0,16);
        usedIndex.Add(bspawnIndex);
        ball = Instantiate(ballPrefab);
        ball.transform.position = new Vector3(Random.Range(centers[bspawnIndex,0]-offset*offsetMultiplier,centers[bspawnIndex,0]+offset*offsetMultiplier),
            22.222f,Random.Range(centers[bspawnIndex,1]-offset*offsetMultiplier,centers[bspawnIndex,1]+offset*offsetMultiplier));

        for (int i = 0; i < circleCount; i++)
        {
            circles.Add(Instantiate(circle) as GameObject);
            circles[circles.Count - 1].GetComponent<Circle>().index = circleIndex;
            
            int cspawnIndex = Random.Range(0,16);
            while (usedIndex.Contains(cspawnIndex))
            {
                cspawnIndex = Random.Range(0, 16);
            }
            usedIndex.Add(cspawnIndex);
            
            circles[circles.Count - 1].transform.position = new Vector3(Random.Range(centers[cspawnIndex,0]-offset*offsetMultiplier,centers[cspawnIndex,0]+offset*offsetMultiplier),
                22.222f,Random.Range(centers[cspawnIndex,1]-offset*offsetMultiplier,centers[cspawnIndex,1]+offset*offsetMultiplier));
            circles[circles.Count - 1].transform.GetChild(0).transform.Rotate(0, Random.Range(-45.0f, 45.0f), 0, Space.Self);
            circles[circles.Count - 1].transform.GetChild(1).GetComponent<TextMesh>().text = (circleIndex + 1).ToString();
            circles[circles.Count - 1].transform.SetParent(this.transform);
            circleIndex++;
        }
        
        circles[0].GetComponent<Circle>().isNext = true;
        
    }

    public void updateNextIndex(int nextCrcIndex)
    {
        if (nextCrcIndex < circleCount)
        {
            circles[nextCrcIndex].GetComponent<Circle>().isNext = true;
        }
    }

    public void GameOver()
    {
        gameOver = true;
        gameOverPopup.GetComponentInChildren<TextMeshProUGUI>().text = currentTime.ToString("0.00");
        gameOverPopup.SetActive(true);
    }
    
}
