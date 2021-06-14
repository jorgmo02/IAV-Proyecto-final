using System.Collections;
using System.Collections.Generic;
using TMPro;
using UnityEngine;
using UnityEngine.SceneManagement;

public class ResetScene : MonoBehaviour
{
    private void Start()
    {
        Debug.Log("AAAAAAAAAAAAAAAstart");
        Time.timeScale = 1;
    }
    public void LoadScene(string name)
    {
        SceneManager.LoadScene(name);
    }

    public void Exit()
    {
        Application.Quit();
    }
}
