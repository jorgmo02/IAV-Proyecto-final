using UnityEngine;

public class Pickable : MonoBehaviour
{
    public void Pick()
    {
        GameManager.Instance.ProcessPickable(this);
        this.gameObject.SetActive(false);
    }
}
