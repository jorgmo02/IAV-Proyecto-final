using UnityEngine;
using UnityEngine.Assertions;

/// <summary>
/// 
/// Comportamiento de abandonar al bebé o de cogerlo. Provocará que el bebé llore
/// (o no, dependiendo de quién lo coja).
/// 
/// </summary>
public abstract class HandleBaby : MonoBehaviour
{
    [SerializeField] private Baby baby = null;
    
    // Se asigna el valor del objeto padre en el inspector para que se pueda animar.
    [SerializeField] private Transform babyParentItem = null;

    [SerializeField] private float pickDistance = 1.5f;

    protected Ray pickRay;

    [SerializeField] Picker picker;

    // Start is called before the first frame update
    void Start()
    {
        Assert.IsNotNull(baby, "Asigna el bebé en el inspector");
        if (babyParentItem == null) babyParentItem = transform;
    }

    public Baby GetBaby() { return baby; }

    public void PickUnpick()
    {
        // Check if player picked some item already
        if (baby.currentPicker == picker)
        {
            baby.Unpick();
        }
        else if (baby.currentPicker == Picker.None)
        {
            // If not picked already, try to pick item in front of the player
            TryPick();
        }
        else Debug.Log("La otra persona tiene al bebé");
    }

    public abstract void SetRaycast();

    public void TryPick()
    {
        // set direction of picking
        // Create ray (abstract, implementation depends on who picks)
        SetRaycast();
        RaycastHit hit;
        // Shot ray to find object to pick
        if (Physics.Raycast(pickRay, out hit, pickDistance))
        {
            Debug.Log("JAJA COGISTE");
            // Check if object is the baby
            GameObject gO = hit.transform.gameObject;
            var pickable = gO.GetComponent<Baby>();
            // If object is the baby
            if (pickable)
            {
                // Pick it
                baby.Pick(babyParentItem, picker);
            }
            else
            {
                Debug.Log("UPS PERO NO ERA EL BEBE");
                Debug.Log(gO.name);
            }
        }
        else Debug.Log("No se pudo coger al bebé");
    }
}
