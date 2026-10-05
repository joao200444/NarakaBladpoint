#include "pch.h"
#include "console_window.h"
#include "hookframe.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_dx12.h"
#include <iostream>
#include <io.h>
#include <fcntl.h>

// Declarar função externa da hookframe
extern void RenderBPRSUI();

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK ConsoleWindow::ConsoleWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wParam, lParam))
        return true;

    switch (msg) {
    case WM_SIZE:
        if (wParam != SIZE_MINIMIZED) {
            auto& instance = Get();
            if (instance.m_pd3dDevice != nullptr) {
                instance.WaitForLastSubmittedFrame();
                instance.CleanupRenderTarget();
                HRESULT result = instance.m_pSwapChain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT);
                assert(SUCCEEDED(result) && "Failed to resize swapchain.");
                instance.CreateRenderTarget();
            }
        }
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU)
            return 0;
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    
    auto& instance = Get();
    if (instance.m_originalWndProc)
        return CallWindowProc(instance.m_originalWndProc, hwnd, msg, wParam, lParam);
        
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

bool ConsoleWindow::CreateDeviceD3D12(HWND hWnd) {
    DXGI_SWAP_CHAIN_DESC1 sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = NUM_BACK_BUFFERS;
    sd.Width = 0;
    sd.Height = 0;
    sd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    sd.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
    sd.Scaling = DXGI_SCALING_STRETCH;
    sd.Stereo = FALSE;

    D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;
    if (D3D12CreateDevice(nullptr, featureLevel, IID_PPV_ARGS(&m_pd3dDevice)) != S_OK)
        return false;

    D3D12_DESCRIPTOR_HEAP_DESC desc = {};
    desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    desc.NumDescriptors = NUM_BACK_BUFFERS;
    desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    desc.NodeMask = 1;
    if (m_pd3dDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_pd3dRtvDescHeap)) != S_OK)
        return false;

    SIZE_T rtvDescriptorSize = m_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_pd3dRtvDescHeap->GetCPUDescriptorHandleForHeapStart();
    for (UINT i = 0; i < NUM_BACK_BUFFERS; i++) {
        m_mainRenderTargetDescriptor[i] = rtvHandle;
        rtvHandle.ptr += rtvDescriptorSize;
    }

    desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    desc.NumDescriptors = 1;
    desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    if (m_pd3dDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_pd3dSrvDescHeap)) != S_OK)
        return false;

    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.NodeMask = 1;
    if (m_pd3dDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_pd3dCommandQueue)) != S_OK)
        return false;

    for (UINT i = 0; i < NUM_FRAMES_IN_FLIGHT; i++)
        if (m_pd3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_frameContext[i].CommandAllocator)) != S_OK)
            return false;

    if (m_pd3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_frameContext[0].CommandAllocator, nullptr, IID_PPV_ARGS(&m_pd3dCommandList)) != S_OK ||
        m_pd3dCommandList->Close() != S_OK)
        return false;

    if (m_pd3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)) != S_OK)
        return false;

    m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    if (m_fenceEvent == nullptr)
        return false;

    IDXGIFactory4* dxgiFactory = nullptr;
    IDXGISwapChain1* swapChain1 = nullptr;
    if (CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory)) != S_OK)
        return false;
    if (dxgiFactory->CreateSwapChainForHwnd(m_pd3dCommandQueue, hWnd, &sd, nullptr, nullptr, &swapChain1) != S_OK)
        return false;
    if (swapChain1->QueryInterface(IID_PPV_ARGS(&m_pSwapChain)) != S_OK)
        return false;
    swapChain1->Release();
    dxgiFactory->Release();
    m_pSwapChain->SetMaximumFrameLatency(NUM_BACK_BUFFERS);
    m_hSwapChainWaitableObject = m_pSwapChain->GetFrameLatencyWaitableObject();

    CreateRenderTarget();
    return true;
}

void ConsoleWindow::CleanupDeviceD3D12() {
    CleanupRenderTarget();
    if (m_pSwapChain) { m_pSwapChain->SetFullscreenState(false, nullptr); m_pSwapChain->Release(); m_pSwapChain = nullptr; }
    if (m_hSwapChainWaitableObject != nullptr) { CloseHandle(m_hSwapChainWaitableObject); }
    for (UINT i = 0; i < NUM_FRAMES_IN_FLIGHT; i++)
        if (m_frameContext[i].CommandAllocator) { m_frameContext[i].CommandAllocator->Release(); m_frameContext[i].CommandAllocator = nullptr; }
    if (m_pd3dCommandQueue) { m_pd3dCommandQueue->Release(); m_pd3dCommandQueue = nullptr; }
    if (m_pd3dCommandList) { m_pd3dCommandList->Release(); m_pd3dCommandList = nullptr; }
    if (m_pd3dRtvDescHeap) { m_pd3dRtvDescHeap->Release(); m_pd3dRtvDescHeap = nullptr; }
    if (m_pd3dSrvDescHeap) { m_pd3dSrvDescHeap->Release(); m_pd3dSrvDescHeap = nullptr; }
    if (m_fence) { m_fence->Release(); m_fence = nullptr; }
    if (m_fenceEvent) { CloseHandle(m_fenceEvent); m_fenceEvent = nullptr; }
    if (m_pd3dDevice) { m_pd3dDevice->Release(); m_pd3dDevice = nullptr; }
}

void ConsoleWindow::CreateRenderTarget() {
    for (UINT i = 0; i < NUM_BACK_BUFFERS; i++) {
        ID3D12Resource* pBackBuffer = nullptr;
        m_pSwapChain->GetBuffer(i, IID_PPV_ARGS(&pBackBuffer));
        m_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, m_mainRenderTargetDescriptor[i]);
        m_mainRenderTargetResource[i] = pBackBuffer;
    }
}

void ConsoleWindow::CleanupRenderTarget() {
    WaitForLastSubmittedFrame();
    for (UINT i = 0; i < NUM_BACK_BUFFERS; i++)
        if (m_mainRenderTargetResource[i]) { m_mainRenderTargetResource[i]->Release(); m_mainRenderTargetResource[i] = nullptr; }
}

void ConsoleWindow::WaitForLastSubmittedFrame() {
    FrameContext* frameCtx = &m_frameContext[m_frameIndex % NUM_FRAMES_IN_FLIGHT];
    UINT64 fenceValue = frameCtx->FenceValue;
    if (fenceValue == 0)
        return;

    frameCtx->FenceValue = 0;
    if (m_fence->GetCompletedValue() >= fenceValue)
        return;

    m_fence->SetEventOnCompletion(fenceValue, m_fenceEvent);
    WaitForSingleObject(m_fenceEvent, INFINITE);
}

bool ConsoleWindow::Initialize() {
    if (m_running)
        return false;

    m_running = true;
    m_renderThread = std::thread(&ConsoleWindow::RenderThread, this);
    return true;
}

void ConsoleWindow::Shutdown() {
    m_running = false;
    if (m_renderThread.joinable())
        m_renderThread.join();
}

void ConsoleWindow::RenderThread() {
    // Criar console primeiro
    AllocConsole();
    
    // Obter handle da janela do console
    m_consoleHwnd = GetConsoleWindow();
    if (!m_consoleHwnd) {
        printf("[ERROR] Failed to get console window handle\n");
        return;
    }
    
    // Configurar console
    SetConsoleTitle(L"BPRS Console Interface");
    
    // Desabilitar QuickEdit Mode para permitir que o ImGui receba eventos de mouse
    HANDLE hInput = GetStdHandle(STD_INPUT_HANDLE);
    DWORD prev_mode;
    GetConsoleMode(hInput, &prev_mode);
    SetConsoleMode(hInput, prev_mode & ~ENABLE_QUICK_EDIT_MODE);
    
    // Redimensionar console
    RECT consoleRect;
    GetWindowRect(m_consoleHwnd, &consoleRect);
    SetWindowPos(m_consoleHwnd, HWND_TOP, 100, 100, 1280, 800, SWP_SHOWWINDOW);
    
    // Redirecionar stdout, stdin, stderr para o console
    FILE* pCout;
    FILE* pCin;
    FILE* pCerr;
    freopen_s(&pCout, "CONOUT$", "w", stdout);
    freopen_s(&pCin, "CONIN$", "r", stdin);
    freopen_s(&pCerr, "CONOUT$", "w", stderr);
    
    // Fazer o console aceitar input
    std::ios::sync_with_stdio(true);
    
    printf("[INFO] Console Interface Initialized\n");
    printf("[INFO] Creating DirectX 12 overlay on console window...\n");

    // Criar dispositivo D3D12 usando a janela do console
    if (!CreateDeviceD3D12(m_consoleHwnd)) {
        printf("[ERROR] Failed to create D3D12 device on console window\n");
        CleanupDeviceD3D12();
        return;
    }

    printf("[INFO] DirectX 12 device created successfully on console window\n");

    // NÃO inicializar ImGui aqui - usar o contexto do hookframe
    printf("[INFO] Console Interface ready! Press TAB to toggle menu\n");

    // Subclassificar o procedimento da janela do console
    m_originalWndProc = (WNDPROC)SetWindowLongPtr(m_consoleHwnd, GWLP_WNDPROC, (LONG_PTR)ConsoleWndProc);

    ImVec4 clear_color = ImVec4(0.0f, 0.0f, 0.0f, 0.0f); // Transparente para não interferir com o console

    MSG msg;
    ZeroMemory(&msg, sizeof(msg));
    while (m_running && msg.message != WM_QUIT) {
        if (PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            continue;
        }

        // Aguardar frame latency
        WaitForSingleObjectEx(m_hSwapChainWaitableObject, 1000, true);

        // Inicializar ImGui para este frame usando a função centralizada da hookframe
        if (!ImGui::GetCurrentContext()) {
            IMGUI_CHECKVERSION();
            InitImGui();
            
            ImGuiIO& io = ImGui::GetIO();
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
            
            if (!ImGui_ImplWin32_Init(m_consoleHwnd)) {
                printf("[ERROR] ImGui_ImplWin32_Init failed\n");
            }
            
            ImGui_ImplDX12_InitInfo init_info = {};
            init_info.Device = m_pd3dDevice;
            init_info.CommandQueue = m_pd3dCommandQueue;
            init_info.NumFramesInFlight = NUM_FRAMES_IN_FLIGHT;
            init_info.RTVFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
            init_info.DSVFormat = DXGI_FORMAT_UNKNOWN;
            init_info.SrvDescriptorHeap = m_pd3dSrvDescHeap;
            init_info.LegacySingleSrvCpuDescriptor = m_pd3dSrvDescHeap->GetCPUDescriptorHandleForHeapStart();
            init_info.LegacySingleSrvGpuDescriptor = m_pd3dSrvDescHeap->GetGPUDescriptorHandleForHeapStart();

            if (!ImGui_ImplDX12_Init(&init_info)) {
                printf("[ERROR] ImGui_ImplDX12_Init failed\n");
            }
                
            printf("[INFO] ImGui Context and backends initialized using hookframe logic. Backend: %s\n", 
                io.BackendRendererName ? io.BackendRendererName : "NULL");
        }

        ImGui_ImplDX12_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        // Chamar a função de UI completa da hookframe
        RenderBPRSUI();

        ImGui::Render();

        FrameContext* frameCtx = &m_frameContext[m_frameIndex % NUM_FRAMES_IN_FLIGHT];
        UINT backBufferIdx = m_pSwapChain->GetCurrentBackBufferIndex();

        frameCtx->CommandAllocator->Reset();

        D3D12_RESOURCE_BARRIER barrier = {};
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        barrier.Transition.pResource = m_mainRenderTargetResource[backBufferIdx];
        barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
        m_pd3dCommandList->Reset(frameCtx->CommandAllocator, nullptr);
        m_pd3dCommandList->ResourceBarrier(1, &barrier);

        const float clear_color_with_alpha[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
        m_pd3dCommandList->ClearRenderTargetView(m_mainRenderTargetDescriptor[backBufferIdx], clear_color_with_alpha, 0, nullptr);
        m_pd3dCommandList->OMSetRenderTargets(1, &m_mainRenderTargetDescriptor[backBufferIdx], FALSE, nullptr);
        m_pd3dCommandList->SetDescriptorHeaps(1, &m_pd3dSrvDescHeap);
        ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), m_pd3dCommandList);
        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
        m_pd3dCommandList->ResourceBarrier(1, &barrier);
        m_pd3dCommandList->Close();

        m_pd3dCommandQueue->ExecuteCommandLists(1, (ID3D12CommandList* const*)&m_pd3dCommandList);

        m_pSwapChain->Present(1, 0);

        UINT64 fenceValue = m_fenceLastSignaledValue + 1;
        m_pd3dCommandQueue->Signal(m_fence, fenceValue);
        m_fenceLastSignaledValue = fenceValue;
        m_frameContext[m_frameIndex % NUM_FRAMES_IN_FLIGHT].FenceValue = fenceValue;
        m_frameIndex++;

        Sleep(16); // ~60 FPS
    }

    WaitForLastSubmittedFrame();

    ImGui_ImplDX12_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D12();
    
    // Fechar console
    FreeConsole();
}