#pragma once
#include <Windows.h>
#include <d3d11.h>
#include <d3d12.h> // Adicionado suporte a D3D12
#include <dxgi.h>
#include <dxgi1_4.h> // Necessário para IDXGISwapChain3
#include "Kiro/kiero.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_dx11.h"
#include "imgui/imgui_impl_dx12.h" // Adicionado header D3D12

// Forward declaration para ID3D12CommandQueue (necessario para hook DX12)
struct ID3D12CommandQueue;
struct ID3D12CommandList;

typedef HRESULT(__stdcall* Present) (IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);
typedef HRESULT(__stdcall* Present1) (IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT PresentFlags, const DXGI_PRESENT_PARAMETERS* pPresentParameters);

// Typedef correto para ExecuteCommandLists (Index 54 do ID3D12CommandQueue)
typedef void(__stdcall* ExecuteCommandLists)(ID3D12CommandQueue* pCommandQueue, UINT NumCommandLists, ID3D12CommandList* const* ppCommandLists);

typedef LRESULT(CALLBACK* WNDPROC)(HWND, UINT, WPARAM, LPARAM);
typedef uintptr_t PTR;

extern Present oPresent;
extern Present1 oPresent1;
extern ExecuteCommandLists oExecuteCommandLists;

HRESULT __stdcall hkPresent(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);
HRESULT __stdcall hkPresent1(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT PresentFlags, const DXGI_PRESENT_PARAMETERS* pPresentParameters);
void __stdcall hkExecuteCommandLists(ID3D12CommandQueue* pCommandQueue, UINT NumCommandLists, ID3D12CommandList* const* ppCommandLists);

extern HMODULE g_module;

extern volatile bool g_unloadRequested;

extern volatile bool g_showLoginOnce;

extern int g_imguiW;
extern int g_imguiH;

// Função exportada para desenho da UI (usada pelo Hook e pelo Overlay)
void DrawImGuiContent();
void InitImGui(); // Exported for Overlay

// Sistema seguro de detecção de teclas via WndProc (alternativa ao GetAsyncKeyState)
// Retorna true se a tecla virtual está pressionada (mais seguro que GetAsyncKeyState)
bool IsKeyPressed(int virtualKey);
