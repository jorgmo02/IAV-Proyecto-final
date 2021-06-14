using UnityEngine;
using UnityEngine.UI;

public class InitialTextManager : MonoBehaviour
{
    [SerializeField] Text initialText;

    private void OnTriggerStay(Collider other)
    {
        if(!GameManager.Instance.sePuedeSalir) initialText.enabled = true;
    }
    private void OnTriggerExit(Collider other)
    {
        initialText.enabled = false;
    }
}
