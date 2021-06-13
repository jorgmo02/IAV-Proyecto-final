using UnityEngine;

public class PlayerHandleBaby : HandleBaby
{
    [SerializeField] Camera playerCamera;
 
    public override void SetRaycast()
    {
        pickRay = playerCamera.ViewportPointToRay(Vector3.one * 0.5f);
    }
}

