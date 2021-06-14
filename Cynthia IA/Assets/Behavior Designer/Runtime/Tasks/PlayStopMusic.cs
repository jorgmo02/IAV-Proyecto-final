using UnityEngine;

namespace BehaviorDesigner.Runtime.Tasks
{
    public class PlayStopMusic : Action
    {
        [SerializeField] private SharedGameObject target;
        [SerializeField] SharedBool stop = false;
        AudioSource audioSource;

        public override void OnStart()
        {
            audioSource = target.Value.GetComponent<AudioSource>();
            if (stop.Value)
            {
                Debug.Log("stopped");
                audioSource.Stop();
            }
            else if (!audioSource.isPlaying)
            {
                Debug.Log("played");
                audioSource.Play();
            }
        }
    }
}