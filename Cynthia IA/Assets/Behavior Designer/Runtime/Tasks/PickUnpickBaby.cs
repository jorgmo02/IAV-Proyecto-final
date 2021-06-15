using UnityEngine;

namespace BehaviorDesigner.Runtime.Tasks
{
    public class PickUnpickBaby : Action
    {
        [SerializeField] HandleBaby handler;
        [SerializeField] SharedBool wantToPick = true;

        public override TaskStatus OnUpdate()
        {
            handler.PickUnpick();
            bool thisHasPicked = handler.GetPicker() == handler.GetBaby().currentPicker;

            if (wantToPick.Value && thisHasPicked || !wantToPick.Value && !thisHasPicked)
                return TaskStatus.Success;

            return TaskStatus.Failure;
        }
    }
}