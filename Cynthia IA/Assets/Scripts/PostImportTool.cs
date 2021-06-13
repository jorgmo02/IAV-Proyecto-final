
#if UNITY_EDITOR
using UnityEngine;
using System.Collections;
using UnityEngine.UI;
using UnityEditor;
using System.IO;
using System.Collections.Generic;
using System;
using Random = UnityEngine.Random;

public class PostImportTool : AssetPostprocessor
{   
	class Section
    {
		public int Offset;
		public int NumIndices;
		public int Materialindex;
		public string MaterialName;
		public Material Mat;
	}
	class MeshBinding
    {
		public Mesh mesh;
		public MeshRenderer renderer;
    }
	void GetFloatsFromLine( string Line, ref List<float> Floats )
	{
		Floats.Clear();

		string[] Tokens = Line.Split(new char[]{ ' ' } );
		
		for( int u = 1; u < Tokens.Length; u++ )
		{
			try
			{
				float Val = float.Parse( Tokens[u] );
				Floats.Add( Val );
			}
			catch( FormatException e )
			{
				Debug.LogError(e.Message);
			}
		}
	}
	private void OnPostprocessModel(GameObject gameobject)   
	{
		UnityEditor.ModelImporter importer = this.assetImporter as UnityEditor.ModelImporter;

		string Path = importer.assetPath;
		if( !Path.Contains( ".obj" ) )
			return;


		string OBJFile = File.ReadAllText( Path );

		List<float> Floats = new List<float>();
		List<Color> Colors = new List<Color>();
		List<Vector2> Texcoords1 = new List<Vector2>();
		List<Vector3> Normals = new List<Vector3>();
		List<Vector4> Tangents = new List<Vector4>();
		bool ReplaceNormals = false;
		List<Section> Sections = new List<Section>();
		string[] Lines = OBJFile.Split( new char[]{ '\n' } );
		for(int i=0; i<Lines.Length; i++ )
        {
			string Line = Lines[i];
			if (Line.StartsWith("v "))
            {
				string[] Tokens = Line.Split(new char[]{ ' ' } );
				Floats.Clear();
				for(int u=1; u<Tokens.Length; u++ )//0 should be "v"
                {
					try
					{
						float Val = float.Parse( Tokens[u] );
						Floats.Add( Val );
					}
					catch(FormatException e )
					{
						Debug.LogError(e.Message);
					}
				}

				if ( Floats.Count == 7 )
                {
					Colors.Add( new Color( Floats[3], Floats[4], Floats[5], Floats[6] ) );
				}
			}
			else if( Line.StartsWith( "vt1 " ) )
			{
				GetFloatsFromLine( Line, ref Floats );
				if( Floats.Count == 2 )
				{
					Texcoords1.Add( new Vector2( Floats[0], Floats[1] ) );
				}
			}
			else if( ReplaceNormals && Line.StartsWith( "vn " ) )
			{
				GetFloatsFromLine( Line, ref Floats );
				if( Floats.Count == 3 )
				{
					Normals.Add( new Vector3( Floats[0], Floats[1], Floats[2] ) );
				}
			}
			else if( Line.StartsWith( "tan " ) )
			{
				GetFloatsFromLine( Line, ref Floats );
				if( Floats.Count == 4 )
				{
					Tangents.Add( new Vector4( Floats[0], Floats[1], Floats[2], Floats[3] ) );
				}
			}
			//"#Section %d Offset %d NumIndices %d MaterialIndex %d %s\n"
			if( Line.StartsWith( "#Section " ) )
			{
				string[] Tokens = Line.Split(new char[]{ ' ' } );
				if( Tokens.Length == 9 )
				{
					Section NewSection = new Section();
					NewSection.Offset = int.Parse( Tokens[3] );
					NewSection.NumIndices = int.Parse( Tokens[5] );
					//NewSection.Materialindex = int.Parse( Tokens[7] );
					NewSection.MaterialName = Tokens[8].Trim();
					Sections.Add( NewSection );
				}
			}
		}

		MeshBinding[] MeshArray = GetMeshes( gameobject);
		if( MeshArray.Length == 0 )
			return;

		MeshBinding Binding = MeshArray[0];
		Mesh m = Binding.mesh;

		if( Colors.Count > 0 )
		{
			if( m.vertices.Length != Colors.Count )
			{
				Debug.LogError( " Mesh vertex count != color count for " + gameobject.name );
			}
			else
			{
				m.colors = Colors.ToArray();
			}
		}
		if ( Texcoords1.Count > 0 )
        {
			if( m.vertices.Length != Texcoords1.Count )
			{
				Debug.LogError( " Mesh vertex count != Texcoords1 count for " + gameobject.name );
			}
			else
			{
				m.uv2 = Texcoords1.ToArray();
			}
		}

		if( Sections.Count > 1 )
		{
			SetupSubmeshes( m, Sections.ToArray(), gameobject.name );
		}

		Material[] UsedMaterials = GetSectionMaterials( Sections.ToArray() );
		Binding.renderer.sharedMaterials = UsedMaterials;

		if( ReplaceNormals && Normals.Count > 0 )
		{
			if( m.vertices.Length != Normals.Count )
			{
				Debug.LogError( " Mesh vertex count != Texcoords1 count " + gameobject.name );
			}
			else
			{
				m.normals = Normals.ToArray();
			}
		}
		if( Tangents.Count > 0 )
		{
			if ( m.tangents.Length == 0 || m.tangents.Length == Tangents.Count )
			{
				m.tangents = Tangents.ToArray();
			}
			else
			{
				Debug.LogError( " Mesh vertex count != Tangents count " + gameobject.name );
			}
		}
	}
	Material[] GetSectionMaterials( Section[] sections )
	{
		Material[] AllMaterials = UnityEngine.Resources.FindObjectsOfTypeAll<Material>();
		Material[] UsedMaterials = new Material[sections.Length];

		for( int i = 0; i < sections.Length; i++ )
		{
			Section s = sections[i];

			if( !s.MaterialName.Equals( "None" ) )
			{
				for( int m = 0; m < AllMaterials.Length; m++ )
				{
					if( AllMaterials[m].name.Equals( s.MaterialName ) )
					{
						s.Mat = AllMaterials[m];
						UsedMaterials[i] = s.Mat;
						break;
					}
				}
			}
		}

		return UsedMaterials;
	}
	void SetupSubmeshes( Mesh mesh, Section[] sections, String Name )
	{		
		int[] AllIndices = mesh.GetIndices( 0 );
		mesh.subMeshCount = sections.Length;

		for(int i = 0; i<sections.Length; i++ )
        {
			Section s = sections[i];		
			
			int[] SectionIndices = new int[ s.NumIndices ];

			for(int u=0; u< s.NumIndices; u++ )
            {
				if ( s.Offset + u < AllIndices.Length)
					SectionIndices[u] = AllIndices[ s.Offset + u ];
				else
                {
					Debug.LogError( "AllIndices is too small for " + Name );
					break;
				}
			}

			mesh.SetIndices( SectionIndices, MeshTopology.Triangles, i );
		}		
	}
	MeshBinding[] GetMeshes( GameObject gameObject )
	{
		List<MeshBinding> MeshArray = new List<MeshBinding>();
		MeshFilter[] MFArray = gameObject.GetComponentsInChildren<MeshFilter>();
		if( MFArray != null )
		{
			for( int i = 0; i < MFArray.Length; i++ )
			{
				MeshFilter MF = MFArray[i];
				Mesh m = MFArray[i].sharedMesh;

				MeshBinding NewMeshBinding = new MeshBinding();
				NewMeshBinding.mesh = m;
				NewMeshBinding.renderer = MF.gameObject.GetComponent<MeshRenderer>();
				MeshArray.Add( NewMeshBinding );
			}
		}

		return MeshArray.ToArray();
	}
}
#endif