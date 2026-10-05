#pragma once
#include "imgui/imgui.h"

namespace CustomControls {
    // Checkbox customizada com seta em vez de X
    bool CheckboxWithArrow(const char* label, bool* v);
    
    // Aplicar estilo customizado para todas as checkboxes do projeto
    void SetupCustomCheckboxRendering();
}