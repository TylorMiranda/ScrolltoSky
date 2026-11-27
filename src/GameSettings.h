#pragma once

// Lookup for engine INI settings (Skyrim.ini, then SkyrimPrefs.ini).
namespace GameSettings
{
	inline RE::Setting* Find(std::string_view a_name)
	{
		if (auto ini = RE::INISettingCollection::GetSingleton()) {
			if (auto s = ini->GetSetting(a_name)) {
				return s;
			}
		}
		if (auto prefs = RE::INIPrefSettingCollection::GetSingleton()) {
			if (auto s = prefs->GetSetting(a_name)) {
				return s;
			}
		}
		return nullptr;
	}

	inline std::optional<float> GetFloat(std::string_view a_name)
	{
		if (auto s = Find(a_name)) {
			return s->data.f;
		}
		return std::nullopt;
	}

	inline bool SetFloat(std::string_view a_name, float a_value)
	{
		if (auto s = Find(a_name)) {
			s->data.f = a_value;
			return true;
		}
		return false;
	}
}
