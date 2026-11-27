#include "Hooks.h"

#include "MapTransition.h"
#include "Zoom.h"

namespace
{
	// TESCameraState vtable slot for Update on SE/AE (VR is 0x4; VR is not targeted).
	constexpr std::size_t kUpdateIdx = 0x3;

	struct ThirdPersonUpdate
	{
		static void thunk(RE::ThirdPersonState* a_this, RE::BSTSmartPointer<RE::TESCameraState>& a_next)
		{
			Zoom::Get().PreUpdate(a_this);
			func(a_this, a_next);
		}
		static inline REL::Relocation<decltype(thunk)> func;

		static void Install()
		{
			REL::Relocation<std::uintptr_t> vtbl{ RE::VTABLE_ThirdPersonState[0] };
			func = vtbl.write_vfunc(kUpdateIdx, thunk);
			logs::info("hooked ThirdPersonState::Update");
		}
	};

	struct MapWorldUpdate
	{
		static void thunk(RE::MapCameraStates::World* a_this, RE::BSTSmartPointer<RE::TESCameraState>& a_next)
		{
			MapTransition::Get().OnMapCameraPreUpdate(a_this);
			func(a_this, a_next);
		}
		static inline REL::Relocation<decltype(thunk)> func;

		static void Install()
		{
			REL::Relocation<std::uintptr_t> vtbl{ RE::VTABLE_MapCameraStates__World[0] };
			func = vtbl.write_vfunc(kUpdateIdx, thunk);
			logs::info("hooked MapCameraStates::World::Update");
		}
	};
}

void Hooks::Install()
{
	ThirdPersonUpdate::Install();
	MapWorldUpdate::Install();
}
