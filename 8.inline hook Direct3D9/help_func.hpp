#pragma once
#include <Windows.h>

//检查函数的返回值
void check_error(bool state,const char* str = nullptr)
{
	if (state) return;

	char buffer[1024 * 2];
	wsprintf(buffer, "发生错误 : %s", str);
	MessageBox(nullptr, buffer, nullptr, MB_OK | MB_ICONHAND);
	exit(-1);
}

