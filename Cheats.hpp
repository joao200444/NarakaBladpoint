#pragma once

// Namespace centralizado para configurações de cheats
namespace Cheats {
    
    // AOE Hit System
    namespace AOEHit {
        inline bool enabled = false;
        inline float GetRadius() { return 5.0f; }
    }
    
    // FOV Changer
    namespace FOVChanger {
        inline bool enabled = false;
    }
    
    // Track Bar Freeze
    namespace TrackBarFreeze {
        inline bool enabled = false;
    }
    
    // Speed Hack
    namespace SpeedHack {
        inline bool enabled = false;
        inline float speedMultiplier = 1.0f;
    }
    
    // Attack Speed
    namespace AttackSpeed {
        inline bool enabled = false;
        inline float attackSpeedMultiplier = 1.0f; // Máximo seguro: 10x (não 15x!)
    }
    
    // Infinite Jump
    namespace InfiniteJump {
        inline bool enabled = false;
    }
    
    // God Mode
    namespace GodMode {
        inline bool enabled = false;
    }
    
    // No Cooldown
    namespace NoCooldown {
        inline bool enabled = false;
    }
    
    // Damage Multiplier
    namespace DamageMultiplier {
        inline bool enabled = false;
        inline float multiplier = 9999.0f; // Multiplicador de dano
    }

    // Damage Hack (Old Version) - EM MANUTENÇÃO
    namespace DamageHack {
        inline bool enabled = false;
        inline float multiplier = 9999.0f;
    }
    
    // Mob Vacuum - Updated with distance controls
    namespace MobVacuum {
        inline bool enabled = false;
        inline bool pullEnemies = true; // Opção para puxar inimigos
        inline bool pullDrops = true; // Opção para puxar drops
        inline float range = 40.0f; // Range em metros
        inline float vacuumDistance = 3.0f; // Distância do player (1-10m)
        inline float heightOffset = 0.0f; // Offset de altura
        inline bool showDistance = false; // Mostrar distância na tela
    }
    
    // Critical Hit
    namespace CriticalHit {
        inline bool enabled = false;
    }
    
    // Instant Ranged AOE
    namespace InstantRangedAOE {
        inline bool enabled = false;
        inline int rangeMultiplier = 10;
    }
    
    // Instant Fishing
    namespace InstantFishing {
        inline bool enabled = false;
        inline float speedMultiplier = 10.0f;
    }
    
    // Auto Fishing
    namespace AutoFishing {
        inline bool enabled = false;
        inline bool autoCast = true;
        inline bool autoReel = true;
        inline bool autoPerfect = false;
        inline float reactionTime = 0.5f;
    }
    
    // Teleport To Chest
    namespace TeleportToChest {
        inline bool enabled = false;
        inline bool checkPosition = false; // ✅ NOVO: Verificador de posição do player
    }
    
    // Infinite Stamina
    namespace InfiniteStamina {
        inline bool enabled = false;
        inline float staminaValue = 999999.0f;
    }
    
    // FOV (Field of View)
    namespace FOV {
        inline bool enabled = false;
        inline float fovValue = 90.0f; // FOV padrão é geralmente 60-75, 90 é mais amplo
    }
    

    
    // Gift Box Duplicator
    namespace GiftBoxDuplicator {
        inline bool enabled = false;
        inline int duplicateCount = 5;
        inline int delayMs = 100;
    }
    
    // Kill Aura
    namespace KillAura {
        inline bool enabled = false;
        inline float radius = 100.0f;
    }
    
    // Skip Cutscene & Dialog
    namespace SkipCutscene {
        inline bool enabled = false;
        inline bool autoSkip = false; // Auto-skip quando detectar cutscene/dialog
    }

    // Auto Pickup
    namespace AutoPickup {
        inline bool enabled = false;
        inline float range = 15.0f;
    }

    // NoClip
    namespace NoClip {
        inline bool enabled = false;
        inline float flySpeed = 10.0f;
    }

    // AutoSpin
    namespace AutoSpin {
        inline bool enabled = false;
        inline float speed = 10.0f; // 0 a 90 graus por frame
    }

    // Dump Enemies Magnet
    namespace DumpEnemiesMagnet {
        inline bool enabled = false;
        inline float distance = 3.0f;           // Distância da câmera (1-20m)
        inline bool freezeHeight = false;       // Manter altura original
        inline bool excludeLocalPlayer = true;  // Excluir o jogador local
        inline bool debug = false;              // Modo debug
    }

}
