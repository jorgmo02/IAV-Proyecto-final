using System.Collections.Generic;
using UnityEditor;
using UnityEngine;
using UnityEngine.AI;

namespace BehaviorDesigner.Runtime.Tasks
{
    public class SelectCuna : Action
    {
        [SerializeField] NavMeshAgent agent;
        [SerializeField] SharedGameObject parentCuna;
        [SerializeField] SharedGameObject result;
        List<Transform> cunas;

        public override void OnStart()
        {
            RellenaLista();
            PickNearest();
        }
        void RellenaLista()
        {
            cunas = new List<Transform>();
            foreach(Transform child in parentCuna.Value.transform)
            {
                cunas.Add(child.transform.GetChild(0));
            }
        }

        void PickNearest()
        {
            Transform target = null;
            float minDist = float.PositiveInfinity;

            foreach (Transform cuna in cunas)
            {
                float len = FromAToB(agent.transform, cuna);
                if (len < minDist)
                {
                    target = cuna;
                    minDist = len;
                }
            }

            if (target != null)
            {
                Selection.activeObject = target.gameObject;
                result.Value = target.gameObject;
            }
            else Debug.LogError("Target was null");
        }

        private float FromAToB(Transform PointA, Transform PointB)
        {
            if (PointA == null || PointB == null)
                return float.PositiveInfinity;

            NavMesh.SamplePosition(PointA.position, out NavMeshHit hitA, 2f, NavMesh.AllAreas);
            NavMesh.SamplePosition(PointB.position, out NavMeshHit hitB, 2f, NavMesh.AllAreas);

            NavMeshPath path = new NavMeshPath();
            if (NavMesh.CalculatePath(hitA.position, hitB.position, NavMesh.AllAreas, path))
            {
                Debug.DrawLine(hitA.position + Vector3.up, hitB.position + Vector3.up, Color.red, 10f, true); // a red line float in air
                int cnt = path.corners.Length;

                float distance = 0f;
                for (int i = 0; i < cnt - 1; i++)
                {
                    distance += (path.corners[i] - path.corners[i + 1]).magnitude;
                    Debug.DrawLine(path.corners[i], path.corners[i + 1], Color.green, 10f, true);
                }
                Debug.Log($"Total distance {distance:F2}");
                return distance;
            }
            else
            {
                Debug.LogError("Mission Fail");
            }
            return float.PositiveInfinity;
        }

        void PickRandom()
        {
            int a = Random.Range(0, cunas.Count);
            Debug.Log(cunas[a].name);
            result.Value = cunas[a].gameObject;
        }
    }
}