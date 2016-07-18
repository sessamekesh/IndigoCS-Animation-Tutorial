#pragma once

#include <d3d11.h>
#include <d3dcompiler.h>
#include <wrl.h>
#include "Vec3.h"
#include "Dirtyable.h"
#include "Matrix.h"
#include "Material.h"

using Microsoft::WRL::ComPtr;

#include <DirectXMath.h>

struct PSCBuffer_PerObject_Type
{
	Material ObjectMaterial;
};

struct VSCBuffer_PerFrame_Type
{
	Matrix View;
	Matrix Proj;
};

struct VSCBuffer_PerObject_Type
{
	Matrix World;
};

class DebugShader
{
public:
	struct Vertex
	{
	public:
		DirectX::XMFLOAT3 Position;
	};

public:
	DebugShader();
	DebugShader(const DebugShader&) = delete;
	~DebugShader() = default;

	bool Initialize(ComPtr<ID3D11Device> device, const char* vsFname, const char* psFname);
	bool Render(ComPtr<ID3D11DeviceContext> context, int nVertsToDraw);

	// Shader property setters
	void SetWorldMatrix(const Matrix& world);
	void SetViewMatrix(const Matrix& view);
	void SetProjMatrix(const Matrix& proj);

	void SetObjectMaterial(Material);

private:
	Dirtyable<VSCBuffer_PerFrame_Type> vs_cb_frame_;
	Dirtyable<VSCBuffer_PerObject_Type> vs_cb_object_;
	Dirtyable<PSCBuffer_PerObject_Type> ps_cb_object_;

private:
	ComPtr<ID3D11VertexShader> vertShader_;
	ComPtr<ID3D11PixelShader> pixelShader_;
	ComPtr<ID3D11InputLayout> inputLayout_;
	ComPtr<ID3D11Buffer> vs_cb_frame_buffer_;
	ComPtr<ID3D11Buffer> vs_cb_object_buffer_;
	ComPtr<ID3D11Buffer> ps_cb_object_buffer_;
};