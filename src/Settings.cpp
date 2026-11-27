#include "Settings.h"

namespace
{
	std::string Trim(std::string s)
	{
		const auto first = s.find_first_not_of(" \t\r\n");
		if (first == std::string::npos) {
			return {};
		}
		const auto last = s.find_last_not_of(" \t\r\n");
		return s.substr(first, last - first + 1);
	}

	std::string Lower(std::string s)
	{
		std::ranges::transform(s, s.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
		return s;
	}
}

void Settings::Load()
{
	constexpr auto path = "Data/SKSE/Plugins/ScrollToSky.ini";

	std::ifstream file(path);
	if (!file.is_open()) {
		logs::warn("{} not found, using defaults", path);
		return;
	}

	// key (lowercase, "section.key") -> raw value
	std::unordered_map<std::string, std::string> kv;
	std::string section;
	std::string line;
	while (std::getline(file, line)) {
		line = Trim(line);
		if (line.empty() || line[0] == ';' || line[0] == '#') {
			continue;
		}
		if (line.front() == '[' && line.back() == ']') {
			section = Lower(Trim(line.substr(1, line.size() - 2)));
			continue;
		}
		const auto eq = line.find('=');
		if (eq == std::string::npos) {
			continue;
		}
		auto value = Trim(line.substr(eq + 1));
		if (const auto c = value.find(';'); c != std::string::npos) {
			value = Trim(value.substr(0, c));
		}
		kv[section + "." + Lower(Trim(line.substr(0, eq)))] = value;
	}

	auto getF = [&](const char* key, float& out) {
		if (auto it = kv.find(key); it != kv.end()) {
			try {
				out = std::stof(it->second);
			} catch (...) {
				logs::warn("bad float for {}: {}", key, it->second);
			}
		}
	};
	auto getI = [&](const char* key, int& out) {
		if (auto it = kv.find(key); it != kv.end()) {
			try {
				out = std::stoi(it->second);
			} catch (...) {
				logs::warn("bad int for {}: {}", key, it->second);
			}
		}
	};
	auto getB = [&](const char* key, bool& out) {
		if (auto it = kv.find(key); it != kv.end()) {
			const auto v = Lower(it->second);
			out = (v == "1" || v == "true" || v == "yes" || v == "on");
		}
	};

	getF("zoom.fmaxzoomdistance", maxZoomDistance);
	getF("zoom.fzoomstep", zoomStep);
	getF("zoom.fdistancescaling", distanceScaling);
	getF("zoom.fsmoothing", smoothing);

	getB("map.benablemaptransition", enableMapTransition);
	getI("map.ioverscrollnotches", overscrollNotches);
	getF("map.foverscrollwindow", overscrollWindow);
	getB("map.bstartmapzoomedin", startMapZoomedIn);
	getB("map.bblockincombat", blockInCombat);

	getB("return.bscrollreturn", scrollReturn);
	getI("return.ireturnnotches", returnNotches);

	getB("debug.bdebuglogging", debugLogging);

	maxZoomDistance = (std::max)(maxZoomDistance, 100.0f);
	zoomStep = std::clamp(zoomStep, 0.005f, 0.5f);
	distanceScaling = std::clamp(distanceScaling, 0.0f, 10.0f);
	smoothing = std::clamp(smoothing, 0.5f, 60.0f);
	overscrollNotches = (std::max)(overscrollNotches, 1);
	returnNotches = (std::max)(returnNotches, 1);

	logs::info("settings: maxDist={} step={} scaling={} smoothing={} map={} overscroll={} scrollReturn={}",
		maxZoomDistance, zoomStep, distanceScaling, smoothing, enableMapTransition, overscrollNotches,
		scrollReturn);
}
