#pragma once

#define _CRT_SECURE_NO_WARNINGS
#define DEBUG_STRING

#include <Windows.h>
#include <TlHelp32.h>
#include <Psapi.h>

#ifdef DEBUG_STRING
#include <stdio.h>
#endif // DEBUG_STRING

using handle = HANDLE;
using hwnd = HWND;
using dword = DWORD;
using hmodule = HMODULE;
using hresult = HRESULT;

void error(bool state, const char* text = nullptr)
{
	if (state) return;

	char buffer[4000];
	ZeroMemory(buffer, 4000);
	wsprintf(buffer, "发生错误 : %s", text);

#ifdef  DEBUG_STRING
	printf("%s\n", buffer);
#endif

	MessageBox(nullptr, buffer, nullptr, MB_OK);
	exit(-1);
}

void warning(const char* text, bool state = false)
{
	if (state) return;

	char buffer[5000];
	ZeroMemory(buffer, 5000);
	wsprintf(buffer, "警告 : %s", text);

#ifdef DEBUG_STRING
	printf("%s \n",buffer);
#endif // DEBUG_STRING

	MessageBox(nullptr, buffer, nullptr, MB_OK);
}

struct module_information
{
	handle module_handle;
	char module_name[1024];
	char* module_data;
	int module_address;
	int module_size;
	void alloc(int size)
	{
		module_size = size;
		module_data = (char*)VirtualAlloc(nullptr, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
		error(module_data, "申请内存失败");
	}
	void release()
	{
		if (module_data) VirtualFree(module_data, 0, MEM_RELEASE);
		module_data = nullptr;
	}
};

dword read_memory(handle process, int address, void* recv, int size)
{
	dword read_size;
	ReadProcessMemory(process, (LPCVOID)address, recv, size, &read_size);
	return read_size;
}

dword write_memory(handle process,int address,void* data,int size)
{
	dword write_size;
	WriteProcessMemory(process, (LPVOID)address, data, size, &write_size);
	return write_size;
}

void* alloc_memory(int size)
{
	return VirtualAlloc(nullptr, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
}

void free_memory(void* ptr)
{
	if(ptr)VirtualFree(ptr, 0, MEM_RELEASE);
	ptr = nullptr;
}

handle get_process_handle(dword process_id, dword access = PROCESS_ALL_ACCESS)
{
	handle process_handle = OpenProcess(access, FALSE, process_id);
	error(process_handle, "打开进程句柄失败");

#ifdef DEBUG_STRING
	printf("进程号码 : %8x \n", (unsigned int)process_id);
	printf("进程句柄 : %8x \n", (unsigned int)process_handle);
	printf("\n");
#endif // DEBUG_STRING

	return process_handle;
}

dword get_process_id(const char* process_name)
{
	handle snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	error(snap != INVALID_HANDLE_VALUE, "CreateToolhelp32Snapshot失败");

	PROCESSENTRY32 process_info;
	ZeroMemory(&process_info, sizeof(process_info));
	process_info.dwSize = sizeof(process_info);

	char target[1024];
	ZeroMemory(target, 1024);
	strncpy(target, process_name, strlen(process_name));
	_strupr(target);

	BOOL state = Process32First(snap, &process_info);
	while (state)
	{
		if (strncmp(_strupr(process_info.szExeFile), target, strlen(target)) == 0)
		{
#ifdef DEBUG_STRING
			printf("进程名称 : %s \n",process_info.szExeFile);
			printf("进程ID : %d \n", process_info.th32ProcessID);
			printf("\n");
#endif // DEBUG_STRING

			CloseHandle(snap);
			return process_info.th32ProcessID;
		}
		state = Process32Next(snap, &process_info);
	}

	CloseHandle(snap);
	warning("进程查找失败!");
	return 0;
}

void get_module_info(handle process_handle, dword process_id, const char* module_name, struct module_information& info)
{
	handle snap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, process_id);
	error(snap != INVALID_HANDLE_VALUE, "CreateToolhelp32Snapshot失败");

	MODULEENTRY32 module_info;
	ZeroMemory(&module_info, sizeof(module_info));
	module_info.dwSize = sizeof(module_info);

	char target[1024];
	ZeroMemory(target, 1024);
	strncpy(target, module_name, strlen(module_name));
	_strupr(target);

	BOOL state = Module32First(snap, &module_info);
	while (state)
	{
		if (strncmp(_strupr(module_info.szModule), target, strlen(target)) == 0)
		{
			info.module_address = (int)module_info.modBaseAddr;
			info.module_handle = module_info.hModule;
			info.alloc(module_info.modBaseSize);
			ZeroMemory(info.module_name, sizeof(info.module_name));
			strncpy(info.module_name, module_info.szModule, strlen(module_info.szModule));

			dword size = read_memory(process_handle, info.module_address, info.module_data, info.module_size);
			error(size, "读取内存错误");

#ifdef DEBUG_STRING
			printf("模块名称 : %s \n", module_info.szModule);
			printf("模块基址 : %8x \n",(unsigned int)module_info.modBaseAddr);
			printf("模块大小 : %8x \n",module_info.modBaseSize);
			printf("实际读取 : %8x \n", size);
			printf("\n");
#endif // DEBUG_STRING

			CloseHandle(snap);
			return;
		}
		state = Module32Next(snap, &module_info);
	}

	CloseHandle(snap);
	warning("!!!查找不到指定模块!!!");
}

int find_pattern(handle process, struct module_information& module, const char* pattern, int index = 0)
{
	const char* start = module.module_data;
	const int length = strlen(pattern);

	int count = 0;
	for (int i = 0; i < module.module_size; i++)
	{
		if (start[i] == pattern[0] || pattern[0] == '?')
		{
			int j = 1;
			for (; j < length; j++) if (start[i + j] != pattern[j] && pattern[j] != '?') break;
			if (j == length && count++ == index) return module.module_address + i;
		}
	}

	warning("!!!无法匹配到该内存特征!!!");
	return -1;
}



