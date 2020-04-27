#pragma once

#pragma once

#include <d3d9.h>
#include <d3dx9.h>
#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")
#include <dwmapi.h>
#pragma comment(lib, "dwmapi.lib")

#include <iostream>
#include<Windows.h>

using namespace std;

static MARGINS Margin;
static LPDIRECT3D9              g_pD3D = NULL;
static LPDIRECT3DDEVICE9        g_pd3dDevice = NULL;
static D3DPRESENT_PARAMETERS    g_d3dpp = {};
static ID3DXLine* pLine = 0;
static ID3DXFont* Font;

static HWND 辅助窗口句柄, GameHwnd;
static RECT 窗口矩形;
static int 窗口宽, 窗口高;

//注册窗口需要用到的窗口类
static WNDCLASSEX wClass;


//画矩形，文字之类的单独放在这个函数里
typedef void(*Draw)();
static Draw Render;


//窗口消息处理函数
LRESULT WinProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam);

bool 初始化D3D();

void 创建透明窗口(HWND 游戏窗口句柄, Draw 绘制函数);

void 窗口消息循环();

void 画线(D3DCOLOR Color, float X1, float Y1, float X2, float Y2, float Width);

void 绘制文字(float X, float Y, const char* Str, D3DCOLOR Color);

void 画框(float X, float Y, float W, float H, float Width, D3DCOLOR Color);

void 绘制开始();

void 绘制结束();





