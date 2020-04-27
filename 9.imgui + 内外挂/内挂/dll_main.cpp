#include "do_cheats.hpp"
#include <process.h>

int __stdcall DllMain(void* _DllHandle, unsigned long _Reason, void* _Reserved)
{
	if (_Reason == DLL_PROCESS_ATTACH) _beginthreadex(nullptr, 0, initialize_d3d9, nullptr, 0, nullptr);
	if (_Reason == DLL_PROCESS_DETACH) un_load();
	return 1;
}