/*
 * state.cpp
 * Desk Buddy — Application State Machine Manager (implementation)
 */

#include "state.h"

StateManager::StateManager(InputManager &input, LEDManager &led, PomodoroManager &pomodoro)
  : _input(input),
    _led(led),
    _pomodoro(pomodoro),
    _state(DeskBuddyState::IDLE),
    _previousState(DeskBuddyState::IDLE),
    _stateEnteredAt(0),
    _bootGuardUntil(0),
    _settingsIndex(0)
{}

void StateManager::begin() {
  // Initialize all sub-managers from here so DeskBuddy.ino
  // only needs to call stateManager.begin() in setup()
  _input.begin();
  _led.begin();
  _pomodoro.begin();

  _state          = DeskBuddyState::IDLE;
  _previousState  = DeskBuddyState::IDLE;
  _stateEnteredAt = millis();
  _bootGuardUntil = millis() + 500;   // ignore all inputs for first 500ms

  _led.setState(DeskBuddyState::IDLE);

  Serial.println("[State] Started in IDLE");
  Serial.println("[State] Boot guard active — ignoring inputs for 500ms");
}

// ============================================================
// MAIN UPDATE — called every loop()
// ============================================================

void StateManager::update() {
  // Always update LED and pomodoro regardless of boot guard
  _led.update();
  _pomodoro.update();

  // Still update input state during boot guard so debounce
  // logic settles, but don't act on any transitions yet
  _input.update();
  if (millis() < _bootGuardUntil) return;

  // Global: MODE button cycles to next state from anywhere
  // (except LISTENING / SPEAKING which are audio-controlled)
  if (_state != DeskBuddyState::LISTENING &&
      _state != DeskBuddyState::SPEAKING) {
    if (_input.wasModePressed()) {
      DeskBuddyState next = _nextCycleState(_state);
      _transitionTo(next);
      return;
    }
  }

  // Global: BACK from anywhere returns to IDLE
  // (except already IDLE, and audio states)
  if (_state != DeskBuddyState::IDLE &&
      _state != DeskBuddyState::LISTENING &&
      _state != DeskBuddyState::SPEAKING) {
    if (_input.wasBackPressed()) {
      if (_state == DeskBuddyState::POMODORO) {
        _pomodoro.reset();
      }
      _transitionTo(DeskBuddyState::IDLE);
      return;
    }
  }

  // Per-state handling
  switch (_state) {
    case DeskBuddyState::IDLE:      _handleIdle();      break;
    case DeskBuddyState::CLOCK:     _handleClock();     break;
    case DeskBuddyState::POMODORO:  _handlePomodoro();  break;
    case DeskBuddyState::WEATHER:   _handleWeather();   break;
    case DeskBuddyState::PAT:       _handlePat();       break;
    case DeskBuddyState::SETTINGS:  _handleSettings();  break;
    default: break;
  }
}

// ============================================================
// PUBLIC ACCESSORS
// ============================================================

DeskBuddyState StateManager::getState() {
  return _state;
}

void StateManager::setState(DeskBuddyState newState) {
  _transitionTo(newState);
}

uint8_t StateManager::getSettingsIndex() {
  return _settingsIndex;
}

void StateManager::printState() {
  Serial.printf("[State] Current: %s | Elapsed: %lums\n",
    _stateName(_state),
    millis() - _stateEnteredAt);
}

// ============================================================
// PER-STATE HANDLERS
// ============================================================

void StateManager::_handleIdle() {
  // Touch in IDLE → PAT reaction
  if (_input.wasTouched()) {
    _transitionTo(DeskBuddyState::PAT);
  }
}

void StateManager::_handleClock() {
  // Clock is passive — display module will render it
  // SELECT could be used later to toggle 12h/24h format
}

void StateManager::_handlePomodoro() {
  // SELECT → start / pause / resume
  if (_input.wasSelectPressed()) {
    if (_pomodoro.isIdle()) {
      _pomodoro.start();
      Serial.println("[Pomodoro] Started");
    } else if (_pomodoro.isPaused()) {
      _pomodoro.resume();
      Serial.println("[Pomodoro] Resumed");
    } else {
      _pomodoro.pause();
      Serial.println("[Pomodoro] Paused");
    }
  }

  // Reflect Pomodoro phase in LED
  if (_pomodoro.phaseJustChanged()) {
    if (_pomodoro.isWorkSession()) {
      _led.setColor(255, 20, 0, LEDEffect::SOLID);    // red = work
      Serial.println("[Pomodoro] Work session");
    } else if (_pomodoro.isBreak()) {
      _led.setColor(0, 200, 50, LEDEffect::PULSE);    // green pulse = break
      Serial.println("[Pomodoro] Break time");
    } else if (_pomodoro.isDone()) {
      _led.setColor(0, 200, 255, LEDEffect::BLINK);   // cyan blink = all done
      Serial.println("[Pomodoro] All cycles complete");
    } else if (_pomodoro.isPaused()) {
      _led.setColor(255, 200, 0, LEDEffect::BLINK);   // yellow blink = paused
    }
  }

  // Print remaining time every 60 seconds
  static unsigned long _lastPrint = 0;
  if (millis() - _lastPrint > 60000) {
    _lastPrint = millis();
    uint32_t rem = _pomodoro.getRemainingSeconds();
    Serial.printf("[Pomodoro] %lu min %lu sec remaining | Cycles: %d/%d\n",
      rem / 60, rem % 60,
      _pomodoro.getCompletedCycles(),
      _pomodoro.getTotalCycles());
  }
}

void StateManager::_handleWeather() {
  // Weather is passive — will be driven by WiFi/API module later
  // SELECT could force a refresh
}

void StateManager::_handlePat() {
  // Auto-return to IDLE after PAT_DURATION_MS
  if ((millis() - _stateEnteredAt) >= PAT_DURATION_MS) {
    _transitionTo(DeskBuddyState::IDLE);
  }
}

void StateManager::_handleSettings() {
  // SELECT cycles the highlighted row. Actually editing a row's value
  // (brightness/WiFi/volume/timezone) is still a placeholder — this
  // just gives settingsSelectedIndex a real data source for the display.
  if (_input.wasSelectPressed()) {
    _settingsIndex = (_settingsIndex + 1) % SETTINGS_ITEM_COUNT;
    Serial.printf("[Settings] Row: %d\n", _settingsIndex);
  }
}

// ============================================================
// PRIVATE HELPERS
// ============================================================

void StateManager::_transitionTo(DeskBuddyState newState) {
  if (newState == _state) return;   // no-op if already in this state

  _previousState  = _state;
  _state          = newState;
  _stateEnteredAt = millis();

  if (newState == DeskBuddyState::SETTINGS) {
    _settingsIndex = 0;   // always start the menu on the first row
  }

  _led.setState(newState);

  Serial.printf("[State] %s → %s\n",
    _stateName(_previousState),
    _stateName(_state));
}

DeskBuddyState StateManager::_nextCycleState(DeskBuddyState current) {
  switch (current) {
    case DeskBuddyState::IDLE:      return DeskBuddyState::CLOCK;
    case DeskBuddyState::CLOCK:     return DeskBuddyState::POMODORO;
    case DeskBuddyState::POMODORO:  return DeskBuddyState::WEATHER;
    case DeskBuddyState::WEATHER:   return DeskBuddyState::PAT;
    case DeskBuddyState::PAT:       return DeskBuddyState::SETTINGS;
    case DeskBuddyState::SETTINGS:  return DeskBuddyState::IDLE;
    default:                        return DeskBuddyState::IDLE;
  }
}

const char* StateManager::_stateName(DeskBuddyState state) {
  switch (state) {
    case DeskBuddyState::IDLE:      return "IDLE";
    case DeskBuddyState::CLOCK:     return "CLOCK";
    case DeskBuddyState::POMODORO:  return "POMODORO";
    case DeskBuddyState::WEATHER:   return "WEATHER";
    case DeskBuddyState::LISTENING: return "LISTENING";
    case DeskBuddyState::SPEAKING:  return "SPEAKING";
    case DeskBuddyState::PAT:       return "PAT";
    case DeskBuddyState::SETTINGS:  return "SETTINGS";
    default:                        return "UNKNOWN";
  }
}
