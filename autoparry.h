#pragma once

// Inicializar auto-parry (instalar hooks)
void autoparry_init();

// Tick do auto-parry (chamar a cada frame)
void autoparry_tick();

// Configurações
void autoparry_set_enabled(bool v);
bool autoparry_get_enabled();

void autoparry_set_debug(bool v);
bool autoparry_get_debug();

// Funções de debug
void* autoparry_get_local_player();
