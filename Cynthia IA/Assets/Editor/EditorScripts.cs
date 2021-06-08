using System.Collections.Generic;
using UnityEngine;
using UnityEditor;

public class SelectAll : MonoBehaviour
{
    private static List<GameObject> childGOs;

    [MenuItem("Tools/Select All &a")]
    private static void NewMenuOption()
    {
        GameObject obj = Selection.activeGameObject;
        Transform t = obj.transform;
        childGOs = new List<GameObject>();
        childGOs.Add(obj);
        GetAllChildren(t);
        GameObject[] gOs = childGOs.ToArray();
        Selection.objects = gOs;
    }

    static void GetAllChildren(Transform t)
    {
        foreach (Transform childT in t)
        {
            childGOs.Add(childT.gameObject);
            if (childT.childCount > 0)
            {
                GetAllChildren(childT);
            }
        }
    }
}
public class SelectOnlyChildrenWithName : MonoBehaviour
{
    private static List<GameObject> childGOs = new List<GameObject>();
    private static GameObject[] objs;

    [MenuItem("Tools/Select Only Children With Name \"StaticMeshComponent0\" &n")]
    private static void NewMenuOption()
    {
        objs = Selection.gameObjects;
        childGOs = new List<GameObject>();
        foreach (var v in objs) {
            GetAllChildren(v.transform);
        }
        Selection.objects = childGOs.ToArray();
    }

    static void GetAllChildren(Transform t)
    {
        foreach (Transform childT in t)
        {
            if(childT.name == "StaticMeshComponent0") childGOs.Add(childT.gameObject);
            if (childT.childCount > 0) {
                GetAllChildren(childT);
            }
        }
    }
}
