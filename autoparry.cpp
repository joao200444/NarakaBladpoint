#include <Windows.h>
#include "autoparry.h"
#include "vec.h"
#include "esp.h"  // Para os RVAs
#include "offsets.h"  // RVAs centralizados
#include "Detours-4.0.1/src/detours.h"  // Para hooks

struct Vector3Struct {
    float x, y, z;
};

// Configurações do auto-parry
static bool g_autoParryEnabled = false;  // Desativado por padrão
static bool g_autoParryDebug = false;
static void* g_localPlayerActor = nullptr;
static bool g_localPlayerCaptured = false;

// Typedefs
typedef void* (__fastcall *GetMainPlayerFunc)(void* self);
typedef void* (__fastcall *GetAliveCharListFunc)(void* self);
typedef void (__fastcall *GetPositionFunc)(Vector3Struct* out, void* transform);
typedef UINT (__stdcall *SendInputFunc)(UINT, LPINPUT, int);  // SendInput do user32.dll

// Estrutura de List<T> do IL2CPP
struct IL2CPPList {
    void* klass;
    void* monitor;
    void* items;
    int size;
    int version;
};

struct IL2CPPArray {
    void* klass;
    void* monitor;
    void* bounds;
    int max_length;
};

// Variáveis globais
static HMODULE g_gameAssembly = nullptr;
static HMODULE g_user32 = nullptr;  // Para SendInput
extern void* g_characterManagerInstance;

// Funções auxiliares
static void EnsureGameAssemblyLoaded()
{
    if (g_gameAssembly)
        return;
    
    g_gameAssembly = GetModuleHandleA("GameAssembly_Super.dll");
}

static void EnsureUser32Loaded()
{
    if (g_user32)
        return;
    
    g_user32 = GetModuleHandleA("user32.dll");
}

// Função para criar INPUT structure para SendInput (equivalente a createKeyInput do JS)
static INPUT create_key_input(WORD scanCode, bool isKeyUp)
{
    INPUT input = {};
    input.type = INPUT_KEYBOARD;
    input.ki.wVk = 0;  // Usar scan code
    input.ki.wScan = scanCode;
    input.ki.dwFlags = KEYEVENTF_SCANCODE;
    if (isKeyUp)
        input.ki.dwFlags |= KEYEVENTF_KEYUP;
    input.ki.time = 0;
    input.ki.dwExtraInfo = 0;
    return input;
}

// Função para clicar L via SendInput (equivalente a autoParry do JS)
static void press_L_key()
{
    __try
    {
        EnsureUser32Loaded();
        if (!g_user32)
        {
            if (g_autoParryDebug)
                printf("[AUTO-PARRY] user32.dll não carregado\n");
            return;
        }
        
        SendInputFunc sendInput = (SendInputFunc)GetProcAddress(g_user32, "SendInput");
        if (!sendInput)
        {
            if (g_autoParryDebug)
                printf("[AUTO-PARRY] SendInput não encontrado\n");
            return;
        }
        
        // Scan code da tecla L = 0x26
        WORD scanCodeL = 0x26;
        
        // Key down
        INPUT inputDown = create_key_input(scanCodeL, false);
        sendInput(1, &inputDown, sizeof(INPUT));
        
        // Pequeno delay
        Sleep(10);
        
        // Key up
        INPUT inputUp = create_key_input(scanCodeL, true);
        sendInput(1, &inputUp, sizeof(INPUT));
        
        if (g_autoParryDebug)
            printf("[AUTO-PARRY] L key pressed via SendInput!\n");
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        if (g_autoParryDebug)
            printf("[AUTO-PARRY] Exception ao pressionar L\n");
    }
}

// Hook para ExecCombo - detecta quando inimigo EXECUTA ataque e clica L
typedef void (__fastcall *ExecComboFunc)(void* actor, void* comboStateGroup, int skillInput, int inputIndex, int comboType, float transitionDuration, bool skipPrepare);
static ExecComboFunc g_originalExecCombo = nullptr;

static void __fastcall hooked_ExecCombo(void* actor, void* comboStateGroup, int skillInput, int inputIndex, int comboType, float transitionDuration, bool skipPrepare)
{
    // Chamar original primeiro
    g_originalExecCombo(actor, comboStateGroup, skillInput, inputIndex, comboType, transitionDuration, skipPrepare);
    
    if (!g_autoParryEnabled || !actor)
        return;
    
    __try
    {
        // Ignorar se for o jogador local
        if (g_localPlayerCaptured && actor == g_localPlayerActor)
            return;
        
        // INIMIGO EXECUTOU ATAQUE! CLICAR L IMEDIATAMENTE!
        if (g_autoParryDebug)
            printf("[AUTO-PARRY] Enemy attack executed! Pressing L key now!\n");
        
        // Clicar L imediatamente
        press_L_key();
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        if (g_autoParryDebug)
            printf("[AUTO-PARRY] Exception in ExecCombo hook\n");
    }
}

// Funções públicas
void autoparry_set_enabled(bool v) 
{ 
    g_autoParryEnabled = v;
    
    if (v && g_autoParryDebug)
        printf("[AUTO-PARRY] Enabled\n");
}

bool autoparry_get_enabled() { return g_autoParryEnabled; }

void autoparry_set_debug(bool v) { g_autoParryDebug = v; }
bool autoparry_get_debug() { return g_autoParryDebug; }

void* autoparry_get_local_player() { return g_localPlayerActor; }

void autoparry_init()
{
    EnsureGameAssemblyLoaded();
    if (!g_gameAssembly)
    {
        if (g_autoParryDebug)
            printf("[AUTO-PARRY] Failed to load GameAssembly\n");
        return;
    }
    
    // Instalar hooks usando Detours
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    
    // Hook ExecCombo - detecta quando inimigo EXECUTA ataque e clica L
    g_originalExecCombo = (ExecComboFunc)((uintptr_t)g_gameAssembly + RVA_EXEC_COMBO);
    DetourAttach(&(PVOID&)g_originalExecCombo, hooked_ExecCombo);
    
    LONG error = DetourTransactionCommit();
    
    if (error == NO_ERROR)
    {
        if (g_autoParryDebug)
            printf("[AUTO-PARRY] Hook ExecCombo installed successfully!\n");
    }
    else
    {
        if (g_autoParryDebug)
            printf("[AUTO-PARRY] Failed to install hook! Error: %ld\n", error);
    }
}

void autoparry_tick()
{
    if (!g_autoParryEnabled)
        return;
    
    static DWORD lastDebugPrint = 0;
    DWORD currentTime = GetTickCount();
    
    // Debug: printar a cada 5 segundos que está ativo
    if (g_autoParryDebug && (currentTime - lastDebugPrint) > 5000)
    {
        printf("[AUTO-PARRY] Tick running... enabled=%d\n", g_autoParryEnabled);
        lastDebugPrint = currentTime;
    }
    
    __try
    {
        EnsureGameAssemblyLoaded();
        if (!g_gameAssembly || !g_characterManagerInstance)
        {
            if (g_autoParryDebug && (currentTime - lastDebugPrint) > 5000)
                printf("[AUTO-PARRY] GameAssembly or CharacterManager not available\n");
            return;
        }
        
        // Capturar jogador local na primeira vez
        if (!g_localPlayerCaptured)
        {
            GetAliveCharListFunc getAliveCharList = (GetAliveCharListFunc)((uintptr_t)g_gameAssembly + RVA_GET_ALIVE_CHAR_LIST);
            void* charList = getAliveCharList(g_characterManagerInstance);
            
            if (charList)
            {
                IL2CPPList* list = (IL2CPPList*)charList;
                if (list->items && list->size > 0)
                {
                    IL2CPPArray* array = (IL2CPPArray*)list->items;
                    void** actors = (void**)((uintptr_t)array + 0x20);
                    
                    // Primeiro actor = jogador local
                    g_localPlayerActor = actors[0];
                    g_localPlayerCaptured = true;
                    
                    if (g_autoParryDebug)
                        printf("[AUTO-PARRY] Local player captured: %p\n", g_localPlayerActor);
                }
            }
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        if (g_autoParryDebug)
            printf("[AUTO-PARRY] Exception in tick\n");
    }
}
