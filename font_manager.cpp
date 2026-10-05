#include "font_manager.h"
#include "imgui/imgui.h"
#include "fonts/zh-cn.h"

namespace FontManager {
    ImFont* titleFont = nullptr;
    ImFont* headerFont = nullptr;
    ImFont* bodyFont = nullptr;
    ImFont* buttonFont = nullptr;
    
    void Initialize() {
        LoadCustomFonts();
        ApplyProfessionalStyle();
    }
    
    bool LoadCustomFonts() {
        ImGuiIO& io = ImGui::GetIO();

        // Configuração de fonte
        ImFontConfig config;
        config.OversampleH = 3;
        config.OversampleV = 3;
        config.PixelSnapH = true;

        // Ranges Unicode para suportar símbolos especiais
        static const ImWchar ranges[] = {
            0x0020, 0x00FF, // Basic Latin + Latin Supplement
            0x2000, 0x206F, // General Punctuation
            0x2190, 0x21FF, // Arrows
            0x2600, 0x26FF, // Miscellaneous Symbols
            0x2700, 0x27BF, // Dingbats
            0,
        };

        // Carregar fonte zh-cn embutida (não comprimida) em diferentes tamanhos
        titleFont = io.Fonts->AddFontFromMemoryTTF(
            (void*)zh_cn,
            7232220,
            26.0f,
            &config,
            ranges
        );

        headerFont = io.Fonts->AddFontFromMemoryTTF(
            (void*)zh_cn,
            7232220,
            20.0f,
            &config,
            ranges
        );

        bodyFont = io.Fonts->AddFontFromMemoryTTF(
            (void*)zh_cn,
            7232220,
            16.0f,
            &config,
            ranges
        );

        buttonFont = io.Fonts->AddFontFromMemoryTTF(
            (void*)zh_cn,
            7232220,
            18.0f,
            &config,
            ranges
        );

        // Fallback para fonte padrão se algo der errado
        if (!titleFont) titleFont = io.Fonts->AddFontDefault();
        if (!headerFont) headerFont = io.Fonts->AddFontDefault();
        if (!bodyFont) bodyFont = io.Fonts->AddFontDefault();
        if (!buttonFont) buttonFont = io.Fonts->AddFontDefault();

        return true;
    }
    
    void ApplyProfessionalStyle() {
        ImGuiStyle& style = ImGui::GetStyle();
        
        style.AntiAliasedLines = true;
        style.AntiAliasedFill = true;
        
        // Bordas arredondadas suaves e profissionais
        style.WindowRounding = 12.0f;
        style.FrameRounding = 8.0f;
        style.ScrollbarRounding = 10.0f;
        style.GrabRounding = 8.0f;
        style.ChildRounding = 10.0f;
        style.PopupRounding = 10.0f;
        style.TabRounding = 8.0f;
        style.FrameBorderSize = 1.0f;
        style.ChildBorderSize = 1.0f;
        style.WindowBorderSize = 1.0f;

        style.WindowPadding = ImVec2(18, 18);
        style.FramePadding = ImVec2(14, 10);
        style.ItemSpacing = ImVec2(14, 10);
        style.ItemInnerSpacing = ImVec2(10, 8);
        style.ScrollbarSize = 16.0f;
        style.GrabMinSize = 14.0f;
        style.WindowTitleAlign = ImVec2(0.5f, 0.5f);

        // ===== CORES PROFISSIONAIS RED/CRIMSON =====
        ImVec4* colors = style.Colors;
        
        colors[ImGuiCol_WindowBg] = ImVec4(0.12f, 0.06f, 0.06f, 0.98f);
        colors[ImGuiCol_ChildBg] = ImVec4(0.14f, 0.08f, 0.08f, 1.0f);
        colors[ImGuiCol_PopupBg] = ImVec4(0.12f, 0.06f, 0.06f, 0.98f);
        
        colors[ImGuiCol_Text] = ImVec4(1.00f, 0.95f, 0.95f, 1.00f);
        colors[ImGuiCol_TextDisabled] = ImVec4(0.60f, 0.50f, 0.50f, 1.00f);
        
        colors[ImGuiCol_Border] = ImVec4(0.85f, 0.30f, 0.30f, 0.65f);
        colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.30f);
        
        colors[ImGuiCol_FrameBg] = ImVec4(0.22f, 0.12f, 0.12f, 1.0f);
        colors[ImGuiCol_FrameBgHovered] = ImVec4(0.70f, 0.20f, 0.20f, 0.50f);
        colors[ImGuiCol_FrameBgActive] = ImVec4(0.85f, 0.25f, 0.25f, 0.70f);
        
        colors[ImGuiCol_TitleBg] = ImVec4(0.10f, 0.04f, 0.04f, 1.00f);
        colors[ImGuiCol_TitleBgActive] = ImVec4(0.18f, 0.08f, 0.08f, 1.00f);
        colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.08f, 0.03f, 0.03f, 0.85f);
        
        colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.08f, 0.08f, 1.00f);
        
        colors[ImGuiCol_ScrollbarBg] = ImVec4(0.14f, 0.08f, 0.08f, 0.95f);
        colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.80f, 0.30f, 0.30f, 0.85f);
        colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.90f, 0.35f, 0.35f, 0.95f);
        colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(1.00f, 0.40f, 0.40f, 1.00f);
        
        colors[ImGuiCol_CheckMark] = ImVec4(1.00f, 0.40f, 0.40f, 1.0f);
        
        colors[ImGuiCol_SliderGrab] = ImVec4(0.90f, 0.35f, 0.35f, 1.0f);
        colors[ImGuiCol_SliderGrabActive] = ImVec4(1.00f, 0.45f, 0.45f, 1.0f);
        
        colors[ImGuiCol_Button] = ImVec4(0.70f, 0.25f, 0.25f, 0.85f);
        colors[ImGuiCol_ButtonHovered] = ImVec4(0.85f, 0.30f, 0.30f, 1.0f);
        colors[ImGuiCol_ButtonActive] = ImVec4(1.00f, 0.40f, 0.40f, 1.0f);
        
        colors[ImGuiCol_Header] = ImVec4(0.75f, 0.25f, 0.25f, 0.85f);
        colors[ImGuiCol_HeaderHovered] = ImVec4(0.88f, 0.30f, 0.30f, 1.0f);
        colors[ImGuiCol_HeaderActive] = ImVec4(1.00f, 0.40f, 0.40f, 1.0f);
        
        colors[ImGuiCol_Separator] = ImVec4(0.75f, 0.25f, 0.25f, 0.65f);
        colors[ImGuiCol_SeparatorHovered] = ImVec4(0.85f, 0.30f, 0.30f, 0.85f);
        colors[ImGuiCol_SeparatorActive] = ImVec4(1.00f, 0.40f, 0.40f, 1.00f);
        
        colors[ImGuiCol_Tab] = ImVec4(0.20f, 0.10f, 0.10f, 0.90f);
        colors[ImGuiCol_TabHovered] = ImVec4(0.85f, 0.30f, 0.30f, 1.0f);
        colors[ImGuiCol_TabActive] = ImVec4(0.75f, 0.25f, 0.25f, 1.0f);
        colors[ImGuiCol_TabUnfocused] = ImVec4(0.14f, 0.08f, 0.08f, 1.0f);
        colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.22f, 0.12f, 0.12f, 1.0f);
        
        colors[ImGuiCol_ResizeGrip] = ImVec4(0.80f, 0.30f, 0.30f, 0.40f);
        colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.90f, 0.35f, 0.35f, 0.70f);
        colors[ImGuiCol_ResizeGripActive] = ImVec4(1.00f, 0.45f, 0.45f, 0.95f);
    }

    void ApplyRubickStyle() {
        ImGuiStyle& style = ImGui::GetStyle();
        
        // ===== TEMA CYBERPUNK MODERNO (Inspirado SoloLevelling) =====
        style.WindowRounding = 8.0f;
        style.FrameRounding = 6.0f;
        style.PopupRounding = 8.0f;
        style.ScrollbarRounding = 8.0f;
        style.GrabRounding = 6.0f;
        style.TabRounding = 6.0f;
        style.ChildRounding = 6.0f;
        
        style.WindowPadding = ImVec2(15, 15);
        style.FramePadding = ImVec2(12, 8);
        style.ItemSpacing = ImVec2(12, 8);
        style.ItemInnerSpacing = ImVec2(8, 6);
        style.IndentSpacing = 25.0f;
        style.ScrollbarSize = 14.0f;
        style.GrabMinSize = 12.0f;
        style.WindowTitleAlign = ImVec2(0.5f, 0.5f);
        
        // ===== CORES CYBERPUNK ROXO/AZUL =====
        ImVec4* colors = style.Colors;
        
        // Background escuro com tom azulado
        colors[ImGuiCol_WindowBg] = ImVec4(0.08f, 0.08f, 0.12f, 0.95f);
        colors[ImGuiCol_ChildBg] = ImVec4(0.10f, 0.10f, 0.15f, 1.0f);
        colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.12f, 0.95f);
        
        // Texto branco brilhante
        colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
        colors[ImGuiCol_TextDisabled] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
        
        // Bordas roxas neon
        colors[ImGuiCol_Border] = ImVec4(0.6f, 0.3f, 0.9f, 0.5f);
        colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        
        // Frames com tom roxo escuro
        colors[ImGuiCol_FrameBg] = ImVec4(0.15f, 0.15f, 0.20f, 1.0f);
        colors[ImGuiCol_FrameBgHovered] = ImVec4(0.6f, 0.2f, 0.8f, 0.4f);
        colors[ImGuiCol_FrameBgActive] = ImVec4(0.7f, 0.3f, 0.9f, 0.6f);
        
        // Title bar escuro
        colors[ImGuiCol_TitleBg] = ImVec4(0.07f, 0.07f, 0.10f, 1.00f);
        colors[ImGuiCol_TitleBgActive] = ImVec4(0.10f, 0.10f, 0.15f, 1.00f);
        colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.05f, 0.05f, 0.08f, 0.75f);
        
        // Menu bar
        colors[ImGuiCol_MenuBarBg] = ImVec4(0.10f, 0.10f, 0.15f, 1.00f);
        
        // Scrollbar
        colors[ImGuiCol_ScrollbarBg] = ImVec4(0.10f, 0.10f, 0.15f, 0.90f);
        colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.6f, 0.3f, 0.8f, 0.80f);
        colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.7f, 0.4f, 0.9f, 0.90f);
        colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.9f, 0.5f, 1.0f, 1.00f);
        
        // CheckMark roxo brilhante
        colors[ImGuiCol_CheckMark] = ImVec4(0.8f, 0.4f, 1.0f, 1.0f);
        
        // Sliders roxos
        colors[ImGuiCol_SliderGrab] = ImVec4(0.7f, 0.3f, 0.9f, 1.0f);
        colors[ImGuiCol_SliderGrabActive] = ImVec4(0.9f, 0.5f, 1.0f, 1.0f);
        
        // Botões roxos com gradiente
        colors[ImGuiCol_Button] = ImVec4(0.5f, 0.2f, 0.7f, 0.8f);
        colors[ImGuiCol_ButtonHovered] = ImVec4(0.7f, 0.3f, 0.9f, 1.0f);
        colors[ImGuiCol_ButtonActive] = ImVec4(0.9f, 0.5f, 1.0f, 1.0f);
        
        // Headers roxos
        colors[ImGuiCol_Header] = ImVec4(0.6f, 0.2f, 0.8f, 0.8f);
        colors[ImGuiCol_HeaderHovered] = ImVec4(0.7f, 0.3f, 0.9f, 1.0f);
        colors[ImGuiCol_HeaderActive] = ImVec4(0.9f, 0.5f, 1.0f, 1.0f);
        
        // Separadores sutis
        colors[ImGuiCol_Separator] = ImVec4(0.5f, 0.2f, 0.7f, 0.60f);
        colors[ImGuiCol_SeparatorHovered] = ImVec4(0.7f, 0.3f, 0.9f, 0.80f);
        colors[ImGuiCol_SeparatorActive] = ImVec4(0.9f, 0.5f, 1.0f, 1.00f);
        
        // Tabs
        colors[ImGuiCol_Tab] = ImVec4(0.15f, 0.15f, 0.20f, 0.8f);
        colors[ImGuiCol_TabHovered] = ImVec4(0.7f, 0.3f, 0.9f, 1.0f);
        colors[ImGuiCol_TabActive] = ImVec4(0.6f, 0.2f, 0.8f, 1.0f);
        colors[ImGuiCol_TabUnfocused] = ImVec4(0.10f, 0.10f, 0.15f, 1.0f);
        colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.15f, 0.15f, 0.20f, 1.0f);
        
        // Resize grip
        colors[ImGuiCol_ResizeGrip] = ImVec4(0.6f, 0.3f, 0.8f, 0.30f);
        colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.7f, 0.4f, 0.9f, 0.60f);
        colors[ImGuiCol_ResizeGripActive] = ImVec4(0.9f, 0.5f, 1.0f, 0.90f);
    }
    
    void RenderGlowText(const char* text, ImVec2 pos, ImU32 color, ImU32 glowColor, float glowSize) {
        auto drawList = ImGui::GetWindowDrawList();
        
        // Desenhar múltiplas camadas para efeito de brilho
        for (int i = 0; i < 4; i++) {
            float offset = glowSize * (i + 1) * 0.5f;
            ImU32 layerColor = (glowColor & 0x00FFFFFF) | ((int)(128 / (i + 1)) << 24);
            
            drawList->AddText(ImVec2(pos.x + offset, pos.y), layerColor, text);
            drawList->AddText(ImVec2(pos.x - offset, pos.y), layerColor, text);
            drawList->AddText(ImVec2(pos.x, pos.y + offset), layerColor, text);
            drawList->AddText(ImVec2(pos.x, pos.y - offset), layerColor, text);
        }
        
        // Texto principal
        drawList->AddText(pos, color, text);
    }
    
    void RenderOutlineText(const char* text, ImVec2 pos, ImU32 textColor, ImU32 outlineColor, float thickness) {
        auto drawList = ImGui::GetWindowDrawList();
        
        // Desenhar contorno
        for (int x = -1; x <= 1; x++) {
            for (int y = -1; y <= 1; y++) {
                if (x == 0 && y == 0) continue;
                drawList->AddText(ImVec2(pos.x + x * thickness, pos.y + y * thickness), outlineColor, text);
            }
        }
        
        // Texto principal
        drawList->AddText(pos, textColor, text);
    }
    
    void RenderGradientText(const char* text, ImVec2 pos, ImU32 colorTop, ImU32 colorBottom) {
        auto drawList = ImGui::GetWindowDrawList();
        ImVec2 textSize = ImGui::CalcTextSize(text);
        
        // Simular gradiente com múltiplas linhas
        int steps = 8;
        float stepHeight = textSize.y / steps;
        
        for (int i = 0; i < steps; i++) {
            float t = (float)i / (steps - 1);
            
            // Interpolação manual de cores
            ImU32 r1 = (colorTop >> 0) & 0xFF;
            ImU32 g1 = (colorTop >> 8) & 0xFF;
            ImU32 b1 = (colorTop >> 16) & 0xFF;
            ImU32 a1 = (colorTop >> 24) & 0xFF;
            
            ImU32 r2 = (colorBottom >> 0) & 0xFF;
            ImU32 g2 = (colorBottom >> 8) & 0xFF;
            ImU32 b2 = (colorBottom >> 16) & 0xFF;
            ImU32 a2 = (colorBottom >> 24) & 0xFF;
            
            ImU32 r = (ImU32)(r1 + (r2 - r1) * t);
            ImU32 g = (ImU32)(g1 + (g2 - g1) * t);
            ImU32 b = (ImU32)(b1 + (b2 - b1) * t);
            ImU32 a = (ImU32)(a1 + (a2 - a1) * t);
            
            ImU32 color = (a << 24) | (b << 16) | (g << 8) | r;
            
            ImVec2 clipMin = ImVec2(pos.x, pos.y + i * stepHeight);
            ImVec2 clipMax = ImVec2(pos.x + textSize.x, pos.y + (i + 1) * stepHeight);
            
            drawList->PushClipRect(clipMin, clipMax);
            drawList->AddText(pos, color, text);
            drawList->PopClipRect();
        }
    }
}