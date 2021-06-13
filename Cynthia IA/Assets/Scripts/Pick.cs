using UnityEngine;
using UnityEngine.Assertions;

/// <summary>
/// Componente creado para poder coger el bebé desde el player
/// </summary>
public class Pick : MonoBehaviour
{
    PlayerHandleBaby handleBaby;

    // Start is called before the first frame update
    void Start()
    {
        handleBaby = GetComponent<PlayerHandleBaby>();
        Assert.IsNotNull(handleBaby, "HandleBaby no se ha asignado correctamente");
    }

    // Update is called once per frame
    void Update()
    {
        if (Input.GetMouseButtonDown(0))
        {
            handleBaby.PickUnpick();
        }
    }
}
