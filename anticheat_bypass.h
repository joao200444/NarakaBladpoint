#pragma once
#include <Windows.h>
#include <winternl.h>
#include <Psapi.h>

// Função para remover entrada da lista (implementação manual)
inline void RemoveEntryList(PLIST_ENTRY Entry) {
    PLIST_ENTRY Blink = Entry->Blink;
    PLIST_ENTRY Flink = Entry->Flink;
    Blink->Flink = Flink;
    Flink->Blink = Blink;
}

// Estruturas customizadas para bypass (evitando conflito com winternl.h)
typedef struct _CUSTOM_LDR_DATA_TABLE_ENTRY {
    LIST_ENTRY InLoadOrderLinks;
    LIST_ENTRY InMemoryOrderLinks;
    LIST_ENTRY InInitializationOrderLinks;
    PVOID DllBase;
    PVOID EntryPoint;
    ULONG SizeOfImage;
    UNICODE_STRING FullDllName;
    UNICODE_STRING BaseDllName;
} CUSTOM_LDR_DATA_TABLE_ENTRY, *PCUSTOM_LDR_DATA_TABLE_ENTRY;

// Funções de bypass
inline void HideDLLFromPEB(HMODULE hMod) {
    try {
        PPEB peb = (PPEB)__readgsqword(0x60);
        if (!peb || !peb->Ldr) return;
        
        PEB_LDR_DATA* ldr = peb->Ldr;
        
        PLIST_ENTRY current = ldr->InMemoryOrderModuleList.Flink;
        while (current && current != &ldr->InMemoryOrderModuleList) {
            PCUSTOM_LDR_DATA_TABLE_ENTRY entry = CONTAINING_RECORD(current, CUSTOM_LDR_DATA_TABLE_ENTRY, InMemoryOrderLinks);
            if (entry && entry->DllBase == hMod) {
                // Remover da lista de módulos carregados
                RemoveEntryList(&entry->InLoadOrderLinks);
                RemoveEntryList(&entry->InMemoryOrderLinks);
                RemoveEntryList(&entry->InInitializationOrderLinks);
                break;
            }
            current = current->Flink;
        }
    } catch (...) {
        // Ignorar erros - bypass opcional
    }
}

inline void AntiDebugBypass() {
    try {
        PPEB peb = (PPEB)__readgsqword(0x60);
        if (peb) {
            peb->BeingDebugged = FALSE;
        }
        
        // Patch algumas funções de detecção
        HMODULE ntdll = GetModuleHandleA("ntdll.dll");
        if (ntdll) {
            // Patch NtQueryInformationProcess
            FARPROC pNtQuery = GetProcAddress(ntdll, "NtQueryInformationProcess");
            if (pNtQuery) {
                DWORD oldProtect;
                if (VirtualProtect(pNtQuery, 1, PAGE_EXECUTE_READWRITE, &oldProtect)) {
                    *(BYTE*)pNtQuery = 0xC3; // RET
                    VirtualProtect(pNtQuery, 1, oldProtect, &oldProtect);
                }
            }
            
            // Patch NtSetInformationThread
            FARPROC pNtSetInfo = GetProcAddress(ntdll, "NtSetInformationThread");
            if (pNtSetInfo) {
                DWORD oldProtect;
                if (VirtualProtect(pNtSetInfo, 1, PAGE_EXECUTE_READWRITE, &oldProtect)) {
                    *(BYTE*)pNtSetInfo = 0xC3; // RET
                    VirtualProtect(pNtSetInfo, 1, oldProtect, &oldProtect);
                }
            }
        }
    } catch (...) {
        // Ignorar erros - bypass opcional
    }
}

inline void DelayedInitialization() {
    // Aguardar o jogo carregar completamente
    DWORD startTime = GetTickCount();
    while (GetTickCount() - startTime < 15000) { // 15 segundos
        Sleep(1000);
        
        // Verificar se o processo principal está estável
        PROCESS_MEMORY_COUNTERS pmc;
        if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
            // Se a memória estabilizou acima de 500MB, o jogo provavelmente carregou
            if (pmc.WorkingSetSize > 500 * 1024 * 1024) {
                Sleep(3000); // Aguardar mais um pouco
                break;
            }
        }
    }
}