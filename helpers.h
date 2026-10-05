#pragma once

#include <Windows.h>
#include <intrin.h>
#include <cstdio>
#include <string>
#include <memory>

#include "Il2cpp/wrapper.hpp"
#include "Il2cpp/image.hpp"
#include "Il2cpp/class.hpp"
#include "Il2cpp/method.hpp"
#include "hookframe.h"

#include "vec.h"
#include "vec2.h"

#ifndef UNITY_CALLING_CONVENTION
#define UNITY_CALLING_CONVENTION __fastcall*
#endif


namespace Functions
{


	inline bool get_screen_size(int& sw, int& sh)
	{
		static std::unique_ptr<Wrapper> w;
		if (!w)
			w = std::make_unique<Wrapper>();

		auto img = w->get_image("UnityEngine.CoreModule.dll");
		if (!img)
			return false;

		auto cls = img->get_class("Screen", "UnityEngine");
		if (!cls)
			return false;

		auto mW = cls->get_method("get_width", 0);
		auto mH = cls->get_method("get_height", 0);
		if (!mW || !mH)
			return false;

		const void* rw = mW->invoke_static();
		const void* rh = mH->invoke_static();
		if (!rw || !rh)
			return false;

		void* uw = Il2cpp::object_unbox(const_cast<void*>(rw));
		void* uh = Il2cpp::object_unbox(const_cast<void*>(rh));
		if (!uw || !uh)
			return false;

		sw = *reinterpret_cast<int*>(uw);
		sh = *reinterpret_cast<int*>(uh);
		return sw > 0 && sh > 0;
	}


	inline void* get_main_camera()
	{
		static std::unique_ptr<Wrapper> w;
		if (!w)
			w = std::make_unique<Wrapper>();

	auto img = w->get_image("UnityEngine.CoreModule.dll");
		if (!img)
		{
			return nullptr;
		}

		auto cls = img->get_class("Camera", "UnityEngine");
		if (!cls)
		{
			return nullptr;
		}

		auto m = cls->get_method("get_main", 0);
		if (!m)
		{
			return nullptr;
		}

		auto cam = const_cast<void*>(m->invoke_static());
		return cam;
	}

inline bool world_to_screen(const Vector3& world, Vector2& screen)
	{
		void* cam = get_main_camera();
		if (!cam)
			return false;

		static std::unique_ptr<Wrapper> w;
		if (!w)
			w = std::make_unique<Wrapper>();

	auto img = w->get_image("UnityEngine.CoreModule.dll");
		if (!img)
			return false;

		auto cls = img->get_class("Camera", "UnityEngine");
		if (!cls)
			return false;

		auto m2 = cls->get_method("WorldToScreenPoint", 2);
		auto m1 = cls->get_method("WorldToScreenPoint", 1);
		Vector3 temp = {};
		const Vector3* v = nullptr;
		if (m2)
		{
			int eye = 2;
			void* params[2] = { (void*)&world, &eye };
			const void* ret = m2->invoke(cam, params);
			if (ret)
			{
				void* unboxed = Il2cpp::object_unbox(const_cast<void*>(ret));
				if (unboxed)
				{
					v = reinterpret_cast<const Vector3*>(unboxed);
				}
			}
		}
		else if (m1)
		{
			void* params[1] = { (void*)&world };
			const void* ret = m1->invoke(cam, params);
			if (ret)
			{
				void* unboxed = Il2cpp::object_unbox(const_cast<void*>(ret));
				if (unboxed)
					v = reinterpret_cast<const Vector3*>(unboxed);
			}
		}
		if (!v || (v->x == 0.0f && v->y == 0.0f && v->z == 0.0f))
		{
			auto mi = cls->get_method("WorldToScreenPoint_Injected", 3);
			if (!mi)
				return false;
			int eye = 2;
			void* iparams[3] = { (void*)&world, (void*)&eye, (void*)&temp };
			mi->invoke(cam, iparams);
			v = &temp;
		}
		int sw = 0, sh = 0;
		if (g_imguiW > 0 && g_imguiH > 0)
		{
			sw = g_imguiW;
			sh = g_imguiH;
		}
		else if (!get_screen_size(sw, sh))
		{
			sw = GetSystemMetrics(SM_CXSCREEN);
			sh = GetSystemMetrics(SM_CYSCREEN);
		}
		float sx = v->x;
		float sy = (float)sh - v->y;
		if (v->z < 0.0f)
			return false;
		if (sx < 0.0f || sx > (float)sw || sy < 0.0f || sy > (float)sh)
			return false;
		screen = Vector2(sx, sy);
		return true;
	}

	// Versão estendida que mostra entidades mesmo atrás ou fora da tela
	inline bool world_to_screen_extended(const Vector3& world, Vector2& screen, bool& behindCamera)
	{
		void* cam = get_main_camera();
		if (!cam)
			return false;

		static std::unique_ptr<Wrapper> w;
		if (!w)
			w = std::make_unique<Wrapper>();

		auto img = w->get_image("UnityEngine.CoreModule.dll");
		if (!img)
			return false;

		auto cls = img->get_class("Camera", "UnityEngine");
		if (!cls)
			return false;

		auto m2 = cls->get_method("WorldToScreenPoint", 2);
		auto m1 = cls->get_method("WorldToScreenPoint", 1);
		Vector3 temp = {};
		const Vector3* v = nullptr;
		if (m2)
		{
			int eye = 2;
			void* params[2] = { (void*)&world, &eye };
			const void* ret = m2->invoke(cam, params);
			if (ret)
			{
				void* unboxed = Il2cpp::object_unbox(const_cast<void*>(ret));
				if (unboxed)
				{
					v = reinterpret_cast<const Vector3*>(unboxed);
				}
			}
		}
		else if (m1)
		{
			void* params[1] = { (void*)&world };
			const void* ret = m1->invoke(cam, params);
			if (ret)
			{
				void* unboxed = Il2cpp::object_unbox(const_cast<void*>(ret));
				if (unboxed)
					v = reinterpret_cast<const Vector3*>(unboxed);
			}
		}
		if (!v || (v->x == 0.0f && v->y == 0.0f && v->z == 0.0f))
		{
			auto mi = cls->get_method("WorldToScreenPoint_Injected", 3);
			if (!mi)
				return false;
			int eye = 2;
			void* iparams[3] = { (void*)&world, (void*)&eye, (void*)&temp };
			mi->invoke(cam, iparams);
			v = &temp;
		}
		int sw = 0, sh = 0;
		if (g_imguiW > 0 && g_imguiH > 0)
		{
			sw = g_imguiW;
			sh = g_imguiH;
		}
		else if (!get_screen_size(sw, sh))
		{
			sw = GetSystemMetrics(SM_CXSCREEN);
			sh = GetSystemMetrics(SM_CYSCREEN);
		}
		
		behindCamera = (v->z < 0.0f);
		
		float sx = v->x;
		float sy = (float)sh - v->y;
		
		// Se está atrás, inverter coordenadas para mostrar na tela
		if (behindCamera)
		{
			sx = (float)sw - sx;
			sy = (float)sh - sy;
		}
		
		// Clamp para bordas da tela se estiver fora
		if (sx < 0.0f) sx = 0.0f;
		if (sx > (float)sw) sx = (float)sw;
		if (sy < 0.0f) sy = 0.0f;
		if (sy > (float)sh) sy = (float)sh;
		
		screen = Vector2(sx, sy);
		return true;
	}

} // namespace Functions


