using UnityEngine;

namespace BehaviorDesigner.Runtime.Tasks.Unity.UnityTransform
{
    [TaskCategory("Unity/Transform")]
    [TaskDescription("Rotates the transform so the forward vector points at worldPosition. Returns Success.")]
    public class LookAt : Action
    {
        [Tooltip("The GameObject that the task operates on. If null the task GameObject is used.")]
        public SharedGameObject targetGameObject;
        [Tooltip("The GameObject to look at. If null the world position will be used.")]
        public SharedGameObject targetLookAt;
        [Tooltip("Point to look at")]
        public SharedVector3 worldPosition;
        [Tooltip("Vector specifying the upward direction")]
        public Vector3 worldUp;
        [Tooltip("Constraints")]
        public SharedBool constraintX = false, constraintY = false, constraintZ = false;

        private Vector3 rotations;

        private Transform targetTransform;
        private GameObject prevGameObject;

        public override void OnStart()
        {
            var currentGameObject = GetDefaultGameObject(targetGameObject.Value);
            if (currentGameObject != prevGameObject) {
                targetTransform = currentGameObject.GetComponent<Transform>();
                prevGameObject = currentGameObject;
            }
            rotations = new Vector3(
                (constraintX.Value) ? 0 : 1,
                (constraintY.Value) ? 0 : 1,
                (constraintZ.Value) ? 0 : 1
            );
        }

        public override TaskStatus OnUpdate()
        {
            if (targetTransform == null) {
                Debug.LogWarning("Transform is null");
                return TaskStatus.Failure;
            }

            Vector3 euler = targetTransform.rotation.eulerAngles;

            if (targetLookAt.Value != null) {
                targetTransform.LookAt(targetLookAt.Value.transform);
            } else {
                targetTransform.LookAt(worldPosition.Value, worldUp);
            }
            Vector3 rotated = targetTransform.rotation.eulerAngles;
            rotated.Scale(rotations);

            targetTransform.rotation = Quaternion.Euler(rotated);

            return TaskStatus.Success;
        }

        public override void OnReset()
        {
            targetGameObject = null;
            targetLookAt = null;
            worldPosition = Vector3.up;
            worldUp = Vector3.up;
        }
    }
}