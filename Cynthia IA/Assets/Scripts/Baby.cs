using System;
using UnityEngine;

public enum Picker { Cynthia, Player, None };

public class Baby : MonoBehaviour
{
    [SerializeField] public Picker currentPicker = Picker.Cynthia;

    [SerializeField] Vector3 positionOffset = new Vector3(.0f, .0f, .369f);
    [SerializeField] Vector3 rotation = new Vector3(3.67217088f, 237.532501f, 10.2394657f);

    private CapsuleCollider capsCollider;
    private Rigidbody rb;

    private void Start()
    {
        capsCollider = GetComponent<CapsuleCollider>();
        rb = GetComponent<Rigidbody>();
    }

    public void Pick(Transform babyParentItem, Picker picker)
    {
        transform.SetParent(babyParentItem);
        transform.localPosition = positionOffset;
        transform.localRotation = Quaternion.Euler(rotation);
        capsCollider.enabled = false;
        rb.isKinematic = true;
        currentPicker = picker;
    }
    public void Unpick()
    {
        transform.SetParent(null);
        currentPicker = Picker.None;
        capsCollider.enabled = true;
        rb.isKinematic = false;
    }
}
