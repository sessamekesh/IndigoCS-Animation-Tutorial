// Vertex buffer to process vertices in a basic context
//  Each vertex has a position and a normal

//
// STRUCT DEFS
//
struct BasicPosNormVertexInput
{
	float4 Position : POSITION;
	float4 Normal : NORMAL;
};

struct BasicPosNormVertexOutput
{
	float4 Position : SV_POSITION;
	float4 WorldPosition : POSITION;
	float4 Normal : NORMAL;
};

//
// CBUFFERS
//
cbuffer PerObject : register(b0)
{
	matrix mModel;
}

cbuffer PerFrame : register(b1)
{
	matrix mView;
	matrix mProj;
}

BasicPosNormVertexOutput main( BasicPosNormVertexInput vin)
{
	BasicPosNormVertexOutput vout;

	vin.Position.w = 1.f;
	vin.Normal.w = 0.f;

	vout.Position = mul(vin.Position, mModel);
	vout.Position = mul(vout.Position, mView);
	vout.Position = mul(vout.Position, mProj);
	
	vout.WorldPosition = mul(vin.Position, mModel);

	vout.Normal = mul(vin.Normal, mModel);

	return vout;
}