#include "MapTransition.h"

#include "GameSettings.h"
#include "Settings.h"
#include "Zoom.h"

namespace
{
	float Seconds(Clock::duration d)
	{
		return std::chrono::duration<float>(d).count();
	}
}

void MapTransition::Init()
{
	_iniFloor = GameSettings::GetFloat("fMapWorldMinHeight:MapMenu");
	if (_iniFloor) {
		logs::info("map floor from fMapWorldMinHeight = {}", *_iniFloor);
	} else {
		logs::info("fMapWorldMinHeight not found; map floor will be learned at runtime");
	}
}

// ---------------------------------------------------------------------------
// Third person -> map
// ---------------------------------------------------------------------------

void MapTransition::OnOverscrollOut()
{
	const auto& cfg = Settings::Get();
	if (!cfg.enableMapTransition) {
		return;
	}

	const auto now = Clock::now();
	if (_outCount == 0 || Seconds(now - _outFirst) > cfg.overscrollWindow) {
		_outCount = 0;
		_outFirst = now;
	}

	if (++_outCount >= cfg.overscrollNotches) {
		_outCount = 0;
		if (CanOpenMap()) {
			OpenMap();
		} else if (cfg.debugLogging) {
			logs::info("overscroll reached but map unavailable here");
		}
	}
}

bool MapTransition::CanOpenMap() const
{
	if (_mapOpen) {
		return false;
	}
	auto player = RE::PlayerCharacter::GetSingleton();
	if (!player) {
		return false;
	}
	auto cell = player->GetParentCell();
	if (!cell || !cell->IsExteriorCell() || !player->GetWorldspace()) {
		return false;
	}
	if (Settings::Get().blockInCombat && player->IsInCombat()) {
		return false;
	}
	auto ui = RE::UI::GetSingleton();
	if (!ui || ui->GameIsPaused()) {
		return false;
	}
	return true;
}

void MapTransition::OpenMap()
{
	auto queue = RE::UIMessageQueue::GetSingleton();
	if (!queue) {
		return;
	}
	_openedByUs = true;
	_forceLowFrames = Settings::Get().startMapZoomedIn ? 4 : 0;
	queue->AddMessage(RE::MapMenu::MENU_NAME, RE::UI_MESSAGE_TYPE::kShow, nullptr);
	logs::info("scroll-out: opening world map");
}

// ---------------------------------------------------------------------------
// Map -> third person
// ---------------------------------------------------------------------------

void MapTransition::OnMenu(bool a_opening)
{
	if (a_opening) {
		_mapOpen = true;
		_zValid = false;
		_inCount = 0;
		_lastIn = {};
		return;
	}

	_mapOpen = false;
	_openedByUs = false;
	_forceLowFrames = 0;

	if (_closedByUs) {
		_closedByUs = false;
		Zoom::Get().RequestReturnFromMap();
		logs::info("scroll-in: map closed, returning camera to player");
	}
}

void MapTransition::OnMapCameraPreUpdate(RE::MapCameraStates::World* a_state)
{
	if (!_mapOpen) {
		return;
	}

	// Milestone 5 groundwork: start the map at its lowest height so the
	// scroll-out reads as one continuous motion.
	if (_forceLowFrames > 0) {
		--_forceLowFrames;
		if (const auto floor = _iniFloor ? _iniFloor : _learnedFloor) {
			a_state->currentPosition.z = *floor;
		}
	}

	_z = a_state->currentPosition.z;
	_zValid = true;
}

void MapTransition::OnMapWheel(bool a_zoomOut)
{
	const auto& cfg = Settings::Get();
	if (!cfg.enableReturn || !_zValid) {
		return;
	}

	if (a_zoomOut) {
		_inCount = 0;
		return;
	}

	const auto now = Clock::now();

	bool atFloor = false;
	for (const auto& floor : { _iniFloor, _learnedFloor }) {
		if (floor) {
			const float tol = std::abs(*floor) * 0.02f + 10.0f;
			atFloor = atFloor || (_z <= *floor + tol);
		}
	}

	// Learn the floor: two spaced-out zoom-in notches with no height change.
	if (!atFloor && _lastIn != Clock::time_point{} && Seconds(now - _lastIn) > 0.2f &&
		std::abs(_z - _zAtLastIn) < 1.0f) {
		_learnedFloor = _z;
		atFloor = true;
		logs::info("learned map floor height = {}", _z);
	}
	_lastIn = now;
	_zAtLastIn = _z;

	if (!atFloor) {
		_inCount = 0;
		return;
	}

	if (_inCount == 0 || Seconds(now - _inFirst) > cfg.overscrollWindow) {
		_inCount = 0;
		_inFirst = now;
	}
	if (++_inCount >= cfg.returnNotches) {
		_inCount = 0;
		CloseMap();
	}
}

void MapTransition::CloseMap()
{
	auto queue = RE::UIMessageQueue::GetSingleton();
	if (!queue) {
		return;
	}
	_closedByUs = true;
	queue->AddMessage(RE::MapMenu::MENU_NAME, RE::UI_MESSAGE_TYPE::kHide, nullptr);
}
