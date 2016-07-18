#include "Material.hlsli"
#include "LightDefs.hlsli"

// Shader type: Basic
// Pixel coloration method:
//  - Material
//  - Single Directional light

cbuffer PerObject : register(b0)
{
	Material ObjectMaterial;
}

cbuffer PerScene : register(b1)
{
	DirectionalLight DirectionalLight1;
}

cbuffer PerFrame : register(b2)
{
	float4 CameraPosition;
}

struct BasicPosNormVertexOutput
{
	float4 Position : SV_POSITION;
	float4 WorldPosition : POSITION;
	float4 Normal : NORMAL;
};

float4 main(BasicPosNormVertexOutput pin) : SV_TARGET
{
	// For now, just return the material...
	float4 ambient = float4(0.f, 0.f, 0.f, 0.f);
	float4 diffuse = float4(0.f, 0.f, 0.f, 0.f);
	float4 specular = float4(0.f, 0.f, 0.f, 0.f);

	ComputeDirectionalLight(ObjectMaterial.AmbientColor, ObjectMaterial.DiffuseColor, ObjectMaterial.SpecularColor,
		DirectionalLight1, normalize(pin.Normal.xyz), normalize(CameraPosition - pin.WorldPosition), ambient, diffuse, specular);

	//return ObjectMaterial.AmbientColor;
	return saturate(ambient + diffuse + saturate(specular));
}