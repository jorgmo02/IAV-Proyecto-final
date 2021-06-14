using System.Collections;
using System.Collections.Generic;
using UnityEngine;

/// <summary>
/// copia la posici�n y rotaci�n del objetivo en la propia
/// </summary>
public class Follow : MonoBehaviour
{
    [SerializeField] Transform target;

    private void LateUpdate()
    {
        transform.position = target.position;
        transform.rotation = target.rotation;
    }
}
