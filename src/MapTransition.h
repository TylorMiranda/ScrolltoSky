#pragma once

// Milestones 3 + 4: open the world map on overscroll past max zoom, and close it
// when the player keeps scrolling in past the map's lowest camera height.
class MapTransition
{
public:
	static MapTransition& Get()
	{
		static MapTransition m;
		return m;
	}

	void Init();

	// Third-person side
	void OnOverscrollOut();
	void ResetOverscroll() { _outCount = 0; }

	// Map side
	void OnMapWheel(bool a_zoomOut);
	void OnMapButton(std::uint32_t a_mouseButton);
	void OnMapCameraPreUpdate(RE::MapCameraStates::World* a_state);
	void OnMenu(bool a_opening);

	[[nodiscard]] bool MapOpen() const { return _mapOpen; }

private:
	bool CanOpenMap() const;
	void OpenMap();
	void CloseMap(float a_landingZoom);
	void LogButtonConflict() const;

	// third-person overscroll
	int               _outCount = 0;
	Clock::time_point _outFirst{};

	// map state
	bool  _mapOpen = false;
	bool  _openedByUs = false;
	bool  _closedByUs = false;
	float _landingZoom = 1.0f;
	int   _forceLowFrames = 0;
	float _z = 0.0f;
	bool  _zValid = false;

	// map floor: engine INI value if present, otherwise learned from a stalled zoom
	std::optional<float> _iniFloor;
	std::optional<float> _learnedFloor;
	float             _zAtLastIn = 0.0f;
	Clock::time_point _lastIn{};

	int               _inCount = 0;
	Clock::time_point _inFirst{};
};
