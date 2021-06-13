using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class Follow : MonoBehaviour
{
    [SerializeField] Transform target;

    private void Start()
    {
    }

    private void LateUpdate()
    {
        transform.position = target.position;
        transform.rotation = target.rotation;
    }
}
