using System.Collections.Generic;
using UnityEngine;

namespace BehaviorDesigner.Runtime.Tasks
{
    public class PickRandomCuna : Action
    {
        [SerializeField] SharedGameObject parentCuna;
        [SerializeField] SharedGameObject target;

        public override void OnStart()
        {
            PickNewRandom();
        }
        void PickNewRandom()
        {
            List<Transform> cunas = new List<Transform>();
            foreach(Transform child in parentCuna.Value.transform)
            {
                cunas.Add(transform);
            }
            int a = Random.Range(0, cunas.Count);
            Debug.Log(cunas[a].name);
            target.Value = cunas[a].GetChild(0).gameObject;
        }
    }
}