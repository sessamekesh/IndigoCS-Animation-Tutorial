#include "DemoApp.h"
#include "OffBrandChewy.h"
#include <cinttypes>
#include <cfloat>
#include <cassert>
#include "Logger.h"

namespace
{

static const LPCWSTR APP_NAME = L"JuiceBox";

HINSTANCE g_hInst = nullptr;
HWND g_hWnd = nullptr;

static const std::uint32_t WINDOW_X = 100;
static const std::uint32_t WINDOW_Y = 100;
static const std::uint32_t WINDOW_WIDTH = 1920;
static const std::uint32_t WINDOW_HEIGHT = 1080;

std::shared_ptr<IScene> g_activeScene = nullptr;
std::shared_ptr<IScene> g_nextScene = nullptr;
std::future<bool> g_nextSceneLoaded;

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if ((msg == WM_KEYDOWN && wParam == VK_ESCAPE) || msg == WM_QUIT || msg == WM_DESTROY)
	{
		PostQuitMessage(0);
		return 0;
	}

	if (g_activeScene)
	{
		return g_activeScene->WndProc(hWnd, msg, wParam, lParam);
	}
	else
	{
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}
}

};

void Run(HINSTANCE hInst)
{
	g_hInst = hInst;

	WNDCLASSEX wc = { 0 };
	wc.lpfnWndProc = WndProc;
	wc.cbSize = sizeof(wc);
	wc.hIcon = LoadIcon(hInst, IDI_APPLICATION);
	wc.hIconSm = wc.hIcon;
	wc.hInstance = hInst;
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpszClassName = APP_NAME;

	Logger::Log("Registering window class");

	RegisterClassEx(&wc);

	Logger::Log("Creating window");
	g_hWnd = CreateWindow(APP_NAME, L"D3D11 Demo", WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME ^ WS_MAXIMIZEBOX ^ WS_MINIMIZEBOX, WINDOW_X, WINDOW_Y, WINDOW_WIDTH, WINDOW_HEIGHT, nullptr, nullptr, hInst, nullptr);
	assert(g_hWnd);
	
	ShowWindow(g_hWnd, SW_SHOWDEFAULT); // screw it

	ComPtr<ID3D11Device> device;
	ComPtr<ID3D11DeviceContext> context;

	Logger::Log("Creating D3D Device");
	UINT creationFlags = 0x00;

#if defined(DEBUG) | defined(_DEBUG)
	//creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_FEATURE_LEVEL fls = D3D_FEATURE_LEVEL_11_0;
	HRESULT hr = D3D11CreateDevice(
		nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, creationFlags,
		&fls, 1u, D3D11_SDK_VERSION, &device, nullptr, &context);
	if (FAILED(hr))
	{
		Logger::Log("Could not create D3D device!");
		return;
	}

	Logger::Log("Loading Off Brand Chewy Scene");
	g_activeScene = std::make_shared<OffBrandChewy>(g_hWnd, device, context);
	if (g_activeScene->LoadScene().get())
	{
		Logger::Log("Loading finished. Beginning main render loop.");
	}
	else
	{
		Logger::Log("Failed to load Off Brand Chewy Scene!");
		return;
	}

	MSG msg = { 0 };
	std::chrono::high_resolution_clock::time_point lastFrame = std::chrono::high_resolution_clock::now();
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, nullptr, 0x00, 0x00, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		std::shared_ptr<IScene> ns = g_activeScene->NextScene();
		if (g_nextScene && g_nextScene != ns)
		{
			Logger::Log("Next scene change. Unloading previous next scene");
			if (g_nextScene->UnloadScene().get())
			{
				g_nextScene = nullptr;
			}
			else
			{
				break; // Failed to unload previous scene
			}
		}

		if (ns && !g_nextScene)
		{
			g_nextScene = ns;
			g_nextSceneLoaded = g_nextScene->LoadScene();
		}

		if (ns && ns == g_nextScene)
		{
			if (g_nextSceneLoaded.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
			{
				if (g_nextSceneLoaded.get())
				{
					g_activeScene = g_nextScene;
					g_nextScene = nullptr;
				}
				else
				{
					break;
				}
			}
		}

		std::chrono::high_resolution_clock::time_point thisFrame = std::chrono::high_resolution_clock::now();
		float dt = std::chrono::duration_cast<std::chrono::microseconds>(thisFrame - lastFrame).count() / 1000000.f;
		lastFrame = thisFrame;

		if (!g_activeScene->Update(dt)) break;
		if (!g_activeScene->Render()) break;
	}

	Logger::Log("Finished app! Quitting...");

	DestroyWindow(g_hWnd);
	UnregisterClass(APP_NAME, hInst);
}