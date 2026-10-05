#pragma once

void autospin_set_enabled(bool v);
bool autospin_get_enabled();
void autospin_set_speed(float v);
float autospin_get_speed();
void autospin_tick(void** actors, int actorCount);
void autospin_init();
