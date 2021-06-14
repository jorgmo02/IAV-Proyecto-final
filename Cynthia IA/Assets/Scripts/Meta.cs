using UnityEngine;
using UnityEngine.SceneManagement;

public class Meta : MonoBehaviour
{
    private void OnCollisionEnter(Collision collision)
    {
        SceneManager.LoadScene("End");
        Cursor.lockState = CursorLockMode.None;
        Cursor.visible = true;
    }
}
