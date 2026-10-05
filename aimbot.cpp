#include <Windows.h>
#include <cmath>
#include <vector>
#include "aimbot.h"
#include "hookframe.h"
#include "imgui/imgui.h"
#include "vec.h"   // Usar Vector3 do projeto
#include "vec2.h"  // Usar Vector2 do projeto
#include "offsets.h"  // RVAs centralizados
#include "dynamic_offsets.h"  // RVAs dinâmicos editáveis

struct Vector3Struct {
    float x, y, z;
};

// Configurações do aimbot
static bool g_aimbotEnabled = false;
static float g_aimbotFov = 100.0f;        // FOV em pixels (raio do círculo)
static float g_aimbotMinDistance = 5.0f;  // Distância mínima em metros
static float g_aimbotSmooth = 2.0f;       // Suavização (1 = instantâneo, maior = mais suave)
static float g_aimbotStrength = 2.0f;     // Força da puxada (0.1 = fraco, 2.0 = forte)
static bool g_aimbotDebug = false;        // Debug mode
static float g_aimbotHeightOffset = 0.0f; // Offset de altura (-1.0 = pés, 0.0 = centro, +1.0 = cabeça)
static bool g_aimbotHideFov = false;      // Esconder círculo do FOV

// Typedefs
typedef void* (__fastcall *GetMainCameraFunc)();
typedef void* (__fastcall *GetTransformFunc)(void* self);
typedef void (__fastcall *GetPositionFunc)(Vector3Struct* out, void* self);
typedef void (__fastcall *WorldToScreenFunc)(Vector3Struct* out, void* camera, Vector3Struct* worldPos);
typedef void* (__fastcall *GetAliveCharListFunc)(void* self);
typedef bool (__fastcall *IsLocalClientFunc)(void* self);

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
extern void* g_characterManagerInstance;  // Declaração externa do esp.cpp

// Funções auxiliares
static void EnsureGameAssemblyLoaded()
{
    if (!g_gameAssembly)
    {
        g_gameAssembly = GetModuleHandleA("GameAssembly_Super.dll");
    }
}

static bool get_actormodel_position(void* actor, Vector3& pos)
{
    if (!actor) return false;
    
    EnsureGameAssemblyLoaded();
    if (!g_gameAssembly) return false;
    
    __try
    {
        // Ler o campo transform usando o offset
        void* transform = *(void**)((uintptr_t)actor + DynamicOffsets::actorModelTransformOffset);
        if (!transform) return false;
        
        // Obter posição
        GetPositionFunc getPosition = (GetPositionFunc)((uintptr_t)g_gameAssembly + DynamicOffsets::rvaGetPosition);
        Vector3Struct posStruct;
        getPosition(&posStruct, transform);
        
        pos.x = posStruct.x;
        pos.y = posStruct.y;
        pos.z = posStruct.z;
        
        return true;
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        return false;
    }
}

static bool world_to_screen(void* camera, const Vector3& worldPos, Vector2& screenPos, bool& behindCamera)
{
    if (!camera) return false;
    
    EnsureGameAssemblyLoaded();
    if (!g_gameAssembly) return false;
    
    __try
    {
        WorldToScreenFunc worldToScreen = (WorldToScreenFunc)((uintptr_t)g_gameAssembly + DynamicOffsets::rvaWorldToScreen);
        
        Vector3Struct worldPosStruct = {worldPos.x, worldPos.y, worldPos.z};
        Vector3Struct screenPosStruct = {0, 0, 0};
        
        worldToScreen(&screenPosStruct, camera, &worldPosStruct);
        
        behindCamera = (screenPosStruct.z <= 0.0f);
        
        if (behindCamera)
            return false;
        
        // Converter para pixels
        RECT rect;
        GetClientRect(GetForegroundWindow(), &rect);
        float screenWidth = (float)(rect.right - rect.left);
        float screenHeight = (float)(rect.bottom - rect.top);
        
        if (screenPosStruct.x >= 0.0f && screenPosStruct.x <= 1.0f &&
            screenPosStruct.y >= 0.0f && screenPosStruct.y <= 1.0f)
        {
            screenPos.x = screenPosStruct.x * screenWidth;
            screenPos.y = screenPosStruct.y * screenHeight;
        }
        else
        {
            screenPos.x = screenPosStruct.x;
            screenPos.y = screenHeight - screenPosStruct.y;
        }
        
        return true;
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        return false;
    }
}

static float calculate_distance(const Vector3& pos1, const Vector3& pos2)
{
    float dx = pos1.x - pos2.x;
    float dy = pos1.y - pos2.y;
    float dz = pos1.z - pos2.z;
    return sqrtf(dx * dx + dy * dy + dz * dz);
}

static float calculate_distance_2d(const Vector2& pos1, const Vector2& pos2)
{
    float dx = pos1.x - pos2.x;
    float dy = pos1.y - pos2.y;
    return sqrtf(dx * dx + dy * dy);
}

static void smooth_aim(float targetX, float targetY, float smooth, float strength)
{
    if (smooth < 1.0f) smooth = 1.0f;
    if (strength < 0.1f) strength = 0.1f;
    
    // Obter centro da tela
    RECT rect;
    GetClientRect(GetForegroundWindow(), &rect);
    float centerX = (float)(rect.right - rect.left) * 0.5f;
    float centerY = (float)(rect.bottom - rect.top) * 0.5f;
    
    // Calcular delta
    float deltaX = targetX - centerX;
    float deltaY = targetY - centerY;
    
    // Aplicar suavização e força
    deltaX = (deltaX / smooth) * strength;
    deltaY = (deltaY / smooth) * strength;
    
    // Limitar movimento máximo por frame para evitar saltos
    const float maxMove = 50.0f;
    if (deltaX > maxMove) deltaX = maxMove;
    if (deltaX < -maxMove) deltaX = -maxMove;
    if (deltaY > maxMove) deltaY = maxMove;
    if (deltaY < -maxMove) deltaY = -maxMove;
    
    // Usar mouse_event para movimento relativo
    if (fabs(deltaX) > 0.1f || fabs(deltaY) > 0.1f)
    {
        mouse_event(MOUSEEVENTF_MOVE, (DWORD)deltaX, (DWORD)deltaY, 0, 0);
        
        if (g_aimbotDebug)
            printf("[AIMBOT] Moving mouse: delta=(%.2f, %.2f)\n", deltaX, deltaY);
    }
}

// Funções públicas
void aimbot_set_enabled(bool v) { g_aimbotEnabled = v; }
bool aimbot_get_enabled() { return g_aimbotEnabled; }

void aimbot_set_fov(float v) { g_aimbotFov = v; }
float aimbot_get_fov() { return g_aimbotFov; }

void aimbot_set_min_distance(float v) { g_aimbotMinDistance = v; }
float aimbot_get_min_distance() { return g_aimbotMinDistance; }

void aimbot_set_smooth(float v) { g_aimbotSmooth = v; }
float aimbot_get_smooth() { return g_aimbotSmooth; }

void aimbot_set_strength(float v) { g_aimbotStrength = v; }
float aimbot_get_strength() { return g_aimbotStrength; }

// Vetor para armazenar atores visíveis (compartilhado com ESP)
static std::vector<ActorData> g_visibleActors;

void aimbot_set_debug(bool v) { g_aimbotDebug = v; }
bool aimbot_get_debug() { return g_aimbotDebug; }

void aimbot_set_height_offset(float v) { g_aimbotHeightOffset = v; }
float aimbot_get_height_offset() { return g_aimbotHeightOffset; }

void aimbot_set_hide_fov(bool v) { g_aimbotHideFov = v; }
bool aimbot_get_hide_fov() { return g_aimbotHideFov; }

// Função chamada pelo ESP para atualizar lista de atores visíveis
void aimbot_update_actors(const std::vector<ActorData>& actors)
{
    g_visibleActors = actors;
}

void aimbot_render()
{
    if (!g_aimbotEnabled)
        return;
    
    // Obter ImGui draw list
    ImGuiIO& io = ImGui::GetIO();
    ImDrawList* draw = ImGui::GetForegroundDrawList();
    
    // Centro da tela
    float centerX = io.DisplaySize.x * 0.5f;
    float centerY = io.DisplaySize.y * 0.5f;
    
    // Desenhar círculo do FOV (apenas se não estiver escondido)
    if (!g_aimbotHideFov)
    {
        draw->AddCircle(ImVec2(centerX, centerY), g_aimbotFov, IM_COL32(255, 255, 255, 150), 64, 2.0f);
    }
    
    // Desenhar crosshair no centro
    float crossSize = 10.0f;
    draw->AddLine(ImVec2(centerX - crossSize, centerY), ImVec2(centerX + crossSize, centerY), IM_COL32(0, 255, 0, 200), 2.0f);
    draw->AddLine(ImVec2(centerX, centerY - crossSize), ImVec2(centerX, centerY + crossSize), IM_COL32(0, 255, 0, 200), 2.0f);
}

void aimbot_tick()
{
    if (!g_aimbotEnabled)
        return;

    // Verificar se o botão direito do mouse está pressionado
    if (!IsKeyPressed(VK_RBUTTON))
        return;
    
    if (g_visibleActors.empty())
    {
        if (g_aimbotDebug)
            printf("[AIMBOT] No visible actors\n");
        return;
    }
    
    if (g_aimbotDebug)
        printf("[AIMBOT] Right button pressed, checking %d visible actors...\n", (int)g_visibleActors.size());
    
    // Centro da tela
    RECT rect;
    GetClientRect(GetForegroundWindow(), &rect);
    float centerX = (float)(rect.right - rect.left) * 0.5f;
    float centerY = (float)(rect.bottom - rect.top) * 0.5f;
    Vector2 screenCenter(centerX, centerY);
    
    // Encontrar o alvo mais próximo do centro da tela dentro do FOV
    void* bestTarget = nullptr;
    float bestDistance = FLT_MAX;
    Vector2 bestScreenPos;
    int validTargets = 0;
    
    for (const auto& actorData : g_visibleActors)
    {
        // Ignorar entidades muito próximas
        if (actorData.distance < g_aimbotMinDistance)
            continue;
        
        validTargets++;
        
        // Calcular distância 2D do centro da tela
        Vector2 actorScreen(actorData.screenPosX, actorData.screenPosY);
        float distance2D = calculate_distance_2d(screenCenter, actorScreen);
        
        if (g_aimbotDebug && validTargets <= 3)
            printf("[AIMBOT] Target %d: 3D=%.1fm, 2D=%.1fpx, Screen=(%.1f,%.1f)\n", 
                   validTargets, actorData.distance, distance2D, actorData.screenPosX, actorData.screenPosY);
        
        // Verificar se está dentro do FOV
        if (distance2D > g_aimbotFov)
            continue;
        
        // Encontrar o mais próximo do centro
        if (distance2D < bestDistance)
        {
            bestDistance = distance2D;
            bestTarget = actorData.actor;
            bestScreenPos = actorScreen;
        }
    }
    
    if (g_aimbotDebug)
        printf("[AIMBOT] Valid targets: %d, Best distance: %.1fpx\n", validTargets, bestDistance);
    
    // Se encontrou um alvo, mirar nele
    if (bestTarget)
    {
        if (g_aimbotDebug)
            printf("[AIMBOT] Aiming at target: (%.1f, %.1f)\n", bestScreenPos.x, bestScreenPos.y);
        
        smooth_aim(bestScreenPos.x, bestScreenPos.y, g_aimbotSmooth, g_aimbotStrength);
    }
}
