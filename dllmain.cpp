#include <Windows.h>
#include <cstdio>
#include <Psapi.h>
#include "Kiro/kiero.h"
#include "hookframe.h"
#include "auth.h"
#include "anticheat_bypass.h"
#include "console_window.h"
#include "noclip.h"
#include "autospin.h"
#include "autoparry.h"
#include "features/Radar.hpp"

#pragma comment(lib, "Psapi.lib")




#pragma message("Compiler version: " _MSC_FULL_VER)
void RunIl2cppScan(HMODULE module);

HMODULE g_module = nullptr;



volatile bool g_consoleActivationRequested = false;

DWORD WINAPI ConsoleInputThread(LPVOID)
{
    printf("[INFO] Type 'y' and press Enter to initialize ImGui...\n");
    while (true) {
        char buffer[10];
        if (fgets(buffer, sizeof(buffer), stdin)) {
            if (buffer[0] == 'y' || buffer[0] == 'Y') {
                printf("[DEBUG] 'y' received! Requesting ImGui activation...\n");
                g_consoleActivationRequested = true;
                break;
            }
        }
        Sleep(100);
    }
    return 0;
}

DWORD WINAPI MainThread(LPVOID lpReserved)
{
    printf("[DEBUG] MainThread started\n");
    
    // Iniciar thread de leitura do console
    CreateThread(nullptr, 0, ConsoleInputThread, nullptr, 0, nullptr);
    
    // BYPASS ANTICHEAT - Aguardar carregamento completo do jogo (15-20 segundos)
    printf("[DEBUG] Waiting 15 seconds for game to load...\n");
    Sleep(15000);
    
    printf("[DEBUG] Applying anti-debug bypass...\n");
    // BYPASS ANTICHEAT - Anti-debug simples
    try {
        PPEB peb = (PPEB)__readgsqword(0x60);
        if (peb) {
            peb->BeingDebugged = FALSE;
        }
    } catch (...) {
        // Ignorar erros
    }
    
    // ============================================
    // SISTEMA DE AUTENTICAÇÃO KEYAUTH - ATIVADO
    // ============================================
    // A inicialização será feita quando F11 for pressionado
    // A validação será feita via interface ImGui
    // O menu principal só aparecerá após autenticação bem-sucedida

    // Aguardar mais um pouco antes de inicializar hooks
    printf("[DEBUG] Waiting 3 more seconds...\n");
    Sleep(3000);

    printf("[DEBUG] Initializing Kiero hooks (Forcing DX11)...\n");
    bool init_hook = false;
    int attempts = 0;
    do
    {
        // Forçar D3D11 conforme solicitado pelo usuário
        if (kiero::init(kiero::RenderType::D3D11) == kiero::Status::Success)
        {
            printf("[DEBUG] Kiero (DX11) initialized successfully!\n");
            kiero::bind(8, (void**)&oPresent, hkPresent);
            init_hook = true;
        }
        Sleep(500);
        attempts++;
        if (attempts > 20) {
            printf("[DEBUG] Failed to initialize Kiero after 20 attempts\n");
            break;
        }
    } while (!init_hook);

    if (!init_hook) {
        printf("[DEBUG] Hook initialization failed - exiting\n");
        return TRUE;
    }

    // IL2CPP SCAN DESATIVADO - usando RVAs diretos
    printf("[DEBUG] IL2CPP scan disabled - using direct RVA offsets\n");
    // RunIl2cppScan(static_cast<HMODULE>(lpReserved));
    
    // Inicializar NoClip e AutoSpin
    printf("[DEBUG] Initializing NoClip and AutoSpin...\n");
    noclip_init();
    autospin_init();
    printf("[DEBUG] NoClip and AutoSpin initialized successfully\n");
    
    // Inicializar Auto-Parry
    printf("[DEBUG] Initializing Auto-Parry...\n");
    autoparry_init();
    printf("[DEBUG] Auto-Parry initialized successfully\n");
    
    // Inicializar Radar
    printf("[DEBUG] Initializing Radar...\n");
    Features::Radar::Initialize();
    printf("[DEBUG] Radar initialized successfully\n");
    
    printf("[DEBUG] MainThread completed\n");
    return TRUE;
}



BOOL WINAPI DllMain(HMODULE hMod, DWORD dwReason, LPVOID lpReserved)
{
    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hMod);

        // BYPASS ANTICHEAT - Aguardar antes de inicializar
        // (Ocultação de DLL desabilitada para evitar problemas)

    // Console HABILITADO para debug
    AllocConsole();
    #if defined(_MSC_VER)
    FILE* f;
    freopen_s(&f, "CONOUT$", "w", stdout);
    freopen_s(&f, "CONOUT$", "w", stderr);
    freopen_s(&f, "CONIN$",  "r", stdin);
    #else
    freopen("CONOUT$", "w", stdout);
    freopen("CONOUT$", "w", stderr);
    freopen("CONIN$",  "r", stdin);
    #endif
    
    printf("[DEBUG] Console initialized\n");

    CreateThread(nullptr, 0, MainThread, hMod, 0, nullptr);
        g_module = hMod;
        break;
    case DLL_PROCESS_DETACH:
        ConsoleWindow::Get().Shutdown();
        kiero::shutdown();
        break;
    }
    return TRUE;
}