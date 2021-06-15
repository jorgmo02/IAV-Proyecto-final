using UnityEngine;

namespace BehaviorDesigner.Runtime.Tasks
{
    class CalmBaby : Action
    {
        [SerializeField] Baby baby;

        public override void OnStart()
        {
            baby.Calmar();
        }
    }
}
