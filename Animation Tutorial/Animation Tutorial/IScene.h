#pragma once

#include <future>
#include <memory>

#include <d3d11.h>
#include <wrl.h>
using Microsoft::WRL::ComPtr;

class IScene
{
public:
	IScene(HWND hWnd, ComPtr<ID3D11Device> device, ComPtr<ID3D11DeviceContext> context)
		: hWnd_(hWnd)
		, device_(device)
		, context_(context)
	{}

	virtual std::future<bool> LoadScene() = 0;
	virtual std::shared_ptr<IScene> NextScene() = 0;
	virtual std::future<bool> UnloadScene() = 0;
	virtual bool Update(float dt) = 0;
	virtual bool Render() = 0;
	virtual LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) = 0;

protected:
	HWND hWnd_;
	ComPtr<ID3D11Device> device_;
	ComPtr<ID3D11DeviceContext> context_;
};