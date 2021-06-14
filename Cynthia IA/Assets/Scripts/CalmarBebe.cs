using System;
using UnityEngine;

class CalmarBebe : MonoBehaviour
{
    Baby baby;
    AudioSource babyAudioSource;
    [SerializeField] KeyCode teclaCalmar = KeyCode.Space;
    [SerializeField] [Range (0.0001f, 1.0f)] float ratioCalmar = 1.0f;

    private void Start()
    {
        baby = GetComponent<Collect>().GetBaby();
        babyAudioSource = baby.GetComponent<AudioSource>();
    }

    private void Update()
    {
        if (Input.GetKeyDown(teclaCalmar) && baby.currentPicker == Picker.Player)
        {
            if (babyAudioSource.volume > 0)
                babyAudioSource.volume -= ratioCalmar * Time.deltaTime;
            else baby.Calmar();
        }
    }
}