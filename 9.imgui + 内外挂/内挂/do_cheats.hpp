#pragma once

#include "help_func.hpp"
#include "inline_hook.hpp"

#include <stdio.h>
#include <d3d9.h>
#pragma comment(lib,"d3d9.lib")

#include "imgui/imgui.h"
#include "imgui/imgui_impl_dx9.h"
#include "imgui/imgui_impl_win32.h"

IDirect3D9* g_direct3d9 = nullptr;
IDirect3DDevice9* g_direct3ddevice9 = nullptr;
D3DPRESENT_PARAMETERS g_present;

inline_hook* g_Reset_hook = nullptr;
inline_hook* g_EndScene_hook = nullptr;
inline_hook* g_DrawIndexedPrimitive_hook = nullptr;

WNDPROC g_original_proc = nullptr;


extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK self_proc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
		return true;

	return CallWindowProcW(g_original_proc, hWnd, uMsg, wParam, lParam);
}

void initialize_imgui(IDirect3DDevice9* direct3ddevice9)
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsLight();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.IniFilename = nullptr;
	io.LogFilename = nullptr;

	ImGui_ImplWin32_Init(FindWindowW(L"Direct3DWindowClass", nullptr));
	ImGui_ImplDX9_Init(direct3ddevice9);


}



HRESULT __stdcall self_Reset(IDirect3DDevice9* direct3ddevice9, D3DPRESENT_PARAMETERS* pPresentationParameters)
{
	g_Reset_hook->restore_address();

	//---
	ImGui_ImplDX9_InvalidateDeviceObjects();
	HRESULT result = direct3ddevice9->Reset(pPresentationParameters);
	ImGui_ImplDX9_CreateDeviceObjects();

	//----

	g_Reset_hook->motify_address();
	return result;
}


HRESULT __stdcall self_EndScene(IDirect3DDevice9* direct3ddevice9)
{
	static bool first_call = true;
	if (first_call)
	{
		first_call = false;
		initialize_imgui(direct3ddevice9);
		g_original_proc = (WNDPROC)SetWindowLongA(FindWindowW(L"Direct3DWindowClass", nullptr), GWL_WNDPROC, (LONG)self_proc);
	}
	g_EndScene_hook->restore_address();
	

	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	ImGui::Begin("new window");
	ImGui::Text("this test windows");
	ImGui::End();

	ImGui::EndFrame();
	ImGui::Render();
	ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());

	HRESULT result = direct3ddevice9->EndScene();
	g_EndScene_hook->motify_address();

	return result;
}

HRESULT __stdcall self_DrawIndexedPrimitive(IDirect3DDevice9* direct3ddevice9,D3DPRIMITIVETYPE type, INT BaseVertexIndex, UINT MinVertexIndex, UINT NumVertices, UINT startIndex, UINT primCount)
{
	return 1;
}

//³õÊ¼»¯D3D9
unsigned int __stdcall initialize_d3d9(void* data)
{
#ifdef _DEBUG
	//AllocConsole();
	//SetConsoleTitleA("test");
	//freopen("CON", "w", stdout);
#endif // DEBUG

	g_direct3d9 = Direct3DCreate9(D3D_SDK_VERSION);
	check_error(g_direct3d9, "Direct3DCreate9Ê§°Ü");

	memset(&g_present, 0, sizeof(g_present));
	g_present.Windowed = TRUE;
	g_present.SwapEffect = D3DSWAPEFFECT_DISCARD;
	g_present.BackBufferFormat = D3DFMT_UNKNOWN;
	g_present.EnableAutoDepthStencil = TRUE;
	g_present.AutoDepthStencilFormat = D3DFMT_D16;

	HRESULT result = g_direct3d9->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, FindWindowW(L"Direct3DWindowClass", nullptr),
		D3DCREATE_SOFTWARE_VERTEXPROCESSING, &g_present, &g_direct3ddevice9);
	check_error(result == 0, "CreateDeviceÊ§°Ü");

	int* direct3d9_table = (int*)*(int*)g_direct3d9;
	int* direct3ddevice9_table = (int*)*(int*)g_direct3ddevice9;

	g_Reset_hook = new inline_hook(direct3ddevice9_table[16],(int)self_Reset);
	g_EndScene_hook = new inline_hook(direct3ddevice9_table[42], (int)self_EndScene);

	g_Reset_hook->motify_address();
	g_EndScene_hook->motify_address();

	return 0;
}

void un_load()
{
	g_Reset_hook->restore_address();
	g_EndScene_hook->restore_address();
}


