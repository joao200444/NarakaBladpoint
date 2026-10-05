#include "Radar.hpp"
#include "../esp.h"
#include "../aimbot.h"
#include "../helpers.h"
#include <cmath>
#include <cstdio>

namespace Features {
namespace Radar {

static bool g_radarEnabled = false;
static float g_radarSize = 200.0f;
static float g_radarRange = 100.0f;
static ImVec2 g_radarPos = ImVec2(50, 50);

void Initialize()
{
    g_radarEnabled = false;
    g_radarSize = 200.0f;
    g_radarRange = 100.0f;
    g_radarPos = ImVec2(50, 50);
    printf("[Radar] System initialized\n");
}

void Shutdown()
{
    g_radarEnabled = false;
    printf("[Radar] System shutdown\n");
}

void Render()
{
    if (!g_radarEnabled)
        return;
        
    try {
        Vector3 camPos;
        if (!get_camera_position(camPos))
            return;

        ImDrawList* draw = ImGui::GetBackgroundDrawList();
        if (!draw)
            return;
            
        ImVec2 radarCenter = ImVec2(g_radarPos.x + g_radarSize * 0.5f, g_radarPos.y + g_radarSize * 0.5f);
        float radarRadius = g_radarSize * 0.5f;
        
        RenderRadarBackground(draw, radarCenter, radarRadius);
        RenderRadarEntities(draw, radarCenter, radarRadius);
        RenderRadarUI(draw, g_radarPos);
        
    } catch (...) {
    }
}

void SetEnabled(bool enabled)
{
    g_radarEnabled = enabled;
}

bool IsEnabled()
{
    return g_radarEnabled;
}

void SetSize(float size)
{
    if (size >= 100.0f && size <= 400.0f)
        g_radarSize = size;
}

float GetSize()
{
    return g_radarSize;
}

void SetRange(float range)
{
    if (range >= 50.0f && range <= 500.0f)
        g_radarRange = range;
}

float GetRange()
{
    return g_radarRange;
}

void SetPosition(float x, float y)
{
    g_radarPos.x = x;
    g_radarPos.y = y;
}

void GetPosition(float& x, float& y)
{
    x = g_radarPos.x;
    y = g_radarPos.y;
}

void RenderRadarBackground(ImDrawList* draw, ImVec2 center, float radius)
{
    draw->AddCircleFilled(center, radius, IM_COL32(0, 0, 0, 150), 32);
    draw->AddCircle(center, radius, IM_COL32(255, 255, 255, 200), 32, 2.0f);
    
    draw->AddCircleFilled(center, 4.0f, IM_COL32(0, 255, 0, 255));
    draw->AddCircle(center, 4.0f, IM_COL32(255, 255, 255, 200), 0, 1.5f);
    
    float quarterRange = radius * 0.25f;
    float halfRange = radius * 0.5f;
    float threeQuarterRange = radius * 0.75f;
    
    draw->AddCircle(center, quarterRange, IM_COL32(255, 255, 255, 40), 32, 1.0f);
    draw->AddCircle(center, halfRange, IM_COL32(255, 255, 255, 40), 32, 1.0f);
    draw->AddCircle(center, threeQuarterRange, IM_COL32(255, 255, 255, 40), 32, 1.0f);
    
    draw->AddLine(ImVec2(center.x - radius, center.y), ImVec2(center.x + radius, center.y), IM_COL32(255, 255, 255, 60), 1.0f);
    draw->AddLine(ImVec2(center.x, center.y - radius), ImVec2(center.x, center.y + radius), IM_COL32(255, 255, 255, 60), 1.0f);
}

void RenderRadarEntities(ImDrawList* draw, ImVec2 center, float radius)
{
    Vector3 camPos;
    if (!get_camera_position(camPos))
        return;
    
    for (const auto& actor : g_visibleActors)
    {
        float dx = actor.worldPosX - camPos.x;
        float dz = actor.worldPosZ - camPos.z;
        float distance = actor.distance;
        
        if (distance > g_radarRange)
            continue;
            
        float radarScale = radius / g_radarRange;
        float radarX = center.x + dx * radarScale;
        float radarY = center.y + dz * radarScale;
        
        float distFromCenter = sqrtf((radarX - center.x) * (radarX - center.x) + (radarY - center.y) * (radarY - center.y));
        if (distFromCenter > radius - 5.0f)
            continue;
        
        ImU32 color = IM_COL32(0, 255, 0, 255);
        
        draw->AddCircleFilled(ImVec2(radarX, radarY), 3.0f, color);
        draw->AddCircle(ImVec2(radarX, radarY), 3.0f, IM_COL32(255, 255, 255, 180), 0, 1.0f);
        
        if (distance < g_radarRange * 0.3f) {
            char distText[16];
            snprintf(distText, sizeof(distText), "%.0fm", distance);
            ImVec2 textPos = ImVec2(radarX + 5, radarY - 8);
            draw->AddText(textPos, IM_COL32(255, 255, 255, 200), distText);
        }
    }
}

void RenderRadarUI(ImDrawList* draw, ImVec2 position)
{
    ImVec2 titlePos = ImVec2(position.x, position.y - 25);
    draw->AddRectFilled(ImVec2(titlePos.x - 5, titlePos.y - 2), ImVec2(titlePos.x + 50, titlePos.y + 16), IM_COL32(0, 0, 0, 180), 4.0f);
    draw->AddText(titlePos, IM_COL32(0, 255, 255, 255), "RADAR");
    
    char rangeText[32];
    snprintf(rangeText, sizeof(rangeText), "%.0fm", g_radarRange);
    ImVec2 rangePos = ImVec2(position.x + g_radarSize - 35, position.y - 25);
    draw->AddRectFilled(ImVec2(rangePos.x - 3, rangePos.y - 2), ImVec2(rangePos.x + 35, rangePos.y + 16), IM_COL32(0, 0, 0, 180), 4.0f);
    draw->AddText(rangePos, IM_COL32(255, 255, 100, 255), rangeText);
    
    ImVec2 radarCenter = ImVec2(position.x + g_radarSize * 0.5f, position.y + g_radarSize * 0.5f);
    float radius = g_radarSize * 0.5f;
    
    draw->AddText(ImVec2(radarCenter.x - 4, radarCenter.y - radius - 15), IM_COL32(255, 255, 255, 200), "N");
    draw->AddText(ImVec2(radarCenter.x - 4, radarCenter.y + radius + 5), IM_COL32(255, 255, 255, 200), "S");
    draw->AddText(ImVec2(radarCenter.x + radius + 5, radarCenter.y - 8), IM_COL32(255, 255, 255, 200), "E");
    draw->AddText(ImVec2(radarCenter.x - radius - 15, radarCenter.y - 8), IM_COL32(255, 255, 255, 200), "W");
}

}   
}   