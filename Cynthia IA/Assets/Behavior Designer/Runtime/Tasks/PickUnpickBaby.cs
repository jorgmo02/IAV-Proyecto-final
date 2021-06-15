using UnityEngine;

namespace BehaviorDesigner.Runtime.Tasks
{
    public class PickUnpickBaby : Action
    {
        [SerializeField] HandleBaby handler;

        public override void OnStart()
        {
            handler.PickUnpick();
        }
    }
}