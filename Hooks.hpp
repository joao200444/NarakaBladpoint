#pragma once
#include <Windows.h>
#include <iostream>
#include "Cheats.hpp"

// Includes das features (arquivos removidos)
// #include "features/SmartGodMode.hpp" // REMOVIDO
// #include "features/SpeedHack.hpp" // REMOVIDO
// #include "features/NoCooldown.hpp" // REMOVIDO
// #include "features/AttackSpeed.hpp" // REMOVIDO
// #include "features/InstantRangedAOE.hpp" // REMOVIDO
// #include "features/InfiniteJump.hpp" // REMOVIDO
// #include "features/AOEHit.hpp" // REMOVIDO
// #include "features/TrackBarFreeze.hpp" // REMOVIDO
// #include "features/InstantFishing.hpp" // REMOVIDO
// #include "features/AutoFishing.hpp" // REMOVIDO
// #include "features/TeleportToChest.hpp" // REMOVIDO
// #include "features/InfiniteStamina.hpp" // REMOVIDO
// #include "features/FOV.hpp" // REMOVIDO
// #include "features/DamageMultiplier.hpp" // REMOVIDO
// #include "features/MobVacuum.hpp" // REMOVIDO
// #include "features/SkipCutscene.hpp" // REMOVIDO
// #include "features/UnityLogSuppressor.hpp" // REMOVIDO

// Sistema de Hooks - Instala todas as features
namespace Hooks {

    static HMODULE GameAssembly_Super = nullptr;

    // Instalar God Mode
    void InstallAll(HMODULE GameAssembly_SuperModule) {
        GameAssembly_Super = GameAssembly_SuperModule;
        
        printf("[DEBUG] Hooks::InstallAll() - TODOS OS HOOKS DESATIVADOS\n");
        
        // TODOS OS HOOKS DESATIVADOS PARA EVITAR CRASH
        /*
        // PAUSADO TEMPORARIAMENTE - BLOQUEAR LOGS DO GAME (usa offsets)
        // Features::UnityLogSuppressor::Install(GameAssembly_SuperModule);
        
        // REATIVADO - God Mode
        Features::SmartGodMode::Install(GameAssembly_SuperModule);
        
        // REATIVADO - Speed Hack
        Features::SpeedHack::Install(GameAssembly_SuperModule);
        
        // PAUSADO TEMPORARIAMENTE - Damage Multiplier (usa offsets)
        // Features::DamageMultiplier::Install(GameAssembly_SuperModule);
        
        // DESATIVADO - Damage Hack (Old Version)
        // Features::DamageHack::Install(GameAssembly_SuperModule);
        
        // PAUSADO TEMPORARIAMENTE - Mob Vacuum (usa offsets)
        // Features::MobVacuum::Install(GameAssembly_SuperModule);
        
        // PAUSADO TEMPORARIAMENTE - Skip Cutscene & Dialog (usa offsets)
        // Features::SkipCutscene::Install(GameAssembly_SuperModule);
        
        // FOV Changer - REATIVADO
        Features::FOV::Install(GameAssembly_SuperModule);

        // PAUSADO TEMPORARIAMENTE - NoClip (usa offsets)
        // Features::NoClip::Install(GameAssembly_SuperModule);
        
        // PAUSADO TEMPORARIAMENTE - AOE Hit System (usa offsets)
        // Features::AOESystem::Install(GameAssembly_SuperModule);
        
        // REATIVADO - Attack Speed (HOOKS DESABILITADOS - AGUARDANDO ATUALIZAÇÃO DOS RVAs)
        Features::AttackSpeed::Install(GameAssembly_SuperModule);
        
        // DESATIVADO - Infinite Stamina (EM MANUTENÇÃO)
        // Features::InfiniteStamina::Install(GameAssembly_SuperModule);
        
        // REATIVADO - Infinite Jump
        InfiniteJumpHooks::Install(GameAssembly_SuperModule);
        
        // TODAS AS OUTRAS FEATURES DESATIVADAS

        //
        // Features::CriticalHitUltimate::Install(GameAssembly_SuperModule);
        // Features::InstantRangedAOE::Install(GameAssembly_SuperModule);
        // Features::NoCooldown::Install(GameAssembly_SuperModule);
        // Features::InstantFishing::Install(GameAssembly_SuperModule);
        */
    }

    void UninstallAll() {
        // TODOS OS HOOKS DESATIVADOS - Nada para desinstalar
        printf("[DEBUG] Hooks::UninstallAll() - Nada a desinstalar\n");
    }
}
