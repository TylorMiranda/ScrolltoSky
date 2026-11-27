#pragma once

struct Settings
{
	// [Zoom]
	float maxZoomDistance = 3000.0f;  // game units; vanilla fVanityModeMaxDist is 600
	float zoomStep = 0.06f;           // normalized zoom per wheel notch near the player
	float distanceScaling = 2.0f;     // step grows with distance: step * (1 + scaling * zoom)
	float smoothing = 9.0f;           // easing speed, higher = snappier

	// [Map]
	bool  enableMapTransition = true;
	int   overscrollNotches = 2;      // extra notches past max zoom before the map opens
	float overscrollWindow = 0.8f;    // seconds; notches must land inside this window
	bool  startMapZoomedIn = true;    // open the map at its lowest height (experimental)
	bool  blockInCombat = false;

	// [Return]
	bool  clickReturn = true;          // mouse button on the map returns to max third-person distance
	int   returnButton = 2;            // mouse button id: 2 = wheel click
	bool  scrollReturn = false;        // scroll in past the map's lowest height returns to the player
	int   returnNotches = 2;           // notches past the map floor before closing (scrollReturn)
	float scrollReturnDistance = 210.0f;  // game units the camera jumps to (210 ~= 3 m)

	// [Debug]
	bool  debugLogging = false;

	static Settings& Get()
	{
		static Settings s;
		return s;
	}

	void Load();
};
