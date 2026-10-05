#pragma once

#include <cstdint>

// ============================================================================
// RVAs Dinâmicos - Editáveis via aba Development
// ============================================================================

namespace DynamicOffsets {
    // Unity Methods
    extern uintptr_t rvaGetMainCamera;
    extern uintptr_t rvaGetTransform;
    extern uintptr_t rvaGetPosition;
    extern uintptr_t rvaSetPosition;
    extern uintptr_t rvaGetEulerAngles;
    extern uintptr_t rvaSetEulerAngles;
    extern uintptr_t rvaWorldToScreen;
    extern uintptr_t rvaFindObject;
    extern uintptr_t rvaGetViewMatrix;
    extern uintptr_t rvaGetProjectionMatrix;

    // Game Methods (Naraka Bladepoint)
    extern uintptr_t rvaGetAliveCharList;
    extern uintptr_t rvaGetMainPlayer;
    extern uintptr_t rvaGetModelType;
    extern uintptr_t rvaGetActorModelFid;
    extern uintptr_t rvaGetLogicBoneTransform;
    extern uintptr_t rvaIsLocalClient;
    extern uintptr_t rvaGetHp;
    extern uintptr_t rvaGetHpMax;
    extern uintptr_t rvaGetShield;
    extern uintptr_t rvaGetShieldMax;

    // Offsets
    extern int actorModelTransformOffset;
    extern int blockParryAgentOffset;

    // Função para inicializar com valores do offsets.h
    void Initialize();

    // Função para atualizar RVAs (chamada pelo botão Reconectar)
    void UpdateFromUI(
        uintptr_t rvaGetMainCamera_,
        uintptr_t rvaGetTransform_,
        uintptr_t rvaGetPosition_,
        uintptr_t rvaSetPosition_,
        uintptr_t rvaGetEulerAngles_,
        uintptr_t rvaSetEulerAngles_,
        uintptr_t rvaWorldToScreen_,
        uintptr_t rvaFindObject_,
        uintptr_t rvaGetViewMatrix_,
        uintptr_t rvaGetProjectionMatrix_,
        uintptr_t rvaGetAliveCharList_,
        uintptr_t rvaGetMainPlayer_,
        uintptr_t rvaGetModelType_,
        uintptr_t rvaGetActorModelFid_,
        uintptr_t rvaGetLogicBoneTransform_,
        uintptr_t rvaIsLocalClient_,
        uintptr_t rvaGetHp_,
        uintptr_t rvaGetHpMax_,
        uintptr_t rvaGetShield_,
        uintptr_t rvaGetShieldMax_,
        int actorModelTransformOffset_,
        int blockParryAgentOffset_
    );
}
