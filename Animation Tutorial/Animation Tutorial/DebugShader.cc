#include "DebugShader.h"
#include "Logger.h"
#include <fstream>
#include <cinttypes>

#define VALIDATE(hr, msg) if (FAILED(hr)) { Logger::Log(msg); return false; }

DebugShader::DebugShader()
	: vs_cb_frame_({ Matrix(), Matrix() })
	, vs_cb_object_({ Matrix() })
	, ps_cb_object_({ Material::BasicGray })
	, vertShader_(nullptr)
	, pixelShader_(nullptr)
	, inputLayout_(nullptr)
	, vs_cb_frame_buffer_(nullptr)
	, vs_cb_object_buffer_(nullptr)
	, ps_cb_object_buffer_(nullptr)
{}

bool DebugShader::Initialize(ComPtr<ID3D11Device> device, const char * vsFname, const char * psFname)
{
	HRESULT hr = { 0 };
	char* vsData;
	char* psData;
	std::uint32_t vsDataLength;
	std::uint32_t psDataLength;

	D3D11_INPUT_ELEMENT_DESC inputLayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	unsigned int numElements = _countof(inputLayout);
	D3D11_BUFFER_DESC vscbPerFrameDesc = { 0 };
	D3D11_BUFFER_DESC vscbPerObjectDesc = { 0 };
	D3D11_BUFFER_DESC pscbPerObjectDesc = { 0 };

	std::ifstream vin(vsFname, std::ios::binary);
	if (!vin) { Logger::Log("Failed to open vertex shader file!"); return false; }

	vin.seekg(0, std::ios::end);
	vsDataLength = (std::uint32_t)vin.tellg();
	vin.seekg(0, std::ios::beg);
	vsData = new char[vsDataLength];
	vin.read(vsData, vsDataLength);
	vin.close();

	hr = device->CreateVertexShader(vsData, vsDataLength, nullptr, &vertShader_);
	VALIDATE(hr, "Failed to create vertex shader!");

	hr = device->CreateInputLayout(inputLayout, numElements, vsData, vsDataLength, &inputLayout_);
	VALIDATE(hr, "Failed to create input layout!");

	delete[] vsData; vsData = nullptr;

	std::ifstream pin(psFname, std::ios::binary);
	if (!pin) { Logger::Log("Failed to open pixel shader file!"); return false; }

	pin.seekg(0, std::ios::end);
	psDataLength = (std::uint32_t)pin.tellg();
	pin.seekg(0, std::ios::beg);
	psData = new char[psDataLength];
	pin.read(psData, psDataLength);
	pin.close();

	hr = device->CreatePixelShader(psData, psDataLength, nullptr, &pixelShader_);
	VALIDATE(hr, "Failed to create pixel shader!");

	delete[] psData; psData = nullptr;

	// Buffer descriptions
	vscbPerFrameDesc.Usage = pscbPerObjectDesc.Usage = vscbPerObjectDesc.Usage = D3D11_USAGE_DYNAMIC;
	vscbPerFrameDesc.BindFlags = pscbPerObjectDesc.BindFlags = vscbPerObjectDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	vscbPerFrameDesc.CPUAccessFlags = pscbPerObjectDesc.CPUAccessFlags = vscbPerObjectDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vscbPerFrameDesc.MiscFlags = pscbPerObjectDesc.MiscFlags = vscbPerObjectDesc.MiscFlags = 0x00;
	vscbPerFrameDesc.StructureByteStride = pscbPerObjectDesc.StructureByteStride = vscbPerObjectDesc.StructureByteStride = 0;

	vscbPerFrameDesc.ByteWidth = sizeof(VSCBuffer_PerFrame_Type);
	vscbPerObjectDesc.ByteWidth = sizeof(VSCBuffer_PerObject_Type);
	pscbPerObjectDesc.ByteWidth = sizeof(PSCBuffer_PerObject_Type);

	hr = device->CreateBuffer(&vscbPerFrameDesc, nullptr, &vs_cb_frame_buffer_);
	VALIDATE(hr, "Failed to create VS per-frame CBuffer");

	hr = device->CreateBuffer(&vscbPerObjectDesc, nullptr, &vs_cb_object_buffer_);
	VALIDATE(hr, "Failed to create VS per-object CBuffer");

	hr = device->CreateBuffer(&pscbPerObjectDesc, nullptr, &ps_cb_object_buffer_);
	VALIDATE(hr, "Failed to create PS per-object CBuffer");

	return true;
}

bool DebugShader::Render(ComPtr<ID3D11DeviceContext> context, int nVertsToDraw)
{
	HRESULT hr = { 0 };

	context->IASetInputLayout(inputLayout_.Get());
	context->VSSetShader(vertShader_.Get(), nullptr, 0);
	context->PSSetShader(pixelShader_.Get(), nullptr, 0);

	if (vs_cb_frame_.IsDirty())
	{
		D3D11_MAPPED_SUBRESOURCE vscbFrameMapping;
		VSCBuffer_PerFrame_Type* mappedData;

		hr = context->Map(vs_cb_frame_buffer_.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0x00, &vscbFrameMapping);
		VALIDATE(hr, "Failed to map vertex VS per-frame CBuffer for CPU writing");
		mappedData = (VSCBuffer_PerFrame_Type*)vscbFrameMapping.pData;
		mappedData->Proj = vs_cb_frame_.Get().Proj;
		mappedData->View = vs_cb_frame_.Get().View;
		context->Unmap(vs_cb_frame_buffer_.Get(), 0);

		vs_cb_frame_.Clean();
	}
	context->VSSetConstantBuffers(1, 1, vs_cb_frame_buffer_.GetAddressOf());

	if (vs_cb_object_.IsDirty())
	{
		D3D11_MAPPED_SUBRESOURCE vscbObjectMapping;
		VSCBuffer_PerObject_Type* mappedData;

		hr = context->Map(vs_cb_object_buffer_.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0x00, &vscbObjectMapping);
		VALIDATE(hr, "Failed to map VS per-object CBuffer for CPU writing");
		mappedData = (VSCBuffer_PerObject_Type*)vscbObjectMapping.pData;
		mappedData->World = vs_cb_object_.Get().World;
		context->Unmap(vs_cb_object_buffer_.Get(), 0);

		vs_cb_object_.Clean();
	}
	context->VSSetConstantBuffers(0, 1, vs_cb_object_buffer_.GetAddressOf());

	if (ps_cb_object_.IsDirty())
	{
		D3D11_MAPPED_SUBRESOURCE pscbObjectMapping;
		PSCBuffer_PerObject_Type* mappedData;

		hr = context->Map(ps_cb_object_buffer_.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0x00, &pscbObjectMapping);
		VALIDATE(hr, "Failed to map PS per-object CBuffer for CPU writing");
		mappedData = (PSCBuffer_PerObject_Type*)pscbObjectMapping.pData;
		mappedData->ObjectMaterial = ps_cb_object_.Get().ObjectMaterial;
		context->Unmap(ps_cb_object_buffer_.Get(), 0);

		ps_cb_object_.Clean();
	}
	context->PSSetConstantBuffers(0, 1, ps_cb_object_buffer_.GetAddressOf());

	// Perform actual draw calls
	context->DrawIndexed(nVertsToDraw, 0, 0);

	return true;
}

void DebugShader::SetWorldMatrix(const Matrix & world)
{
	vs_cb_object_.Set({ world });
}

void DebugShader::SetViewMatrix(const Matrix & view)
{
	VSCBuffer_PerFrame_Type fbuff = vs_cb_frame_.Get();
	fbuff.View = view.Transpose();
	vs_cb_frame_.Set(fbuff);
}

void DebugShader::SetProjMatrix(const Matrix & proj)
{
	VSCBuffer_PerFrame_Type fbuff = vs_cb_frame_.Get();
	fbuff.Proj = proj.Transpose();
	vs_cb_frame_.Set(fbuff);
}

void DebugShader::SetObjectMaterial(Material material)
{
	ps_cb_object_.Get().ObjectMaterial = material;
	ps_cb_object_.Dirty();
}
