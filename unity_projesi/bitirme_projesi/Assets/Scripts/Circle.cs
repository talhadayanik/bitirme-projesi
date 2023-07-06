using UnityEngine;

public class Circle : MonoBehaviour
{
    public Material passedMaterial;
    public int index;
    public bool isNext;
    public GameObject ball;
    public GameObject passCheck;
    public GameObject passCheck2;

    public AudioSource _audioSource;
    public AudioClip succ;
    public float volume = 0.1f;

    private void Awake()
    {
        ball = GameObject.FindWithTag("ball");
    }

    void Update()
    {
        if (isNext)
        {
            if(ball.GetComponent<SphereCollider>().bounds.Intersects(passCheck.GetComponent<BoxCollider>().bounds) &
               ball.GetComponent<SphereCollider>().bounds.Intersects(passCheck2.GetComponent<BoxCollider>().bounds))
            {
                _audioSource.PlayOneShot(succ,volume);
                if (index < 9)
                {
                    GetComponentInParent<GameManager>().updateNextIndex(index + 1);
                }
                else
                {
                    GetComponentInParent<GameManager>().GameOver();
                }
                GetComponentInParent<GameManager>().circles[index].
                    GetComponentInChildren<MeshRenderer>().material = passedMaterial;
                GetComponentInChildren<TextMesh>().color = Color.green;
                isNext = false;
            }
        }
    }
}
