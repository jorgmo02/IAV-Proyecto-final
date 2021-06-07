Shader "Unreal/MI_Modern_Bed_Pillows"
{
	Properties
	{
		_MainTex("MainTex (RGB)", 2D) = "white" {}				Material_Texture2D_0( "Tex0", 2D ) = "white" {}
		Material_Texture2D_1( "Tex1", 2D ) = "white" {}
		Material_Texture2D_2( "Tex2", 2D ) = "white" {}
		Material_Texture2D_3( "Tex3", 2D ) = "white" {}
		Material_Texture2D_4( "Tex4", 2D ) = "white" {}
		Material_Texture2D_5( "Tex5", 2D ) = "white" {}
		Material_Texture2D_6( "Tex6", 2D ) = "white" {}
		Material_Texture2D_7( "Tex7", 2D ) = "white" {}
		Material_Texture2D_8( "Tex8", 2D ) = "white" {}
		Material_Texture2D_9( "Tex9", 2D ) = "white" {}
		Material_Texture2D_10( "Tex10", 2D ) = "white" {}
		Material_Texture2D_11( "Tex11", 2D ) = "white" {}


		View_BufferSizeAndInvSize( "View_BufferSizeAndInvSize", Vector ) = ( 1920,1080,0.00052, 0.00092 )//1920,1080,1/1920, 1/1080
	}
	SubShader 
	{
		 Tags { "RenderType" = "Opaque" }
		//BLEND_ON Tags { "RenderType" = "Transparent"  "Queue" = "Transparent" }
		
		//Blend SrcAlpha OneMinusSrcAlpha
		//Cull Off

		CGPROGRAM

		#include "UnityPBSLighting.cginc"
		 #pragma surface surf Standard vertex:vert addshadow
		//BLEND_ON #pragma surface surf Standard vertex:vert alpha:fade addshadow
		
		#pragma target 5.0

		#define NUM_TEX_COORD_INTERPOLATORS 1
		#define NUM_CUSTOM_VERTEX_INTERPOLATORS 0

		struct Input
		{
			//float3 Normal;
			float2 uv_MainTex : TEXCOORD0;
			float2 uv2_Material_Texture2D_0 : TEXCOORD1;
			float4 color : COLOR;
			float4 tangent;
			//float4 normal;
			float3 viewDir;
			float4 screenPos;
			float3 worldPos;
			//float3 worldNormal;
			float3 normal2;
			INTERNAL_DATA
		};
		void vert( inout appdata_full i, out Input o )
		{
			float3 p_normal = mul( float4( i.normal, 0.0f ), unity_WorldToObject );
			//half4 p_tangent = mul( unity_ObjectToWorld,i.tangent );

			//half3 normal_input = normalize( p_normal.xyz );
			//half3 tangent_input = normalize( p_tangent.xyz );
			//half3 binormal_input = cross( p_normal.xyz,tangent_input.xyz ) * i.tangent.w;
			UNITY_INITIALIZE_OUTPUT( Input, o );

			//o.worldNormal = p_normal;
			o.normal2 = p_normal;
			o.tangent = i.tangent;
			//o.binormal_input = binormal_input;
		}
		uniform sampler2D _MainTex;
		/*
		struct SurfaceOutputStandard
		{
		fixed3 Albedo;		// base (diffuse or specular) color
		fixed3 Normal;		// tangent space normal, if written
		half3 Emission;
		half Metallic;		// 0=non-metal, 1=metal
		// Smoothness is the user facing name, it should be perceptual smoothness but user should not have to deal with it.
		// Everywhere in the code you meet smoothness it is perceptual smoothness
		half Smoothness;	// 0=rough, 1=smooth
		half Occlusion;		// occlusion (default 1)
		fixed Alpha;		// alpha for transparencies
		};
		*/


		#define Texture2D sampler2D
		#define TextureCube samplerCUBE
		#define SamplerState int
		//struct Material
		//{
			//samplers start
			uniform sampler2D    Material_Texture2D_0;
			uniform SamplerState Material_Texture2D_0Sampler;
			uniform sampler2D    Material_Texture2D_1;
			uniform SamplerState Material_Texture2D_1Sampler;
			uniform sampler2D    Material_Texture2D_2;
			uniform SamplerState Material_Texture2D_2Sampler;
			uniform sampler2D    Material_Texture2D_3;
			uniform SamplerState Material_Texture2D_3Sampler;
			uniform sampler2D    Material_Texture2D_4;
			uniform SamplerState Material_Texture2D_4Sampler;
			uniform sampler2D    Material_Texture2D_5;
			uniform SamplerState Material_Texture2D_5Sampler;
			uniform sampler2D    Material_Texture2D_6;
			uniform SamplerState Material_Texture2D_6Sampler;
			uniform sampler2D    Material_Texture2D_7;
			uniform SamplerState Material_Texture2D_7Sampler;
			uniform sampler2D    Material_Texture2D_8;
			uniform SamplerState Material_Texture2D_8Sampler;
			uniform sampler2D    Material_Texture2D_9;
			uniform SamplerState Material_Texture2D_9Sampler;
			uniform sampler2D    Material_Texture2D_10;
			uniform SamplerState Material_Texture2D_10Sampler;
			uniform sampler2D    Material_Texture2D_11;
			uniform SamplerState Material_Texture2D_11Sampler;
			
		//};
		struct MaterialStruct
		{
			float4 VectorExpressions[8];
			float4 ScalarExpressions[4];
		};
		struct ViewStruct
		{
			float GameTime;
			float MaterialTextureMipBias;
			SamplerState MaterialTextureBilinearWrapedSampler;
			SamplerState MaterialTextureBilinearClampedSampler;
			float4 PrimitiveSceneData[ 40 ];
			float2 TemporalAAParams;
			float2 ViewRectMin;
			float4 ViewSizeAndInvSize;
		};
		struct ResolvedViewStruct
		{
			float3 WorldCameraOrigin;
			float4 ScreenPositionScaleBias;
			float4x4 TranslatedWorldToView;
			float4x4 TranslatedWorldToCameraView;
			float4x4 ViewToTranslatedWorld;
			float4x4 CameraViewToTranslatedWorld;
		};
		struct PrimitiveStruct
		{
			float4x4 WorldToLocal;
			float4x4 LocalToWorld;
		};

		ViewStruct View;
		ResolvedViewStruct ResolvedView;
		PrimitiveStruct Primitive;
		uniform float4 View_BufferSizeAndInvSize;
		uniform int Material_Wrap_WorldGroupSettings;

		//#define Primitive_WorldToLocal unity_WorldToObject

		#include "UnrealCommon.cginc"

		MaterialStruct Material;void InitializeExpressions()
{
	Material.VectorExpressions[0] = float4(0.000000,0.000000,0.000000,0.000000);//
	Material.VectorExpressions[1] = float4(0.000000,0.000000,0.000000,0.000000);//
	Material.VectorExpressions[2] = float4(0.933333,0.850980,0.690196,1.000000);//
	Material.VectorExpressions[3] = float4(0.933333,0.850980,0.690196,0.000000);//
	Material.VectorExpressions[4] = float4(0.130000,0.124446,0.104129,1.000000);//
	Material.VectorExpressions[5] = float4(0.130000,0.124446,0.104129,0.000000);//
	Material.VectorExpressions[6] = float4(0.070000,0.068708,0.056000,1.000000);//
	Material.VectorExpressions[7] = float4(0.070000,0.068708,0.056000,0.000000);//
	Material.ScalarExpressions[0] = float4(75.000000,2.000000,75.000000,2.000000);
	Material.ScalarExpressions[1] = float4(75.000000,2.000000,0.000000,75.000000);
	Material.ScalarExpressions[2] = float4(75.000000,75.000000,0.000000,0.800000);
	Material.ScalarExpressions[3] = float4(0.800000,0.900000,0.900000,0.900000);
}void CalcPixelMaterialInputs(in out FMaterialPixelParameters Parameters, in out FPixelMaterialInputs PixelMaterialInputs)
{
	// Initial calculations (required for Normal)
	MaterialFloat2 Local0 = (Parameters.TexCoords[0].xy * Material.ScalarExpressions[0].x);
	MaterialFloat4 Local1 = UnpackNormalMap(Texture2DSampleBias(Material_Texture2D_0, Material_Texture2D_0Sampler,Local0,View.MaterialTextureMipBias));
	MaterialFloat3 Local2 = (Local1.rgb * Material.ScalarExpressions[0].y);
	MaterialFloat3 Local3 = (1.00000000 - Local2);
	MaterialFloat4 Local4 = UnpackNormalMap(Texture2DSampleBias(Material_Texture2D_1, Material_Texture2D_1Sampler,Parameters.TexCoords[0].xy,View.MaterialTextureMipBias));
	MaterialFloat3 Local5 = (1.00000000 - Local4.rgb);
	MaterialFloat3 Local6 = (Local3 * Local5);
	MaterialFloat3 Local7 = (1.00000000 - Local6);
	MaterialFloat4 Local8 = ProcessMaterialColorTextureLookup(Texture2DSampleBias(Material_Texture2D_2, Material_Texture2D_2Sampler,Parameters.TexCoords[0].xy,View.MaterialTextureMipBias));
	MaterialFloat3 Local9 = (Local7 * Local8.b);
	MaterialFloat2 Local10 = (Parameters.TexCoords[0].xy * Material.ScalarExpressions[0].z);
	MaterialFloat4 Local11 = UnpackNormalMap(Texture2DSampleBias(Material_Texture2D_3, Material_Texture2D_3Sampler,Local10,View.MaterialTextureMipBias));
	MaterialFloat3 Local12 = (Local11.rgb * Material.ScalarExpressions[0].w);
	MaterialFloat3 Local13 = (1.00000000 - Local12);
	MaterialFloat3 Local14 = (Local13 * Local5);
	MaterialFloat3 Local15 = (1.00000000 - Local14);
	MaterialFloat3 Local16 = (Local15 * Local8.g);
	MaterialFloat3 Local17 = (Local9 + Local16);
	MaterialFloat2 Local18 = (Parameters.TexCoords[0].xy * Material.ScalarExpressions[1].x);
	MaterialFloat4 Local19 = UnpackNormalMap(Texture2DSampleBias(Material_Texture2D_4, Material_Texture2D_4Sampler,Local18,View.MaterialTextureMipBias));
	MaterialFloat3 Local20 = (Local19.rgb * Material.ScalarExpressions[1].y);
	MaterialFloat3 Local21 = (1.00000000 - Local20);
	MaterialFloat3 Local22 = (Local21 * Local5);
	MaterialFloat3 Local23 = (1.00000000 - Local22);
	MaterialFloat3 Local24 = (Local23 * Local8.r);
	MaterialFloat3 Local25 = (Local17 + Local24);

	// The Normal is a special case as it might have its own expressions and also be used to calculate other inputs, so perform the assignment here
	PixelMaterialInputs.Normal = Local25;


	// Note that here MaterialNormal can be in world space or tangent space
	float3 MaterialNormal = GetMaterialNormal(Parameters, PixelMaterialInputs);

#if MATERIAL_TANGENTSPACENORMAL
#if SIMPLE_FORWARD_SHADING
	Parameters.WorldNormal = float3(0, 0, 1);
#endif

#if FEATURE_LEVEL >= FEATURE_LEVEL_SM4
	// Mobile will rely on only the final normalize for performance
	MaterialNormal = normalize(MaterialNormal);
#endif

	// normalizing after the tangent space to world space conversion improves quality with sheared bases (UV layout to WS causes shrearing)
	// use full precision normalize to avoid overflows
	Parameters.WorldNormal = TransformTangentNormalToWorld(Parameters.TangentToWorld, MaterialNormal);

#else //MATERIAL_TANGENTSPACENORMAL

	Parameters.WorldNormal = normalize(MaterialNormal);

#endif //MATERIAL_TANGENTSPACENORMAL

#if MATERIAL_TANGENTSPACENORMAL
	// flip the normal for backfaces being rendered with a two-sided material
	Parameters.WorldNormal *= Parameters.TwoSidedSign;
#endif

	Parameters.ReflectionVector = ReflectionAboutCustomWorldNormal(Parameters, Parameters.WorldNormal, false);

#if !PARTICLE_SPRITE_FACTORY
	Parameters.Particle.MotionBlurFade = 1.0f;
#endif // !PARTICLE_SPRITE_FACTORY

	// Now the rest of the inputs
	MaterialFloat3 Local26 = lerp(MaterialFloat3(0.00000000,0.00000000,0.00000000),Material.VectorExpressions[1].rgb,MaterialFloat(Material.ScalarExpressions[1].z));
	MaterialFloat2 Local27 = (Parameters.TexCoords[0].xy * Material.ScalarExpressions[1].w);
	MaterialFloat4 Local28 = ProcessMaterialColorTextureLookup(Texture2DSampleBias(Material_Texture2D_5, Material_Texture2D_5Sampler,Local27,View.MaterialTextureMipBias));
	MaterialFloat3 Local29 = (Material.VectorExpressions[3].rgb * Local28.rgb);
	MaterialFloat3 Local30 = (Local29 * Local8.g);
	MaterialFloat2 Local31 = (Parameters.TexCoords[0].xy * Material.ScalarExpressions[2].x);
	MaterialFloat4 Local32 = ProcessMaterialColorTextureLookup(Texture2DSampleBias(Material_Texture2D_6, Material_Texture2D_6Sampler,Local31,View.MaterialTextureMipBias));
	MaterialFloat3 Local33 = (Material.VectorExpressions[5].rgb * Local32.rgb);
	MaterialFloat3 Local34 = (Local33 * Local8.r);
	MaterialFloat3 Local35 = (Local30 + Local34);
	MaterialFloat2 Local36 = (Parameters.TexCoords[0].xy * Material.ScalarExpressions[2].y);
	MaterialFloat4 Local37 = ProcessMaterialColorTextureLookup(Texture2DSampleBias(Material_Texture2D_7, Material_Texture2D_7Sampler,Local36,View.MaterialTextureMipBias));
	MaterialFloat3 Local38 = (Material.VectorExpressions[7].rgb * Local37.rgb);
	MaterialFloat3 Local39 = (Local38 * Local8.b);
	MaterialFloat3 Local40 = (Local35 + Local39);
	MaterialFloat Local41 = (Material.ScalarExpressions[2].z * Local8.b);
	MaterialFloat Local42 = (Material.ScalarExpressions[2].w * Local8.g);
	MaterialFloat Local43 = (Local41 + Local42);
	MaterialFloat Local44 = (Material.ScalarExpressions[3].x * Local8.r);
	MaterialFloat Local45 = (Local43 + Local44);
	MaterialFloat2 Local46 = (Parameters.TexCoords[0].xy * 2.00000000);
	MaterialFloat4 Local47 = ProcessMaterialColorTextureLookup(Texture2DSampleBias(Material_Texture2D_8, Material_Texture2D_8Sampler,Local46,View.MaterialTextureMipBias));
	MaterialFloat Local48 = (Material.ScalarExpressions[3].y * Local8.b);
	MaterialFloat3 Local49 = (Local47.rgb * Local48);
	MaterialFloat4 Local50 = ProcessMaterialColorTextureLookup(Texture2DSampleBias(Material_Texture2D_9, Material_Texture2D_9Sampler,Local46,View.MaterialTextureMipBias));
	MaterialFloat Local51 = (Material.ScalarExpressions[3].z * Local8.g);
	MaterialFloat3 Local52 = (Local50.rgb * Local51);
	MaterialFloat3 Local53 = (Local49 + Local52);
	MaterialFloat4 Local54 = ProcessMaterialColorTextureLookup(Texture2DSampleBias(Material_Texture2D_10, Material_Texture2D_10Sampler,Local46,View.MaterialTextureMipBias));
	MaterialFloat Local55 = (Material.ScalarExpressions[3].w * Local8.r);
	MaterialFloat3 Local56 = (Local54.rgb * Local55);
	MaterialFloat3 Local57 = (Local53 + Local56);
	MaterialFloat4 Local58 = ProcessMaterialColorTextureLookup(Texture2DSampleBias(Material_Texture2D_11, Material_Texture2D_11Sampler,Parameters.TexCoords[0].xy,View.MaterialTextureMipBias));

	PixelMaterialInputs.EmissiveColor = Local26;
	PixelMaterialInputs.Opacity = 1.00000000;
	PixelMaterialInputs.OpacityMask = 1.00000000;
	PixelMaterialInputs.BaseColor = Local40;
	PixelMaterialInputs.Metallic = Local45;
	PixelMaterialInputs.Specular = 0.50000000;
	PixelMaterialInputs.Roughness = Local57;
	PixelMaterialInputs.Anisotropy = 0.00000000;
	PixelMaterialInputs.Tangent = MaterialFloat3(1.00000000,0.00000000,0.00000000);
	PixelMaterialInputs.Subsurface = 0;
	PixelMaterialInputs.AmbientOcclusion = Local58.rgb;
	PixelMaterialInputs.Refraction = 0;
	PixelMaterialInputs.PixelDepthOffset = 0.00000000;
	PixelMaterialInputs.ShadingModel = 1;


#if MATERIAL_USES_ANISOTROPY
	Parameters.WorldTangent = CalculateAnisotropyTangent(Parameters, PixelMaterialInputs);
#else
	Parameters.WorldTangent = 0;
#endif
}

		void surf( Input In, inout SurfaceOutputStandard o )
		{
			InitializeExpressions();

			float4 Z4 = float4( 0, 0, 0, 0 );

			float3 UnrealWorldPos = float3( In.worldPos.x, In.worldPos.y, In.worldPos.z );
			
			float3 UnrealNormal = In.normal2;

			FMaterialPixelParameters Parameters;
			#if NUM_TEX_COORD_INTERPOLATORS > 0			
				Parameters.TexCoords[ 0 ] = float2( In.uv_MainTex.x, 1.0 - In.uv_MainTex.y );
			#endif
			#if NUM_TEX_COORD_INTERPOLATORS > 1
				Parameters.TexCoords[ 1 ] = float2( In.uv2_Material_Texture2D_0.x, 1.0 - In.uv2_Material_Texture2D_0.y );
			#endif
			#if NUM_TEX_COORD_INTERPOLATORS > 2
			for( int i = 2; i < NUM_TEX_COORD_INTERPOLATORS; i++ )
			{
				Parameters.TexCoords[ i ] = float2( In.uv_MainTex.x, 1.0 - In.uv_MainTex.y );
			}
			#endif
			Parameters.VertexColor = In.color;
			Parameters.WorldNormal = UnrealNormal;
			Parameters.ReflectionVector = half3( 0, 0, 1 );
			//Parameters.CameraVector = normalize( _WorldSpaceCameraPos.xyz - UnrealWorldPos.xyz );
			Parameters.CameraVector = mul( ( float3x3 )unity_CameraToWorld, float3( 0, 0, 1 ) ) * -1;
			Parameters.LightVector = half3( 0, 0, 0 );
			Parameters.SvPosition = float4( In.screenPos.x, In.screenPos.y, 0, 0 );
			Parameters.ScreenPosition = float4( In.screenPos.x, In.screenPos.y, 0, 0 );

			Parameters.UnMirrored = 1;

			Parameters.TwoSidedSign = 1;
			

			float3 InWorldNormal = UnrealNormal;
			float4 InTangent = In.tangent;
			float4 tangentWorld = float4( UnityObjectToWorldDir( InTangent.xyz ), InTangent.w );
			tangentWorld.xyz = normalize( tangentWorld.xyz );
			float3x3 tangentToWorld = CreateTangentToWorldPerVertex( InWorldNormal, tangentWorld.xyz, tangentWorld.w );
			Parameters.TangentToWorld = tangentToWorld;

			//WorldAlignedTexturing in UE relies on the fact that coords there are 100x larger, prepare values for that
			//but watch out for any computation that might get skewed as a side effect
			UnrealWorldPos = UnrealWorldPos * 100;

			//Parameters.TangentToWorld = half3x3( float3( 1, 1, 1 ), float3( 1, 1, 1 ), UnrealNormal.xyz );
			Parameters.AbsoluteWorldPosition = UnrealWorldPos;
			Parameters.WorldPosition_CamRelative = UnrealWorldPos;
			Parameters.WorldPosition_NoOffsets = UnrealWorldPos;

			Parameters.WorldPosition_NoOffsets_CamRelative = Parameters.WorldPosition_CamRelative;
			Parameters.LightingPositionOffset = float3( 0, 0, 0 );

			Parameters.AOMaterialMask = 0;

			Parameters.Particle.RelativeTime = 0;
			Parameters.Particle.MotionBlurFade;
			Parameters.Particle.Random = 0;
			Parameters.Particle.Velocity = half4( 1, 1, 1, 1 );
			Parameters.Particle.Color = half4( 1, 1, 1, 1 );
			Parameters.Particle.TranslatedWorldPositionAndSize = float4( UnrealWorldPos, 0 );
			Parameters.Particle.MacroUV = half4(0,0,1,1);
			Parameters.Particle.DynamicParameter = half4(0,0,0,0);
			Parameters.Particle.LocalToWorld = float4x4( Z4, Z4, Z4, Z4 );
			Parameters.Particle.Size = float2(1,1);
			Parameters.TexCoordScalesParams = float2( 0, 0 );
			Parameters.PrimitiveId = 0;

			FPixelMaterialInputs PixelMaterialInputs = ( FPixelMaterialInputs)0;
			PixelMaterialInputs.Normal = float3( 0, 0, 1 );
			PixelMaterialInputs.ShadingModel = 0;

			//Extra
			View.GameTime = -_Time.y;// _Time is (t/20, t, t*2, t*3), run in reverse because it works better with ElementalDemo
			View.MaterialTextureMipBias = 0.0;
			View.TemporalAAParams = float2( 0, 0 );
			View.ViewRectMin = float2( 0, 0 );
			View.ViewSizeAndInvSize = View_BufferSizeAndInvSize;
			for( int i2 = 0; i2 < 40; i2++ )
				View.PrimitiveSceneData[ i2 ] = float4( 0, 0, 0, 0 );
			ResolvedView.WorldCameraOrigin = _WorldSpaceCameraPos.xyz;
			ResolvedView.ScreenPositionScaleBias = float4( 1, 1, 0, 0 );
			ResolvedView.TranslatedWorldToView = unity_MatrixV;
			ResolvedView.TranslatedWorldToCameraView = unity_MatrixV;
			ResolvedView.ViewToTranslatedWorld = unity_MatrixInvV;
			ResolvedView.CameraViewToTranslatedWorld = unity_MatrixInvV;
			Primitive.WorldToLocal = unity_WorldToObject;
			Primitive.LocalToWorld = unity_ObjectToWorld;
			CalcPixelMaterialInputs( Parameters, PixelMaterialInputs );

			float4 DiffuseTexel = tex2D(_MainTex, In.uv_MainTex );

			//Debug
			//PixelMaterialInputs.BaseColor = Texture2DSample( Material_Texture2D_1, Material_Texture2D_1Sampler, Parameters.TexCoords[ 0 ].xy );

			o.Albedo = PixelMaterialInputs.BaseColor.rgb;
			o.Alpha = PixelMaterialInputs.Opacity;
			//if( PixelMaterialInputs.OpacityMask < 0.01 ) discard;

			o.Metallic = PixelMaterialInputs.Metallic;
			o.Smoothness = 1.0 - PixelMaterialInputs.Roughness;
			o.Normal = normalize( PixelMaterialInputs.Normal );
			o.Emission = PixelMaterialInputs.EmissiveColor.rgb;
		}
		ENDCG
	}
	Fallback "Diffuse"
}