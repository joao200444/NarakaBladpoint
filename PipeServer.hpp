#pragma once
#include <windows.h>
#include <string>
#include <thread>
#include <vector>
#include <sstream>
#include "Cheats.hpp"
#include "esp.h"
// #include "features/tp.hpp" // REMOVIDO
// #include "features/AOEHit.hpp" // REMOVIDO

namespace PipeServer {
    inline HANDLE hPipe = INVALID_HANDLE_VALUE;
    inline bool running = true;

    // Helper para processar comandos
    inline void ProcessCommand(const std::string& cmd) {
        if (cmd.empty()) return;

        std::stringstream ss(cmd);
        std::string feature, enabledStr, valueStr;

        if (!std::getline(ss, feature, ':')) return;
        if (!std::getline(ss, enabledStr, ':')) return;
        if (!std::getline(ss, valueStr, ':')) valueStr = "0";

        bool enabled = (enabledStr == "1");
        float value = 0;
        try {
            value = std::stof(valueStr);
        } catch (...) {}

        // Mapeamento de Features - ESP
        if (feature == "ESP") esp_set_enabled(enabled);
        else if (feature == "ESP_BOX") esp_set_boxes(enabled);
        else if (feature == "ESP_3DBOX") esp_set_3d_boxes(enabled);
        else if (feature == "ESP_3DBOX_FILLED") esp_set_3d_box_filled(enabled);
        else if (feature == "ESP_NAME") esp_set_names(enabled);
        else if (feature == "ESP_LINE") esp_set_lines(enabled);
        else if (feature == "ESP_DISTANCE") esp_set_distance(enabled);
        else if (feature == "ESP_LINE_POS") esp_set_line_position((int)value);
        else if (feature == "ESP_CIRCLE") esp_set_circles(enabled);
        else if (feature == "ESP_TRIANGLE") esp_set_triangles(enabled);
        else if (feature == "ESP_DOT") esp_set_dots(enabled);
        else if (feature == "ESP_CROSSHAIR") esp_set_crosshairs(enabled);
        
        // ESP Filters (Entity Types)
        else if (feature == "FILTER_CHARACTER") esp_set_type_filter(8, enabled);
        else if (feature == "FILTER_ENEMY") esp_set_type_filter(16, enabled);
        else if (feature == "FILTER_NPC") esp_set_type_filter(256, enabled);
        else if (feature == "FILTER_INTERACTIVE") esp_set_type_filter(32, enabled);
        else if (feature == "FILTER_PROJECTILE") esp_set_type_filter(64, enabled);
        else if (feature == "FILTER_CREATURE") esp_set_type_filter(4096, enabled);
        else if (feature == "FILTER_ENEMYPART") esp_set_type_filter(16384, enabled);
        else if (feature == "FILTER_GODENTITY") esp_set_type_filter(8192, enabled);
        else if (feature == "FILTER_SOCIALBUILDING") esp_set_type_filter(32768, enabled);
        
        // Interactive Sub-Filters
        else if (feature == "INTERACTIVE_ENABLED") esp_set_filter_interactive_enabled(enabled);
        else if (feature == "INTERACTIVE_SHOWALL") esp_set_filter_show_all(enabled);
        else if (feature == "INTERACTIVE_CHESTS") esp_set_filter_chests(enabled);
        else if (feature == "INTERACTIVE_COLLECTABLES") esp_set_filter_collectables(enabled);
        else if (feature == "INTERACTIVE_INSECTS") esp_set_filter_insects(enabled);
        else if (feature == "INTERACTIVE_FLOWERS") esp_set_filter_flowers(enabled);
        else if (feature == "INTERACTIVE_HOLES") esp_set_filter_holes(enabled);
        else if (feature == "INTERACTIVE_CAMPFIRES") esp_set_filter_campfires(enabled);
        else if (feature == "INTERACTIVE_DROPS") esp_set_filter_drops(enabled);
        else if (feature == "INTERACTIVE_BOXES") esp_set_filter_boxes(enabled);
        else if (feature == "INTERACTIVE_OBSTACLES") esp_set_filter_obstacles(enabled);
        
        // Player / Movement
        else if (feature == "INFINITEJUMP") Cheats::InfiniteJump::enabled = enabled;
        else if (feature == "NOCLIP") Cheats::NoClip::enabled = enabled;
        else if (feature == "NOCLIP_SPEED") Cheats::NoClip::flySpeed = value;
        else if (feature == "GODMODE") Cheats::GodMode::enabled = enabled;
        else if (feature == "SPEEDHACK") {
            Cheats::SpeedHack::enabled = enabled;
            Cheats::SpeedHack::speedMultiplier = value;
        }
        else if (feature == "SPEEDHACK_VALUE") Cheats::SpeedHack::speedMultiplier = value;
        
        // Combat
        else if (feature == "KILLAURA") Cheats::KillAura::enabled = enabled;
        else if (feature == "KILLAURA_RADIUS") Cheats::KillAura::radius = value;
        else if (feature == "DAMAGEMULTIPLIER") Cheats::DamageMultiplier::enabled = enabled;
        else if (feature == "DAMAGEMULTIPLIER_VALUE") Cheats::DamageMultiplier::multiplier = value;
        else if (feature == "ATTACKSPEED") Cheats::AttackSpeed::enabled = enabled;
        else if (feature == "ATTACKSPEED_VALUE") Cheats::AttackSpeed::attackSpeedMultiplier = value;
        else if (feature == "CRITICALHIT") Cheats::CriticalHit::enabled = enabled;
        // else if (feature == "AOEHIT") { // REMOVIDO
        //     Cheats::AOEHit::enabled = enabled;
        // }
        // else if (feature == "AOEHIT_RADIUS") ::Features::AOESystem::SetRadius(value); // REMOVIDO
        
        // Farming / Teleport (features removidas - comentadas)
        // else if (feature == "MOBVACUUM") Cheats::MobVacuum::enabled = enabled;
        // else if (feature == "MOBVACUUM_DISTANCE") Cheats::MobVacuum::vacuumDistance = value;
        // else if (feature == "MOBVACUUM_RANGE") Cheats::MobVacuum::range = value;
        // else if (feature == "MOBVACUUM_HEIGHT") Cheats::MobVacuum::heightOffset = value;
        // else if (feature == "SKIP_CUTSCENE") Cheats::SkipCutscene::enabled = enabled;
        // else if (feature == "ITEMREPLICATOR") Cheats::GiftBoxDuplicator::enabled = enabled;
        // else if (feature == "ITEMREPLICATOR_COUNT") Cheats::GiftBoxDuplicator::duplicateCount = (int)value;
        // else if (feature == "CHECKPOSITION") Cheats::TeleportToChest::checkPosition = enabled;
        // else if (feature == "TELEPORT_NEAREST") {
        //     // Ativa o sinal de teleporte que é processado no Features::Teleport::Update()
        //     Cheats::TeleportToChest::enabled = true;
        // }
        
        // Visuals (features removidas - comentadas)
        // else if (feature == "FOVCHANGER") Cheats::FOV::enabled = enabled;
        // else if (feature == "FOV_VALUE") Cheats::FOV::fovValue = value;
        
        // INIT command from UI
        else if (feature == "INIT") {
            printf("[Pipe] UI requested INIT signal\n");
        }
    }

    inline void ServerThread() {
        while (running) {
            hPipe = CreateNamedPipeA(
                "\\\\.\\pipe\\SmunkPipe",
                PIPE_ACCESS_DUPLEX,
                PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
                1, 1024, 1024, 0, NULL
            );

            if (hPipe == INVALID_HANDLE_VALUE) {
                Sleep(1000);
                continue;
            }

            if (ConnectNamedPipe(hPipe, NULL) ? TRUE : (GetLastError() == ERROR_PIPE_CONNECTED)) {
                char buffer[2048];
                DWORD bytesRead;

                while (ReadFile(hPipe, buffer, sizeof(buffer) - 1, &bytesRead, NULL) && bytesRead > 0) {
                    buffer[bytesRead] = '\0';
                    std::string data(buffer);
                    
                    std::stringstream ss(data);
                    std::string line;
                    while (std::getline(ss, line, '\n')) {
                        if (!line.empty()) {
                            ProcessCommand(line);
                        }
                    }
                }
            }
            CloseHandle(hPipe);
        }
    }

    inline void Start() {
        std::thread(ServerThread).detach();
    }
}
