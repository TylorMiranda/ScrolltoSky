# Scroll to Sky

Scroll the third-person camera out past the player, keep going, and you are on the world map. Scroll in on the map and you drop back to your character.

Native SKSE plugin, one DLL for every flat Skyrim runtime.

## Requirements

- Skyrim SE 1.5.97 or AE 1.6.x (Steam or GOG)
- SKSE matching your runtime
- Address Library for SKSE Plugins (the "All in one" package for SE/AE, or the GOG build)

Compatible with A Quality World Map (Vivid, Classic, and Paper). On the Paper map, zoom is locked by that mod, so scroll return stays inactive and click return is the way back.

## Install (MO2)

Install the release zip as a mod. Config: `SKSE/Plugins/ScrollToSky.ini`. Log: `Documents/My Games/Skyrim Special Edition/SKSE/ScrollToSky.log`.

## Milestones

| # | Feature | Status |
|---|---------|--------|
| 1 | Extended third-person max zoom (`fVanityModeMaxDist`) | Implemented, untested in game |
| 2 | Eased, distance-scaled wheel zoom | Implemented, untested in game |
| 3 | Overscroll past max zoom opens the world map | Implemented, untested in game |
| 4 | Return from map: wheel click lands at max third-person distance (default); optional scroll-in past map floor jumps to minimum third-person distance | Implemented, untested in game |
| 5 | Map opens at lowest height, preload, alignment polish | Groundwork only (`bStartMapZoomedIn`) |

## How it works

- `ThirdPersonState::Update` (vtable hook, slot 3): plugin owns the normalized zoom value and writes it into `targetZoomOffset`/`currentZoomOffset` before the engine runs, with frame-rate independent exponential easing.
- `BSInputDeviceManager` sink: mouse wheel notches drive the zoom target, overscroll counting, and map-side scroll-in.
- `UIMessageQueue`: shows/hides `MapMenu`.
- `MapCameraStates::World::Update` (vtable hook, slot 3): reads map camera height to detect the zoom floor (from `fMapWorldMinHeight:MapMenu`, or learned at runtime when zoom stalls).

## Build

CI builds on every push (GitHub Actions, `windows-latest`, xmake). Artifact is an MO2-ready `SKSE/Plugins` folder.

Local:

```
git clone --recursive <repo>
xmake config -m releasedbg --skyrim_vr=n
xmake build
```

Requires Visual Studio 2022 C++ workload and xmake.
