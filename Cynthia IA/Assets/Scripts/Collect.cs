using UnityEngine;
using UnityEngine.Assertions;
using UnityEngine.UI;

/// <summary>
/// Componente creado para poder coger el bebé desde el player, además de otros objetos
/// </summary>
public class Collect : HandleBaby
{
    [SerializeField] Camera playerCamera;
    [SerializeField] Text textPick;
    [SerializeField] Text textUnpick;

    /// <summary>
    /// override del método del padre, sirve para que la clase HandleBaby sea multiusos
    /// </summary>
    public override void SetRaycast()
    {
        pickRay = playerCamera.ViewportPointToRay(Vector3.one * 0.5f);
    }

    /// <summary>
    /// input del jugador
    /// </summary>
    void Update()
    {
        ManageText();
        if (Input.GetMouseButtonDown(0))
        {
            GameObject gO = PickUnpick();
            if (gO != null)
            {
                Pickable p = gO.GetComponent<Pickable>();
                if (p != null)
                {
                    p.Pick();
                }
            }
        }
    }

    /// <summary>
    /// Muestra u oculta el texto de los controles del bebé
    /// </summary>
    void ManageText()
    {
        if (picker != baby.currentPicker)
        {
            textUnpick.enabled = false;

            Ray ray = playerCamera.ViewportPointToRay(Vector3.one * 0.5f);
            RaycastHit hit;
            if (Physics.Raycast(ray, out hit, pickDistance))
            {
                if (hit.transform.gameObject.name == "Baby")
                    textPick.enabled = true;
                else
                    textPick.enabled = false;
            }
            else
            {
                textPick.enabled = false;
            }
        }
        else
        {
            textPick.enabled = false;
            textUnpick.enabled = true;
        }
    }
}
