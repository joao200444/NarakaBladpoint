#include <Windows.h>
#include <cstdio>
#include <cstring>
#include <memory>
#include "Il2cpp/wrapper.hpp"
#include "helpers.h"
#include "Hooks.hpp"

// Variáveis globais para armazenar as classes e funções Mono carregadas
static HMODULE g_GameAssembly_Super = nullptr;
static void* g_MonoDomain = nullptr;
static void* g_AssemblyCSharpImage = nullptr;
static void* g_UnityEngineCoreImage = nullptr;
static void* g_CharacterManagerClass = nullptr;
static void* g_ActorModelClass = nullptr;
static void* g_ActorVisualClass = nullptr;
static void* g_CameraClass = nullptr;

// Ponteiros para funções Mono
typedef void* (*mono_get_root_domain_t)();
typedef void* (*mono_thread_attach_t)(void*);
typedef void* (*mono_assembly_get_image_t)(void*);
typedef void* (*mono_domain_assembly_iter_t)(void*, void**);
typedef const char* (*mono_image_get_name_t)(void*);
typedef int (*il2cpp_image_get_class_count_t)(void*);
typedef void* (*il2cpp_image_get_class_t)(void*, int);
typedef const char* (*mono_class_get_name_t)(void*);
typedef const char* (*mono_class_get_namespace_t)(void*);

static mono_get_root_domain_t g_mono_get_root_domain = nullptr;
static mono_thread_attach_t g_mono_thread_attach = nullptr;
static mono_assembly_get_image_t g_mono_image_get_assembly = nullptr;
static mono_domain_assembly_iter_t g_mono_domain_get_assemblies_iter = nullptr;
static mono_image_get_name_t g_mono_image_get_name = nullptr;
static il2cpp_image_get_class_count_t g_il2cpp_image_get_class_count = nullptr;
static il2cpp_image_get_class_t g_il2cpp_image_get_class = nullptr;
static mono_class_get_name_t g_mono_class_get_name = nullptr;
static mono_class_get_namespace_t g_mono_class_get_namespace = nullptr;

// Funções getter para o ESP acessar as classes carregadas
void* GetMonoDomain() { return g_MonoDomain; }
void* GetAssemblyCSharpImage() { return g_AssemblyCSharpImage; }
void* GetUnityEngineCoreImage() { return g_UnityEngineCoreImage; }
void* GetCharacterManagerClass() { return g_CharacterManagerClass; }
void* GetActorModelClass() { return g_ActorModelClass; }
void* GetActorVisualClass() { return g_ActorVisualClass; }
void* GetCameraClass() { return g_CameraClass; }
HMODULE GetGameAssemblySuper() { return g_GameAssembly_Super; }

void RunIl2cppScan(HMODULE module)
{
	printf("[DEBUG] IL2CPP scan DISABLED - using RVAs directly\n");
	
	// Apenas carregar o módulo GameAssembly_Super.dll
	for (int attempts = 0; attempts < 60; attempts++) {
		g_GameAssembly_Super = GetModuleHandleA("GameAssembly_Super.dll");
		if (g_GameAssembly_Super) {
			printf("[DEBUG] GameAssembly_Super.dll found at: 0x%p\n", g_GameAssembly_Super);
			printf("[DEBUG] Ready to use RVA offsets\n");
			return;
		}
		Sleep(1000);
	}
	
	printf("[DEBUG] GameAssembly_Super.dll not found\n");
}


