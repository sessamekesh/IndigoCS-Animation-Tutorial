#include "ShaderPNS4_MD1.h"
#include <fstream>
#include "Logger.h"

#ifndef VALIDATE
#define VALIDATE(hr, msg) if (FAILED(hr)) { Logger::Log(msg); return false; }
#endif

ShaderPNS4_MD1::ShaderPNS4_MD1()
	: vs_cb_frame_({ Matrix(), Matrix() })
	, vs_cb_object_({ Matrix() })
	, ps_cb_object_({ Material::BasicGray })
	, ps_cb_scene_({ DirectionalLight(Color::White, Color::White, Color::White, Vec3::Zero, 1.f) })
	, ps_cb_frame_({ Vec4(0.f, 0.f, 0.f, 1.f) })
	, vertShader_(nullptr)
	, pixelShader_(nullptr)
	, inputLayout_(nullptr)
	, vs_cb_frame_buffer_(nullptr)
	, vs_cb_object_buffer_(nullptr)
	, ps_cb_object_buffer_(nullptr)
	, ps_cb_scene_buffer_(nullptr)
	, ps_cb_frame_buffer_(nullptr)
{}

std::future<bool> ShaderPNS4_MD1::Initialize(ComPtr<ID3D11Device> device)
{
	return std::async(std::launch::async, [this, device] {
		const char* vsFname = "./BasicPosNorm.cso";
		const char* psFname = "./BasicMaterialDirectional1.ps.cso";
		
		std::uint32_t vsDataLength = 0u;
		std::uint32_t psDataLength = 0u;

		HRESULT hr = { 0 };

		D3D11_INPUT_ELEMENT_DESC inputLayout[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};
		std::uint32_t numElements = _countof(inputLayout);

		std::future<char*> vsData = std::async(std::launch::async, [this, &vsDataLength, vsFname] {
			std::ifstream vin(vsFname, std::ios::binary);
			if (!vin)
			{
				Logger::Log("Failed to open vertex shader file for reading");
				return (char*)nullptr;
			}

			vin.seekg(0, std::ios::end);
			vsDataLength = (std::uint32_t)vin.tellg();
			vin.seekg(0, std::ios::beg);
			char* vsBytecode = new char[vsDataLength];
			vin.read(vsBytecode, vsDataLength);
			return vsBytecode;
		});

		std::future<char*> psData = std::async(std::launch::async, [this, &psDataLength, psFname] {
			std::ifstream pin(psFname, std::ios::binary);
			if (!pin)
			{
				Logger::Log("Failed to open pixel shader file for reading");
				return (char*)nullptr;
			}

			pin.seekg(0, std::ios::end);
			psDataLength = (std::uint32_t)pin.tellg();
			pin.seekg(0, std::ios::beg);
			char* psBytecode = new char[psDataLength];
			pin.read(psBytecode, psDataLength);

			return psBytecode;
		});

		// While the vertex and pixel shader are loading, go ahead and create the buffers.
		D3D11_BUFFER_DESC bufferDesc = { 0 };
		bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		bufferDesc.MiscFlags = 0x00;
		bufferDesc.StructureByteStride = 0x00;

		bufferDesc.ByteWidth = sizeof(ShaderPNS4_MD1::VSCBuffer_PerObject_Type);
		hr = device->CreateBuffer(&bufferDesc, nullptr, &vs_cb_object_buffer_);
		VALIDATE(hr, "Failed to create vertex buffer per-object constant buffer");

		bufferDesc.ByteWidth = sizeof(ShaderPNS4_MD1::VSCBuffer_PerFrame_Type);
		hr = device->CreateBuffer(&bufferDesc, nullptr, &vs_cb_frame_buffer_);
		VALIDATE(hr, "Failed to create vertex buffer per-frame constant buffer");

		bufferDesc.ByteWidth = sizeof(ShaderPNS4_MD1::PSCBuffer_PerObject_Type);
		hr = device->CreateBuffer(&bufferDesc, nullptr, &ps_cb_object_buffer_);
		VALIDATE(hr, "Failed to create pixel buffer per-object constant buffer");

		bufferDesc.ByteWidth = sizeof(ShaderPNS4_MD1::PSCBuffer_PerScene_Type);
		hr = device->CreateBuffer(&bufferDesc, nullptr, &ps_cb_scene_buffer_);
		VALIDATE(hr, "Failed to create pixel buffer per-scene constant buffer");

		bufferDesc.ByteWidth = sizeof(ShaderPNS4_MD1::PSCBuffer_PerFrame_Type);
		hr = device->CreateBuffer(&bufferDesc, nullptr, &ps_cb_frame_buffer_);
		VALIDATE(hr, "Failed to create pixel buffer per-frame constant buffer");

		char* vsDataResult = vsData.get();
		if (vsDataResult == nullptr) return false;

		hr = device->CreateVertexShader(vsDataResult, vsDataLength, nullptr, &vertShader_);
		VALIDATE(hr, "Failed to create vertex shader!");

		hr = device->CreateInputLayout(inputLayout, numElements, vsDataResult, vsDataLength, &inputLayout_);
		VALIDATE(hr, "Failed to create input layout!");

		delete[] vsDataResult;

		char* psDataResult = psData.get();
		if (psDataResult == nullptr)
		{
			return false;
		}

		hr = device->CreatePixelShader(psDataResult, psDataLength, nullptr, &pixelShader_);
		VALIDATE(hr, "Failed to create pixel shader!");

		delete[] psDataResult;

		return true;
	});
}

bool ShaderPNS4_MD1::Render(ComPtr<ID3D11DeviceContext> context, std::uint32_t nVertsToDraw)
{
	HRESULT hr = { 0 };

	context->IASetInputLayout(inputLayout_.Get());
	context->VSSetShader(vertShader_.Get(), nullptr, 0);
	context->PSSetShader(pixelShader_.Get(), nullptr, 0);

	if (vs_cb_frame_.IsDirty())
	{
		D3D11_MAPPED_SUBRESOURCE vscb;
		VSCBuffer_PerFrame_Type* data;

		hr = context->Map(vs_cb_frame_buffer_.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0x00, &vscb);
		VALIDATE(hr, "Failed to map vertex per-frame constant buffer for CPU writing");

		data = (VSCBuffer_PerFrame_Type*)vscb.pData;
		data->Proj = vs_cb_frame_.Get().Proj;
		data->View = vs_cb_frame_.Get().View;

		context->Unmap(vs_cb_frame_buffer_.Get(), 0);

		vs_cb_frame_.Clean();
	}

	if (vs_cb_object_.IsDirty())
	{
		D3D11_MAPPED_SUBRESOURCE vscb;
		VSCBuffer_PerObject_Type* data;

		hr = context->Map(vs_cb_object_buffer_.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0x00, &vscb);
		VALIDATE(hr, "Failed to map vertex per-object constant buffer for CPU writing");

		data = (VSCBuffer_PerObject_Type*)vscb.pData;
		data->Model = vs_cb_object_.Get().Model;

		context->Unmap(vs_cb_object_buffer_.Get(), 0);

		vs_cb_object_.Clean();
	}
	ID3D11Buffer* vsCBuffers[] = { vs_cb_object_buffer_.Get(), vs_cb_frame_buffer_.Get() };
	context->VSSetConstantBuffers(0, _countof(vsCBuffers), vsCBuffers);

	if (ps_cb_object_.IsDirty())
	{
		D3D11_MAPPED_SUBRESOURCE pscb;
		PSCBuffer_PerObject_Type* data;

		hr = context->Map(ps_cb_object_buffer_.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0x00, &pscb);
		VALIDATE(hr, "Failed to map per-object constant buffer for CPU writing");

		data = (PSCBuffer_PerObject_Type*)pscb.pData;
		data->ObjectMaterial = ps_cb_object_.Get().ObjectMaterial;

		context->Unmap(ps_cb_object_buffer_.Get(), 0);

		ps_cb_object_.Clean();
	}

	if (ps_cb_scene_.IsDirty())
	{
		D3D11_MAPPED_SUBRESOURCE pscb;
		PSCBuffer_PerScene_Type* data;

		hr = context->Map(ps_cb_scene_buffer_.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0x00, &pscb);
		VALIDATE(hr, "Failed to map per scene constant buffer for CPU writing");

		data = (PSCBuffer_PerScene_Type*)pscb.pData;
		data->DirectionalLight1 = ps_cb_scene_.Get().DirectionalLight1;

		context->Unmap(ps_cb_scene_buffer_.Get(), 0);

		ps_cb_scene_.Clean();
	}

	if (ps_cb_frame_.IsDirty())
	{
		D3D11_MAPPED_SUBRESOURCE pscb;
		PSCBuffer_PerFrame_Type* data;

		hr = context->Map(ps_cb_frame_buffer_.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0x00, &pscb);
		VALIDATE(hr, "Failed to map per frame constant buffer for CPU writing");

		data = (PSCBuffer_PerFrame_Type*)pscb.pData;
		data->CameraPosition = ps_cb_frame_.Get().CameraPosition;

		context->Unmap(ps_cb_frame_buffer_.Get(), 0);

		ps_cb_frame_.Clean();
	}

	ID3D11Buffer*psCBuffers[] = { ps_cb_object_buffer_.Get(), ps_cb_scene_buffer_.Get(), ps_cb_frame_buffer_.Get() };
	context->PSSetConstantBuffers(0, _countof(psCBuffers), psCBuffers);

	// Perform actual draw calls
	context->DrawIndexed(nVertsToDraw, 0, 0);

	return true;
}

void ShaderPNS4_MD1::SetModelMatrix(const Matrix& model)
{
	vs_cb_object_.Get().Model = model;
	vs_cb_object_.Dirty();
}

void ShaderPNS4_MD1::SetViewMatrix(const Matrix& view)
{
	vs_cb_frame_.Get().View = view.Transpose();
	vs_cb_frame_.Dirty();
}

void ShaderPNS4_MD1::SetProjMatrix(const Matrix& proj)
{
	vs_cb_frame_.Get().Proj = proj.Transpose();
	vs_cb_frame_.Dirty();
}

void ShaderPNS4_MD1::SetCameraPosition(const Vec3 & position)
{
	ps_cb_frame_.Get().CameraPosition = Vec4(position.x, position.y, position.z, 1.f);
	ps_cb_frame_.Dirty();
}

void ShaderPNS4_MD1::SetObjectMaterial(const Material& mat)
{
	ps_cb_object_.Get().ObjectMaterial = mat;
	ps_cb_object_.Dirty();
}

void ShaderPNS4_MD1::SetDirectionalLight1(const DirectionalLight& dl)
{
	ps_cb_scene_.Get().DirectionalLight1 = dl;
	ps_cb_scene_.Dirty();
}