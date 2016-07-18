struct DebugPixelInput
{
	float4 Position : SV_POSITION;
};

cbuffer PerObject : register(b0)
{
	matrix mWorld;
}

cbuffer PerFrame : register(b1)
{
	matrix mView;
	matrix mProj;
}

struct DebugVertexInput
{
	float4 Position : POSITION;
};

DebugPixelInput main(DebugVertexInput vin)
{
	DebugPixelInput tr;

	vin.Position.w = 1.f;

	tr.Position = mul(vin.Position, mWorld);
	tr.Position = mul(tr.Position, mView);
	tr.Position = mul(tr.Position, mProj);

	return tr;
}