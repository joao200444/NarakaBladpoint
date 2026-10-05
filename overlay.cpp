#include "overlay.h"
#include "hookframe.h" // Para acessar as flags globais e funcoes de desenho (se extrairmos)
#include <dwmapi.h>
#include <iostream>
#include <string>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dwmapi.lib")

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace Overlay {
    HWND hWindow = NULL;
    HWND hGameWindow = NULL;
    bool isRunning = false;
    
    ID3D11Device* device = nullptr;
    ID3D11DeviceContext* context = nullptr;
    IDXGISwapChain* swapChain = nullptr;
    ID3D11RenderTargetView* renderTargetView = nullptr;
    
    int windowWidth = 1920;
    int windowHeight = 1080;

    LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
        if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
            return true;

        switch (msg) {
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        case WM_KEYDOWN:
             if (wParam == VK_TAB) {
                 // Passar o input para o sistema de menu se necessario
             }
             break;
        }
        return DefWindowProc(hWnd, msg, wParam, lParam);
    }

    bool InitDX11() {
        DXGI_SWAP_CHAIN_DESC scd;
        ZeroMemory(&scd, sizeof(DXGI_SWAP_CHAIN_DESC));
        scd.BufferCount = 2;
        scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        scd.OutputWindow = hWindow;
        scd.SampleDesc.Count = 1;
        scd.Windowed = TRUE;
        scd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

        HRESULT hr = D3D11CreateDeviceAndSwapChain(
            nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, nullptr, 0,
            D3D11_SDK_VERSION, &scd, &swapChain, &device, nullptr, &context);

        if (FAILED(hr)) return false;

        ID3D11Texture2D* pBackBuffer = nullptr;
        swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBackBuffer);
        device->CreateRenderTargetView(pBackBuffer, nullptr, &renderTargetView);
        pBackBuffer->Release();

        return true;
    }

    void SetupWindow() {
        WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, "OverlayClass", NULL };
        RegisterClassEx(&wc);

        // Encontrar a janela do jogo (Tentar nomes comuns)
        hGameWindow = FindWindowA("UnrealWindow", NULL); 
        if (!hGameWindow) hGameWindow = FindWindowA(NULL, "ArkAscended"); // Tentar pelo titulo se classe falhar

        RECT rect = { 0, 0, 1920, 1080 };
        if (hGameWindow) {
            GetWindowRect(hGameWindow, &rect);
            windowWidth = rect.right - rect.left;
            windowHeight = rect.bottom - rect.top;
        }

        hWindow = CreateWindowExA(
            WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED | WS_EX_TOOLWINDOW,
            "OverlayClass", "Overlay",
            WS_POPUP,
            rect.left, rect.top, windowWidth, windowHeight,
            NULL, NULL, wc.hInstance, NULL
        );

        // Tornar fundo transparente
        SetLayeredWindowAttributes(hWindow, RGB(0, 0, 0), 0, LWA_COLORKEY);
        SetLayeredWindowAttributes(hWindow, 0, 255, LWA_ALPHA);

        // DwmExtendFrameIntoClientArea para transparência total
        MARGINS margins = { -1 };
        DwmExtendFrameIntoClientArea(hWindow, &margins);

        ShowWindow(hWindow, SW_SHOW);
        UpdateWindow(hWindow);
    }

    bool Initialize() {
        SetupWindow();
        if (!InitDX11()) return false;

        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.IniFilename = nullptr; // Nao salvar ini
        
        // Setup Dear ImGui style (copiado do hookframe ou padrao)
        ImGui::StyleColorsDark();

        ImGui_ImplWin32_Init(hWindow);
        ImGui_ImplDX11_Init(device, context);
        
        // Inicializar Auth e Fontes aqui tambem se necessario
        // InitImGui(); // Funcao do hookframe.cpp se for visivel
        
        isRunning = true;
        return true;
    }

    void Cleanup() {
        ImGui_ImplDX11_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();

        if (renderTargetView) renderTargetView->Release();
        if (swapChain) swapChain->Release();
        if (context) context->Release();
        if (device) device->Release();

        DestroyWindow(hWindow);
        UnregisterClass("OverlayClass", GetModuleHandle(NULL));
    }

    // Declaracao externa da funcao de renderizacao do hookframe
    // Precisaremos refatorar hookframe.cpp para expor apenas a logica de UI
    extern void DrawImGuiContent(); 

    void RunLoop() {
        MSG msg;
        ZeroMemory(&msg, sizeof(msg));

        while (msg.message != WM_QUIT && isRunning) {
            if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE)) {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
                continue;
            }

            // Reposicionar janela se o jogo mover
            if (hGameWindow) {
                RECT rect;
                GetWindowRect(hGameWindow, &rect);
                int w = rect.right - rect.left;
                int h = rect.bottom - rect.top;
                if (w != windowWidth || h != windowHeight || rect.left != 0 /*simplificacao*/) { // Verificar mudanca real
                     MoveWindow(hWindow, rect.left, rect.top, w, h, TRUE);
                     windowWidth = w;
                     windowHeight = h;
                }
            }

            // Render Frame
            ImGui_ImplDX11_NewFrame();
            ImGui_ImplWin32_NewFrame();
            ImGui::NewFrame();

            // Aqui chamamos a logica do menu existente!
            // Em vez de chamar RenderImGui (que tem logica DX12 interna), chamaremos DrawImGuiContent
            DrawImGuiContent(); 

            ImGui::Render();
            const float clear_color_with_alpha[4] = { 0.0f, 0.0f, 0.0f, 0.0f }; // Transparente
            context->OMSetRenderTargets(1, &renderTargetView, nullptr);
            context->ClearRenderTargetView(renderTargetView, clear_color_with_alpha);
            ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
            swapChain->Present(1, 0);
            
            Sleep(1); // Reduzir uso de CPU
        }
        Cleanup();
    }
}
