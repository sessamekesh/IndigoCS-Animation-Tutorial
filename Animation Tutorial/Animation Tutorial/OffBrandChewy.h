#pragma once

#include "IScene.h"
#include "SceneGraph.h"
#include <dxgi1_4.h>
#include <vector>
#include "IKeyEventListener.h"
#include "DebugShader.h"
#include "DebugCamera.h"
#include "BasicShaderMD.h"
#include "ShaderPNS4_MD1.h"

#include "RoadBaseModel.h"

class OffBrandChewy : public IScene
{
public:
	OffBrandChewy(HWND hWnd, ComPtr<ID3D11Device> device, ComPtr<ID3D11DeviceContext> context)
		: IScene(hWnd, device, context)
		, swapChain_(nullptr)
		, renderTargetView_(nullptr)
		, depthStencilBuffer_(nullptr)
		, depthStencilState_(nullptr)
		, rasterState_(nullptr)
		, depthStencilView_(nullptr)
		, viewport_()
		, sceneGraph_()
		, projMatrix_(PerspectiveLH(Radians(90), 1920.f / 1080.f, 0.1f, 200.f))
		, camera_(nullptr)
		, debugShader_(nullptr)
		, keyListeners_(0)
	{}

	virtual std::future<bool> LoadScene() override;
	virtual std::shared_ptr<IScene> NextScene() override;
	virtual std::future<bool> UnloadScene() override;
	virtual bool Update(float dt) override;
	virtual bool Render() override;
	virtual LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) override;

// Rendering
protected:
	ComPtr<IDXGISwapChain1> swapChain_;
	ComPtr<ID3D11RenderTargetView> renderTargetView_;
	ComPtr<ID3D11Texture2D> depthStencilBuffer_;
	ComPtr<ID3D11DepthStencilState> depthStencilState_;
	ComPtr<ID3D11RasterizerState> rasterState_;
	ComPtr<ID3D11DepthStencilView> depthStencilView_;

	D3D11_VIEWPORT viewport_;

// Scene
protected:
	SceneGraph sceneGraph_;
	Dirtyable<Matrix> projMatrix_;
	std::shared_ptr<DebugCamera> camera_;

protected:
	std::shared_ptr<DebugShader> debugShader_;
	std::shared_ptr<BasicShaderMD> basicMDShader_;
	std::shared_ptr<ShaderPNS4_MD1> shaderPNS4MD1_;

// Logical
protected:
	std::vector<std::shared_ptr<IKeyEventListener>> keyListeners_;

private:
	bool InitD3D();
	bool InitScene();
};