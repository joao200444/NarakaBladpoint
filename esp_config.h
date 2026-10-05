#pragma once

// Global Configuration Functions (All Smunk Mods Settings)
void esp_save_config();    // Saves all settings (renamed for compatibility)
void esp_load_config();    // Loads all settings (renamed for compatibility)
void esp_auto_save_config(); // Auto-saves all settings (renamed for compatibility)
void esp_load_config_after_login(); // Loads config only after successful login