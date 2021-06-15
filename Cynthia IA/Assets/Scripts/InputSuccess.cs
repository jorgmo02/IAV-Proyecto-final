using UnityEngine;

namespace BehaviorDesigner.Runtime.Tasks
{
    public class InputSuccess : Conditional
    {
        [SerializeField] KeyCode keyCode = KeyCode.Escape;
        public override TaskStatus OnUpdate()
        {
            if (Input.GetKey(keyCode))
            {
                Debug.Log("Input");
                return TaskStatus.Success;
            }
            return TaskStatus.Failure;
        }
    }
}
