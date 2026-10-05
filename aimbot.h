#pragma once

#include <vector>

// Forward declarations - usar as estruturas do projeto
struct Vector3;
struct Vector2;

// Estrutura para compartilhar dados de atores entre ESP e Aimbot
struct ActorData {
    void* actor;
    float worldPosX, worldPosY, worldPosZ;
    float screenPosX, screenPosY;
    float distance;
};

// Aimbot settings
void aimbot_set_enabled(bool v);
bool aimbot_get_enabled();

void aimbot_set_fov(float v);
float aimbot_get_fov();

void aimbot_set_min_distance(float v);
float aimbot_get_min_distance();

void aimbot_set_smooth(float v);
float aimbot_get_smooth();

void aimbot_set_strength(float v);
float aimbot_get_strength();

void aimbot_set_debug(bool v);
bool aimbot_get_debug();

void aimbot_set_height_offset(float v);
float aimbot_get_height_offset();

void aimbot_set_hide_fov(bool v);
bool aimbot_get_hide_fov();

// Atualizar lista de atores visíveis (chamado pelo ESP)
void aimbot_update_actors(const std::vector<ActorData>& actors);

// Aimbot render (desenhar FOV)
void aimbot_render();

// Aimbot tick (chamado a cada frame)
void aimbot_tick();
