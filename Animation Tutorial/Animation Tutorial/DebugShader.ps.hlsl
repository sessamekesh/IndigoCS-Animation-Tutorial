#include "Material.hlsli"

cbuffer PerObject : register(b0)
{
	Material ObjectMaterial;
};

struct DebugPixelInput
{
	float4 Position : SV_POSITION;
};

float4 main(DebugPixelInput pin) : SV_TARGET
{
	return ObjectMaterial.AmbientColor;
}