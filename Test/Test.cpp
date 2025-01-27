#include <iostream>
#include <Windows.h>

__int64(__fastcall* NtUserSetGestureConfig)(void* a1) = nullptr;

int main()
{
	LoadLibraryA("user32.dll");
	LoadLibraryA("win32u.dll");

	const HMODULE win32u = GetModuleHandleA("win32u.dll");
	if (!win32u)
		return false;

	*(void**)&NtUserSetGestureConfig = GetProcAddress(win32u, "NtUserSetGestureConfig");

	std::cout << std::hex << NtUserSetGestureConfig((void*)0x233) << "\n";
	return 0;
}

