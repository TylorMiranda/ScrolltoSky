#pragma once

// Milestones 1 + 2: extended third-person zoom range with eased, distance-scaled steps.
//
// The engine maps ThirdPersonState::currentZoomOffset (normalized, ~[fMinCurrentZoom, 1])
// onto [fVanityModeMinDist, fVanityModeMaxDist]. We raise fVanityModeMaxDist and own the
// normalized value ourselves, writing it into the state right before the engine's Update.
class Zoom
{
public:
	static Zoom& Get()
	{
		static Zoom z;
		return z;
	}

	void Init();

	// Called from the input sink. a_zoomOut: true = wheel down.
	void OnWheel(bool a_zoomOut);

	// Called from the ThirdPersonState::Update hook, before the original.
	void PreUpdate(RE::ThirdPersonState* a_state);

	// Called after our own map close: next update places the camera at a_normalized, no easing.
	void RequestReturnFromMap(float a_normalized);

	// Converts a camera distance in game units to the engine's normalized zoom value.
	[[nodiscard]] float DistanceToNormalized(float a_units) const;

	[[nodiscard]] float Target() const { return _target; }

private:
	float _min = 0.0f;
	float _target = 0.0f;
	float _current = 0.0f;
	float _lastWritten = 0.0f;
	bool  _synced = false;
	bool  _wheelSinceUpdate = false;
	bool  _returnPending = false;
	float _returnZoom = 1.0f;
	Clock::time_point _lastUpdate{};
};
