using System.Collections;
using System.Collections.Generic;
using UnityEngine;

/// <summary>
/// copia la posición y rotación del objetivo en la propia
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
