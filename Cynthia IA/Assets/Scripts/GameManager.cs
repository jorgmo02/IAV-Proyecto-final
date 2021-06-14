using System.Collections.Generic;
using UnityEngine;

/// <summary>
/// Gestiona la escena
/// </summary>
public class GameManager : MonoBehaviour
{
    [SerializeField] Pickable objetivoColeccionable;

    [SerializeField] GameObject obstaculoSalida;

    [SerializeField] GameObject salas;

    List<Transform> salasList;

    public bool sePuedeSalir { get; private set; } = false;

    public static GameManager Instance { get; private set; }

    private void Awake()
    {
        if (Instance != null && Instance != this)
        {
            Destroy(this.gameObject);
        }
        else
        {
            Instance = this;
        }
        
        salasList = new List<Transform>();
        
        foreach (Transform child in salas.transform)
        {
            salasList.Add(child);
        }
    }

    public List<Transform> GetSalas()
    {
        return salasList;
    }

    /// <summary>
    /// Actualiza las variables del juego en función del objeto recogido
    /// </summary>
    /// <param name="go">Objeto recogido</param>
    public void ProcessPickable(Pickable go)
    {
        if (go == objetivoColeccionable)
        {
            obstaculoSalida.SetActive(false);
            sePuedeSalir = true;
        }
    }
}