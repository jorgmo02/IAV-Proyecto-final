using UnityEngine;
using UnityEngine.SceneManagement;

public class LoseOnContact : MonoBehaviour
{
    private void OnTriggerEnter(Collider other)
    {
        if(other.gameObject.CompareTag("Player"))
            SceneManager.LoadScene("Lose");
    }
}
