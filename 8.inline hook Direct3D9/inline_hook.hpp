#pragma once
#include <Windows.h>

constexpr int byte_length = 5;

class inline_hook
{
private:
	using uchar = unsigned char;
	using dword = DWORD;

	uchar m_original_byte[byte_length];//原始函数处的汇编指令
	uchar m_self_byte[byte_length];//我们构造的汇编指令

	int m_original_address;//原始函数的地址
	int m_self_address;//我们函数的地址

	dword motify_memory_attributes(int address, dword attributes = PAGE_EXECUTE_READWRITE)
	{
		dword old_attributes;
		VirtualProtect(reinterpret_cast<void*>(address), byte_length, attributes, &old_attributes);
		return old_attributes;
	}

public:
	inline_hook(int original_address, int self_address) : m_original_address(original_address), m_self_address(self_address)
	{
		
		m_self_byte[0] = '\xe9';

		//计算偏移
		int offset = self_address - (original_address + byte_length);

		//构造跳转到我们函数的byte
		memcpy(&m_self_byte[1], &offset, byte_length - 1);

		//修改内存属性
		dword attributes = motify_memory_attributes(original_address);

		//保存原始函数地址的byte
		memcpy(m_original_byte, reinterpret_cast<void*>(original_address), byte_length);

		//恢复内存属性
		motify_memory_attributes(original_address, attributes); 
	}

	void motify_address()
	{
		dword attributes = motify_memory_attributes(m_original_address);

		//写入我们构造的byte实现inline hook
		memcpy(reinterpret_cast<void*>(m_original_address), m_self_byte, byte_length);

		motify_memory_attributes(m_original_address, attributes);
	}

	void restore_address()
	{
		dword attributes = motify_memory_attributes(m_original_address);

		memcpy(reinterpret_cast<void*>(m_original_address), m_original_byte, byte_length);

		motify_memory_attributes(m_original_address, attributes);
	}

};


