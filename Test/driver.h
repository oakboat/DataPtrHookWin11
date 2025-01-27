#pragma once
#include <iostream>
#include <Windows.h>
#include "../DataPtrHookWin11/command.h"
__int64(__fastcall* NtUserSetGestureConfig)(void* a1) = nullptr;

bool init()
{
	LoadLibraryA("user32.dll");
	LoadLibraryA("win32u.dll");

	const HMODULE win32u = GetModuleHandleA("win32u.dll");
	if (!win32u)
		return false;

	*(void**)&NtUserSetGestureConfig = GetProcAddress(win32u, "NtUserSetGestureConfig");
}

bool ping()
{
	COMMAND cmd{};
	cmd.magic = 0x233;
	cmd.type = 1;
	return NtUserSetGestureConfig(&cmd) == 0x666;
}

ULONG64 get_base(DWORD pid)
{
	COMMAND cmd{};
	cmd.magic = 0x233;
	cmd.type = 2;
	cmd.pid = pid;
	return NtUserSetGestureConfig(&cmd);
}

bool read(DWORD pid, ULONG64 address, PVOID buffer, SIZE_T size)
{
	COMMAND cmd{};
	cmd.magic = 0x233;
	cmd.type = 3;
	cmd.pid = pid;
	cmd.address = address;
	cmd.buffer = buffer;
	cmd.size = size;
	return NtUserSetGestureConfig(&cmd);
}

template<typename T>
T read(DWORD pid, ULONG64 address)
{
	T result{};
	read(pid, address, &result, sizeof(T));
	return result;
}

bool write(DWORD pid, ULONG64 address, PVOID buffer, SIZE_T size)
{
	COMMAND cmd{};
	cmd.magic = 0x233;
	cmd.type = 4;
	cmd.pid = pid;
	cmd.address = address;
	cmd.buffer = buffer;
	cmd.size = size;
	return NtUserSetGestureConfig(&cmd);
}

template<typename T>
bool write(DWORD pid, ULONG64 address, T data)
{
	return write(pid, address, &data, sizeof(T));
}
