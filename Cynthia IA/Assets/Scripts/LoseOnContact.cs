using UnityEngine;
using UnityEngine.SceneManagement;

public class LoseOnContact : MonoBehaviour
{
    private void OnTriggerEnter(Collider other)
    {
        SceneManager.LoadScene("Lose");
        Cursor.lockState = CursorLockMode.None;
        Cursor.visible = true;
    }
}
