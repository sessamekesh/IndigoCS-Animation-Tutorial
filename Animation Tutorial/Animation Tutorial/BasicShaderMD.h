#pragma once

#include "Material.h"
#include <d3d11.h>
#include "DirectionalLight.h"
#include "maffs.h"
#include <wrl.h>
#include <future>
#include "Dirtyable.h"
#include <vector>

using Microsoft::WRL::ComPtr;

class BasicShaderMD
{
public:
	struct Vertex
	{
	public:
		Vec4 Position;
		Vec4 Normal;

	public:
		Vertex(Vec4 pos, Vec4 norm)
			: Position(pos)
			, Normal(norm)
		{}

		Vertex()
			: Vertex(Vec4(), Vec4())
		{}
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
	BasicShaderMD();
	BasicShaderMD(const BasicShaderMD&) = delete;
	~BasicShaderMD() = default;

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