#pragma once
#include <Windows.h>
#include <cstdint>
#include "../imgui/imgui.h"
#include "../vec.h"

namespace Features {
namespace Radar {
    
    void Initialize();
    void Shutdown();
    void Render();
    
    void SetEnabled(bool enabled);
    bool IsEnabled();
    
    void SetSize(float size);
    float GetSize();
    
    void SetRange(float range);
    float GetRange();
    
    void SetPosition(float x, float y);
    void GetPosition(float& x, float& y);
    
    void RenderRadarBackground(ImDrawList* draw, ImVec2 center, float radius);
    void RenderRadarEntities(ImDrawList* draw, ImVec2 center, float radius);
    void RenderRadarUI(ImDrawList* draw, ImVec2 position);
    
}   
}   