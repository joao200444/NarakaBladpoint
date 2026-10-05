#include <Windows.h>
#include <fstream>
#include <iostream>
#include <string>
#include "esp.h"
#include "Cheats.hpp"
// #include "features/Radar.hpp" // TEMPORARIAMENTE DESABILITADO

// using json = nlohmann::json; // TEMPORARIAMENTE DESABILITADO

const std::string CONFIG_FILE = "smunk_config.json";
static bool g_configLoaded = false;

void esp_save_config()
{
    // SISTEMA TEMPORARIAMENTE DESABILITADO PARA EVITAR CRASHES
    printf("[Config] Save function temporarily disabled to prevent crashes\n");
    return;
    
    /*
    try {
        // json config; // DESABILITADO
        
        // ========================================
        // COMBAT SETTINGS
        // ========================================
        config["combat"]["god_mode"] = Cheats::GodMode::enabled;
        config["combat"]["attack_speed"] = Cheats::AttackSpeed::enabled;
        config["combat"]["attack_speed_multiplier"] = Cheats::AttackSpeed::attackSpeedMultiplier;

        config["combat"]["no_cooldown"] = Cheats::NoCooldown::enabled;
        // config["combat"]["critical_hit"] = Cheats::CriticalHit::enabled;
        config["combat"]["instant_ranged_aoe"] = Cheats::InstantRangedAOE::enabled;
        config["combat"]["instant_ranged_aoe_range"] = Cheats::InstantRangedAOE::rangeMultiplier;
        
        // ========================================
        // MOVEMENT SETTINGS
        // ========================================
        config["movement"]["speed_hack"] = Cheats::SpeedHack::enabled;
        config["movement"]["speed_multiplier"] = Cheats::SpeedHack::speedMultiplier;
        config["movement"]["infinite_jump"] = Cheats::InfiniteJump::enabled;
        config["movement"]["infinite_stamina"] = Cheats::InfiniteStamina::enabled;
        
        // ========================================
        // FARMING SETTINGS
        // ========================================
        config["farming"]["instant_fishing"] = Cheats::InstantFishing::enabled;
        config["farming"]["instant_fishing_speed"] = Cheats::InstantFishing::speedMultiplier;
        config["farming"]["gift_box_duplicator"] = Cheats::GiftBoxDuplicator::enabled;
        
        // ========================================
        // VISUAL SETTINGS
        // ========================================
        // ESP Main Settings
        config["visual"]["esp_enabled"] = esp_get_enabled();
        config["visual"]["esp_lines"] = esp_get_lines();
        config["visual"]["esp_names"] = esp_get_names();
        config["visual"]["esp_distance"] = esp_get_distance();
        config["visual"]["esp_line_position"] = esp_get_line_position();
        
        // ESP Visual Types
        config["visual"]["esp_boxes"] = esp_get_boxes();
        config["visual"]["esp_circles"] = esp_get_circles();
        config["visual"]["esp_triangles"] = esp_get_triangles();
        config["visual"]["esp_dots"] = esp_get_dots();
        config["visual"]["esp_crosshairs"] = esp_get_crosshairs();
        
        // ESP Box Settings
        float w, h;
        esp_get_box(w, h);
        config["visual"]["esp_box_width"] = w;
        config["visual"]["esp_box_height"] = h;
        
        // ESP Entity Filters
        json filters;
        for (int i = 0; i < 64; i++) {
            filters[std::to_string(i)] = esp_get_type_filter(i);
        }
        config["visual"]["esp_filters"] = filters;
        
        // FOV Settings
        config["visual"]["fov_enabled"] = Cheats::FOV::enabled;
        config["visual"]["fov_value"] = Cheats::FOV::fovValue;
        
        // Radar Settings
        config["visual"]["radar_enabled"] = Features::Radar::IsEnabled();
        config["visual"]["radar_size"] = Features::Radar::GetSize();
        config["visual"]["radar_range"] = Features::Radar::GetRange();
        
        // Write to file
        std::ofstream file(CONFIG_FILE);
        file << config.dump(4);
        file.close();
        
        printf("[Smunk Config] All settings saved to %s\n", CONFIG_FILE.c_str());
    }
    catch (const std::exception& e) {
        printf("[Smunk Config] Error saving config: %s\n", e.what());
    }
    */ // FIM DO CÓDIGO DESABILITADO
}

void esp_load_config()
{
    // SISTEMA TEMPORARIAMENTE DESABILITADO PARA EVITAR CRASHES
    printf("[Config] Load function temporarily disabled to prevent crashes\n");
    g_configLoaded = true;
    return;
    
    /*
    // Only load config after successful login
    if (g_configLoaded) {
        return; // Already loaded
    }
    
    try {
        std::ifstream file(CONFIG_FILE);
        if (!file.is_open()) {
            printf("[Smunk Config] Config file not found, using defaults\n");
            g_configLoaded = true; // Mark as loaded even if file doesn't exist
            return;
        }
        
        json config;
        file >> config;
        file.close();
        
        // ========================================
        // COMBAT SETTINGS
        // ========================================
        if (config.contains("combat")) {
            json combat = config["combat"];
            if (combat.contains("god_mode"))
                Cheats::GodMode::enabled = combat["god_mode"];
            if (combat.contains("attack_speed"))
                Cheats::AttackSpeed::enabled = combat["attack_speed"];
            if (combat.contains("attack_speed_multiplier"))
                Cheats::AttackSpeed::attackSpeedMultiplier = combat["attack_speed_multiplier"];
            if (combat.contains("multi_attack"))

            if (combat.contains("no_cooldown"))
                Cheats::NoCooldown::enabled = combat["no_cooldown"];
            // if (combat.contains("critical_hit"))
            //     Cheats::CriticalHit::enabled = combat["critical_hit"];
            if (combat.contains("instant_ranged_aoe"))
                Cheats::InstantRangedAOE::enabled = combat["instant_ranged_aoe"];
            if (combat.contains("instant_ranged_aoe_range"))
                Cheats::InstantRangedAOE::rangeMultiplier = combat["instant_ranged_aoe_range"];
        }
        
        // ========================================
        // MOVEMENT SETTINGS
        // ========================================
        if (config.contains("movement")) {
            json movement = config["movement"];
            if (movement.contains("speed_hack"))
                Cheats::SpeedHack::enabled = movement["speed_hack"];
            if (movement.contains("speed_multiplier"))
                Cheats::SpeedHack::speedMultiplier = movement["speed_multiplier"];
            if (movement.contains("infinite_jump"))
                Cheats::InfiniteJump::enabled = movement["infinite_jump"];
            if (movement.contains("infinite_stamina"))
                Cheats::InfiniteStamina::enabled = movement["infinite_stamina"];
        }
        
        // ========================================
        // FARMING SETTINGS
        // ========================================
        if (config.contains("farming")) {
            json farming = config["farming"];
            if (farming.contains("instant_fishing"))
                Cheats::InstantFishing::enabled = farming["instant_fishing"];
            if (farming.contains("instant_fishing_speed"))
                Cheats::InstantFishing::speedMultiplier = farming["instant_fishing_speed"];
            if (farming.contains("gift_box_duplicator"))
                Cheats::GiftBoxDuplicator::enabled = farming["gift_box_duplicator"];
        }
        
        // ========================================
        // VISUAL SETTINGS
        // ========================================
        if (config.contains("visual")) {
            json visual = config["visual"];
            
            // ESP Main Settings
            if (visual.contains("esp_enabled"))
                esp_set_enabled(visual["esp_enabled"]);
            if (visual.contains("esp_lines"))
                esp_set_lines(visual["esp_lines"]);
            if (visual.contains("esp_names"))
                esp_set_names(visual["esp_names"]);
            if (visual.contains("esp_distance"))
                esp_set_distance(visual["esp_distance"]);
            if (visual.contains("esp_line_position"))
                esp_set_line_position(visual["esp_line_position"]);
                
            // ESP Visual Types
            if (visual.contains("esp_boxes"))
                esp_set_boxes(visual["esp_boxes"]);
            if (visual.contains("esp_circles"))
                esp_set_circles(visual["esp_circles"]);
            if (visual.contains("esp_triangles"))
                esp_set_triangles(visual["esp_triangles"]);
            if (visual.contains("esp_dots"))
                esp_set_dots(visual["esp_dots"]);
            if (visual.contains("esp_crosshairs"))
                esp_set_crosshairs(visual["esp_crosshairs"]);
                
            // ESP Box Settings
            if (visual.contains("esp_box_width") && visual.contains("esp_box_height")) {
                float w = visual["esp_box_width"];
                float h = visual["esp_box_height"];
                esp_set_box(w, h);
            }
            
            // ESP Entity Filters
            if (visual.contains("esp_filters")) {
                json filters = visual["esp_filters"];
                for (int i = 0; i < 64; i++) {
                    std::string key = std::to_string(i);
                    if (filters.contains(key)) {
                        esp_set_type_filter(i, filters[key]);
                    }
                }
            }
            
            // FOV Settings
            if (visual.contains("fov_enabled"))
                Cheats::FOV::enabled = visual["fov_enabled"];
            if (visual.contains("fov_value"))
                Cheats::FOV::fovValue = visual["fov_value"];
                
            // Radar Settings
            if (visual.contains("radar_enabled"))
                Features::Radar::SetEnabled(visual["radar_enabled"]);
            if (visual.contains("radar_size"))
                Features::Radar::SetSize(visual["radar_size"]);
            if (visual.contains("radar_range"))
                Features::Radar::SetRange(visual["radar_range"]);
        }
        
        printf("[Smunk Config] All settings loaded from %s\n", CONFIG_FILE.c_str());
        g_configLoaded = true; // Mark as successfully loaded
    }
    catch (const std::exception& e) {
        printf("[Smunk Config] Error loading config: %s\n", e.what());
        g_configLoaded = true; // Mark as loaded even with error to prevent retry
    }
    */ // FIM DO CÓDIGO DESABILITADO
}

void esp_auto_save_config()
{
    // SISTEMA TEMPORARIAMENTE DESABILITADO PARA EVITAR CRASHES
    return;
    
    /*
    static unsigned long long lastSaveTime = 0;
    unsigned long long currentTime = GetTickCount64();
    
    // Only auto-save if config has been loaded (user is logged in)
    if (!g_configLoaded) {
        return;
    }
    
    // Auto-save every 10 seconds (increased due to more data)
    if (currentTime - lastSaveTime > 10000) {
        esp_save_config();
        lastSaveTime = currentTime;
    }
    */
}

void esp_load_config_after_login()
{
    // SISTEMA TEMPORARIAMENTE DESABILITADO PARA EVITAR CRASHES
    printf("[Config] Load after login temporarily disabled to prevent crashes\n");
    g_configLoaded = true;
    return;
    
    /*
    if (!g_configLoaded) {
        esp_load_config();
        printf("[Smunk Config] Configuration loaded after successful login\n");
    }
    */
}