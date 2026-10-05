#pragma once
#include "vec.h"
#include <cstdint>
#include <vector>
#include "offsets.h"  // RVAs centralizados

// Forward declaration
struct ActorData;
struct ImDrawList;
struct ImVec2;

// Variável global exportada para o aimbot
extern void* g_characterManagerInstance;

// Vector global para compartilhar atores visíveis com aimbot, autoparry, noclip e autospin
extern std::vector<ActorData> g_visibleActors;

// Função para obter array de ActorModels (usada pelo ESP, NoClip e AutoSpin)
void** GetAllActorModels(int& count);

void esp_render();
void esp_set_enabled(bool v);
bool esp_get_enabled();
void esp_set_filter(bool showChars, bool showMonsters);
void esp_get_filter(bool& showChars, bool& showMonsters);
void esp_set_box(float w, float h);
void esp_get_box(float& w, float& h);
void esp_set_type_filter(int typeId, bool enabled);
bool esp_get_type_filter(int typeId);

bool esp_get_entity_type_value(const char* fieldName, int& out);

void esp_set_auto_open_boxes(bool v);
bool esp_get_auto_open_boxes();
void esp_set_auto_open_radius(float r);
float esp_get_auto_open_radius();

void esp_set_debug(bool v);
bool esp_get_debug();

void esp_set_lines(bool v);
bool esp_get_lines();
void esp_set_names(bool v);
bool esp_get_names();
void esp_set_distance(bool v);
bool esp_get_distance();

// ESP Visual Types Functions
void esp_set_boxes(bool v);
bool esp_get_boxes();
void esp_set_circles(bool v);
bool esp_get_circles();
void esp_set_triangles(bool v);
bool esp_get_triangles();
void esp_set_dots(bool v);
bool esp_get_dots();
void esp_set_crosshairs(bool v);
bool esp_get_crosshairs();
void esp_set_3d_boxes(bool v);
bool esp_get_3d_boxes();
void esp_set_3d_box_filled(bool v);
bool esp_get_3d_box_filled();
void esp_set_skeleton(bool v);
bool esp_get_skeleton();

// ESP Line Position Functions
void esp_set_line_position(int pos);
int esp_get_line_position();

// Interactive Object Filter Functions
void esp_set_filter_interactive_enabled(bool v);
bool esp_get_filter_interactive_enabled();
void esp_set_filter_show_all(bool v);
bool esp_get_filter_show_all();
void esp_set_filter_chests(bool v);
bool esp_get_filter_chests();
void esp_set_filter_collectables(bool v);
bool esp_get_filter_collectables();
void esp_set_filter_insects(bool v);
bool esp_get_filter_insects();
void esp_set_filter_flowers(bool v);
bool esp_get_filter_flowers();
void esp_set_filter_holes(bool v);
bool esp_get_filter_holes();
void esp_set_filter_campfires(bool v);
bool esp_get_filter_campfires();
void esp_set_filter_drops(bool v);
bool esp_get_filter_drops();
void esp_set_filter_boxes(bool v);
bool esp_get_filter_boxes();
void esp_set_filter_obstacles(bool v);
bool esp_get_filter_obstacles();

// Functions for Radar system
bool get_camera_position(Vector3& pos);
void** get_zentities_array(size_t& count);
bool get_entity_type(void* ent, int& type);
bool get_entity_position(void* ent, Vector3& pos);
bool get_entity_name(void* ent, char* nameBuffer, size_t bufferSize);
unsigned int get_color_for_type(int type);

// Funções de teste de RVA para debug
struct RvaTestResult {
    const char* name;
    bool success;
    const char* message;
};

void test_rva_get_main_camera(RvaTestResult& result);
void test_rva_get_transform(RvaTestResult& result);
void test_rva_get_position(RvaTestResult& result);
void test_rva_get_alive_char_list(RvaTestResult& result);
void test_rva_get_main_player(RvaTestResult& result);
void test_rva_get_hp(RvaTestResult& result);
void test_rva_get_shield(RvaTestResult& result);
void test_rva_is_local_client(RvaTestResult& result);






