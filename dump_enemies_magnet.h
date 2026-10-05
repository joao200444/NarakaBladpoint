#pragma once
#include "vec.h"
#include <vector>

// Estrutura para armazenar dados de entidade magnetizada
struct MagnetizedEntity {
    void* actorModel;          // Ponteiro para o ActorModel
    Vector3 originalPosition;  // Posição original (backup)
    Vector3 magnetPosition;    // Posição magnetizada (3m da câmera)
    bool isActive;             // Se está ativamente magnetizada
    unsigned long long lastUpdateMs; // Timestamp da última atualização
};

// Funções principais do Dump Enemies Magnet
void magnet_set_enabled(bool enabled);
bool magnet_get_enabled();

void magnet_set_distance(float distance);
float magnet_get_distance();

void magnet_set_freeze_height(bool freeze);
bool magnet_get_freeze_height();

void magnet_set_exclude_local_player(bool exclude);
bool magnet_get_exclude_local_player();

// Função de update (chamar no loop principal)
void magnet_update();

// Função para obter entidades magnetizadas (para debug/UI)
const std::vector<MagnetizedEntity>& magnet_get_entities();

// Função para limpar todas as entidades magnetizadas
void magnet_clear_all();

// Função para debug
void magnet_set_debug(bool enabled);
bool magnet_get_debug();
