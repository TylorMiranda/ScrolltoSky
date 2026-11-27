#include "Events.h"
#include "Hooks.h"
#include "MapTransition.h"
#include "Settings.h"
#include "Zoom.h"

namespace
{
	void OnMessage(SKSE::MessagingInterface::Message* a_msg)
	{
		if (a_msg->type == SKSE::MessagingInterface::kDataLoaded) {
			// INI settings collections are populated by now.
			Zoom::Get().Init();
			MapTransition::Get().Init();
			Events::Install();
		}
	}
}

SKSE_PLUGIN_LOAD(const SKSE::LoadInterface* a_skse)
{
	SKSE::Init(a_skse);

	const auto ver = a_skse->RuntimeVersion();
	logs::info("Scroll to Sky v0.1.3 loading on Skyrim {}", ver.string());

	Settings::Get().Load();
	Hooks::Install();

	if (auto messaging = SKSE::GetMessagingInterface()) {
		messaging->RegisterListener(OnMessage);
	}

	logs::info("Scroll to Sky loaded");
	return true;
}
