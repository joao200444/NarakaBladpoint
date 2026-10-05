#include "noclip.h"
#include "Cheats.hpp"
#include "vec.h"
#include "esp.h"
#include "hookframe.h"
#include <Windows.h>
#include <cmath>

static HMODULE g_gameAssembly = nullptr;

typedef void(__fastcall* GetPositionFunc)(Vector3* out, void* transform);
typedef void(__fastcall* SetPositionFunc)(void* transform, Vector3* pos);
typedef void(__fastcall* GetEulerAnglesFunc)(Vector3* out, void* transform);

static GetPositionFunc get_position_func = nullptr;
static SetPositionFunc set_position_func = nullptr;
static GetEulerAnglesFunc get_euler_angles_func = nullptr;

void noclip_set_enabled(bool v) { Cheats::NoClip::enabled = v; }
bool noclip_get_enabled() { return Cheats::NoClip::enabled; }
void noclip_set_speed(float v) { Cheats::NoClip::flySpeed = v; }
float noclip_get_speed() { return Cheats::NoClip::flySpeed; }

void noclip_init() {
    g_gameAssembly = GetModuleHandleA("GameAssembly_Super.dll");
    if (!g_gameAssembly) return;
    
    uintptr_t base = (uintptr_t)g_gameAssembly;
    get_position_func = (GetPositionFunc)(base + RVA_GET_POSITION);
    set_position_func = (SetPositionFunc)(base + RVA_SET_POSITION);
    get_euler_angles_func = (GetEulerAnglesFunc)(base + RVA_GET_EULER_ANGLES);
}

void noclip_tick(void** actors, int actorCount) {
    if (!Cheats::NoClip::enabled) return;
    
    // DEBUG: Mostrar quantos actors recebemos
    static int lastPrintTime = 0;
    int currentTime = GetTickCount();
    if (currentTime - lastPrintTime > 1000) {  // Print a cada 1 segundo
        printf("[NOCLIP] Received %d actors\n", actorCount);
        lastPrintTime = currentTime;
    }
    
    if (!actors || actorCount == 0) return;
    if (!get_position_func || !set_position_func || !get_euler_angles_func) return;
    
    // Primeiro actor = jogador local
    void* player = actors[0];
    if (!player) return;
    
    // Obter transform (offset 0x60)
    void* transform = *(void**)((uintptr_t)player + 0x60);
    if (!transform || (uintptr_t)transform < 0x1000) return;
    
    // Obter posição atual
    Vector3 pos;
    get_position_func(&pos, transform);
    
    // Obter câmera e direções
    typedef void* (__fastcall *GetMainCameraFunc)();
    typedef void* (__fastcall *GetTransformFunc)(void* self);
    
    GetMainCameraFunc getMainCamera = (GetMainCameraFunc)((uintptr_t)g_gameAssembly + RVA_GET_MAIN_CAMERA);
    GetTransformFunc getTransform = (GetTransformFunc)((uintptr_t)g_gameAssembly + RVA_GET_TRANSFORM);
    
    void* camera = getMainCamera();
    if (!camera) return;
    
    void* cameraTransform = getTransform(camera);
    if (!cameraTransform) return;
    
    Vector3 angles;
    get_euler_angles_func(&angles, cameraTransform);
    
    // Converter para vetores
    float yawRad = angles.y * 3.14159265f / 180.0f;
    float pitchRad = angles.x * 3.14159265f / 180.0f;
    
    Vector3 forward, right;
    forward.x = sinf(yawRad) * cosf(pitchRad);
    forward.y = -sinf(pitchRad);
    forward.z = cosf(yawRad) * cosf(pitchRad);
    
    right.x = cosf(yawRad);
    right.y = 0.0f;
    right.z = -sinf(yawRad);
    
    float speed = Cheats::NoClip::flySpeed * 0.1f;
    bool moved = false;

    if (IsKeyPressed('W')) {
        pos.x += forward.x * speed;
        pos.y += forward.y * speed;
        pos.z += forward.z * speed;
        moved = true;
    }
    if (IsKeyPressed('S')) {
        pos.x -= forward.x * speed;
        pos.y -= forward.y * speed;
        pos.z -= forward.z * speed;
        moved = true;
    }
    if (IsKeyPressed('A')) {
        pos.x -= right.x * speed;
        pos.y -= right.y * speed;
        pos.z -= right.z * speed;
        moved = true;
    }
    if (IsKeyPressed('D')) {
        pos.x += right.x * speed;
        pos.y += right.y * speed;
        pos.z += right.z * speed;
        moved = true;
    }
    if (IsKeyPressed('K')) {
        pos.y += speed;
        moved = true;
    }
    if (IsKeyPressed(VK_SHIFT)) {
        pos.y -= speed;
        moved = true;
    }
    
    if (moved) {
        set_position_func(transform, &pos);
    }
}
