#pragma once

void noclip_set_enabled(bool v);
bool noclip_get_enabled();
void noclip_set_speed(float v);
float noclip_get_speed();
void noclip_tick(void** actors, int actorCount);
void noclip_init();
