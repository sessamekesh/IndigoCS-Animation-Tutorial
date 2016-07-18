#pragma once

// One thing I don't like is having to make a bunch of different classes, a different shader class
//  for every shader I decide I might want. The solution might not be in making a general shader class,
//  but instead in making sure only a small amount of shaders are actually used in a project. Perhaps
//  naming them better. Perhaps including them as part of the model even. Or in a shader registry.

#include <future>
#include "Vec4.h"
#include "Matrix.h"
#include "Dirtyable.h"
#include <d3d11.h>
#include "Material.h"
#include "DirectionalLight.h"
#include "Transform.h"
#include <wrl.h>
#include <vector>
using Microsoft::WRL::ComPtr;

class ShaderPNS4_MD1
{
public:
	struct Vertex
	{
	public:
		Vec4 Position;
		Vec4 Normal;
	};

protected:
	struct VSCBuffer_PerObject_Type
	{
		Matrix Model;
	};

	struct VSCBuffer_PerFrame_Type
	{
		Matrix View;
		Matrix Proj;
	};

	struct PSCBuffer_PerObject_Type
	{
		Material ObjectMaterial;
	};

	struct PSCBuffer_PerFrame_Type
	{
		Vec4 CameraPosition;
	};

	struct PSCBuffer_PerScene_Type
	{
		DirectionalLight DirectionalLight1;
	};

public:
	ShaderPNS4_MD1();
	ShaderPNS4_MD1(const ShaderPNS4_MD1&) = delete;
	~ShaderPNS4_MD1() = default;

	std::future<bool> Initialize(ComPtr<ID3D11Device> device);
	bool Render(ComPtr<ID3D11DeviceContext> context, std::uint32_t nVertsToDraw);

	// Shader property setters
	void SetModelMatrix(const Matrix& model);
	void SetViewMatrix(const Matrix& view);
	void SetProjMatrix(const Matrix& proj);
	void SetCameraPosition(const Vec3& position);

	void SetObjectMaterial(const Material& mat);
	void SetDirectionalLight1(const DirectionalLight& dl1);

private:
	Dirtyable<VSCBuffer_PerFrame_Type> vs_cb_frame_;
	Dirtyable<VSCBuffer_PerObject_Type> vs_cb_object_;
	Dirtyable<PSCBuffer_PerObject_Type> ps_cb_object_;
	Dirtyable<PSCBuffer_PerScene_Type> ps_cb_scene_;
	Dirtyable<PSCBuffer_PerFrame_Type> ps_cb_frame_;

private:
	ComPtr<ID3D11VertexShader> vertShader_;
	ComPtr<ID3D11PixelShader> pixelShader_;
	ComPtr<ID3D11InputLayout> inputLayout_;
	ComPtr<ID3D11Buffer> vs_cb_frame_buffer_;
	ComPtr<ID3D11Buffer> vs_cb_object_buffer_;
	ComPtr<ID3D11Buffer> ps_cb_object_buffer_;
	ComPtr<ID3D11Buffer> ps_cb_scene_buffer_;
	ComPtr<ID3D11Buffer> ps_cb_frame_buffer_;
};