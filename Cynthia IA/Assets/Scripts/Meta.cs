using UnityEngine;
using UnityEngine.SceneManagement;

public class Meta : MonoBehaviour
{
    [SerializeField] GameObject player;
    private void OnCollisionEnter(Collision collision)
    {
        if (collision.gameObject == player)
            SceneManager.LoadScene("End");
    }
}
