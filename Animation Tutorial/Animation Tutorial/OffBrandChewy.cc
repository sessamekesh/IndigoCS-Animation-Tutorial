#include "OffBrandChewy.h"
#include "Logger.h"
#include <sstream>
#include <string>

#include "MixamoCharacter.h"

#define VALIDATE(hr, msg) if (FAILED(hr)) { Logger::Log(msg); return false; }

std::future<bool> OffBrandChewy::LoadScene()
{
	return std::async(std::launch::async, [this] {
		Logger::Log("Initializing D3D in OffBrandChewy scene");
		if (!InitD3D()) return false;

		Logger::Log("Initializing scene objects in OffBrandChewy scene");
		if (!InitScene()) return false;
		
		return true;
	});
}

std::shared_ptr<IScene> OffBrandChewy::NextScene()
{
	return nullptr;
}

std::future<bool> OffBrandChewy::UnloadScene()
{
	return std::async(std::launch::async, [this] {
		return true;
	});
}

bool OffBrandChewy::Update(float dt)
{
	if (!camera_->Update(dt)) return false;
	
	if (!sceneGraph_.Update(dt)) return false;

	return true;
}

bool OffBrandChewy::Render()
{
	// Set pipeline state for scene
	context_->RSSetState(rasterState_.Get());
	context_->OMSetDepthStencilState(depthStencilState_.Get(), 1);
	context_->OMSetRenderTargets(1, renderTargetView_.GetAddressOf(), depthStencilView_.Get());
	context_->RSSetViewports(1, &viewport_);

	// Clear color and depth/stencil buffers
	Color clearColor = Color::Marina;
	float color[4] = { clearColor.r, clearColor.g, clearColor.b, clearColor.a };
	context_->ClearRenderTargetView(renderTargetView_.Get(), color);
	context_->ClearDepthStencilView(depthStencilView_.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0x00);

	// Set world/proj transformations for appropriate shaders
	if (projMatrix_.IsDirty())
	{
		debugShader_->SetProjMatrix(projMatrix_.Get());
		basicMDShader_->SetProjMatrix(projMatrix_.Get());
		shaderPNS4MD1_->SetProjMatrix(projMatrix_.Get());
		projMatrix_.Clean();
	}

	if (camera_->IsDirty())
	{
		debugShader_->SetViewMatrix(camera_->GetViewMatrix());
		basicMDShader_->SetViewMatrix(camera_->GetViewMatrix());
		basicMDShader_->SetCameraPosition(camera_->GetPosition());
		shaderPNS4MD1_->SetViewMatrix(camera_->GetViewMatrix());
		shaderPNS4MD1_->SetCameraPosition(camera_->GetPosition());
		camera_->Clean();
	}

	// Render the scene graph!
	if (!sceneGraph_.Render()) return false;

	swapChain_->Present(1, 0);

	return true;
}

LRESULT CALLBACK OffBrandChewy::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_KEYDOWN:
		for (auto keyListener : keyListeners_)
		{
			keyListener->OnKeyPress(KeyEvent((char)wParam, KEY_EVENT_TYPE::PRESSED));
		}
		return 0;
	case WM_KEYUP:
		for (auto keyListener : keyListeners_)
		{
			keyListener->OnKeyPress(KeyEvent((char)wParam, KEY_EVENT_TYPE::RELEASED));
		}
		return 0;
	default:
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}
}

bool OffBrandChewy::InitD3D()
{
	HRESULT hr = { 0 };

	RECT wndRect = { 0 };
	GetWindowRect(hWnd_, &wndRect);
	int width = wndRect.right - wndRect.left;
	int height = wndRect.bottom - wndRect.top;

	// Get refresh rate information and whatnot
	ComPtr<IDXGIFactory2> factory;
	ComPtr<IDXGIAdapter> adapter;
	ComPtr<IDXGIOutput> output;
	hr = CreateDXGIFactory2(0x00, IID_PPV_ARGS(&factory));
	VALIDATE(hr, "Failed to create DXGI Factory");

	//
	// Create swap chain
	//
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = { 0 };
	swapChainDesc.Width = width;
	swapChainDesc.Height = height;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.Stereo = false;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 2;
	swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	swapChainDesc.Flags = 0x00;

	hr = factory->CreateSwapChainForHwnd(device_.Get(), hWnd_, &swapChainDesc, nullptr, nullptr, &swapChain_);
	VALIDATE(hr, "Failed to create swap chain for window");

	//
	// Create render target view for back buffer
	//
	{
		ComPtr<ID3D11Texture2D> backBuffer;
		hr = swapChain_->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
		VALIDATE(hr, "Failed to get back buffer pointer");

		hr = device_->CreateRenderTargetView(backBuffer.Get(), nullptr, &renderTargetView_);
		VALIDATE(hr, "Failed to create render target view to back buffer");
	}

	//
	// Create depth stencil buffer
	//
	D3D11_TEXTURE2D_DESC depthBufferDesc = { 0 };
	depthBufferDesc.Width = width;
	depthBufferDesc.Height = height;
	depthBufferDesc.MipLevels = 1;
	depthBufferDesc.ArraySize = 1;
	depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthBufferDesc.SampleDesc.Count = 1;
	depthBufferDesc.SampleDesc.Quality = 0;
	depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthBufferDesc.CPUAccessFlags = 0x00;
	depthBufferDesc.MiscFlags = 0x00;

	hr = device_->CreateTexture2D(&depthBufferDesc, nullptr, &depthStencilBuffer_);
	VALIDATE(hr, "Failed to create depth stencil buffer");

	D3D11_DEPTH_STENCIL_DESC depthStencilDesc = { 0 };
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
	depthStencilDesc.StencilEnable = true;
	depthStencilDesc.StencilWriteMask = 0xFFu;
	depthStencilDesc.StencilReadMask = 0xFFu;
	
	depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	hr = device_->CreateDepthStencilState(&depthStencilDesc, &depthStencilState_);
	VALIDATE(hr, "Failed to create depth stencil state");

	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
	depthStencilViewDesc.Flags = 0x00;
	depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;

	hr = device_->CreateDepthStencilView(depthStencilBuffer_.Get(), &depthStencilViewDesc, &depthStencilView_);
	VALIDATE(hr, "Failed to create depth stencil view");

	//
	// Create rasterizer state
	//
	D3D11_RASTERIZER_DESC rDesc;
	ZeroMemory(&rDesc, sizeof(D3D11_RASTERIZER_DESC));
	rDesc.AntialiasedLineEnable = false;
	rDesc.CullMode = D3D11_CULL_BACK;
	rDesc.DepthBias = 0;
	rDesc.DepthBiasClamp = 0.f;
	rDesc.DepthClipEnable = true;
	rDesc.FillMode = D3D11_FILL_SOLID;
	rDesc.FrontCounterClockwise = false;
	rDesc.MultisampleEnable = false;
	rDesc.ScissorEnable = false;
	rDesc.SlopeScaledDepthBias = 0.f;

	hr = device_->CreateRasterizerState(&rDesc, &rasterState_);
	VALIDATE(hr, "Failed to create rasterizer state");

	viewport_.Height = (float)height;
	viewport_.Width = (float)width;
	viewport_.MinDepth = 0.f;
	viewport_.MaxDepth = 1.f;
	viewport_.TopLeftX = 0.f;
	viewport_.TopLeftY = 0.f;

	return true;
}

bool OffBrandChewy::InitScene()
{
	// Camera Details
	camera_ = std::shared_ptr<DebugCamera>(new DebugCamera(Vec3::UnitZ * 1.8f - Vec3::UnitY * 4.f, Vec3::UnitZ * 1.8f + Vec3::UnitY, Vec3::UnitZ));
	camera_->SetMoveSpeed(4.f);
	camera_->SetRotateSpeed(0.95f);
	keyListeners_.push_back(std::shared_ptr<IKeyEventListener>(camera_));

	// Shader creation and initialization
	debugShader_ = std::make_shared<DebugShader>();
	Logger::Log("Initializing debug shader");
	if (!debugShader_->Initialize(device_, "DebugShader.vs.cso", "./DebugShader.ps.cso"))
	{
		Logger::Log("Failed to initialize debug shader");
		return false;
	}

	basicMDShader_ = std::shared_ptr<BasicShaderMD>(new BasicShaderMD());
	Logger::Log("Initializing BasicShaderMD (Material/SingleDirectionalLight)");
	if (!basicMDShader_->Initialize(device_).get())
	{
		Logger::Log("Failed to initialize basic shader");
		return false;
	}
	DirectionalLight light(Color::White * 0.3f, Color::White * 0.99f, Color::White * 2.4f, Vec3(0.34f, 1.f, -0.2f).Normal(), 300.f);
	basicMDShader_->SetDirectionalLight1(light);

	shaderPNS4MD1_ = std::shared_ptr<ShaderPNS4_MD1>(new ShaderPNS4_MD1());
	Logger::Log("Initializing ShaderPNS4_MD1 (Material / Skinning [4 bones] / Normal / SingleDirectionalLight)");
	if (!shaderPNS4MD1_->Initialize(device_).get())
	{
		Logger::Log("Failed to initialize PNS4MD1 shader");
		return false;
	}
	shaderPNS4MD1_->SetDirectionalLight1(light);

	// Object creation
	std::shared_ptr<RoadBaseModel> roadModel = std::shared_ptr<RoadBaseModel>(new RoadBaseModel(basicMDShader_, context_, Transform()));
	std::future<bool> roadModelLoaded = roadModel->Initialize(device_);

	std::shared_ptr<MixamoCharacter> mixamoCharacter = std::shared_ptr<MixamoCharacter>(new MixamoCharacter(shaderPNS4MD1_, context_, Transform(Vec3::Zero, Quaternion(Vec3::UnitX, PI * 0.5f), Vec3(0.015f, 0.015f, 0.015f))));
	std::future<bool> mixamoModelLoaded = mixamoCharacter->Initialize(device_);

	// TODO KAM: Continue to load other items asynchronously here

	if (!roadModelLoaded.get())
	{
		Logger::Log("Failed to load all models, exiting");
		return false;
	}
	sceneGraph_.AddSceneNode("RoadModel", roadModel);

	if (!mixamoModelLoaded.get())
	{
		Logger::Log("Failed to load mixamo character, exiting");
		return false;
	}
	sceneGraph_.AddSceneNode("MixamoCharacter", mixamoCharacter);

	return true;
}