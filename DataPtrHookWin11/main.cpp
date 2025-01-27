#include "main.h"

void* get_system_information(SYSTEM_INFORMATION_CLASS information_class)
{
    unsigned long size = 32;
    char buffer[32];

    ZwQuerySystemInformation(information_class, buffer, size, &size);

    void* info = ExAllocatePoolZero(NonPagedPool, size, 'fuck');
    if (!info)
        return nullptr;

    if (!NT_SUCCESS(ZwQuerySystemInformation(information_class, info, size, &size))) {
        ExFreePool(info);
        return nullptr;
    }

    return info;
}

uintptr_t get_kernel_module(const char* name)
{
    const auto to_lower = [](char* string) -> const char* {
        for (char* pointer = string; *pointer != '\0'; ++pointer) {
            *pointer = (char)(short)tolower(*pointer);
        }

        return string;
        };

    const PRTL_PROCESS_MODULES info = (PRTL_PROCESS_MODULES)get_system_information(SystemModuleInformation);

    if (!info)
        return NULL;

    for (size_t i = 0; i < info->NumberOfModules; ++i) {
        const auto& mod = info->Modules[i];

        if (strcmp(to_lower((char*)mod.FullPathName + mod.OffsetToFileName), name) == 0) {
            const void* address = mod.ImageBase;
            ExFreePool(info);
            return (uintptr_t)address;
        }
    }

    ExFreePool(info);
    return NULL;
}

NTSTATUS find_process(char* process_name, PEPROCESS* process)
{
    PEPROCESS ppEprocess = NULL;
    int pid_index = 0;
    NTSTATUS status = STATUS_UNSUCCESSFUL;
    for (pid_index = 0; pid_index < 30000; pid_index += 4)
    {
        status = PsLookupProcessByProcessId((HANDLE)pid_index, &ppEprocess);
        if (NT_SUCCESS(status))
        {
            auto name = PsGetProcessImageFileName(ppEprocess);
            if (strstr(process_name, name))
            {
                *process = ppEprocess;
                ObDereferenceObject(ppEprocess);
                return STATUS_SUCCESS;
            }
        }
        if (ppEprocess != NULL)
        {
            ObDereferenceObject(ppEprocess);
            ppEprocess = NULL;
        }
    }
    return STATUS_NOT_FOUND;
}

uintptr_t pattern_scan(uintptr_t base, size_t range, const char* pattern, const char* mask)
{
    const auto check_mask = [](const char* base, const char* pattern, const char* mask) -> bool {
        for (; *mask; ++base, ++pattern, ++mask) {
            if (*mask == 'x' && *base != *pattern)
                return false;
        }

        return true;
        };

    range = range - strlen(mask);

    for (size_t i = 0; i < range; ++i) {
        if (check_mask((const char*)base + i, pattern, mask))
            return base + i;
    }

    return NULL;
}

uintptr_t pattern_scan(uintptr_t base, const char* pattern, const char* mask)
{
    const PIMAGE_NT_HEADERS headers = (PIMAGE_NT_HEADERS)(base + ((PIMAGE_DOS_HEADER)base)->e_lfanew);
    const PIMAGE_SECTION_HEADER sections = IMAGE_FIRST_SECTION(headers);

    for (size_t i = 0; i < headers->FileHeader.NumberOfSections; i++) {
        const PIMAGE_SECTION_HEADER section = &sections[i];

        if (section->Characteristics & IMAGE_SCN_MEM_EXECUTE) {
            const uintptr_t match = pattern_scan(base + section->VirtualAddress, section->Misc.VirtualSize, pattern, mask);
            if (match)
                return match;
        }
    }

    return 0;
}

BOOLEAN is_valid(ULONG64 address)
{
    return MmIsAddressValid(PVOID(address));
}

__int64 __fastcall hkNtUserSetGestureConfig(void* a1)
{
    PCOMMAND cmd = (PCOMMAND)a1;
    if (!MmIsAddressValid(cmd) || cmd->magic != 0x233)
        return oNtUserSetGestureConfig(a1);

    switch (cmd->type)
    {
    case 1:
        return 0x666;
    case 2:
    {
        PEPROCESS process;
        if (!NT_SUCCESS(PsLookupProcessByProcessId((HANDLE)cmd->pid, &process)))
            return 0;
        ObDereferenceObject(process);
        return (__int64)PsGetProcessSectionBaseAddress(process);
    }
    case 3:
    {
        PEPROCESS process;
        if (!NT_SUCCESS(PsLookupProcessByProcessId((HANDLE)cmd->pid, &process)))
            return 0;
        ObDereferenceObject(process);
        SIZE_T read;
        return MmCopyVirtualMemory(process, (PVOID)cmd->address, IoGetCurrentProcess(), cmd->buffer, cmd->size, KernelMode, &read);
    }
    case 4:
    {
        PEPROCESS process;
        if (!NT_SUCCESS(PsLookupProcessByProcessId((HANDLE)cmd->pid, &process)))
            return 0;
        ObDereferenceObject(process);
        SIZE_T write;
        MmCopyVirtualMemory(IoGetCurrentProcess(), cmd->buffer, process, (PVOID)cmd->address, cmd->size, KernelMode, &write);
    }
    default:
        break;
    }
    return 0;
}

extern "C" NTSTATUS DriverEntry()
{
    ULONG64 gSessionGlobalSlots;
    NTSTATUS status = STATUS_UNSUCCESSFUL;
    const uintptr_t win32k = get_kernel_module("win32k.sys");
    dbg("win32k %llx\n", win32k)
    if (win32k) {
        gSessionGlobalSlots = pattern_scan(win32k, "\xE8\x00\x00\x00\x00\x8A\xD3", "x????xx");
        dbg("gSessionGlobalSlots %llx\n", gSessionGlobalSlots)
    }
    else {
        return STATUS_UNSUCCESSFUL;
    }

    if (!is_valid(gSessionGlobalSlots))
    {
        return STATUS_UNSUCCESSFUL;
    }

    PEPROCESS process_target{};
    KAPC_STATE apc{};

    if (find_process("explorer.exe", &process_target) == STATUS_SUCCESS && process_target)
    {
        KeStackAttachProcess(process_target, &apc);

        do
        {
            gSessionGlobalSlots = gSessionGlobalSlots + *(int*)(gSessionGlobalSlots + 1) + 5;
            dbg("gSessionGlobalSlots %llx\n", gSessionGlobalSlots)
                if (!is_valid(gSessionGlobalSlots))
                {
                    status = STATUS_UNSUCCESSFUL;
                    break;
                }

            gSessionGlobalSlots = gSessionGlobalSlots + 0x14;
            dbg("gSessionGlobalSlots %llx\n", gSessionGlobalSlots)
                if (!is_valid(gSessionGlobalSlots))
                {
                    status = STATUS_UNSUCCESSFUL;
                    break;
                }

            gSessionGlobalSlots = gSessionGlobalSlots + *(int*)(gSessionGlobalSlots + 3) + 7;
            dbg("gSessionGlobalSlots %llx\n", gSessionGlobalSlots)
                if (!is_valid(gSessionGlobalSlots))
                {
                    status = STATUS_UNSUCCESSFUL;
                    break;
                }

            ULONG64 GetSessionState24H2 = *(ULONG64*)(gSessionGlobalSlots);
            dbg("GetSessionState24H2 %llx\n", GetSessionState24H2)
                if (!is_valid(GetSessionState24H2))
                {
                    status = STATUS_UNSUCCESSFUL;
                    break;
                }

            GetSessionState24H2 = *(ULONG64*)(GetSessionState24H2);
            dbg("GetSessionState24H2 %llx\n", GetSessionState24H2)
                if (!is_valid(GetSessionState24H2))
                {
                    status = STATUS_UNSUCCESSFUL;
                    break;
                }

            ULONG64 pointer = *(ULONG64*)(GetSessionState24H2 + 0x88);
            dbg("pointer %llx\n", pointer)
                if (!is_valid(pointer))
                {
                    status = STATUS_UNSUCCESSFUL;
                    break;
                }

            pointer = *(ULONG64*)(pointer + 0x150);
            dbg("pointer %llx\n", pointer)
                if (!is_valid(pointer))
                {
                    status = STATUS_UNSUCCESSFUL;
                    break;
                }
                
            pointer += 0xC30;
            dbg("pointer %llx\n", pointer)

            if (is_valid(pointer))
            {
                *(void**)&oNtUserSetGestureConfig = _InterlockedExchangePointer((void**)pointer, (void*)hkNtUserSetGestureConfig);
                status = STATUS_SUCCESS;
            }

        } while (false);

        KeUnstackDetachProcess(&apc);
    }
    else
    {
        return STATUS_UNSUCCESSFUL;
    }

    return status;
}