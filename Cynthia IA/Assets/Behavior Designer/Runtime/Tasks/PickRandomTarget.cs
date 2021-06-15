using System.Collections.Generic;
using UnityEngine;

namespace BehaviorDesigner.Runtime.Tasks
{
    public class PickRandomTarget : Action
    {
        [SerializeField] SharedGameObject var;

        public override void OnStart()
        {
            PickNewRandom();
        }
        void PickNewRandom()
        {
            List<Transform> salas = GameManager.Instance.GetSalas();
            int a = Random.Range(0, salas.Count);
            Debug.Log(salas[a].name);
            var.Value = salas[a].gameObject;
        }
    }
}