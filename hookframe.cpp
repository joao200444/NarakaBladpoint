#include "hookframe.h"
#include <detours.h>
#include "esp.h"
#include "aimbot.h"
#include "autoparry.h"
#include "noclip.h"
#include "autospin.h"
#include "dump_enemies_magnet.h"
// #include "features/tp.hpp" // REMOVIDO
#include "auth.h"
#include "font_manager.h"
#include "fonts/zh-cn.h"
#include "PipeServer.hpp"
#include <stdio.h>
#include "Cheats.hpp"
#include "imgui/imgui_internal.h"
#include <fstream>
#include <ctime>
#include <iostream>

// #include "features/GiftBoxDuplicator.hpp" // REMOVIDO
#include "features/Radar.hpp"
// #include "features/SmartGodMode.hpp" // REMOVIDO
// #include "features/MobVacuum.hpp" // REMOVIDO
// #include "features/KillAura.hpp" // REMOVIDO
// #include "features/SkipCutscene.hpp" // REMOVIDO
// #include "features/FOV.hpp" // REMOVIDO
// #include "features/AutoPickup.hpp" // REMOVIDO
#include "esp_config.h"
#include "offsets.h"
#include "dynamic_offsets.h"

// Extern para acessar g_visibleActors do ESP
extern std::vector<ActorData> g_visibleActors;

// Extern para funções do ESP
extern void InstallCharacterManagerHook();
extern void RemoveCharacterManagerHook();

// Extern para funções de teste de RVA
extern void test_rva_get_main_camera(RvaTestResult& result);
extern void test_rva_get_transform(RvaTestResult& result);
extern void test_rva_get_position(RvaTestResult& result);
extern void test_rva_get_alive_char_list(RvaTestResult& result);
extern void test_rva_get_main_player(RvaTestResult& result);
extern void test_rva_get_hp(RvaTestResult& result);
extern void test_rva_get_shield(RvaTestResult& result);
extern void test_rva_is_local_client(RvaTestResult& result);
extern bool get_camera_position(Vector3& pos);

// Função de reconexão de hooks
void ReconnectHooks(
    uintptr_t rvaGetMainCamera_,
    uintptr_t rvaGetTransform_,
    uintptr_t rvaGetPosition_,
    uintptr_t rvaSetPosition_,
    uintptr_t rvaGetEulerAngles_,
    uintptr_t rvaSetEulerAngles_,
    uintptr_t rvaWorldToScreen_,
    uintptr_t rvaFindObject_,
    uintptr_t rvaGetViewMatrix_,
    uintptr_t rvaGetProjectionMatrix_,
    uintptr_t rvaGetAliveCharList_,
    uintptr_t rvaGetMainPlayer_,
    uintptr_t rvaGetModelType_,
    uintptr_t rvaGetActorModelFid_,
    uintptr_t rvaGetLogicBoneTransform_,
    uintptr_t rvaIsLocalClient_,
    uintptr_t rvaGetHp_,
    uintptr_t rvaGetHpMax_,
    uintptr_t rvaGetShield_,
    uintptr_t rvaGetShieldMax_,
    int actorModelTransformOffset_,
    int blockParryAgentOffset_
) {
    printf("[DEBUG] Reconnecting hooks with new RVAs...\n");

    // Remover hooks antigos
    RemoveCharacterManagerHook();

    // Atualizar RVAs dinâmicos
    DynamicOffsets::UpdateFromUI(
        rvaGetMainCamera_,
        rvaGetTransform_,
        rvaGetPosition_,
        rvaSetPosition_,
        rvaGetEulerAngles_,
        rvaSetEulerAngles_,
        rvaWorldToScreen_,
        rvaFindObject_,
        rvaGetViewMatrix_,
        rvaGetProjectionMatrix_,
        rvaGetAliveCharList_,
        rvaGetMainPlayer_,
        rvaGetModelType_,
        rvaGetActorModelFid_,
        rvaGetLogicBoneTransform_,
        rvaIsLocalClient_,
        rvaGetHp_,
        rvaGetHpMax_,
        rvaGetShield_,
        rvaGetShieldMax_,
        actorModelTransformOffset_,
        blockParryAgentOffset_
    );

    // Reinstalar hooks com novos RVAs
    InstallCharacterManagerHook();

    printf("[DEBUG] Hooks reconnected successfully!\n");
}

Present oPresent;
Present1 oPresent1;
ExecuteCommandLists oExecuteCommandLists = NULL;

struct FrameContext {
    ID3D12CommandAllocator* CommandAllocator;
    ID3D12Resource* MainRenderTargetResource;
    D3D12_CPU_DESCRIPTOR_HANDLE MainRenderTargetDescriptor;
};

// DX12 Globals
ID3D12Device* g_pd3dDevice = NULL;
ID3D12DescriptorHeap* g_pd3dRtvDescHeap = NULL;
ID3D12DescriptorHeap* g_pd3dSrvDescHeap = NULL;
ID3D12CommandQueue* g_pd3dCommandQueue = NULL;
ID3D12GraphicsCommandList* g_pd3dCommandList = NULL;
FrameContext* g_frameContext = NULL;
UINT g_frameBufferCount = 0;

void __stdcall hkExecuteCommandLists(ID3D12CommandQueue* pCommandQueue, UINT NumCommandLists, ID3D12CommandList* const* ppCommandLists)
{
    if (!g_pd3dCommandQueue) {
        g_pd3dCommandQueue = pCommandQueue;
        printf("[DEBUG] CommandQueue Captured: %p\n", g_pd3dCommandQueue);
    }
    oExecuteCommandLists(pCommandQueue, NumCommandLists, ppCommandLists);
}

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Variável externa do dllmain.cpp
extern volatile bool g_consoleActivationRequested;

HWND window = NULL;
WNDPROC oWndProc;
ID3D11Device* pDevice = NULL;
ID3D11DeviceContext* pContext = NULL;
ID3D11RenderTargetView* mainRenderTargetView;
volatile bool g_unloadRequested = false;
volatile bool g_GameAssembly_SuperReady = false;
int g_imguiW = 0;
int g_imguiH = 0;
bool init = false;

// Função dummy para satisfazer o linker (console_window.cpp)
void RenderBPRSUI() {}

void SetupImGuiStyle()
{
	ImGuiStyle& style = ImGui::GetStyle();

	style.WindowPadding = ImVec2(15, 15);
	style.WindowRounding = 12.0f;
	style.FramePadding = ImVec2(8, 6);
	style.FrameRounding = 8.0f;
	style.ItemSpacing = ImVec2(10, 8);
	style.ItemInnerSpacing = ImVec2(6, 6);
	style.TouchExtraPadding = ImVec2(0, 0);
	style.IndentSpacing = 20;
	style.ScrollbarSize = 10;
	style.ScrollbarRounding = 5.0f;
	style.GrabMinSize = 10;
	style.GrabRounding = 5.0f;
	style.TabRounding = 6.0f;
	style.ChildRounding = 8.0f;

	ImVec4* colors = style.Colors;

	colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
	colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
	colors[ImGuiCol_WindowBg] = ImVec4(0.08f, 0.09f, 0.12f, 0.95f);
	colors[ImGuiCol_ChildBg] = ImVec4(0.10f, 0.12f, 0.15f, 1.00f);
	colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.09f, 0.12f, 0.98f);
	colors[ImGuiCol_Border] = ImVec4(0.20f, 0.25f, 0.30f, 0.50f);
	colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_FrameBg] = ImVec4(0.12f, 0.15f, 0.20f, 1.00f);
	colors[ImGuiCol_FrameBgHovered] = ImVec4(0.18f, 0.22f, 0.28f, 1.00f);
	colors[ImGuiCol_FrameBgActive] = ImVec4(0.22f, 0.28f, 0.35f, 1.00f);
	colors[ImGuiCol_TitleBg] = ImVec4(0.06f, 0.07f, 0.10f, 1.00f);
	colors[ImGuiCol_TitleBgActive] = ImVec4(0.06f, 0.07f, 0.10f, 1.00f);
	colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.06f, 0.07f, 0.10f, 1.00f);
	colors[ImGuiCol_MenuBarBg] = ImVec4(0.10f, 0.12f, 0.15f, 1.00f);
	colors[ImGuiCol_ScrollbarBg] = ImVec4(0.06f, 0.07f, 0.10f, 1.00f);
	colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.20f, 0.25f, 0.30f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.25f, 0.30f, 0.35f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.30f, 0.35f, 0.40f, 1.00f);
	colors[ImGuiCol_CheckMark] = ImVec4(0.28f, 0.56f, 1.00f, 1.00f);
	colors[ImGuiCol_SliderGrab] = ImVec4(0.28f, 0.56f, 1.00f, 1.00f);
	colors[ImGuiCol_SliderGrabActive] = ImVec4(0.40f, 0.68f, 1.00f, 1.00f);
	colors[ImGuiCol_Button] = ImVec4(0.20f, 0.25f, 0.30f, 1.00f);
	colors[ImGuiCol_ButtonHovered] = ImVec4(0.28f, 0.35f, 0.42f, 1.00f);
	colors[ImGuiCol_ButtonActive] = ImVec4(0.15f, 0.20f, 0.25f, 1.00f);
	colors[ImGuiCol_Header] = ImVec4(0.12f, 0.15f, 0.20f, 1.00f);
	colors[ImGuiCol_HeaderHovered] = ImVec4(0.18f, 0.22f, 0.28f, 1.00f);
	colors[ImGuiCol_HeaderActive] = ImVec4(0.22f, 0.28f, 0.35f, 1.00f);
	colors[ImGuiCol_Separator] = ImVec4(0.20f, 0.25f, 0.30f, 0.50f);
	colors[ImGuiCol_SeparatorHovered] = ImVec4(0.28f, 0.56f, 1.00f, 0.78f);
	colors[ImGuiCol_SeparatorActive] = ImVec4(0.28f, 0.56f, 1.00f, 1.00f);
	colors[ImGuiCol_ResizeGrip] = ImVec4(0.28f, 0.56f, 1.00f, 0.25f);
	colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.28f, 0.56f, 1.00f, 0.67f);
	colors[ImGuiCol_ResizeGripActive] = ImVec4(0.28f, 0.56f, 1.00f, 0.95f);
	colors[ImGuiCol_Tab] = ImVec4(0.10f, 0.12f, 0.15f, 0.86f);
	colors[ImGuiCol_TabHovered] = ImVec4(0.28f, 0.56f, 1.00f, 0.80f);
	colors[ImGuiCol_TabActive] = ImVec4(0.18f, 0.22f, 0.28f, 1.00f);
	colors[ImGuiCol_TabUnfocused] = ImVec4(0.10f, 0.12f, 0.15f, 0.97f);
	colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.15f, 0.20f, 0.25f, 1.00f);
	colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
	colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
	colors[ImGuiCol_PlotHistogram] = ImVec4(0.28f, 0.56f, 1.00f, 1.00f);
	colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.40f, 0.68f, 1.00f, 1.00f);
	colors[ImGuiCol_TableHeaderBg] = ImVec4(0.10f, 0.12f, 0.15f, 1.00f);
	colors[ImGuiCol_TableBorderStrong] = ImVec4(0.20f, 0.25f, 0.30f, 1.00f);
	colors[ImGuiCol_TableBorderLight] = ImVec4(0.15f, 0.18f, 0.22f, 1.00f);
	colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
	colors[ImGuiCol_TextSelectedBg] = ImVec4(0.28f, 0.56f, 1.00f, 0.35f);
	colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
	colors[ImGuiCol_NavHighlight] = ImVec4(0.28f, 0.56f, 1.00f, 1.00f);
	colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
	colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
	colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
}

void InitImGui()
{
	if (ImGui::GetCurrentContext()) return;

	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
	io.MouseDrawCursor = false;
	
	ImFontConfig fontConfig;
	fontConfig.OversampleH = 3;
	fontConfig.OversampleV = 3;
	fontConfig.PixelSnapH = true;

	static const ImWchar ranges[] = {
		0x0020, 0x00FF,
		0x2000, 0x206F,
		0x2190, 0x21FF,
		0x2600, 0x26FF,
		0x2700, 0x27BF,
		0,
	};

	io.Fonts->AddFontFromMemoryTTF((void*)zh_cn, 7232220, 16.0f, &fontConfig, ranges);
	SetupImGuiStyle();
	FontManager::Initialize();
	PipeServer::Start();
}

// Sistema de rastreamento de teclas via WndProc (mais seguro que GetAsyncKeyState)
static bool keyStates[256] = {false}; // Estado de todas as teclas virtuais
static bool altKeyPressed = false;
static bool xKeyPressed = false;

// Variável global para controle do menu (acessível pelo WndProc)
bool showMenu = false;

LRESULT __stdcall WndProc(const HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	// Só processar input do ImGui quando o menu estiver aberto
	if (showMenu && ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
		return true;

	// Passar todas as mensagens para o WndProc original
	return CallWindowProc(oWndProc, hWnd, uMsg, wParam, lParam);
}

// Função helper para verificar se uma tecla está pressionada (alternativa segura ao GetAsyncKeyState)
bool IsKeyPressed(int virtualKey) {
	if (virtualKey >= 0 && virtualKey < 256) {
		return keyStates[virtualKey];
	}
	return false;
}

void RenderImGui(IDXGISwapChain* pSwapChain)
{
	static bool imguiInitRequested = false;

	// Variáveis estáticas para a aba Development
	static bool debugGetMainCamera = false;
	static bool debugGetTransform = false;
	static bool debugGetPosition = false;
	static bool debugSetPosition = false;
	static bool debugGetEulerAngles = false;
	static bool debugSetEulerAngles = false;
	static bool debugWorldToScreen = false;
	static bool debugFindObject = false;
	static bool debugGetViewMatrix = false;
	static bool debugGetProjectionMatrix = false;
	static bool debugGetAliveCharList = false;
	static bool debugGetMainPlayer = false;
	static bool debugGetModelType = false;
	static bool debugGetActorModelFid = false;
	static bool debugGetLogicBoneTransform = false;
	static bool debugIsLocalClient = false;
	static bool debugGetHp = false;
	static bool debugGetHpMax = false;
	static bool debugGetShield = false;
	static bool debugGetShieldMax = false;

	// Variáveis para armazenar RVAs editáveis
	static uintptr_t rvaGetMainCamera = RVA_GET_MAIN_CAMERA;
	static uintptr_t rvaGetTransform = RVA_GET_TRANSFORM;
	static uintptr_t rvaGetPosition = RVA_GET_POSITION;
	static uintptr_t rvaSetPosition = RVA_SET_POSITION;
	static uintptr_t rvaGetEulerAngles = RVA_GET_EULER_ANGLES;
	static uintptr_t rvaSetEulerAngles = RVA_SET_EULER_ANGLES;
	static uintptr_t rvaWorldToScreen = RVA_WORLD_TO_SCREEN;
	static uintptr_t rvaFindObject = RVA_FIND_OBJECTS_OF_TYPE;
	static uintptr_t rvaGetViewMatrix = RVA_GET_VIEW_MATRIX;
	static uintptr_t rvaGetProjectionMatrix = RVA_GET_PROJECTION_MATRIX;
	static uintptr_t rvaGetAliveCharList = RVA_GET_ALIVE_CHAR_LIST;
	static uintptr_t rvaGetMainPlayer = RVA_GET_MAIN_PLAYER;
	static uintptr_t rvaGetModelType = RVA_GET_MODEL_TYPE;
	static uintptr_t rvaGetActorModelFid = RVA_GET_ACTOR_MODEL_FID;
	static uintptr_t rvaGetLogicBoneTransform = RVA_GET_LOGIC_BONE_TRANSFORM;
	static uintptr_t rvaIsLocalClient = RVA_IS_LOCAL_CLIENT;
	static uintptr_t rvaGetHp = RVA_ACTOR_GET_HP;
	static uintptr_t rvaGetHpMax = RVA_ACTOR_GET_HP_MAX;
	static uintptr_t rvaGetShield = RVA_ACTOR_GET_SHIELD;
	static uintptr_t rvaGetShieldMax = RVA_ACTOR_GET_SHIELD_MAX;
	static int actorModelTransformOffset = ACTORMODEL_TRANSFORM_OFFSET;
	static int blockParryAgentOffset = OFFSET_BLOCK_PARRY_AGENT;
	
	// Ativação do ImGui quando 'y' é pressionado
	if (!init && g_consoleActivationRequested) {
		imguiInitRequested = true;
	}
	
	if (!init && !imguiInitRequested) return;
	
	if (!init && imguiInitRequested)
	{
		if (SUCCEEDED(pSwapChain->GetDevice(__uuidof(ID3D11Device), (void**)&pDevice)))
		{
			pDevice->GetImmediateContext(&pContext);
			DXGI_SWAP_CHAIN_DESC sd;
			pSwapChain->GetDesc(&sd);
			window = sd.OutputWindow;
			ID3D11Texture2D* pBackBuffer;
			pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
			pDevice->CreateRenderTargetView(pBackBuffer, NULL, &mainRenderTargetView);
			pBackBuffer->Release();
			oWndProc = (WNDPROC)SetWindowLongPtr(window, GWLP_WNDPROC, (LONG_PTR)WndProc);
			
			InitImGui();
			ImGui_ImplWin32_Init(window);
			ImGui_ImplDX11_Init(pDevice, pContext);
			init = true;
			printf("[DEBUG] ImGui Initialized (DX11)\n");
		}
		return;
	}

	if (!init) return;

	// Toggle menu com ALT+X (usando GetAsyncKeyState apenas para toggle)
	static bool altXPressed = false;
	bool altXCurrentlyPressed = (GetAsyncKeyState(VK_MENU) & 0x8000) && (GetAsyncKeyState('X') & 0x8000);

	if (altXCurrentlyPressed && !altXPressed) {
		showMenu = !showMenu;
		printf("[DEBUG] Menu toggled: %s\n", showMenu ? "ON" : "OFF");
	}
	altXPressed = altXCurrentlyPressed;

	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	// Renderizar menu se estiver ativo
	if (showMenu) {
		ImGui::SetNextWindowSize(ImVec2(900, 650), ImGuiCond_FirstUseEver);
		ImGui::Begin("RiasGremory", &showMenu, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
		
		static int selectedCategory = 0;
		static int selectedTab = 0;
		
		ImVec2 windowSize = ImGui::GetContentRegionAvail();
		
		ImGui::BeginChild("Sidebar", ImVec2(180, 0), true);
		{
			ImGui::SetCursorPosY(20);
			ImGui::SetCursorPosX(20);
			ImGui::Text("RiasGremory");
			ImGui::Spacing();
			ImGui::Separator();
			ImGui::Spacing();
			
			const char* categories[] = {
				"Combat", "Visuals", "Players", "Items",
				"View", "Hud", "Misc", "Main",
				"Movement", "Cloud", "Inventory",
				"Grenades", "Configs"
			};

			for (int i = 0; i < 13; i++) {
				if (ImGui::Selectable(categories[i], selectedCategory == i)) {
					selectedCategory = i;
					selectedTab = 0;
				}
			}
		}
		ImGui::EndChild();
		
		ImGui::SameLine();
		
		ImGui::BeginChild("MainContent", ImVec2(0, 0), false);
		{
			if (selectedCategory == 0) {
				const char* tabs[] = { "GLOBALS", "WEAPONS" };
				if (ImGui::BeginTabBar("CombatTabs")) {
					for (int i = 0; i < 2; i++) {
						if (ImGui::BeginTabItem(tabs[i])) {
							selectedTab = i;
							ImGui::EndTabItem();
						}
					}
					ImGui::EndTabBar();
				}
				
				ImGui::Spacing();
				
				if (selectedTab == 0) {
					float contentWidth = ImGui::GetContentRegionAvail().x;
					
					ImGui::BeginChild("AimbotCard", ImVec2(contentWidth * 0.5f - 10, 0), true);
					{
						ImGui::Text("Aimbot");
						ImGui::Spacing();
						
						bool aimbotEnabled = aimbot_get_enabled();
						if (ImGui::Checkbox("Enable", &aimbotEnabled)) {
							aimbot_set_enabled(aimbotEnabled);
						}
						
						if (aimbotEnabled) {
							ImGui::Spacing();
							
							float fov = aimbot_get_fov();
							if (ImGui::SliderFloat("FOV (Radius)", &fov, 10.0f, 500.0f, "%.0f px")) {
								aimbot_set_fov(fov);
							}
							
							float minDist = aimbot_get_min_distance();
							if (ImGui::SliderFloat("Minimum Distance", &minDist, 0.0f, 20.0f, "%.1f m")) {
								aimbot_set_min_distance(minDist);
							}
							
							float smooth = aimbot_get_smooth();
							if (ImGui::SliderFloat("Smoothing", &smooth, 1.0f, 20.0f, "%.1f")) {
								aimbot_set_smooth(smooth);
							}
							
							float strength = aimbot_get_strength();
							if (ImGui::SliderFloat("Strength (Pull)", &strength, 0.1f, 2.0f, "%.2f")) {
								aimbot_set_strength(strength);
							}
							
							ImGui::Spacing();
							ImGui::Separator();
							ImGui::Text("Aim Height:");
							ImGui::Spacing();
							
							float heightOffset = aimbot_get_height_offset();
							if (ImGui::SliderFloat("Height Offset", &heightOffset, -1.0f, 1.0f, "%.2f")) {
								aimbot_set_height_offset(heightOffset);
							}
							ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "-1.0 = Feet | 0.0 = Center | +1.0 = Head");
							
							ImGui::Spacing();
							ImGui::Separator();
							ImGui::Text("Visual Options:");
							ImGui::Spacing();
							
							bool hideFov = aimbot_get_hide_fov();
							if (ImGui::Checkbox("Hide FOV Circle", &hideFov)) {
								aimbot_set_hide_fov(hideFov);
							}
							ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Hides circle but aimbot still works");
							
							ImGui::Spacing();
							
							bool aimbotDebug = aimbot_get_debug();
							if (ImGui::Checkbox("Debug Mode", &aimbotDebug)) {
								aimbot_set_debug(aimbotDebug);
							}
							
							ImGui::Spacing();
							ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Hold right mouse button to aim");
						}
					}
					ImGui::EndChild();
					
					ImGui::SameLine();
					
					ImGui::BeginChild("TriggerbotCard", ImVec2(0, 0), true);
					{
						ImGui::Text("Triggerbot");
						ImGui::Spacing();
						
						bool triggerbotEnabled = false;
						if (ImGui::Checkbox("Enable", &triggerbotEnabled)) {
							
						}
						
						if (triggerbotEnabled) {
							bool disableWhileFlashed = false;
							ImGui::Checkbox("Disable triggerbot while flashed", &disableWhileFlashed);
							bool disableThroughSmoke = false;
							ImGui::Checkbox("Disable triggerbot through smoke", &disableThroughSmoke);
						}
						
						ImGui::Spacing();
						ImGui::Text("Recoil");
						ImGui::Spacing();
						
						bool recoilEnabled = false;
						ImGui::Checkbox("Enable", &recoilEnabled);
						if (recoilEnabled) {
							bool returnCrosshair = false;
							ImGui::Checkbox("Return crosshair", &returnCrosshair);
							
							float startBullet = 3.0f;
							ImGui::SliderFloat("Start bullet", &startBullet, 0.0f, 10.0f);
							float xAxis = 0.93f;
							ImGui::SliderFloat("X Axis", &xAxis, 0.0f, 1.0f);
							float yAxis = 1.0f;
							ImGui::SliderFloat("Y Axis", &yAxis, 0.0f, 1.0f);
							float smooth = 1.5f;
							ImGui::SliderFloat("Smooth", &smooth, 0.0f, 5.0f);
						}
					}
					ImGui::EndChild();
				}
			} else if (selectedCategory == 1) {
				ImGui::Text("Visuals Category");
			} else if (selectedCategory == 2) {
				ImGui::Text("Players Category");
				
				bool espEnabled = esp_get_enabled();
				if (ImGui::Checkbox("Enable ESP", &espEnabled)) {
					esp_set_enabled(espEnabled);
				}
				
				if (espEnabled) {
					ImGui::Spacing();
					
					bool espDebug = esp_get_debug();
					if (ImGui::Checkbox("Debug Mode", &espDebug)) esp_set_debug(espDebug);
					ImGui::Spacing();
					ImGui::Separator();
					ImGui::Spacing();
					
					bool lines = esp_get_lines();
					if (ImGui::Checkbox("Lines", &lines)) esp_set_lines(lines);
					ImGui::SameLine();
					
					bool names = esp_get_names();
					if (ImGui::Checkbox("Names", &names)) esp_set_names(names);
					ImGui::SameLine();
					
					bool distance = esp_get_distance();
					if (ImGui::Checkbox("Distance", &distance)) esp_set_distance(distance);
					
					bool boxes = esp_get_boxes();
					if (ImGui::Checkbox("Boxes 2D", &boxes)) esp_set_boxes(boxes);
					ImGui::SameLine();
					
					bool circles = esp_get_circles();
					if (ImGui::Checkbox("Circles", &circles)) esp_set_circles(circles);
					ImGui::SameLine();
					
					bool triangles = esp_get_triangles();
					if (ImGui::Checkbox("Triangles", &triangles)) esp_set_triangles(triangles);
					
					bool dots = esp_get_dots();
					if (ImGui::Checkbox("Dots", &dots)) esp_set_dots(dots);
					ImGui::SameLine();
					
					bool crosshairs = esp_get_crosshairs();
					if (ImGui::Checkbox("Crosshairs", &crosshairs)) esp_set_crosshairs(crosshairs);
					
					bool box3d = esp_get_3d_boxes();
					if (ImGui::Checkbox("Boxes 3D", &box3d)) esp_set_3d_boxes(box3d);
					if (box3d) {
						ImGui::SameLine();
						bool box3dFilled = esp_get_3d_box_filled();
						if (ImGui::Checkbox("Filled", &box3dFilled)) esp_set_3d_box_filled(box3dFilled);
					}
					
					bool skeleton = esp_get_skeleton();
					if (ImGui::Checkbox("Skeleton", &skeleton)) esp_set_skeleton(skeleton);
				}
			} else if (selectedCategory == 3) {
				ImGui::Text("Items Category");
			} else if (selectedCategory == 4) {
				ImGui::Text("View Category");
				
				bool radarEnabled = Features::Radar::IsEnabled();
				if (ImGui::Checkbox("Enable Radar", &radarEnabled)) {
					Features::Radar::SetEnabled(radarEnabled);
				}
				
				if (radarEnabled) {
					ImGui::Spacing();
					
					float radarSize = Features::Radar::GetSize();
					if (ImGui::SliderFloat("Radar Size", &radarSize, 100.0f, 400.0f)) {
						Features::Radar::SetSize(radarSize);
					}
					
					float radarRange = Features::Radar::GetRange();
					if (ImGui::SliderFloat("Radar Range", &radarRange, 50.0f, 500.0f)) {
						Features::Radar::SetRange(radarRange);
					}
				}
			} else if (selectedCategory == 5) {
				ImGui::Text("Hud Category");
			} else if (selectedCategory == 6) {
				ImGui::Text("Misc Category");
				
				// Auto-Parry
				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Text("Auto-Parry");
				ImGui::Spacing();
				
				bool autoParryEnabled = autoparry_get_enabled();
				if (ImGui::Checkbox("Enable Auto-Parry", &autoParryEnabled)) {
					autoparry_set_enabled(autoParryEnabled);
				}
				
				if (autoParryEnabled) {
					ImGui::Spacing();
					
					bool autoParryDebug = autoparry_get_debug();
					if (ImGui::Checkbox("Debug Mode", &autoParryDebug)) {
						autoparry_set_debug(autoParryDebug);
					}
					ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Show debug messages in console");
					
					ImGui::Spacing();
					ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Auto-Parry will press L when enemy attacks");
					ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Ignores your own attacks");
				}
				
				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();
				
				// Dump Enemies Magnet
				ImGui::Text("Dump Enemies Magnet");
				ImGui::Spacing();
				
				bool magnetEnabled = magnet_get_enabled();
				if (ImGui::Checkbox("Enable Magnet", &magnetEnabled)) {
					magnet_set_enabled(magnetEnabled);
					Cheats::DumpEnemiesMagnet::enabled = magnetEnabled;
				}
				
				if (magnetEnabled) {
					ImGui::Spacing();
					
					float magnetDistance = magnet_get_distance();
					if (ImGui::SliderFloat("Distance from Camera", &magnetDistance, 1.0f, 20.0f, "%.1f m")) {
						magnet_set_distance(magnetDistance);
						Cheats::DumpEnemiesMagnet::distance = magnetDistance;
					}
					ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Distance to pull enemies in front of camera");
					
					ImGui::Spacing();
					
					bool freezeHeight = magnet_get_freeze_height();
					if (ImGui::Checkbox("Freeze Height", &freezeHeight)) {
						magnet_set_freeze_height(freezeHeight);
						Cheats::DumpEnemiesMagnet::freezeHeight = freezeHeight;
					}
					ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Keep enemies at their original height");
					
					ImGui::Spacing();
					
					bool excludeLocalPlayer = magnet_get_exclude_local_player();
					if (ImGui::Checkbox("Exclude Local Player", &excludeLocalPlayer)) {
						magnet_set_exclude_local_player(excludeLocalPlayer);
						Cheats::DumpEnemiesMagnet::excludeLocalPlayer = excludeLocalPlayer;
					}
					ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Don't magnetize yourself");
					
					ImGui::Spacing();
					
					bool magnetDebug = magnet_get_debug();
					if (ImGui::Checkbox("Debug Mode (Magnet)", &magnetDebug)) {
						magnet_set_debug(magnetDebug);
						Cheats::DumpEnemiesMagnet::debug = magnetDebug;
					}
					
					ImGui::Spacing();
					ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Pulls all enemies to 3m in front of camera");
					ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Positions are locked while enabled");
				}
			} else if (selectedCategory == 7) {
				ImGui::Text("Main Category");
			} else if (selectedCategory == 8) {
				ImGui::Text("Movement Category");
			} else if (selectedCategory == 9) {
				ImGui::Text("Cloud Category");
			} else if (selectedCategory == 10) {
				ImGui::Text("Inventory Category");
			} else if (selectedCategory == 11) {
				ImGui::Text("Grenades Category");
			} else if (selectedCategory == 12) {
				ImGui::Text("Configs Category");
			}
		}
		ImGui::EndChild();
		
		ImGui::End();
	}

	// Renderizar ESP
	esp_render();
	
	// Renderizar Radar
	Features::Radar::Render();
	
	// Renderizar janela de debug do Auto-Parry (canto superior direito)
	if (autoparry_get_enabled() && autoparry_get_debug())
	{
		ImGuiIO& io = ImGui::GetIO();
		ImVec2 windowPos = ImVec2(io.DisplaySize.x - 420.0f, 10.0f);
		ImGui::SetNextWindowPos(windowPos, ImGuiCond_Always);
		ImGui::SetNextWindowSize(ImVec2(410, 0), ImGuiCond_Always);
		
		ImGui::Begin("Auto-Parry Debug", nullptr, 
			ImGuiWindowFlags_NoCollapse | 
			ImGuiWindowFlags_NoResize | 
			ImGuiWindowFlags_AlwaysAutoResize);
		
		ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "AUTO-PARRY DEBUG");
		ImGui::Separator();
		ImGui::Spacing();
		
		// Mostrar informações do jogador local
		void* localPlayer = autoparry_get_local_player();
		if (localPlayer)
		{
			ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Local Player: 0x%p", localPlayer);
		}
		else
		{
			ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Local Player: Not captured");
		}
		
		ImGui::Spacing();
		ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Auto-Parry simplified - only L key press");
		
		ImGui::End();
	}
	
	// Renderizar Aimbot FOV
	aimbot_render();
	
	// Executar Aimbot
	aimbot_tick();
	
	// Executar Auto-Parry
	autoparry_tick();
	
	// Executar Dump Enemies Magnet
	magnet_update();
	
	// NoClip e AutoSpin são executados dentro do esp_render()

	ImGui::EndFrame();
	ImGui::Render();
	
	ID3D11Texture2D* pBackBuffer = nullptr;
	ID3D11RenderTargetView* pRTV = nullptr;
	if (SUCCEEDED(pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer)) && pBackBuffer) {
		if (SUCCEEDED(pDevice->CreateRenderTargetView(pBackBuffer, NULL, &pRTV))) {
			pContext->OMSetRenderTargets(1, &pRTV, nullptr);
			ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
			pRTV->Release();
		}
		pBackBuffer->Release();
	}
}

HRESULT __stdcall hkPresent(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags)
{
	RenderImGui(pSwapChain);
	return oPresent(pSwapChain, SyncInterval, Flags);
}

HRESULT __stdcall hkPresent1(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT PresentFlags, const DXGI_PRESENT_PARAMETERS* pPresentParameters)
{
	RenderImGui(pSwapChain);
	return oPresent1(pSwapChain, SyncInterval, PresentFlags, pPresentParameters);
}
