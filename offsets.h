#pragma once

// ============================================================================
#define RVA_GET_TRANSFORM            0x11639430  // Component.get_transform (UnityEngine)
#define RVA_GET_POSITION             0x1164db50  // Transform.get_position (UnityEngine)
#define RVA_GET_ALIVE_CHAR_LIST      0x570c050   // CharacterManager.get_AllAliveCharacterList (Game)
#define RVA_GET_MAIN_PLAYER          0x4de74c0//0xcc2dfa0   // CharacterManager.GetMainPlayerActorModel (Game)
#define RVA_GET_MAIN_CAMERA          0x1160a730  // get_main (testado)

// RVAs do DAS.hpp (UnityEngine.CoreModule.dll)

#define RVA_SET_POSITION             0x1164dbb0  // Transform.set_position (UnityEngine)
#define RVA_GET_EULER_ANGLES         0x4fd5900   // Transform.get_eulerAngles (UnityEngine)
#define RVA_SET_EULER_ANGLES         0x822b390   // Transform.set_eulerAngles (UnityEngine)
#define RVA_WORLD_TO_SCREEN          0x11619eb0  // Camera.WorldToScreenPoint (UnityEngine)
#define RVA_FIND_OBJECTS_OF_TYPE     0x2a5630   // Object.FindObjectsOfType_2 (UnityEngine)
#define RVA_GET_VIEW_MATRIX          0x11619bb0  // Camera.get_worldToCameraMatrix (UnityEngine)
#define RVA_GET_PROJECTION_MATRIX    0x11619cd0  // Camera.get_projectionMatrix (UnityEngine)

// Game Methods (Naraka Bladepoint)
#define RVA_GET_MODEL_TYPE           0x5a07e40   // ActorKit.GetModelType (Game)
#define RVA_GET_ACTOR_MODEL_FID      0x82297d0   // ActorKit.GetActorModelFid (Game)
#define RVA_GET_LOGIC_BONE_TRANSFORM 0x7a5b230   // ActorModel.GetLogicBoneTransform (Game)
#define RVA_IS_LOCAL_CLIENT          0x4fc6ac0   // ActorModel.IsLocalClient (Game)
#define RVA_ACTOR_GET_HP             0x8829ed0   // ActorModel.get_Hp (Game)
#define RVA_ACTOR_GET_HP_MAX         0x8827710   // ActorModel.get_HpMax (Game)
#define RVA_ACTOR_GET_SHIELD         0x882a080   // ActorModel.get_Shield (Game)
#define RVA_ACTOR_GET_SHIELD_MAX     0x8827920   // ActorModel.get_ShieldMax (Game)
#define RVA_MOVEMENT_UPDATE_MAX_SPEED 0x0000000  // MovementComponent.UpdateMaxSpeed (Game)

// Auto-Parry Specific RVAs (ATUALIZADOS DO frida_autoparry.js)
#define RVA_EXEC_COMBO                0xd6cb5a0   // Detecta quando inimigo executa ataque (Atualizado)
#define RVA_REMOTE_CHARGE_STAGE       0xd6ef310   // Detecta quando inimigo carrega (estágios 1,2,3)
#define RVA_CHECK_COMBO_CHARGE_STAGE  0xd6e26c0   // CheckComboChargeStage (Novo)
#define RVA_COMBO_RELEASE             0xd6bee40   // ComboRelease (Novo)
#define RVA_ENTER_BLOCK_PARRY         0x63ba600   // BlockParryAgent.EnterBlockParry - Executa o parry vermelho


// Offsets (mantidos inalterados)

#define ACTORMODEL_TRANSFORM_OFFSET  0x60        // Offset do campo transform no ActorModel
#define OFFSET_BLOCK_PARRY_AGENT      0x1e0       // Offset do BlockParryAgent no ActorModel
