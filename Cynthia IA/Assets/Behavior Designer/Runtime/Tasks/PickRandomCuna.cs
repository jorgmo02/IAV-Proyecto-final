using System.Collections.Generic;
using UnityEngine;

namespace BehaviorDesigner.Runtime.Tasks
{
    public class PickRandomCuna : Action
    {
        [SerializeField] SharedGameObject parentCuna;
        [SerializeField] SharedVariable outVar;

        public override void OnStart()
        {
            PickNewRandom();
        }
        void PickNewRandom()
        {
            List<Transform> salas = new List<Transform>();
            foreach(GameObject child in parentCuna.Value.transform)
            {
                salas.Add(child.transform);
            }
            int a = Random.Range(0, salas.Count);
            Debug.Log(salas[a].name);
            outVar.SetValue(salas[a].GetChild(0).gameObject);
        }

        public override void OnLateUpdate()
        {
            Debug.Log("JOPE");
        }
    }
}