#include "Zoom.h"

#include "GameSettings.h"
#include "MapTransition.h"
#include "Settings.h"

void Zoom::Init()
{
	const auto& cfg = Settings::Get();

	if (auto old = GameSettings::GetFloat("fVanityModeMaxDist:Camera")) {
		GameSettings::SetFloat("fVanityModeMaxDist:Camera", cfg.maxZoomDistance);
		logs::info("fVanityModeMaxDist: {} -> {}", *old, cfg.maxZoomDistance);
	} else {
		logs::error("fVanityModeMaxDist:Camera not found; zoom range unchanged");
	}

	_min = GameSettings::GetFloat("fMinCurrentZoom:Camera").value_or(0.0f);
	logs::info("normalized zoom floor (fMinCurrentZoom) = {}", _min);
}

void Zoom::OnWheel(bool a_zoomOut)
{
	const auto& cfg = Settings::Get();
	_wheelSinceUpdate = true;

	if (!_synced) {
		return;
	}

	// Step grows with distance so the far end does not take 40 notches.
	const float step = cfg.zoomStep * (1.0f + cfg.distanceScaling * std::max(_target, 0.0f));

	if (a_zoomOut) {
		if (_target >= 1.0f - 1e-4f) {
			MapTransition::Get().OnOverscrollOut();
			return;
		}
		_target = std::min(1.0f, _target + step);
	} else {
		MapTransition::Get().ResetOverscroll();
		_target = std::max(_min, _target - step);
		// At _min the engine's own handler performs the first-person switch.
	}
}

void Zoom::RequestReturnFromMap(float a_normalized)
{
	_returnZoom = std::clamp(a_normalized, _min, 1.0f);
	_returnPending = true;
}

float Zoom::DistanceToNormalized(float a_units) const
{
	// Assumes the engine maps normalized zoom linearly onto [fVanityModeMinDist, fVanityModeMaxDist].
	// Verify in game; adjust here if the curve turns out non-linear.
	const float minDist = GameSettings::GetFloat("fVanityModeMinDist:Camera").value_or(150.0f);
	const float maxDist = GameSettings::GetFloat("fVanityModeMaxDist:Camera").value_or(Settings::Get().maxZoomDistance);
	if (maxDist <= minDist) {
		return 1.0f;
	}
	return std::clamp((a_units - minDist) / (maxDist - minDist), _min, 1.0f);
}

void Zoom::PreUpdate(RE::ThirdPersonState* a_state)
{
	const auto& cfg = Settings::Get();
	const auto  now = Clock::now();

	float dt = 0.016f;
	if (auto timer = RE::BSTimer::GetSingleton()) {
		dt = std::clamp(timer->realTimeDelta, 0.0f, 0.1f);
	}

	// Gap in updates = state was re-entered (load, first-person toggle, menu, mount).
	const bool reentered = !_synced || (now - _lastUpdate) > std::chrono::milliseconds(500);
	_lastUpdate = now;

	if (_returnPending) {
		_returnPending = false;
		_current = _target = _returnZoom;
		_synced = true;
	} else if (reentered) {
		_target = _current = std::clamp(a_state->targetZoomOffset, _min, 1.0f);
		_synced = true;
	} else if (!_wheelSinceUpdate && std::abs(a_state->targetZoomOffset - _lastWritten) > 1e-4f) {
		// Engine moved the zoom for its own reasons (savedZoomOffset restore, script). Follow it.
		_target = _current = std::clamp(a_state->targetZoomOffset, _min, 1.0f);
	}
	_wheelSinceUpdate = false;

	// Frame-rate independent exponential ease.
	const float alpha = 1.0f - std::exp(-cfg.smoothing * dt);
	_current += (_target - _current) * alpha;
	if (std::abs(_target - _current) < 1e-4f) {
		_current = _target;
	}

	a_state->targetZoomOffset = _current;
	a_state->currentZoomOffset = _current;
	_lastWritten = _current;
}
