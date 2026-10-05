#include <Windows.h>
#include <cstdint>
#include <memory>
#include <vector>
#include <cstdio>
#include <unordered_map>
#include <cmath>
#include <cstring>
#include "imgui/imgui.h"
#include <detours.h>  
#include "helpers.h"
#include "vec.h"
#include "vec2.h"
#include "aimbot.h"  
#include "esp.h"     
#include "offsets.h" 
#include "dynamic_offsets.h" 
#include "noclip.h"
#include "autospin.h"
#include "autoparry.h"
struct Vector3Struct {
	float x, y, z;
};
static std::vector<void*> g_entityCache;
static unsigned long long g_entityCacheTimeMs = 0;
static const unsigned long long g_entityCacheTtlMs = 1000;
static std::unordered_map<int, bool> g_typeEnabled;
static bool g_filtersInitialized = false;
static bool g_autoOpenBoxes = false;
static float g_autoOpenRadius = 8.0f;
static std::unordered_map<long long, unsigned long long> g_lastOpenMsByUuid;
static bool g_espEnabled = false;
static bool g_showChars = true;
static bool g_showMonsters = true;
static float g_boxW = 60.0f;
static float g_boxH = 120.0f;
static bool g_espLines = true;
static bool g_espNames = true;
static bool g_espDistance = true;
static bool g_espDebug = false;
static bool g_espBoxes = false;
static bool g_espCircles = false;
static bool g_espTriangles = false;
static bool g_espDots = false;
static bool g_espCrosshairs = false;
static bool g_esp3DBoxes = true;
static bool g_esp3DBoxFilled = false;
static bool g_espSkeleton = false;  
enum LinePosition {
    LINE_CENTER = 0,
    LINE_TOP = 1,
    LINE_BOTTOM = 2
};
static int g_espLinePosition = LINE_CENTER;
static bool g_filterInteractiveEnabled = false;
static bool g_filterShowAll = false;
static bool g_filterChests = false;        
static bool g_filterCollectables = false;  
static bool g_filterInsects = false;       
static bool g_filterFlowers = false;       
static bool g_filterHoles = false;         
static bool g_filterCampfires = false;     
static bool g_filterDrops = false;         
static bool g_filterBoxes = false;         
static bool g_filterObstacles = false;     
static void ensure_filters_initialized()
{
    if (g_filtersInitialized)
        return;
    g_typeEnabled[1] = true;    
    g_typeEnabled[2] = true;    
    g_typeEnabled[4] = true;    
    g_typeEnabled[8] = true;    
    g_filtersInitialized = true;
}
void** get_zentities_array(size_t& count)
{
    count = 0;
    return nullptr;
}
struct ActorTypeInfo {
    bool isPlayer;
    bool isMonster;
    bool isRobot;
    ActorTypeInfo() : isPlayer(false), isMonster(false), isRobot(false) {}
};
enum class BoneId : uint8_t {
    MotionRoot, AnchorNode, CameraNode, UnityCameraNode, gMan_Pelvis,
    gMan_L_Thigh, gMan_L_Calf, gMan_L_Foot, gMan_L_Toe0, LFootEnvCollider,
    gMan_L_knee_help, L_CalfBeAttackCollider, gMan_L_Thigh_Twist1, gMan_L_Thigh_Twist2, gMan_L_Thigh_Twist3,
    L_ThighBeAttackCollider, gMan_R_Thigh, gMan_R_Calf, gMan_R_Foot, gMan_R_Toe0,
    RFootEnvCollider, gMan_R_knee_help, R_CalfBeAttackCollider, gMan_R_Thigh_Twist1, gMan_R_Thigh_Twist2,
    gMan_R_Thigh_Twist3, R_ThighBeAttackCollider, gMan_Spine, gMan_Spine1, gMan_Spine2,
    gMan_L_Clavicle, gMan_L_UpperArm, gMan_L_Forearm, gMan_L_elbow_help, gMan_L_Forearm_Twist1,
    gMan_L_Forearm_Twist2, gMan_L_Forearm_Twist3, gMan_L_Hand, gMan_L_Finger0, gMan_L_Finger01,
    gMan_L_Finger02, gMan_L_Finger1, gMan_L_Finger11, gMan_L_Finger12, gMan_L_Finger2,
    gMan_L_Finger21, gMan_L_Finger22, gMan_L_Finger3, gMan_L_Finger31, gMan_L_Finger32,
    gMan_L_Finger4, gMan_L_Finger41, gMan_L_Finger42, LHandWeapon, LHandEnvCollider,
    AimRocketPoint, chain_link, L_weapon, AimIKPoint, L_ForearmBeAttackCollider,
    gMan_L_UpperArm_Twist1, gMan_L_UpperArm_Twist2, gMan_L_UpperArm_Twist3, gMan_L_UpperArm_Twist4, L_UpperArmBeAttackCollider,
    gMan_Neck, gMan_Head, HeadEnvCollider, HeadBeAttackCollider, gMan_R_Clavicle,
    gMan_R_UpperArm, gMan_R_Forearm, gMan_R_elbow_help, gMan_R_Forearm_Twist1, gMan_R_Forearm_Twist2,
    gMan_R_Forearm_Twist3, gMan_R_Hand, gMan_R_Finger0, gMan_R_Finger01, gMan_R_Finger02,
    gMan_R_Finger1, gMan_R_Finger11, gMan_R_Finger12, gMan_R_Finger2, gMan_R_Finger21,
    gMan_R_Finger22, gMan_R_Finger3, gMan_R_Finger31, gMan_R_Finger32, gMan_R_Finger4,
    gMan_R_Finger41, gMan_R_Finger42, RHandWeapon, RHandEnvCollider, AimRightHandIKPoint,
    R_weapon, R_ForearmBeAttackCollider, gMan_R_UpperArm_Twist1, gMan_R_UpperArm_Twist2, gMan_R_UpperArm_Twist3,
    gMan_R_UpperArm_Twist4, R_UpperArmBeAttackCollider, LBackWeapon, RBackWeapon, LockNode,
    AttachFxNode, BodyBeAttackCollider, LWaistWeapon, RWaistWeapon, PickupRayNode,
    BodyEnvCollider, PreTestEnvHalfHighCollider, PreTestEnvHalfLowCollider, RootEnvCollider
};
typedef void* (__fastcall *GetMainCameraFunc)();
typedef void* (__fastcall *GetTransformFunc)(void* self);
typedef void (__fastcall *GetPositionFunc)(Vector3Struct* out, void* self);  
typedef void (__fastcall *WorldToScreenFunc)(Vector3Struct* out, void* camera, Vector3Struct* worldPos);  
typedef void (__fastcall *GetMatrixFunc)(void* out, void* camera);  
typedef void* (__fastcall *GetNameFunc)(void* self);
typedef void* (__fastcall *FindObjectsOfTypeFunc)();  
typedef void* (__fastcall *GetAliveCharListFunc)(void* self);  
typedef void* (__fastcall *GetMainPlayerFunc)(void* self);  
typedef int (__fastcall *GetModelTypeFunc)(void* self);  
typedef int (__fastcall *GetActorModelFidFunc)(void* self);  
typedef void* (__fastcall *GetLogicBoneTransformFunc)(void* self, BoneId boneId);  
typedef bool (__fastcall *IsLocalClientFunc)(void* self);  
static HMODULE g_gameAssembly = nullptr;
static void EnsureGameAssemblyLoaded();
bool world_to_screen_rva(void* camera, const Vector3& worldPos, Vector2& screenPos, bool& behindCamera);
void* g_characterManagerInstance = nullptr;
typedef void* (__fastcall *GetAliveCharListFunc)(void* self);
static GetAliveCharListFunc g_originalGetAliveCharList = nullptr;
void InstallCharacterManagerHook();
void RemoveCharacterManagerHook();
void* __fastcall Hook_GetAliveCharList(void* self)
{
    if (!g_characterManagerInstance && self)
    {
        g_characterManagerInstance = self;
        if (g_espDebug)
            printf("[ESP] CharacterManager instance captured via 0x5499f10: 0x%p\n", self);
    }
    return g_originalGetAliveCharList(self);
}
void InstallCharacterManagerHook()
{
    EnsureGameAssemblyLoaded();
    if (!g_gameAssembly)
        return;
    void* targetFunc = (void*)((uintptr_t)g_gameAssembly + DynamicOffsets::rvaGetAliveCharList);
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    g_originalGetAliveCharList = (GetAliveCharListFunc)targetFunc;
    DetourAttach(&(PVOID&)g_originalGetAliveCharList, Hook_GetAliveCharList);
    LONG error = DetourTransactionCommit();
    if (error == NO_ERROR)
    {
        if (g_espDebug)
            printf("[ESP] Hook installed successfully on get_AllAliveCharacterList (0x5499f10)\n");
    }
    else
    {
        if (g_espDebug)
            printf("[ESP] Failed to install hook: %ld\n", error);
    }
}
void RemoveCharacterManagerHook()
{
    if (!g_originalGetAliveCharList)
        return;
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourDetach(&(PVOID&)g_originalGetAliveCharList, Hook_GetAliveCharList);
    DetourTransactionCommit();
    if (g_espDebug)
        printf("[ESP] Hook removed\n");
}
static void EnsureGameAssemblyLoaded()
{
    if (!g_gameAssembly)
    {
        g_gameAssembly = GetModuleHandleA("GameAssembly_Super.dll");
        if (!g_gameAssembly)
        {
            printf("[ESP] ERROR: GameAssembly_Super.dll not found!\n");
        }
        else
        {
            printf("[ESP] GameAssembly_Super.dll loaded at: 0x%p\n", g_gameAssembly);
            printf("[ESP] get_MainCamera will be at: 0x%p\n", (void*)((uintptr_t)g_gameAssembly + DynamicOffsets::rvaGetMainCamera));
            printf("[ESP] get_transform will be at: 0x%p\n", (void*)((uintptr_t)g_gameAssembly + DynamicOffsets::rvaGetTransform));
            printf("[ESP] get_Position will be at: 0x%p\n", (void*)((uintptr_t)g_gameAssembly + DynamicOffsets::rvaGetPosition));
        }
    }
}
bool get_camera_position(Vector3& pos)
{
    EnsureGameAssemblyLoaded();
    if (!g_gameAssembly)
    {
        if (g_espDebug)
            printf("[CAMERA] GameAssembly_Super.dll not loaded\n");
        return false;
    }
    __try
    {
        GetMainCameraFunc getMainCamera = (GetMainCameraFunc)((uintptr_t)g_gameAssembly + DynamicOffsets::rvaGetMainCamera);
        void* camera = getMainCamera();
        if (!camera)
        {
            if (g_espDebug)
                printf("[CAMERA] get_MainCamera returned NULL\n");
            return false;
        }
        GetTransformFunc getTransform = (GetTransformFunc)((uintptr_t)g_gameAssembly + DynamicOffsets::rvaGetTransform);
        void* transform = getTransform(camera);
        if (!transform)
        {
            if (g_espDebug)
                printf("[CAMERA] get_transform returned NULL\n");
            return false;
        }
        GetPositionFunc getPosition = (GetPositionFunc)((uintptr_t)g_gameAssembly + DynamicOffsets::rvaGetPosition);
        Vector3Struct posStruct = {0, 0, 0};
        if (g_espDebug)
            printf("[CAMERA] Calling get_position on transform: 0x%p\n", transform);
        getPosition(&posStruct, transform);  
        if (g_espDebug)
            printf("[CAMERA] get_position returned: (%.2f, %.2f, %.2f)\n", posStruct.x, posStruct.y, posStruct.z);
        pos.x = posStruct.x;
        pos.y = posStruct.y;
        pos.z = posStruct.z;
        if (g_espDebug)
            printf("[CAMERA] Position: (%.2f, %.2f, %.2f)\n", pos.x, pos.y, pos.z);
        return true;
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        if (g_espDebug)
            printf("[CAMERA] CRASH - Exception: 0x%X\n", GetExceptionCode());
        return false;
    }
}
bool get_bone_position(void* actorModel, BoneId boneId, Vector3& pos)
{
    if (!actorModel) return false;
    EnsureGameAssemblyLoaded();
    if (!g_gameAssembly) return false;
    __try
    {
        GetLogicBoneTransformFunc getBoneTransform = (GetLogicBoneTransformFunc)((uintptr_t)g_gameAssembly + DynamicOffsets::rvaGetLogicBoneTransform);
        void* boneTransform = getBoneTransform(actorModel, boneId);
        if (!boneTransform)
        {
            if (g_espDebug)
                printf("[SKELETON] Bone %d transform is NULL\n", (int)boneId);
            return false;
        }
        GetPositionFunc getPosition = (GetPositionFunc)((uintptr_t)g_gameAssembly + DynamicOffsets::rvaGetPosition);
        Vector3Struct posStruct;
        getPosition(&posStruct, boneTransform);
        pos.x = posStruct.x;
        pos.y = posStruct.y;
        pos.z = posStruct.z;
        if (g_espDebug)
            printf("[SKELETON] Bone %d position: (%.2f, %.2f, %.2f)\n", (int)boneId, pos.x, pos.y, pos.z);
        return true;
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        if (g_espDebug)
            printf("[SKELETON] Exception getting bone %d position\n", (int)boneId);
        return false;
    }
}
static void draw_skeleton(ImDrawList* draw, void* camera, void* actorModel, ImU32 color)
{
    if (!actorModel || !camera) return;
    struct BoneLine {
        BoneId from;
        BoneId to;
    };
    static const BoneLine skeleton[] = {
        {BoneId::gMan_Pelvis, BoneId::gMan_Spine},
        {BoneId::gMan_Spine, BoneId::gMan_Spine1},
        {BoneId::gMan_Spine1, BoneId::gMan_Spine2},
        {BoneId::gMan_Spine2, BoneId::gMan_Neck},
        {BoneId::gMan_Neck, BoneId::gMan_Head},
        {BoneId::gMan_Spine2, BoneId::gMan_L_Clavicle},
        {BoneId::gMan_L_Clavicle, BoneId::gMan_L_UpperArm},
        {BoneId::gMan_L_UpperArm, BoneId::gMan_L_Forearm},
        {BoneId::gMan_L_Forearm, BoneId::gMan_L_Hand},
        {BoneId::gMan_Spine2, BoneId::gMan_R_Clavicle},
        {BoneId::gMan_R_Clavicle, BoneId::gMan_R_UpperArm},
        {BoneId::gMan_R_UpperArm, BoneId::gMan_R_Forearm},
        {BoneId::gMan_R_Forearm, BoneId::gMan_R_Hand},
        {BoneId::gMan_Pelvis, BoneId::gMan_L_Thigh},
        {BoneId::gMan_L_Thigh, BoneId::gMan_L_Calf},
        {BoneId::gMan_L_Calf, BoneId::gMan_L_Foot},
        {BoneId::gMan_L_Foot, BoneId::gMan_L_Toe0},
        {BoneId::gMan_Pelvis, BoneId::gMan_R_Thigh},
        {BoneId::gMan_R_Thigh, BoneId::gMan_R_Calf},
        {BoneId::gMan_R_Calf, BoneId::gMan_R_Foot},
        {BoneId::gMan_R_Foot, BoneId::gMan_R_Toe0},
    };
    for (const auto& line : skeleton)
    {
        Vector3 fromPos, toPos;
        if (!get_bone_position(actorModel, line.from, fromPos)) continue;
        if (!get_bone_position(actorModel, line.to, toPos)) continue;
        Vector2 fromScreen, toScreen;
        bool behindFrom = false, behindTo = false;
        if (!world_to_screen_rva(camera, fromPos, fromScreen, behindFrom)) continue;
        if (!world_to_screen_rva(camera, toPos, toScreen, behindTo)) continue;
        if (behindFrom || behindTo) continue;
        draw->AddLine(ImVec2(fromScreen.x, fromScreen.y), ImVec2(toScreen.x, toScreen.y), color, 2.0f);
    }
}
bool get_entity_position(void* ent, Vector3& pos)
{
    if (!ent) return false;
    EnsureGameAssemblyLoaded();
    if (!g_gameAssembly) return false;
    GetTransformFunc getTransform = (GetTransformFunc)((uintptr_t)g_gameAssembly + DynamicOffsets::rvaGetTransform);
    void* transform = getTransform(ent);
    if (!transform) return false;
    GetPositionFunc getPosition = (GetPositionFunc)((uintptr_t)g_gameAssembly + DynamicOffsets::rvaGetPosition);
    Vector3Struct posStruct;
    getPosition(&posStruct, transform);  
    pos.x = posStruct.x;
    pos.y = posStruct.y;
    pos.z = posStruct.z;
    return true;
}
bool get_entity_type_info(void* ent, ActorTypeInfo& typeInfo)
{
    if (!ent) return false;
    typeInfo.isPlayer = true;
    typeInfo.isMonster = false;
    typeInfo.isRobot = false;
    return true;
}
bool get_entity_name(void* ent, char* nameBuffer, size_t bufferSize)
{
    if (nameBuffer && bufferSize > 0) nameBuffer[0] = '\0';
    return false;
}
struct IL2CPPDictionary {
    void* klass;
    void* monitor;
    void* entries;  
    int count;
    int version;
    int freeList;
    int freeCount;
    void* comparer;
    void* keys;
    void* values;
};
bool get_actor_hp_shield(void* actorModel, int& hp, int& shield, int& maxHp)
{
    hp = 0;
    shield = 0;
    maxHp = 100;
    if (!actorModel) return false;
    __try
    {
        EnsureGameAssemblyLoaded();
        if (!g_gameAssembly) return false;
        typedef int (__fastcall *GetHpFunc)(void* self);
        typedef int (__fastcall *GetHpMaxFunc)(void* self);
        typedef int (__fastcall *GetShieldFunc)(void* self);
        typedef int (__fastcall *GetShieldMaxFunc)(void* self);
        GetHpFunc getHp = (GetHpFunc)((uintptr_t)g_gameAssembly + DynamicOffsets::rvaGetHp);
        GetHpMaxFunc getHpMax = (GetHpMaxFunc)((uintptr_t)g_gameAssembly + DynamicOffsets::rvaGetHpMax);
        GetShieldFunc getShield = (GetShieldFunc)((uintptr_t)g_gameAssembly + DynamicOffsets::rvaGetShield);
        GetShieldMaxFunc getShieldMax = (GetShieldMaxFunc)((uintptr_t)g_gameAssembly + DynamicOffsets::rvaGetShieldMax);
        hp = getHp(actorModel);
        maxHp = getHpMax(actorModel);
        shield = getShield(actorModel);
        return (hp > 0 && maxHp > 0);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        return false;
    }
}
bool world_to_screen_rva(void* camera, const Vector3& worldPos, Vector2& screenPos, bool& behindCamera)
{
    if (!camera) return false;
    EnsureGameAssemblyLoaded();
    if (!g_gameAssembly) return false;
    __try
    {
        WorldToScreenFunc worldToScreen = (WorldToScreenFunc)((uintptr_t)g_gameAssembly + DynamicOffsets::rvaWorldToScreen);
        if ((uintptr_t)worldToScreen < 0x1000)
            return false;
        Vector3Struct worldPosStruct = {worldPos.x, worldPos.y, worldPos.z};
        Vector3Struct screenPosStruct = {0, 0, 0};
        worldToScreen(&screenPosStruct, camera, &worldPosStruct);
        behindCamera = (screenPosStruct.z <= 0.0f);
        if (behindCamera)
            return false;
        ImGuiIO& io = ImGui::GetIO();
        if (screenPosStruct.x >= 0.0f && screenPosStruct.x <= 1.0f &&
            screenPosStruct.y >= 0.0f && screenPosStruct.y <= 1.0f)
        {
            screenPos.x = screenPosStruct.x * io.DisplaySize.x;
            screenPos.y = screenPosStruct.y * io.DisplaySize.y;
        }
        else
        {
            screenPos.x = screenPosStruct.x;
            screenPos.y = io.DisplaySize.y - screenPosStruct.y;
        }
        if (std::isnan(screenPos.x) || std::isnan(screenPos.y) || 
            std::isinf(screenPos.x) || std::isinf(screenPos.y))
            return false;
        return true;
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        return false;
    }
}
bool get_entity_type(void* ent, int& type)
{
	ActorTypeInfo typeInfo;
	if (!get_entity_type_info(ent, typeInfo))
		return false;
	type = 0;
	if (typeInfo.isPlayer) type = 1;
	else if (typeInfo.isMonster) type = 2;
	else if (typeInfo.isRobot) type = 4;
	return type != 0;
}
static bool should_show_interactive(const char* entityName, int entityType)
{
    if (!(entityType & 32))
        return true;
    if (!g_filterInteractiveEnabled)
        return false;
    if (g_filterShowAll)
        return true;
    char lowerName[256];
    strncpy_s(lowerName, sizeof(lowerName), entityName, _TRUNCATE);
    _strlwr_s(lowerName, sizeof(lowerName));
    if (g_filterChests && (strstr(lowerName, "_trchest_") || strstr(lowerName, "chest") || strstr(lowerName, "treasure") || strstr(lowerName, "reward")))
        return true;
    if (g_filterCollectables && (strstr(lowerName, "_collection_") || strstr(lowerName, "collect") || strstr(lowerName, "item") || strstr(lowerName, "pickup")))
        return true;
    if (g_filterInsects && (strstr(lowerName, "_insect_") || strstr(lowerName, "insect") || strstr(lowerName, "bug")))
        return true;
    if (g_filterFlowers && (strstr(lowerName, "_flower_") || strstr(lowerName, "flower") || strstr(lowerName, "plant")))
        return true;
    if (g_filterHoles && (strstr(lowerName, "point_once") || strstr(lowerName, "hole") || strstr(lowerName, "dig")))
        return true;
    if (g_filterCampfires && (strstr(lowerName, "_campfire_") || strstr(lowerName, "campfire") || strstr(lowerName, "fire")))
        return true;
    if (g_filterDrops && (strstr(lowerName, "drop") || strstr(lowerName, "loot")))
        return true;
    if (g_filterBoxes && (strstr(lowerName, "breakablebox") || strstr(lowerName, "box") || strstr(lowerName, "crate")))
        return true;
    if (g_filterObstacles && (strstr(lowerName, "obstaclerock") || strstr(lowerName, "rock") || strstr(lowerName, "stone")))
        return true;
    return false;
}
static bool get_camera_forward(Vector3& forward)
{
    return false;
}
static bool get_local_player_position(Vector3& pos)
{
    return false;
}
static bool is_entity_in_front(const Vector3& camPos, const Vector3& entityPos, const Vector3& camForward)
{
    Vector3 toEntity;
    toEntity.x = entityPos.x - camPos.x;
    toEntity.y = entityPos.y - camPos.y;
    toEntity.z = entityPos.z - camPos.z;
    float length = sqrtf(toEntity.x * toEntity.x + toEntity.y * toEntity.y + toEntity.z * toEntity.z);
    if (length < 0.001f)
        return false;
    toEntity.x /= length;
    toEntity.y /= length;
    toEntity.z /= length;
    float dot = toEntity.x * camForward.x + toEntity.y * camForward.y + toEntity.z * camForward.z;
    return dot > 0.0f;
}
void esp_set_enabled(bool v) 
{ 
    g_espEnabled = v;
    if (v && !g_originalGetAliveCharList)
    {
        InstallCharacterManagerHook();
    }
}
bool esp_get_enabled() { return g_espEnabled; }
void esp_set_filter(bool showChars, bool showMonsters) { g_showChars = showChars; g_showMonsters = showMonsters; }
void esp_get_filter(bool& showChars, bool& showMonsters) { showChars = g_showChars; showMonsters = g_showMonsters; }
void esp_set_box(float w, float h) { g_boxW = w; g_boxH = h; }
void esp_get_box(float& w, float& h) { w = g_boxW; h = g_boxH; }
void esp_set_type_filter(int typeId, bool enabled)
{
    g_typeEnabled[typeId] = enabled;
}
bool esp_get_type_filter(int typeId)
{
    auto it = g_typeEnabled.find(typeId);
    if (it != g_typeEnabled.end())
        return it->second;
    return false;
}
static float calculate_distance(const Vector3& pos1, const Vector3& pos2)
{
    float dx = pos1.x - pos2.x;
    float dy = pos1.y - pos2.y;
    float dz = pos1.z - pos2.z;
    return sqrtf(dx * dx + dy * dy + dz * dz);
}
unsigned int get_color_for_type(int type)
{
    if (type == 1) return IM_COL32(0, 255, 0, 255);      
    if (type == 2) return IM_COL32(255, 0, 0, 255);      
    if (type == 4) return IM_COL32(255, 165, 0, 255);    
    if (type == 8) return IM_COL32(0, 255, 255, 255);    
    return IM_COL32(255, 255, 255, 255); 
}
static const char* get_type_name(int type)
{
    if (type == 1) return "Player";
    if (type == 2) return "Monster";
    if (type == 4) return "Robot/NPC";
    if (type == 8) return "Interactive";
    return "Unknown";
}
static const char* get_interactive_subtype(const char* entityName, int entityType)
{
    if (!(entityType & 32))
        return "";
    char lowerName[256];
    strncpy_s(lowerName, sizeof(lowerName), entityName, _TRUNCATE);
    _strlwr_s(lowerName, sizeof(lowerName));
    if (strstr(lowerName, "_trchest_"))
        return "Chest";
    if (strstr(lowerName, "_collection_"))
        return "Collectable";
    if (strstr(lowerName, "_insect_"))
        return "Insect";
    if (strstr(lowerName, "_flower_"))
        return "Flower";
    if (strstr(lowerName, "point_once"))
        return "Hole";
    if (strstr(lowerName, "_campfire_"))
        return "Campfire";
    if (strstr(lowerName, "drop"))
        return "Drop";
    if (strstr(lowerName, "breakablebox"))
        return "Box";
    if (strstr(lowerName, "obstaclerock"))
        return "Obstacle";
    return "Interactive";
}
static bool draw_3d_box(ImDrawList* draw, void* camera, const Vector3& worldPos, float width, float height, float depth, ImU32 color, Vector2& topCenter, bool filled)
{
    Vector3 corners[8];
    float halfW = width * 0.5f;
    float halfD = depth * 0.5f;
    corners[0] = Vector3(worldPos.x - halfW, worldPos.y, worldPos.z - halfD); 
    corners[1] = Vector3(worldPos.x + halfW, worldPos.y, worldPos.z - halfD); 
    corners[2] = Vector3(worldPos.x + halfW, worldPos.y, worldPos.z + halfD); 
    corners[3] = Vector3(worldPos.x - halfW, worldPos.y, worldPos.z + halfD); 
    corners[4] = Vector3(worldPos.x - halfW, worldPos.y + height, worldPos.z - halfD); 
    corners[5] = Vector3(worldPos.x + halfW, worldPos.y + height, worldPos.z - halfD); 
    corners[6] = Vector3(worldPos.x + halfW, worldPos.y + height, worldPos.z + halfD); 
    corners[7] = Vector3(worldPos.x - halfW, worldPos.y + height, worldPos.z + halfD); 
    Vector2 screenCorners[8];
    bool behindCamera[8];
    bool allBehind = true;
    for (int i = 0; i < 8; i++)
    {
        if (!world_to_screen_rva(camera, corners[i], screenCorners[i], behindCamera[i]))
        {
            return false; 
        }
        if (!behindCamera[i])
            allBehind = false;
    }
    if (allBehind)
        return false;
    topCenter.x = (screenCorners[4].x + screenCorners[5].x + screenCorners[6].x + screenCorners[7].x) / 4.0f;
    topCenter.y = (screenCorners[4].y + screenCorners[5].y + screenCorners[6].y + screenCorners[7].y) / 4.0f;
    if (filled)
    {
        ImU32 fillColor = IM_COL32(255, 255, 0, 60); 
        draw->AddQuadFilled(
            ImVec2(screenCorners[0].x, screenCorners[0].y),
            ImVec2(screenCorners[1].x, screenCorners[1].y),
            ImVec2(screenCorners[5].x, screenCorners[5].y),
            ImVec2(screenCorners[4].x, screenCorners[4].y),
            fillColor
        );
        draw->AddQuadFilled(
            ImVec2(screenCorners[2].x, screenCorners[2].y),
            ImVec2(screenCorners[3].x, screenCorners[3].y),
            ImVec2(screenCorners[7].x, screenCorners[7].y),
            ImVec2(screenCorners[6].x, screenCorners[6].y),
            fillColor
        );
        draw->AddQuadFilled(
            ImVec2(screenCorners[3].x, screenCorners[3].y),
            ImVec2(screenCorners[0].x, screenCorners[0].y),
            ImVec2(screenCorners[4].x, screenCorners[4].y),
            ImVec2(screenCorners[7].x, screenCorners[7].y),
            fillColor
        );
        draw->AddQuadFilled(
            ImVec2(screenCorners[1].x, screenCorners[1].y),
            ImVec2(screenCorners[2].x, screenCorners[2].y),
            ImVec2(screenCorners[6].x, screenCorners[6].y),
            ImVec2(screenCorners[5].x, screenCorners[5].y),
            fillColor
        );
        draw->AddQuadFilled(
            ImVec2(screenCorners[4].x, screenCorners[4].y),
            ImVec2(screenCorners[5].x, screenCorners[5].y),
            ImVec2(screenCorners[6].x, screenCorners[6].y),
            ImVec2(screenCorners[7].x, screenCorners[7].y),
            fillColor
        );
        draw->AddQuadFilled(
            ImVec2(screenCorners[0].x, screenCorners[0].y),
            ImVec2(screenCorners[1].x, screenCorners[1].y),
            ImVec2(screenCorners[2].x, screenCorners[2].y),
            ImVec2(screenCorners[3].x, screenCorners[3].y),
            fillColor
        );
    }
    draw->AddLine(ImVec2(screenCorners[0].x, screenCorners[0].y), 
                  ImVec2(screenCorners[1].x, screenCorners[1].y), color, 2.0f);
    draw->AddLine(ImVec2(screenCorners[1].x, screenCorners[1].y), 
                  ImVec2(screenCorners[2].x, screenCorners[2].y), color, 2.0f);
    draw->AddLine(ImVec2(screenCorners[2].x, screenCorners[2].y), 
                  ImVec2(screenCorners[3].x, screenCorners[3].y), color, 2.0f);
    draw->AddLine(ImVec2(screenCorners[3].x, screenCorners[3].y), 
                  ImVec2(screenCorners[0].x, screenCorners[0].y), color, 2.0f);
    draw->AddLine(ImVec2(screenCorners[4].x, screenCorners[4].y), 
                  ImVec2(screenCorners[5].x, screenCorners[5].y), color, 2.0f);
    draw->AddLine(ImVec2(screenCorners[5].x, screenCorners[5].y), 
                  ImVec2(screenCorners[6].x, screenCorners[6].y), color, 2.0f);
    draw->AddLine(ImVec2(screenCorners[6].x, screenCorners[6].y), 
                  ImVec2(screenCorners[7].x, screenCorners[7].y), color, 2.0f);
    draw->AddLine(ImVec2(screenCorners[7].x, screenCorners[7].y), 
                  ImVec2(screenCorners[4].x, screenCorners[4].y), color, 2.0f);
    draw->AddLine(ImVec2(screenCorners[0].x, screenCorners[0].y), 
                  ImVec2(screenCorners[4].x, screenCorners[4].y), color, 2.0f);
    draw->AddLine(ImVec2(screenCorners[1].x, screenCorners[1].y), 
                  ImVec2(screenCorners[5].x, screenCorners[5].y), color, 2.0f);
    draw->AddLine(ImVec2(screenCorners[2].x, screenCorners[2].y), 
                  ImVec2(screenCorners[6].x, screenCorners[6].y), color, 2.0f);
    draw->AddLine(ImVec2(screenCorners[3].x, screenCorners[3].y), 
                  ImVec2(screenCorners[7].x, screenCorners[7].y), color, 2.0f);
    return true;
}
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
const char* frida_naraka_js = R"(
var base = Module.findBaseAddress("GameAssembly_Super.dll");
console.log("[+] GameAssembly_Super.dll base: " + base);
var getMainPlayer = base.add(0x4c9ed30);
console.log("[+] Hooking GetMainPlayerActorModel at: " + getMainPlayer);
var characterManagerInstance = null;
Interceptor.attach(getMainPlayer, {
    onEnter: function(args) {
        var thisPtr = this.context.rcx;
        if (!thisPtr.isNull() && !characterManagerInstance) {
            characterManagerInstance = thisPtr;
            console.log("[+] CharacterManager instance captured: " + thisPtr);
            console.log("[*] Copy this address and use in game console:");
            console.log("    esp_set_character_manager " + thisPtr);
        }
    }
});
console.log("[*] Hook installed. Enter the game to capture CharacterManager.");
console.log("[*] Once captured, the address will be displayed above.");
)";
#define CHARACTERMANAGER_INSTANCE 0x0  
void** GetAllActorModels(int& count)
{
    count = 0;
    EnsureGameAssemblyLoaded();
    if (!g_gameAssembly) return nullptr;
    __try
    {
        if (!g_characterManagerInstance)
        {
            if (g_espDebug)
                printf("[CharacterManager] Instance not captured yet - enter the game!\n");
            return nullptr;
        }
        GetAliveCharListFunc getAllAliveList = (GetAliveCharListFunc)((uintptr_t)g_gameAssembly + DynamicOffsets::rvaGetAliveCharList);
        void* listObj = getAllAliveList(g_characterManagerInstance);
        if (!listObj)
        {
            if (g_espDebug)
                printf("[CharacterManager] get_AllAliveCharacterList returned NULL\n");
            return nullptr;
        }
        IL2CPPList* list = (IL2CPPList*)listObj;
        count = list->size;
        if (g_espDebug)
            printf("[CharacterManager] Found %d ActorModels (RVA: 0x%X)\n", count, DynamicOffsets::rvaGetAliveCharList);
        if (count == 0 || !list->items)
            return nullptr;
        IL2CPPArray* itemsArray = (IL2CPPArray*)list->items;
        return (void**)((uintptr_t)itemsArray + sizeof(IL2CPPArray));
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        if (g_espDebug)
            printf("[CharacterManager] CRASH - Exception: 0x%X\n", GetExceptionCode());
        return nullptr;
    }
}
bool get_actormodel_position(void* actorModel, Vector3& pos)
{
    if (!actorModel) return false;
    EnsureGameAssemblyLoaded();
    if (!g_gameAssembly) return false;
    __try
    {
        void* transform = *(void**)((uintptr_t)actorModel + DynamicOffsets::actorModelTransformOffset);
        if (!transform || (uintptr_t)transform < 0x1000)
            return false;
        GetPositionFunc getPosition = (GetPositionFunc)((uintptr_t)g_gameAssembly + DynamicOffsets::rvaGetPosition);
        Vector3Struct posStruct;
        getPosition(&posStruct, transform);
        pos.x = posStruct.x;
        pos.y = posStruct.y;
        pos.z = posStruct.z;
        if (std::isnan(pos.x) || std::isnan(pos.y) || std::isnan(pos.z))
            return false;
        if (std::abs(pos.x) > 100000.0f || std::abs(pos.y) > 100000.0f || std::abs(pos.z) > 100000.0f)
            return false;
        return true;
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        return false;
    }
}
std::vector<ActorData> g_visibleActors;
static unsigned long long g_lastCameraFailTime = 0;
static bool g_waitingForNewMatch = false;
void esp_render()
{
    if (!g_espEnabled)
        return;
    ImDrawList* draw = ImGui::GetBackgroundDrawList();
    EnsureGameAssemblyLoaded();
    if (!g_gameAssembly)
    {
        draw->AddText(ImVec2(10, 10), IM_COL32(255, 0, 0, 255), "[ESP DEBUG] GameAssembly not loaded");
        return;
    }
    if (g_espDebug)
    {
        int y = 10;
        char line[256];
        sprintf_s(line, "[ESP DEBUG] GameAssembly: 0x%p", g_gameAssembly);
        draw->AddText(ImVec2(10, y), IM_COL32(0, 255, 0, 255), line);
        y += 20;
        sprintf_s(line, "[ESP DEBUG] RVA GetAliveCharList: 0x%X", DynamicOffsets::rvaGetAliveCharList);
        draw->AddText(ImVec2(10, y), IM_COL32(0, 255, 255, 255), line);
        y += 20;
        sprintf_s(line, "[ESP DEBUG] RVA GetMainCamera: 0x%X", DynamicOffsets::rvaGetMainCamera);
        draw->AddText(ImVec2(10, y), IM_COL32(0, 255, 255, 255), line);
        y += 20;
        sprintf_s(line, "[ESP DEBUG] RVA GetPosition: 0x%X", DynamicOffsets::rvaGetPosition);
        draw->AddText(ImVec2(10, y), IM_COL32(0, 255, 255, 255), line);
        y += 20;
        sprintf_s(line, "[ESP DEBUG] RVA WorldToScreen: 0x%X", DynamicOffsets::rvaWorldToScreen);
        draw->AddText(ImVec2(10, y), IM_COL32(0, 255, 255, 255), line);
        y += 20;
        draw->AddText(ImVec2(10, y), IM_COL32(255, 255, 255, 255), "---");
        y += 20;
        if (g_characterManagerInstance)
        {
            sprintf_s(line, "[ESP DEBUG] CharacterManager: 0x%p (CAPTURED)", g_characterManagerInstance);
            draw->AddText(ImVec2(10, y), IM_COL32(0, 255, 0, 255), line);
        }
        else
        {
            sprintf_s(line, "[ESP DEBUG] CharacterManager: NOT CAPTURED - Enter game!");
            draw->AddText(ImVec2(10, y), IM_COL32(255, 165, 0, 255), line);
        }
        y += 20;
        __try
        {
            GetMainCameraFunc getMainCamera = (GetMainCameraFunc)((uintptr_t)g_gameAssembly + DynamicOffsets::rvaGetMainCamera);
            void* camera = getMainCamera();
            if (camera)
            {
                sprintf_s(line, "[ESP DEBUG] Camera: 0x%p (OK)", camera);
                draw->AddText(ImVec2(10, y), IM_COL32(0, 255, 0, 255), line);
                y += 20;
                Vector3 camPos;
                if (get_camera_position(camPos))
                {
                    sprintf_s(line, "[ESP DEBUG] Camera Pos: (%.1f, %.1f, %.1f)", camPos.x, camPos.y, camPos.z);
                    draw->AddText(ImVec2(10, y), IM_COL32(0, 255, 0, 255), line);
                    y += 20;
                }
                else
                {
                    sprintf_s(line, "[ESP DEBUG] Camera Pos: FAILED");
                    draw->AddText(ImVec2(10, y), IM_COL32(255, 0, 0, 255), line);
                    y += 20;
                }
            }
            else
            {
                sprintf_s(line, "[ESP DEBUG] Camera: NOT FOUND");
                draw->AddText(ImVec2(10, y), IM_COL32(255, 0, 0, 255), line);
                y += 20;
            }
        }
        __except(EXCEPTION_EXECUTE_HANDLER)
        {
            sprintf_s(line, "[ESP DEBUG] Camera: CRASH (0x%X)", GetExceptionCode());
            draw->AddText(ImVec2(10, y), IM_COL32(255, 0, 0, 255), line);
            y += 20;
        }
        draw->AddText(ImVec2(10, y), IM_COL32(255, 255, 255, 255), "---");
        y += 20;
        if (g_characterManagerInstance)
        {
            __try
            {
                int actorCount = 0;
                void** actors = GetAllActorModels(actorCount);
                if (actors && actorCount > 0)
                {
                    sprintf_s(line, "[ESP DEBUG] Players Alive: %d", actorCount);
                    draw->AddText(ImVec2(10, y), IM_COL32(0, 255, 0, 255), line);
                    y += 20;
                }
                else
                {
                    sprintf_s(line, "[ESP DEBUG] Players Alive: 0 (No actors found)");
                    draw->AddText(ImVec2(10, y), IM_COL32(255, 165, 0, 255), line);
                    y += 20;
                }
            }
            __except(EXCEPTION_EXECUTE_HANDLER)
            {
                sprintf_s(line, "[ESP DEBUG] Players: CRASH (0x%X)", GetExceptionCode());
                draw->AddText(ImVec2(10, y), IM_COL32(255, 0, 0, 255), line);
                y += 20;
            }
        }
        else
        {
            sprintf_s(line, "[ESP DEBUG] Players: Waiting for CharacterManager");
            draw->AddText(ImVec2(10, y), IM_COL32(255, 165, 0, 255), line);
            y += 20;
        }
        draw->AddText(ImVec2(10, y), IM_COL32(255, 255, 255, 255), "---");
        y += 20;
    }
    g_visibleActors.clear();
    __try
    {
        GetMainCameraFunc getMainCamera = (GetMainCameraFunc)((uintptr_t)g_gameAssembly + DynamicOffsets::rvaGetMainCamera);
        void* camera = getMainCamera();
        if (!camera)
        {
            if (g_espDebug)
                draw->AddText(ImVec2(10, 10), IM_COL32(255, 0, 0, 255), "ESP: Camera not found");
            if (g_characterManagerInstance)
            {
                unsigned long long currentTime = GetTickCount64();
                if (g_lastCameraFailTime == 0)
                {
                    g_lastCameraFailTime = currentTime;
                    if (g_espDebug)
                        printf("[ESP] Camera lost - waiting 2 seconds to reset CharacterManager...\n");
                }
                if (currentTime - g_lastCameraFailTime >= 2000)
                {
                    if (g_espDebug)
                        printf("[ESP] Resetting CharacterManager - player left match\n");
                    g_characterManagerInstance = nullptr;
                    g_waitingForNewMatch = true;
                    g_lastCameraFailTime = 0;
                    if (g_espDebug)
                    {
                        draw->AddText(ImVec2(10, 30), IM_COL32(255, 165, 0, 255), 
                                     "CharacterManager reset - waiting for new match...");
                    }
                }
                else
                {
                    if (g_espDebug)
                    {
                        unsigned long long timeLeft = 2000 - (currentTime - g_lastCameraFailTime);
                        char waitText[128];
                        sprintf_s(waitText, "Resetting CharacterManager in %.1fs...", timeLeft / 1000.0f);
                        draw->AddText(ImVec2(10, 30), IM_COL32(255, 165, 0, 255), waitText);
                    }
                }
            }
            return;
        }
        g_lastCameraFailTime = 0;
        if (g_waitingForNewMatch)
        {
            g_waitingForNewMatch = false;
            if (g_espDebug)
                printf("[ESP] Camera found - ready to capture new CharacterManager\n");
        }
        Vector3 camPos;
        if (!get_camera_position(camPos))
        {
            draw->AddText(ImVec2(10, 10), IM_COL32(255, 0, 0, 255), "ESP: Failed to get camera position");
            return;
        }
        char infoText[256];
        sprintf_s(infoText, "Camera: (%.1f, %.1f, %.1f)", 
                  camPos.x, camPos.y, camPos.z);
        if (g_espDebug)
            draw->AddText(ImVec2(10, 10), IM_COL32(0, 255, 0, 255), infoText);
        if (!g_characterManagerInstance)
        {
            if (g_espDebug)
            {
                draw->AddText(ImVec2(10, 30), IM_COL32(255, 165, 0, 255), 
                             "CharacterManager: Waiting for hook...");
                draw->AddText(ImVec2(10, 50), IM_COL32(255, 255, 0, 255), 
                             "Enter the game to capture instance");
            }
            return;
        }
        if (g_espDebug)
        {
            char instanceText[128];
            sprintf_s(instanceText, "CharacterManager: 0x%p", g_characterManagerInstance);
            draw->AddText(ImVec2(10, 30), IM_COL32(0, 255, 0, 255), instanceText);
        }
        int actorCount = 0;
        void** actors = GetAllActorModels(actorCount);
        if (g_espDebug)
        {
            char countText[256];
            sprintf_s(countText, "Actors Alive: %d (RVA: 0x%X)", actorCount, DynamicOffsets::rvaGetAliveCharList);
            draw->AddText(ImVec2(10, 50), IM_COL32(0, 255, 255, 255), countText);
        }
        if (!actors || actorCount == 0)
        {
            return;
        }
        int drawnCount = 0;
        int failedPosition = 0;
        int failedWorldToScreen = 0;
        int behindCameraCount = 0;
        int tooCloseCount = 0;
        int tooFarCount = 0;
        if (g_espDebug)
        {
            char loopText[256];
            sprintf_s(loopText, "Starting loop with %d actors", actorCount);
            draw->AddText(ImVec2(10, 70), IM_COL32(0, 255, 255, 255), loopText);
        }
        for (int i = 0; i < actorCount; i++)
        {
            void* actor = actors[i];
            if (!actor) continue;
            Vector3 actorPos;
            if (!get_actormodel_position(actor, actorPos))
            {
                failedPosition++;
                if (g_espDebug && i < 3)
                {
                    char failText[256];
                    sprintf_s(failText, "Actor %d: Failed to get position", i);
                    draw->AddText(ImVec2(10, 90 + (i * 20)), IM_COL32(255, 0, 0, 255), failText);
                }
                continue;
            }
            Vector3 feetPos = actorPos;
            feetPos.y -= 0.0f;  
            extern float aimbot_get_height_offset();
            float heightOffset = aimbot_get_height_offset();
            Vector3 boxCenter = feetPos;
            boxCenter.y += 1.0f;  
            boxCenter.y += heightOffset * 1.0f;  
            Vector2 screenPos;
            bool behindCamera = false;
            if (!world_to_screen_rva(camera, boxCenter, screenPos, behindCamera))
            {
                failedWorldToScreen++;
                continue;
            }
            if (behindCamera)
            {
                behindCameraCount++;
                continue;
            }
            float distance = calculate_distance(camPos, boxCenter);
            if (distance > 500.0f)
            {
                tooFarCount++;
                continue;
            }
            ActorData actorData;
            actorData.actor = actor;
            actorData.worldPosX = boxCenter.x;
            actorData.worldPosY = boxCenter.y;
            actorData.worldPosZ = boxCenter.z;
            actorData.screenPosX = screenPos.x;
            actorData.screenPosY = screenPos.y;
            actorData.distance = distance;
            g_visibleActors.push_back(actorData);
            if (distance < 5.0f)
            {
                tooCloseCount++;
                continue;
            }
            ImU32 color = IM_COL32(255, 255, 255, 255);  
            ImU32 distColor;
            if (distance < 20.0f)
                distColor = IM_COL32(255, 255, 255, 255);  
            else if (distance < 50.0f)
                distColor = IM_COL32(255, 165, 0, 255);    
            else
                distColor = IM_COL32(255, 0, 0, 255);      
            Vector2 lineEndPoint = screenPos;  
            if (g_esp3DBoxes)
            {
                Vector2 topCenter;
                if (draw_3d_box(draw, camera, feetPos, 1.0f, 2.0f, 1.0f, color, topCenter, g_esp3DBoxFilled))
                {
                    lineEndPoint = topCenter;  
                    char nameText[64];
                    sprintf_s(nameText, "Player %d", i + 1);
                    char distText[64];
                    sprintf_s(distText, "%.0fm", distance);
                    ImVec2 nameSize = ImGui::CalcTextSize(nameText);
                    ImVec2 distSize = ImGui::CalcTextSize(distText);
                    float textX = topCenter.x - (nameSize.x * 0.5f);
                    float textY = topCenter.y - 40;
                    float padding = 4.0f;
                    ImVec2 bgMin(textX - padding, textY - padding);
                    ImVec2 bgMax(textX + nameSize.x + padding, textY + nameSize.y + distSize.y + padding * 2);
                    draw->AddRectFilled(bgMin, bgMax, IM_COL32(0, 0, 0, 180));
                    draw->AddRect(bgMin, bgMax, IM_COL32(255, 255, 255, 255), 0.0f, 0, 1.0f);
                    if (g_espNames)
                    {
                        draw->AddText(ImVec2(textX, textY), IM_COL32(255, 255, 255, 255), nameText);
                    }
                    if (g_espDistance)
                    {
                        float distY = textY + nameSize.y + 2;
                        draw->AddText(ImVec2(textX, distY), distColor, distText);
                    }
                    int hp, shield, maxHp;
                    if (get_actor_hp_shield(actor, hp, shield, maxHp))
                    {
                        float barWidth = 8.0f;
                        float barHeight = 100.0f;
                        float boxWidth = 1.0f; 
                        float boxHeight = 2.0f; 
                        Vector3 bottomLeftWorld = feetPos;
                        bottomLeftWorld.x -= boxWidth * 0.5f;
                        Vector2 bottomLeftScreen;
                        bool behindBottomLeft = false;
                        if (!world_to_screen_rva(camera, bottomLeftWorld, bottomLeftScreen, behindBottomLeft))
                        {
                            float boxScreenWidth = 60.0f;
                            float barX = topCenter.x - boxScreenWidth * 0.5f - barWidth - 10;
                            float barY = topCenter.y;
                            draw->AddRectFilled(ImVec2(barX, barY), ImVec2(barX + barWidth, barY + barHeight), IM_COL32(0, 0, 0, 180));
                            float hpRatio = (float)hp / (float)maxHp;
                            if (hpRatio > 1.0f) hpRatio = 1.0f;
                            if (hpRatio < 0.0f) hpRatio = 0.0f;
                            float hpHeight = barHeight * hpRatio;
                            draw->AddRectFilled(ImVec2(barX, barY + barHeight - hpHeight), ImVec2(barX + barWidth, barY + barHeight), IM_COL32(0, 255, 0, 255));
                            if (shield > 0)
                            {
                                float shieldRatio = (float)shield / (float)maxHp;
                                if (shieldRatio > 1.0f) shieldRatio = 1.0f;
                                float shieldHeight = barHeight * shieldRatio;
                                draw->AddRectFilled(ImVec2(barX, barY + barHeight - shieldHeight), ImVec2(barX + barWidth, barY + barHeight), IM_COL32(0, 150, 255, 255));
                            }
                            draw->AddRect(ImVec2(barX, barY), ImVec2(barX + barWidth, barY + barHeight), IM_COL32(255, 255, 255, 255), 0.0f, 0, 1.0f);
                        }
                        else
                        {
                            Vector3 topLeftWorld = bottomLeftWorld;
                            topLeftWorld.y += boxHeight;
                            Vector2 topLeftScreen;
                            bool behindTopLeft = false;
                            if (world_to_screen_rva(camera, topLeftWorld, topLeftScreen, behindTopLeft))
                            {
                                float barX = bottomLeftScreen.x - barWidth - 5;
                                float barY = topLeftScreen.y;
                                float barH = bottomLeftScreen.y - topLeftScreen.y;
                                draw->AddRectFilled(ImVec2(barX, barY), ImVec2(barX + barWidth, barY + barH), IM_COL32(0, 0, 0, 180));
                                float hpRatio = (float)hp / (float)maxHp;
                                if (hpRatio > 1.0f) hpRatio = 1.0f;
                                if (hpRatio < 0.0f) hpRatio = 0.0f;
                                float hpHeight = barH * hpRatio;
                                draw->AddRectFilled(ImVec2(barX, barY + barH - hpHeight), ImVec2(barX + barWidth, barY + barH), IM_COL32(0, 255, 0, 255));
                                if (shield > 0)
                                {
                                    float shieldRatio = (float)shield / (float)maxHp;
                                    if (shieldRatio > 1.0f) shieldRatio = 1.0f;
                                    float shieldHeight = barH * shieldRatio;
                                    draw->AddRectFilled(ImVec2(barX, barY + barH - shieldHeight), ImVec2(barX + barWidth, barY + barH), IM_COL32(0, 150, 255, 255));
                                }
                                draw->AddRect(ImVec2(barX, barY), ImVec2(barX + barWidth, barY + barH), IM_COL32(255, 255, 255, 255), 0.0f, 0, 1.0f);
                            }
                        }
                    }
                }
            }
            else
            {
                Vector3 headPos = feetPos;
                headPos.y += 2.0f;  
                Vector2 screenHead, screenFeet;
                bool behindHead = false, behindFeet = false;
                if (world_to_screen_rva(camera, headPos, screenHead, behindHead) &&
                    world_to_screen_rva(camera, feetPos, screenFeet, behindFeet))
                {
                    lineEndPoint = screenHead;  
                    float boxHeight = screenFeet.y - screenHead.y;
                    float boxWidth = boxHeight * 0.5f;
                    ImVec2 boxMin(screenHead.x - boxWidth * 0.5f, screenHead.y);
                    ImVec2 boxMax(screenHead.x + boxWidth * 0.5f, screenFeet.y);
                    draw->AddRect(boxMin, boxMax, color, 0.0f, 0, 2.0f);
                    char nameText[64];
                    sprintf_s(nameText, "Player %d", i + 1);
                    char distText[64];
                    sprintf_s(distText, "%.0fm", distance);
                    ImVec2 nameSize = ImGui::CalcTextSize(nameText);
                    ImVec2 distSize = ImGui::CalcTextSize(distText);
                    float textX = screenHead.x - (nameSize.x * 0.5f);
                    float textY = screenHead.y - nameSize.y - distSize.y - 10;
                    float padding = 4.0f;
                    ImVec2 bgMin(textX - padding, textY - padding);
                    ImVec2 bgMax(textX + nameSize.x + padding, textY + nameSize.y + distSize.y + padding * 2);
                    draw->AddRectFilled(bgMin, bgMax, IM_COL32(0, 0, 0, 180));
                    draw->AddRect(bgMin, bgMax, IM_COL32(255, 255, 255, 255), 0.0f, 0, 1.0f);
                    if (g_espNames)
                    {
                        draw->AddText(ImVec2(textX, textY), IM_COL32(255, 255, 255, 255), nameText);
                    }
                    if (g_espDistance)
                    {
                        float distY = textY + nameSize.y + 2;
                        draw->AddText(ImVec2(textX, distY), distColor, distText);
                    }
                    int hp, shield, maxHp;
                    if (get_actor_hp_shield(actor, hp, shield, maxHp))
                    {
                        float barWidth = 8.0f;
                        float barHeight = boxHeight;
                        float barX = boxMin.x - barWidth - 5;
                        float barY = boxMin.y;
                        draw->AddRectFilled(ImVec2(barX, barY), ImVec2(barX + barWidth, barY + barHeight), IM_COL32(0, 0, 0, 180));
                        float hpRatio = (float)hp / (float)maxHp;
                        if (hpRatio > 1.0f) hpRatio = 1.0f;
                        if (hpRatio < 0.0f) hpRatio = 0.0f;
                        float hpHeight = barHeight * hpRatio;
                        draw->AddRectFilled(ImVec2(barX, barY + barHeight - hpHeight), ImVec2(barX + barWidth, barY + barHeight), IM_COL32(0, 255, 0, 255));
                        if (shield > 0)
                        {
                            float shieldRatio = (float)shield / (float)maxHp;
                            if (shieldRatio > 1.0f) shieldRatio = 1.0f;
                            float shieldHeight = barHeight * shieldRatio;
                            draw->AddRectFilled(ImVec2(barX, barY + barHeight - shieldHeight), ImVec2(barX + barWidth, barY + barHeight), IM_COL32(0, 150, 255, 255));
                        }
                        draw->AddRect(ImVec2(barX, barY), ImVec2(barX + barWidth, barY + barHeight), IM_COL32(255, 255, 255, 255), 0.0f, 0, 1.0f);
                    }
                }
            }
            if (g_espLines)
            {
                ImGuiIO& io = ImGui::GetIO();
                ImVec2 lineStart = ImVec2(io.DisplaySize.x * 0.5f, 0.0f);  
                draw->AddLine(lineStart, ImVec2(lineEndPoint.x, lineEndPoint.y), IM_COL32(255, 255, 255, 255), 1.5f);  
            }
            if (g_espSkeleton)
            {
                draw_skeleton(draw, camera, actor, color);
            }
            drawnCount++;
        }
        aimbot_update_actors(g_visibleActors);
        noclip_tick(actors, actorCount);
        autospin_tick(actors, actorCount);
        autoparry_tick();
        if (g_espDebug)
        {
            char drawnText[256];
            sprintf_s(drawnText, "Drawn: %d / %d", drawnCount, actorCount);
            draw->AddText(ImVec2(10, 70), IM_COL32(255, 255, 0, 255), drawnText);
            if (drawnCount == 0 && actorCount > 0)
            {
                int y = 90;
                if (failedPosition > 0)
                {
                    sprintf_s(drawnText, "Failed Position: %d", failedPosition);
                    draw->AddText(ImVec2(10, y), IM_COL32(255, 0, 0, 255), drawnText);
                    y += 20;
                }
                if (failedWorldToScreen > 0)
                {
                    sprintf_s(drawnText, "Failed WorldToScreen: %d", failedWorldToScreen);
                    draw->AddText(ImVec2(10, y), IM_COL32(255, 0, 0, 255), drawnText);
                    y += 20;
                }
                if (behindCameraCount > 0)
                {
                    sprintf_s(drawnText, "Behind Camera: %d", behindCameraCount);
                    draw->AddText(ImVec2(10, y), IM_COL32(255, 165, 0, 255), drawnText);
                    y += 20;
                }
                if (tooCloseCount > 0)
                {
                    sprintf_s(drawnText, "Too Close (<5m): %d", tooCloseCount);
                    draw->AddText(ImVec2(10, y), IM_COL32(255, 165, 0, 255), drawnText);
                    y += 20;
                }
                if (tooFarCount > 0)
                {
                    sprintf_s(drawnText, "Too Far (>500m): %d", tooFarCount);
                    draw->AddText(ImVec2(10, y), IM_COL32(255, 165, 0, 255), drawnText);
                    y += 20;
                }
            }
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        draw->AddText(ImVec2(10, 10), IM_COL32(255, 0, 0, 255), "ESP: CRASH!");
    }
}
void esp_set_auto_open_boxes(bool v) { g_autoOpenBoxes = v; }
bool esp_get_auto_open_boxes() { return g_autoOpenBoxes; }
void esp_set_auto_open_radius(float r) { g_autoOpenRadius = r; }
float esp_get_auto_open_radius() { return g_autoOpenRadius; }
bool esp_get_entity_type_value(const char* fieldName, int& out)
{
    return false;
}
void esp_set_lines(bool v) { g_espLines = v; }
bool esp_get_lines() { return g_espLines; }
void esp_set_names(bool v) { g_espNames = v; }
bool esp_get_names() { return g_espNames; }
void esp_set_distance(bool v) { g_espDistance = v; }
bool esp_get_distance() { return g_espDistance; }
void esp_set_boxes(bool v) { g_espBoxes = v; }
bool esp_get_boxes() { return g_espBoxes; }
void esp_set_circles(bool v) { g_espCircles = v; }
bool esp_get_circles() { return g_espCircles; }
void esp_set_triangles(bool v) { g_espTriangles = v; }
bool esp_get_triangles() { return g_espTriangles; }
void esp_set_dots(bool v) { g_espDots = v; }
bool esp_get_dots() { return g_espDots; }
void esp_set_crosshairs(bool v) { g_espCrosshairs = v; }
bool esp_get_crosshairs() { return g_espCrosshairs; }
void esp_set_3d_boxes(bool v) { g_esp3DBoxes = v; }
bool esp_get_3d_boxes() { return g_esp3DBoxes; }
void esp_set_3d_box_filled(bool v) { g_esp3DBoxFilled = v; }
bool esp_get_3d_box_filled() { return g_esp3DBoxFilled; }
void esp_set_skeleton(bool v) { g_espSkeleton = v; }
bool esp_get_skeleton() { return g_espSkeleton; }
void esp_set_line_position(int pos) { g_espLinePosition = pos; }
int esp_get_line_position() { return g_espLinePosition; }
void esp_set_debug(bool v) { g_espDebug = v; }
bool esp_get_debug() { return g_espDebug; }
void esp_set_filter_interactive_enabled(bool v) { g_filterInteractiveEnabled = v; }
bool esp_get_filter_interactive_enabled() { return g_filterInteractiveEnabled; }
void esp_set_filter_show_all(bool v) { g_filterShowAll = v; }
bool esp_get_filter_show_all() { return g_filterShowAll; }
void esp_set_filter_chests(bool v) { g_filterChests = v; }
bool esp_get_filter_chests() { return g_filterChests; }
void esp_set_filter_collectables(bool v) { g_filterCollectables = v; }
bool esp_get_filter_collectables() { return g_filterCollectables; }
void esp_set_filter_insects(bool v) { g_filterInsects = v; }
bool esp_get_filter_insects() { return g_filterInsects; }
void esp_set_filter_flowers(bool v) { g_filterFlowers = v; }
bool esp_get_filter_flowers() { return g_filterFlowers; }
void esp_set_filter_holes(bool v) { g_filterHoles = v; }
bool esp_get_filter_holes() { return g_filterHoles; }
void esp_set_filter_campfires(bool v) { g_filterCampfires = v; }
bool esp_get_filter_campfires() { return g_filterCampfires; }
void esp_set_filter_drops(bool v) { g_filterDrops = v; }
bool esp_get_filter_drops() { return g_filterDrops; }
void esp_set_filter_boxes(bool v) { g_filterBoxes = v; }
bool esp_get_filter_boxes() { return g_filterBoxes; }
void esp_set_filter_obstacles(bool v) { g_filterObstacles = v; }
bool esp_get_filter_obstacles() { return g_filterObstacles; }
void test_rva_get_main_camera(RvaTestResult& result) {
    result.name = "RVA_GET_MAIN_CAMERA";
    result.success = false;
    result.message = "";
    __try {
        EnsureGameAssemblyLoaded();
        if (!g_gameAssembly) {
            result.message = "GameAssembly not loaded";
            return;
        }
        typedef void* (__fastcall *GetMainCameraFunc)();
        GetMainCameraFunc getMainCamera = (GetMainCameraFunc)((uintptr_t)g_gameAssembly + DynamicOffsets::rvaGetMainCamera);
        void* camera = getMainCamera();
        if (camera && (uintptr_t)camera > 0x1000) {
            result.success = true;
            result.message = "Camera found";
        } else {
            result.message = "Camera returned NULL or invalid";
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        result.message = "Exception when calling get_main";
    }
}
void test_rva_get_transform(RvaTestResult& result) {
    result.name = "RVA_GET_TRANSFORM";
    result.success = false;
    result.message = "";
    __try {
        EnsureGameAssemblyLoaded();
        if (!g_gameAssembly) {
            result.message = "GameAssembly not loaded";
            return;
        }
        if (g_visibleActors.empty()) {
            result.message = "No actors to test";
            return;
        }
        typedef void* (__fastcall *GetTransformFunc)(void* self);
        GetTransformFunc getTransform = (GetTransformFunc)((uintptr_t)g_gameAssembly + DynamicOffsets::rvaGetTransform);
        void* actor = g_visibleActors[0].actor;
        void* transform = getTransform(actor);
        if (transform && (uintptr_t)transform > 0x1000) {
            result.success = true;
            result.message = "Transform found";
        } else {
            result.message = "Transform returned NULL or invalid";
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        result.message = "Exception when calling get_transform";
    }
}
void test_rva_get_position(RvaTestResult& result) {
    result.name = "RVA_GET_POSITION";
    result.success = false;
    result.message = "";
    __try {
        EnsureGameAssemblyLoaded();
        if (!g_gameAssembly) {
            result.message = "GameAssembly not loaded";
            return;
        }
        if (g_visibleActors.empty()) {
            result.message = "No actors to test";
            return;
        }
        typedef void (__fastcall *GetPositionFunc)(Vector3Struct* out, void* self);
        GetPositionFunc getPosition = (GetPositionFunc)((uintptr_t)g_gameAssembly + DynamicOffsets::rvaGetPosition);
        void* actor = g_visibleActors[0].actor;
        void* transform = *(void**)((uintptr_t)actor + DynamicOffsets::actorModelTransformOffset);
        if (!transform) {
            result.message = "Transform is NULL";
            return;
        }
        Vector3Struct pos;
        getPosition(&pos, transform);
        if (pos.x != 0 || pos.y != 0 || pos.z != 0) {
            result.success = true;
            char msg[128];
            sprintf_s(msg, "Position: (%.1f, %.1f, %.1f)", pos.x, pos.y, pos.z);
            result.message = msg;
        } else {
            result.message = "Position is (0, 0, 0)";
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        result.message = "Exception when calling get_position";
    }
}
void test_rva_get_alive_char_list(RvaTestResult& result) {
    result.name = "RVA_GET_ALIVE_CHAR_LIST";
    result.success = false;
    result.message = "";
    __try {
        EnsureGameAssemblyLoaded();
        if (!g_gameAssembly) {
            result.message = "GameAssembly not loaded";
            return;
        }
        if (!g_characterManagerInstance) {
            result.message = "CharacterManager not found";
            return;
        }
        typedef void* (__fastcall *GetAliveCharListFunc)(void* self);
        GetAliveCharListFunc getAllAliveList = (GetAliveCharListFunc)((uintptr_t)g_gameAssembly + DynamicOffsets::rvaGetAliveCharList);
        void* listObj = getAllAliveList(g_characterManagerInstance);
        if (listObj) {
            result.success = true;
            char msg[128];
            sprintf_s(msg, "%d actors found", (int)g_visibleActors.size());
            result.message = msg;
        } else {
            result.message = "List returned NULL";
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        result.message = "Exception when calling get_AllAliveCharacterList";
    }
}
void test_rva_get_main_player(RvaTestResult& result) {
    result.name = "RVA_GET_MAIN_PLAYER";
    result.success = false;
    result.message = "";
    __try {
        EnsureGameAssemblyLoaded();
        if (!g_gameAssembly) {
            result.message = "GameAssembly not loaded";
            return;
        }
        if (!g_characterManagerInstance) {
            result.message = "CharacterManager not found";
            return;
        }
        typedef void* (__fastcall *GetMainPlayerFunc)(void* self);
        GetMainPlayerFunc getMainPlayer = (GetMainPlayerFunc)((uintptr_t)g_gameAssembly + DynamicOffsets::rvaGetMainPlayer);
        void* mainPlayer = getMainPlayer(g_characterManagerInstance);
        if (mainPlayer && (uintptr_t)mainPlayer > 0x1000) {
            result.success = true;
            result.message = "Main player found";
        } else {
            result.message = "Main player returned NULL or invalid";
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        result.message = "Exception when calling GetMainPlayerActorModel";
    }
}
void test_rva_get_hp(RvaTestResult& result) {
    result.name = "RVA_ACTOR_GET_HP";
    result.success = false;
    result.message = "";
    __try {
        EnsureGameAssemblyLoaded();
        if (!g_gameAssembly) {
            result.message = "GameAssembly not loaded";
            return;
        }
        if (g_visibleActors.empty()) {
            result.message = "No actors to test";
            return;
        }
        typedef int (__fastcall *GetHpFunc)(void* self);
        GetHpFunc getHp = (GetHpFunc)((uintptr_t)g_gameAssembly + DynamicOffsets::rvaGetHp);
        void* actor = g_visibleActors[0].actor;
        int hp = getHp(actor);
        if (hp > 0) {
            result.success = true;
            char msg[128];
            sprintf_s(msg, "HP: %d", hp);
            result.message = msg;
        } else {
            result.message = "HP is 0 or invalid";
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        result.message = "Exception when calling get_Hp";
    }
}
void test_rva_get_shield(RvaTestResult& result) {
    result.name = "RVA_ACTOR_GET_SHIELD";
    result.success = false;
    result.message = "";
    __try {
        EnsureGameAssemblyLoaded();
        if (!g_gameAssembly) {
            result.message = "GameAssembly not loaded";
            return;
        }
        if (g_visibleActors.empty()) {
            result.message = "No actors to test";
            return;
        }
        typedef int (__fastcall *GetShieldFunc)(void* self);
        GetShieldFunc getShield = (GetShieldFunc)((uintptr_t)g_gameAssembly + DynamicOffsets::rvaGetShield);
        void* actor = g_visibleActors[0].actor;
        int shield = getShield(actor);
        if (shield >= 0) {
            result.success = true;
            char msg[128];
            sprintf_s(msg, "Shield: %d", shield);
            result.message = msg;
        } else {
            result.message = "Shield is invalid";
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        result.message = "Exception when calling get_Shield";
    }
}
void test_rva_is_local_client(RvaTestResult& result) {
    result.name = "RVA_IS_LOCAL_CLIENT";
    result.success = false;
    result.message = "";
    __try {
        EnsureGameAssemblyLoaded();
        if (!g_gameAssembly) {
            result.message = "GameAssembly not loaded";
            return;
        }
        if (g_visibleActors.empty()) {
            result.message = "No actors to test";
            return;
        }
        typedef bool (__fastcall *IsLocalClientFunc)(void* self);
        IsLocalClientFunc isLocalClient = (IsLocalClientFunc)((uintptr_t)g_gameAssembly + DynamicOffsets::rvaIsLocalClient);
        void* actor = g_visibleActors[0].actor;
        bool isLocal = isLocalClient(actor);
        result.success = true;
        char msg[128];
        sprintf_s(msg, "Is Local: %s", isLocal ? "Yes" : "No");
        result.message = msg;
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        result.message = "Exception when calling IsLocalClient";
    }
}