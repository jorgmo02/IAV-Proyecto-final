using UnityEngine;

namespace BehaviorDesigner.Runtime.Tasks
{
    [TaskIcon("Assets/Behavior Designer Tutorials/Tasks/Editor/{SkinColor}CanSeeObjectIcon.png")]
    public class CanHearObject : Conditional
    {
        [Tooltip("The object that we want to hear")]
        public SharedGameObject targetObject;
        [Tooltip("The hear sensitivity. The lower, the more they hear")]
        public SharedFloat hearSensitivity = 20;
        [Tooltip("The object that is within sight")]
        [HideInInspector] public SharedGameObject returnedObject;

        /// <summary>
        /// Returns success if an object was found otherwise failure
        /// </summary>
        /// <returns></returns>
        public override TaskStatus OnUpdate()
        {
            returnedObject.Value = WithinHearRange(targetObject.Value, hearSensitivity.Value);
            if (returnedObject.Value != null)
            {
                // Return success if an object was found
                return TaskStatus.Success;
            }
            // An object is not within sight so return failure
            return TaskStatus.Failure;
        }

        /// <summary>
        /// Determines if the targetObject is within sight of the transform.
        /// </summary>
        private GameObject WithinHearRange(GameObject targetObject, float hearThreshold)
        {
            if (targetObject == null)
            {
                return null;
            }

            var aSource = targetObject.GetComponent<AudioSource>();
            if (!aSource.isPlaying) return null;

            float distance = (targetObject.transform.position - transform.position).magnitude;
            float dB = aSource.volume * 100 / distance;

            if (dB >= hearThreshold)
            {
                return targetObject; // return the target object meaning it is within sight
            }
            return null;
        }
    }
}