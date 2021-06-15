using UnityEngine;

namespace BehaviorDesigner.Runtime.Tasks
{
    public class CheckHasBaby : Conditional
    {
        [SerializeField] Baby baby;
        [SerializeField] HandleBaby handler;

        public override TaskStatus OnUpdate()
        {
            if (baby.currentPicker == handler.GetPicker())
                return TaskStatus.Success;
            return TaskStatus.Failure;
        }
    }
}