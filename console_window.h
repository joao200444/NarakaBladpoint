#pragma once
#include <windows.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <thread>
#include <atomic>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

class ConsoleWindow {
public:
    static ConsoleWindow& Get() {
        static ConsoleWindow instance;
        return instance;
    }

    bool Initialize();
    void Shutdown();
    bool IsRunning() const { return m_running; }

private:
    ConsoleWindow() = default;
    ~ConsoleWindow() { Shutdown(); }

    void RenderThread();
    static LRESULT CALLBACK ConsoleWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    
    bool CreateDeviceD3D12(HWND hWnd);
    void CleanupDeviceD3D12();
    void CreateRenderTarget();
    void CleanupRenderTarget();
    void WaitForLastSubmittedFrame();
    
    struct FrameContext {
        ID3D12CommandAllocator* CommandAllocator;
        UINT64 FenceValue;
    };

    static constexpr int NUM_FRAMES_IN_FLIGHT = 3;
    static constexpr int NUM_BACK_BUFFERS = 3;

    FrameContext m_frameContext[NUM_FRAMES_IN_FLIGHT] = {};
    UINT m_frameIndex = 0;

    ID3D12Device* m_pd3dDevice = nullptr;
    ID3D12DescriptorHeap* m_pd3dRtvDescHeap = nullptr;
    ID3D12DescriptorHeap* m_pd3dSrvDescHeap = nullptr;
    ID3D12CommandQueue* m_pd3dCommandQueue = nullptr;
    ID3D12GraphicsCommandList* m_pd3dCommandList = nullptr;
    ID3D12Fence* m_fence = nullptr;
    HANDLE m_fenceEvent = nullptr;
    UINT64 m_fenceLastSignaledValue = 0;
    IDXGISwapChain3* m_pSwapChain = nullptr;
    HANDLE m_hSwapChainWaitableObject = nullptr;
    ID3D12Resource* m_mainRenderTargetResource[NUM_BACK_BUFFERS] = {};
    D3D12_CPU_DESCRIPTOR_HANDLE m_mainRenderTargetDescriptor[NUM_BACK_BUFFERS] = {};

    HWND m_hwnd = nullptr;
    HWND m_consoleHwnd = nullptr;
    WNDPROC m_originalWndProc = nullptr;
    std::thread m_renderThread;
    std::atomic<bool> m_running{ false };
};