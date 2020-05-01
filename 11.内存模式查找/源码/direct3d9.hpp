#pragma once
#include "help_func.hpp"

#include <d3d9.h>
#pragma comment(lib,"d3d9.lib")

//如果这个报错请添加Direct3D9的头文件目录和库文件目录
#include <d3dx9.h>
#pragma comment(lib,"d3dx9.lib")

#include <dwmapi.h>
#pragma comment(lib, "dwmapi.lib")

MARGINS g_MARGINS;
IDirect3D9* g_IDirect3D9 = nullptr;
IDirect3DDevice9* g_IDirect3DDevice9 = nullptr;
D3DPRESENT_PARAMETERS g_present;
ID3DXLine* g_ID3DXLine = nullptr;
ID3DXFont* g_ID3DXFont = nullptr;

//操作函数
typedef void(*cheats_function)(void);
cheats_function g_cheating = nullptr;

//初始化diretc3d9设备
void initialize_direct3d9(hwnd hWnd)
{
	g_IDirect3D9 = Direct3DCreate9(D3D_SDK_VERSION);
	error(g_IDirect3D9, "Direct3DCreate9失败");

	ZeroMemory(&g_present, sizeof(g_present));
	g_present.Windowed = TRUE;
	g_present.SwapEffect = D3DSWAPEFFECT_DISCARD;
	g_present.BackBufferFormat = D3DFMT_UNKNOWN;
	g_present.EnableAutoDepthStencil = TRUE;
	g_present.AutoDepthStencilFormat = D3DFMT_D16;
	g_present.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
	hresult result = g_IDirect3D9->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &g_present, &g_IDirect3DDevice9);
	error(result == S_OK, "CreateDevice失败");

	D3DXCreateLine(g_IDirect3DDevice9, &g_ID3DXLine);
	D3DXCreateFontA(g_IDirect3DDevice9, 20, 0, FW_DONTCARE, D3DX_DEFAULT, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, FF_DONTCARE, "Arial", &g_ID3DXFont);//Arial Vernada
}

//绘制线段
void render_line(D3DCOLOR color, float left, float top, float right, float down, float line_size = 1.0f)
{
	D3DXVECTOR2 vextor[2]{ {left,top},{right,down} };
	g_ID3DXLine->SetWidth(line_size);
	g_ID3DXLine->Draw(vextor, 2, color);
}

//绘制文本
void render_text(D3DCOLOR color, long x, long y, const char* text)
{
	RECT rect{ x,y };
	g_ID3DXFont->DrawTextA(nullptr, text, -1, &rect, DT_CALCRECT, color);
	g_ID3DXFont->DrawTextA(nullptr, text, -1, &rect, DT_LEFT, color);
}

//绘制矩形
void render_rect(D3DCOLOR color, float x, float y, float width, float height, float rect_size = 1.0f)
{
	D3DXVECTOR2 vextor[5]{ {x,y},{x + width,y},{x + width,y + height},{x,y + height},{x,y} };
	g_ID3DXLine->SetWidth(rect_size);
	g_ID3DXLine->Draw(vextor, 5, color);
}

//绘制管理器
void render_manager()
{
	if (g_IDirect3DDevice9)
	{
		g_IDirect3DDevice9->Clear(0, 0, D3DCLEAR_TARGET, 0, 1.0f, 0);
		if (SUCCEEDED(g_IDirect3DDevice9->BeginScene()))
		{
			if (g_cheating) g_cheating();
			//render_line(D3DCOLOR_ARGB(255, 0, 255, 255), 100,100, 200, 0);
			//render_text(D3DCOLOR_ARGB(255, 0, 255, 255), 0,0, "我就测试一下");
			//render_rect(D3DCOLOR_ARGB(255, 0, 255, 255), 200, 200, 100, 100);

			g_IDirect3DDevice9->EndScene();
		}
		g_IDirect3DDevice9->Present(0, 0, 0, 0);
	}
}

//清理
void clear_device()
{
	if (g_ID3DXLine) g_ID3DXLine->Release();
	if (g_ID3DXFont) g_ID3DXFont->Release();
	if (g_IDirect3D9) g_IDirect3D9->Release();
	if (g_IDirect3DDevice9) g_IDirect3DDevice9->Release();
	g_ID3DXLine = nullptr;
	g_ID3DXFont = nullptr;
	g_IDirect3D9 = nullptr;
	g_IDirect3DDevice9 = nullptr;
}

//窗口过程
hresult __stdcall window_proc(hwnd hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_CREATE:
		DwmExtendFrameIntoClientArea(hWnd, &g_MARGINS);
		break;
	case WM_PAINT:
		render_manager();
		break;
	case WM_CLOSE:
		clear_device();
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProcA(hWnd, uMsg, wParam, lParam);
	}
	return 1;
}

//获取窗口位置大小
void get_window_size(hwnd target, int& x, int& y, int& width, int& height)
{
	RECT rect;
	GetWindowRect(target, &rect);
	x = rect.left;
	y = rect.top;
	width = rect.right - rect.left;
	height = rect.bottom - rect.top;
	if (GetWindowLongA(target, GWL_STYLE) & WS_CAPTION)
	{
		x += 8;
		width -= 8;
		y += 30;
		height -= 30;
	}
}

//创建透明窗口
hwnd create_transparent_window(hwnd game_hwnd)
{
	const char* window_name = "transparent";

	WNDCLASSEXA window_class;
	ZeroMemory(&window_class, sizeof(window_class));
	window_class.cbSize = sizeof(window_class);
	window_class.hCursor = LoadCursor(0, IDC_ARROW);
	window_class.hInstance = GetModuleHandle(NULL);
	window_class.lpfnWndProc = window_proc;
	window_class.lpszClassName = window_name;
	window_class.style = CS_VREDRAW | CS_HREDRAW;
	hresult result = RegisterClassExA(&window_class);
	error(result, "RegisterClassExA失败");

	int x, y, width, height;
	get_window_size(game_hwnd, x, y, width, height);
	hwnd transparent_hwnd = CreateWindowExA(WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED,
		window_name, window_name, WS_POPUP, x, y, width, height, NULL, NULL, GetModuleHandle(NULL), NULL);
	error(transparent_hwnd, "CreateWindowExA失败");

	SetLayeredWindowAttributes(transparent_hwnd, 0, RGB(0, 0, 0), LWA_COLORKEY);
	UpdateWindow(transparent_hwnd);
	ShowWindow(transparent_hwnd, SW_SHOW);

	return transparent_hwnd;
}

//处理消息
void message_handle(hwnd game_hwnd, hwnd transparent_hwnd)
{
	MSG msg;
	ZeroMemory(&msg, sizeof(msg));
	while (msg.message != WM_QUIT)
	{
		if (PeekMessageA(&msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessageA(&msg);
		}

		int x, y, width, height;
		get_window_size(game_hwnd, x, y, width, height);
		MoveWindow(transparent_hwnd, x, y, width, height, TRUE);
	}
	clear_device();
	return;
}







