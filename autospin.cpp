#include "autospin.h"
#include "Cheats.hpp"
#include "vec.h"
#include "esp.h"
#include <Windows.h>

static HMODULE g_gameAssembly = nullptr;
static float g_currentAngle = 0.0f;
static bool g_angleInitialized = false;

typedef void(__fastcall* GetEulerAnglesFunc)(Vector3* out, void* transform);
typedef void(__fastcall* SetEulerAnglesFunc)(void* transform, Vector3* angles);

static GetEulerAnglesFunc get_euler_angles_func = nullptr;
static SetEulerAnglesFunc set_euler_angles_func = nullptr;

void autospin_set_enabled(bool v) { Cheats::AutoSpin::enabled = v; }
bool autospin_get_enabled() { return Cheats::AutoSpin::enabled; }
void autospin_set_speed(float v) { Cheats::AutoSpin::speed = v; }
float autospin_get_speed() { return Cheats::AutoSpin::speed; }

void autospin_init() {
    g_gameAssembly = GetModuleHandleA("GameAssembly_Super.dll");
    if (!g_gameAssembly) return;
    
    uintptr_t base = (uintptr_t)g_gameAssembly;
    get_euler_angles_func = (GetEulerAnglesFunc)(base + RVA_GET_EULER_ANGLES);
    set_euler_angles_func = (SetEulerAnglesFunc)(base + RVA_SET_EULER_ANGLES);
}

void autospin_tick(void** actors, int actorCount) {
    if (!Cheats::AutoSpin::enabled) {
        g_angleInitialized = false;
        return;
    }
    
    // DEBUG: Mostrar quantos actors recebemos
    static int lastPrintTime = 0;
    int currentTime = GetTickCount();
    if (currentTime - lastPrintTime > 1000) {  // Print a cada 1 segundo
        printf("[AUTOSPIN] Received %d actors\n", actorCount);
        lastPrintTime = currentTime;
    }
    
    if (!actors || actorCount == 0) return;
    if (!get_euler_angles_func || !set_euler_angles_func) return;
    
    // Primeiro actor = jogador local
    void* player = actors[0];
    if (!player) return;
    
    // Obter transform (offset 0x60)
    void* transform = *(void**)((uintptr_t)player + 0x60);
    if (!transform || (uintptr_t)transform < 0x1000) return;
    
    // Ler rotação atual
    Vector3 currentAngles;
    get_euler_angles_func(&currentAngles, transform);
    
    // Inicializar ângulo na primeira vez
    if (!g_angleInitialized) {
        g_currentAngle = currentAngles.y;
        g_angleInitialized = true;
    }
    
    // Incrementar ângulo Y
    g_currentAngle += Cheats::AutoSpin::speed;
    
    if (g_currentAngle >= 360.0f) g_currentAngle -= 360.0f;
    if (g_currentAngle < 0) g_currentAngle += 360.0f;
    
    // Aplicar nova rotação
    Vector3 newAngles = { currentAngles.x, g_currentAngle, currentAngles.z };
    set_euler_angles_func(transform, &newAngles);
}
