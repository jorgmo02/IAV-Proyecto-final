using UnityEngine;

public class NPCHandleBaby : HandleBaby
{
    public override void SetRaycast()
    {
        pickRay = new Ray(transform.position, baby.transform.position);
    }
}
