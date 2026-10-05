#include "dump_enemies_magnet.h"
#include "esp.h"
#include "aimbot.h"
#include <Windows.h>
#include <cmath>
#include <cstdio>
#include <unordered_set>

// Estrutura Vector3 para Unity
struct Vector3Struct {
	float x, y, z;
};

// Funções externas do ESP
extern std::vector<ActorData> g_visibleActors;
extern bool get_camera_position(Vector3& pos);

// Configurações do magnet
static bool g_magnetEnabled = false;
static float g_magnetDistance = 3.0f;  // 3 metros da câmera
static bool g_freezeHeight = false;    // Se true, mantém a altura original
static bool g_excludeLocalPlayer = true;  // Excluir o jogador local
static bool g_magnetDebug = false;

// Lista de entidades magnetizadas
static std::vector<MagnetizedEntity> g_magnetizedEntities;

// RVAs necessários
static HMODULE g_gameAssembly = nullptr;

// Typedefs para chamadas RVA
typedef void* (__fastcall *GetTransformFunc)(void* self);
typedef void (__fastcall *SetPositionFunc)(void* self, Vector3Struct* pos);

// Função auxiliar para garantir que GameAssembly está carregado
static void EnsureMagnetGameAssembly()
{
    if (!g_gameAssembly)
    {
        g_gameAssembly = GetModuleHandleA("GameAssembly_Super.dll");
    }
}

// Função para definir a posição de um ator
static bool set_actor_position(void* actorModel, const Vector3& pos)
{
    if (!actorModel) return false;
    
    EnsureMagnetGameAssembly();
    if (!g_gameAssembly) return false;
    
    __try
    {
        GetTransformFunc getTransform = (GetTransformFunc)((uintptr_t)g_gameAssembly + RVA_GET_TRANSFORM);
        void* transform = getTransform(actorModel);
        if (!transform) return false;
        
        SetPositionFunc setPosition = (SetPositionFunc)((uintptr_t)g_gameAssembly + RVA_SET_POSITION);
        Vector3Struct posStruct = {pos.x, pos.y, pos.z};
        setPosition(transform, &posStruct);
        
        return true;
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        return false;
    }
}

// Função para calcular posição magnetizada
static Vector3 calculate_magnet_position(const Vector3& cameraPos, const Vector3& targetPos, float distance, float originalHeight)
{
    Vector3 magnetPos;
    
    // Calcular direção do alvo em relação à câmera
    Vector3 direction;
    direction.x = targetPos.x - cameraPos.x;
    direction.y = 0.0f; // Ignorar Y para calcular direção horizontal
    direction.z = targetPos.z - cameraPos.z;
    
    // Normalizar direção
    float length = sqrtf(direction.x * direction.x + direction.z * direction.z);
    if (length > 0.001f)
    {
        direction.x /= length;
        direction.z /= length;
    }
    else
    {
        // Se estiver muito perto, usar direção padrão (frente)
        direction.x = 0.0f;
        direction.z = 1.0f;
    }
    
    // Calcular posição magnetizada a 'distance' metros da câmera
    magnetPos.x = cameraPos.x + (direction.x * distance);
    magnetPos.y = g_freezeHeight ? originalHeight : cameraPos.y;
    magnetPos.z = cameraPos.z + (direction.z * distance);
    
    return magnetPos;
}

// Função principal de update do magnet
void magnet_update()
{
    if (!g_magnetEnabled)
    {
        // Se magnet foi desabilitado, limpar todas as entidades
        if (!g_magnetizedEntities.empty())
        {
            magnet_clear_all();
        }
        return;
    }
    
    // Usar o vetor g_visibleActors que o ESP já preencheu com todos os atores e posições
    if (g_visibleActors.empty())
    {
        if (g_magnetDebug)
            printf("[MAGNET] No visible actors from ESP\n");
        return;
    }
    
    // Obter posição da câmera
    Vector3 cameraPos;
    if (!get_camera_position(cameraPos))
    {
        if (g_magnetDebug)
            printf("[MAGNET] Failed to get camera position\n");
        return;
    }
    
    if (g_magnetDebug)
        printf("[MAGNET] Processing %zu actors from ESP\n", g_visibleActors.size());
    
    // Conjunto para rastrear atores ativos neste frame
    std::unordered_set<void*> activeActors;
    
    // Processar cada ator visível do ESP
    for (const auto& actorData : g_visibleActors)
    {
        void* actor = actorData.actor;
        if (!actor) continue;
        
        activeActors.insert(actor);
        
        // Posição atual do ator (já fornecida pelo ESP)
        Vector3 currentPos;
        currentPos.x = actorData.worldPosX;
        currentPos.y = actorData.worldPosY;
        currentPos.z = actorData.worldPosZ;
        
        // Verificar se este ator já está na lista de magnetizados
        MagnetizedEntity* existingEntity = nullptr;
        for (auto& entity : g_magnetizedEntities)
        {
            if (entity.actorModel == actor)
            {
                existingEntity = &entity;
                break;
            }
        }
        
        if (existingEntity)
        {
            // Ator já magnetizado - atualizar posição magnetizada
            existingEntity->magnetPosition = calculate_magnet_position(
                cameraPos, currentPos, g_magnetDistance, existingEntity->originalPosition.y
            );
            existingEntity->lastUpdateMs = GetTickCount64();
            
            // Aplicar nova posição magnetizada
            if (set_actor_position(actor, existingEntity->magnetPosition))
            {
                if (g_magnetDebug)
                    printf("[MAGNET] Updated actor to magnet position (%.2f, %.2f, %.2f)\n", 
                           existingEntity->magnetPosition.x, existingEntity->magnetPosition.y, existingEntity->magnetPosition.z);
            }
        }
        else
        {
            // Novo ator - adicionar à lista de magnetizados
            MagnetizedEntity newEntity;
            newEntity.actorModel = actor;
            newEntity.originalPosition = currentPos;
            newEntity.magnetPosition = calculate_magnet_position(
                cameraPos, currentPos, g_magnetDistance, currentPos.y
            );
            newEntity.isActive = true;
            newEntity.lastUpdateMs = GetTickCount64();
            
            g_magnetizedEntities.push_back(newEntity);
            
            // Aplicar posição magnetizada
            if (set_actor_position(actor, newEntity.magnetPosition))
            {
                if (g_magnetDebug)
                    printf("[MAGNET] Magnetized new actor at position (%.2f, %.2f, %.2f)\n", 
                           newEntity.magnetPosition.x, newEntity.magnetPosition.y, newEntity.magnetPosition.z);
            }
        }
    }
    
    // Remover atores que não estão mais ativos
    auto it = g_magnetizedEntities.begin();
    while (it != g_magnetizedEntities.end())
    {
        if (activeActors.find(it->actorModel) == activeActors.end())
        {
            if (g_magnetDebug)
                printf("[MAGNET] Removing inactive actor from magnet list\n");
            it = g_magnetizedEntities.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

// Funções de configuração
void magnet_set_enabled(bool enabled)
{
    g_magnetEnabled = enabled;
    
    if (!enabled)
    {
        magnet_clear_all();
    }
    
    if (g_magnetDebug)
        printf("[MAGNET] Magnet %s\n", enabled ? "ENABLED" : "DISABLED");
}

bool magnet_get_enabled()
{
    return g_magnetEnabled;
}

void magnet_set_distance(float distance)
{
    g_magnetDistance = distance;
    if (g_magnetDebug)
        printf("[MAGNET] Distance set to %.2f meters\n", distance);
}

float magnet_get_distance()
{
    return g_magnetDistance;
}

void magnet_set_freeze_height(bool freeze)
{
    g_freezeHeight = freeze;
    if (g_magnetDebug)
        printf("[MAGNET] Freeze height %s\n", freeze ? "ENABLED" : "DISABLED");
}

bool magnet_get_freeze_height()
{
    return g_freezeHeight;
}

void magnet_set_exclude_local_player(bool exclude)
{
    g_excludeLocalPlayer = exclude;
    if (g_magnetDebug)
        printf("[MAGNET] Exclude local player %s\n", exclude ? "ENABLED" : "DISABLED");
}

bool magnet_get_exclude_local_player()
{
    return g_excludeLocalPlayer;
}

const std::vector<MagnetizedEntity>& magnet_get_entities()
{
    return g_magnetizedEntities;
}

void magnet_clear_all()
{
    if (g_magnetDebug)
        printf("[MAGNET] Clearing all magnetized entities (%zu total)\n", g_magnetizedEntities.size());
    
    // Opcional: Restaurar posições originais antes de limpar
    // (Comentado porque as entidades podem ter se movido naturalmente)
    /*
    for (auto& entity : g_magnetizedEntities)
    {
        set_actor_position(entity.actorModel, entity.originalPosition);
    }
    */
    
    g_magnetizedEntities.clear();
}

void magnet_set_debug(bool enabled)
{
    g_magnetDebug = enabled;
    printf("[MAGNET] Debug mode %s\n", enabled ? "ENABLED" : "DISABLED");
}

bool magnet_get_debug()
{
    return g_magnetDebug;
}
