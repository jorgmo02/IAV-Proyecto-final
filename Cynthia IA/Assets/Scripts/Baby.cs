using BehaviorDesigner.Runtime.Tasks.Unity.Timeline;
using System;
using UnityEngine;
using UnityEngine.Assertions;
using UnityEngine.Windows;

public enum Picker { Cynthia, Player, None };

/// <summary>
/// Gestiona las variables del bebé: quién lo ha cogido y dónde debe estar
/// También hace que llore en los momentos adecuados
/// </summary>
public class Baby : MonoBehaviour
{
    [SerializeField] public Picker currentPicker = Picker.Cynthia;

    [SerializeField] Vector3 positionOffset = new Vector3(.0f, .0f, .369f);
    [SerializeField] Vector3 rotation = new Vector3(3.67217088f, 237.532501f, 10.2394657f);

    /// <summary>
    /// para activar y desactivar la simulacion fisica
    /// </summary>
    private CapsuleCollider capsCollider;
    private Rigidbody rb;
    private AudioSource audioSource;

    private float initialVolume = 1.0f;

    bool isInsideCuna = false;
    bool isCalmado = true;

    private void Start()
    {
        capsCollider = GetComponent<CapsuleCollider>();
        rb = GetComponent<Rigidbody>();
        audioSource = GetComponent<AudioSource>();
        Assert.IsNotNull(audioSource, "No le has puesto sonido al bebé");
        initialVolume = audioSource.volume;
    }

    void Cry()
    {
        audioSource.volume = initialVolume;
        if(!audioSource.isPlaying) audioSource.Play();
        isCalmado = false;
    }

    void StopCry()
    {
        audioSource.Stop();
        isCalmado = true;
    }

    public void Pick(Transform babyParentItem, Picker picker)
    {
        transform.SetParent(babyParentItem);
        transform.localPosition = positionOffset;
        transform.localRotation = Quaternion.Euler(rotation);
        capsCollider.enabled = false;
        rb.isKinematic = true;
        currentPicker = picker;

        if (currentPicker != Picker.Cynthia) Cry();
        else StopCry();
    }

    public void Unpick()
    {
        transform.SetParent(null);
        rb.isKinematic = false;
        capsCollider.enabled = true;
        currentPicker = Picker.None;
        if (isCalmado)
        {
            if (isInsideCuna) StopCry();
            else Cry();
        }
    }

    public void Calmar()
    {
        StopCry();
    }

    public void OnTriggerStay(Collider other)
    {
        isInsideCuna = other.GetComponent<ElementoCalmador>() != null;
    }
    public void OnTriggerExit(Collider other)
    {
        if (other.GetComponent<ElementoCalmador>() != null)
            isInsideCuna = false;
    }
}
