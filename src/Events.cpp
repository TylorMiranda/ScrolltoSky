#include "Events.h"

#include "MapTransition.h"
#include "Settings.h"
#include "Zoom.h"

namespace
{
	constexpr std::uint32_t kWheelUp = RE::BSWin32MouseDevice::Key::kWheelUp;
	constexpr std::uint32_t kWheelDown = RE::BSWin32MouseDevice::Key::kWheelDown;

	bool PlayerInThirdPerson()
	{
		auto cam = RE::PlayerCamera::GetSingleton();
		return cam && cam->currentState && cam->currentState->id == RE::CameraState::kThirdPerson;
	}

	class InputSink final : public RE::BSTEventSink<RE::InputEvent*>
	{
	public:
		static InputSink* Get()
		{
			static InputSink s;
			return &s;
		}

		RE::BSEventNotifyControl ProcessEvent(RE::InputEvent* const* a_event, RE::BSTEventSource<RE::InputEvent*>*) override
		{
			if (!a_event) {
				return RE::BSEventNotifyControl::kContinue;
			}

			for (auto e = *a_event; e; e = e->next) {
				if (e->GetEventType() != RE::INPUT_EVENT_TYPE::kButton || e->GetDevice() != RE::INPUT_DEVICE::kMouse) {
					continue;
				}
				auto btn = e->AsButtonEvent();
				if (!btn || !btn->IsPressed()) {
					continue;
				}
				const auto id = btn->GetIDCode();
				if (id != kWheelUp && id != kWheelDown) {
					continue;
				}
				const bool zoomOut = (id == kWheelDown);
				Dispatch(zoomOut);
			}
			return RE::BSEventNotifyControl::kContinue;
		}

	private:
		static void Dispatch(bool a_zoomOut)
		{
			auto ui = RE::UI::GetSingleton();
			if (!ui) {
				return;
			}

			if (MapTransition::Get().MapOpen()) {
				// Ignore wheel input meant for the journal, console, etc. layered over the map.
				if (ui->IsMenuOpen(RE::Console::MENU_NAME) || ui->IsMenuOpen(RE::JournalMenu::MENU_NAME)) {
					return;
				}
				MapTransition::Get().OnMapWheel(a_zoomOut);
				return;
			}

			if (ui->GameIsPaused() || ui->IsMenuOpen(RE::Console::MENU_NAME) || !PlayerInThirdPerson()) {
				return;
			}
			if (auto controls = RE::ControlMap::GetSingleton(); controls && !controls->IsMovementControlsEnabled()) {
				return;
			}
			Zoom::Get().OnWheel(a_zoomOut);
		}
	};

	class MenuSink final : public RE::BSTEventSink<RE::MenuOpenCloseEvent>
	{
	public:
		static MenuSink* Get()
		{
			static MenuSink s;
			return &s;
		}

		RE::BSEventNotifyControl ProcessEvent(const RE::MenuOpenCloseEvent* a_event, RE::BSTEventSource<RE::MenuOpenCloseEvent>*) override
		{
			if (a_event && a_event->menuName == RE::MapMenu::MENU_NAME) {
				MapTransition::Get().OnMenu(a_event->opening);
			}
			return RE::BSEventNotifyControl::kContinue;
		}
	};
}

void Events::Install()
{
	if (auto input = RE::BSInputDeviceManager::GetSingleton()) {
		input->AddEventSink(InputSink::Get());
		logs::info("input sink installed");
	} else {
		logs::error("BSInputDeviceManager unavailable");
	}

	if (auto ui = RE::UI::GetSingleton()) {
		ui->AddEventSink<RE::MenuOpenCloseEvent>(MenuSink::Get());
		logs::info("menu sink installed");
	}
}
