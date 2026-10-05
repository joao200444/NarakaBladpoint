#pragma once
#include "imgui/imgui.h"

namespace FontManager {
    // Fontes customizadas
    extern ImFont* titleFont;
    extern ImFont* headerFont;
    extern ImFont* bodyFont;
    extern ImFont* buttonFont;
    
    // Inicializar sistema de fontes
    void Initialize();
    
    // Carregar fontes customizadas
    bool LoadCustomFonts();
    
    // Aplicar estilo profissional Cyan/Dark Blue
    void ApplyProfessionalStyle();
    
    // Helpers para texto estilizado
    void RenderGlowText(const char* text, ImVec2 pos, ImU32 color, ImU32 glowColor, float glowSize = 2.0f);
    void RenderOutlineText(const char* text, ImVec2 pos, ImU32 textColor, ImU32 outlineColor, float thickness = 1.0f);
    void RenderGradientText(const char* text, ImVec2 pos, ImU32 colorTop, ImU32 colorBottom);
}