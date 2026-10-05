#include "dynamic_offsets.h"
#include "offsets.h"

namespace DynamicOffsets {
    // Unity Methods
    uintptr_t rvaGetMainCamera = RVA_GET_MAIN_CAMERA;
    uintptr_t rvaGetTransform = RVA_GET_TRANSFORM;
    uintptr_t rvaGetPosition = RVA_GET_POSITION;
    uintptr_t rvaSetPosition = RVA_SET_POSITION;
    uintptr_t rvaGetEulerAngles = RVA_GET_EULER_ANGLES;
    uintptr_t rvaSetEulerAngles = RVA_SET_EULER_ANGLES;
    uintptr_t rvaWorldToScreen = RVA_WORLD_TO_SCREEN;
    uintptr_t rvaFindObject = RVA_FIND_OBJECTS_OF_TYPE;
    uintptr_t rvaGetViewMatrix = RVA_GET_VIEW_MATRIX;
    uintptr_t rvaGetProjectionMatrix = RVA_GET_PROJECTION_MATRIX;

    // Game Methods (Naraka Bladepoint)
    uintptr_t rvaGetAliveCharList = RVA_GET_ALIVE_CHAR_LIST;
    uintptr_t rvaGetMainPlayer = RVA_GET_MAIN_PLAYER;
    uintptr_t rvaGetModelType = RVA_GET_MODEL_TYPE;
    uintptr_t rvaGetActorModelFid = RVA_GET_ACTOR_MODEL_FID;
    uintptr_t rvaGetLogicBoneTransform = RVA_GET_LOGIC_BONE_TRANSFORM;
    uintptr_t rvaIsLocalClient = RVA_IS_LOCAL_CLIENT;
    uintptr_t rvaGetHp = RVA_ACTOR_GET_HP;
    uintptr_t rvaGetHpMax = RVA_ACTOR_GET_HP_MAX;
    uintptr_t rvaGetShield = RVA_ACTOR_GET_SHIELD;
    uintptr_t rvaGetShieldMax = RVA_ACTOR_GET_SHIELD_MAX;

    // Offsets
    int actorModelTransformOffset = ACTORMODEL_TRANSFORM_OFFSET;
    int blockParryAgentOffset = OFFSET_BLOCK_PARRY_AGENT;

    void Initialize() {
        // Já inicializado com valores do offsets.h
    }

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
    ) {
        rvaGetMainCamera = rvaGetMainCamera_;
        rvaGetTransform = rvaGetTransform_;
        rvaGetPosition = rvaGetPosition_;
        rvaSetPosition = rvaSetPosition_;
        rvaGetEulerAngles = rvaGetEulerAngles_;
        rvaSetEulerAngles = rvaSetEulerAngles_;
        rvaWorldToScreen = rvaWorldToScreen_;
        rvaFindObject = rvaFindObject_;
        rvaGetViewMatrix = rvaGetViewMatrix_;
        rvaGetProjectionMatrix = rvaGetProjectionMatrix_;
        rvaGetAliveCharList = rvaGetAliveCharList_;
        rvaGetMainPlayer = rvaGetMainPlayer_;
        rvaGetModelType = rvaGetModelType_;
        rvaGetActorModelFid = rvaGetActorModelFid_;
        rvaGetLogicBoneTransform = rvaGetLogicBoneTransform_;
        rvaIsLocalClient = rvaIsLocalClient_;
        rvaGetHp = rvaGetHp_;
        rvaGetHpMax = rvaGetHpMax_;
        rvaGetShield = rvaGetShield_;
        rvaGetShieldMax = rvaGetShieldMax_;
        actorModelTransformOffset = actorModelTransformOffset_;
        blockParryAgentOffset = blockParryAgentOffset_;
    }
}
