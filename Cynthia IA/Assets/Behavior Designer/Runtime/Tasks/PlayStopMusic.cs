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
                audioSource.Stop();
            }
            else if (!audioSource.isPlaying)
            {
                audioSource.Play();
            }
        }
    }
}