#pragma once
#include <windef.h>

typedef struct _COMMAND
{
	DWORD magic;
	DWORD type;
	DWORD pid;
	ULONG64 address;
	PVOID buffer;
	SIZE_T size;
} COMMAND, * PCOMMAND;