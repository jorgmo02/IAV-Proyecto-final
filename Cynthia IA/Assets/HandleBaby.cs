using UnityEngine;
using UnityEngine.Assertions;

/// <summary>
/// 
/// Comportamiento de abandonar al bebé o de cogerlo. Provocará que el bebé llore
/// (o no, dependiendo de quién lo coja).
/// 
/// </summary>
public class HandleBaby : MonoBehaviour
{
    [SerializeField] private GameObject baby = null;
    
    // Se asigna el valor del objeto padre en el inspector para que se pueda animar.
    [SerializeField] private Transform babyParentItem = null;

    [SerializeField] Vector3 positionOffset = new Vector3(.0f, .0f, .369f);
    [SerializeField] Vector3 rotation = new Vector3(.0f, .0f, .0f);

    // Start is called before the first frame update
    void Start()
    {
        Assert.IsNotNull(baby, "Asigna el bebé en el inspector");
        if (babyParentItem == null) babyParentItem = transform;
    }

    // Update is called once per frame
    void Update()
    {
        
    }

    void AttachBaby()
    {
        baby.transform.SetParent(babyParentItem);
        baby.transform.localPosition = positionOffset;
        //baby.transform.localRotation = rotation;
    }

    void DeattachBaby()
    {
        baby.transform.SetParent(null);
    }
}
