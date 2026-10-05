#pragma once
#include <Windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include "imgui/imgui.h"
#include "imgui/imgui_impl_dx11.h"
#include "imgui/imgui_impl_win32.h"

namespace Overlay {
    bool Initialize();
    void Cleanup();
    void RunLoop();
    
    // Variáveis globais para acesso externo se necessário
    extern HWND hWindow;
    extern bool isRunning;
}
