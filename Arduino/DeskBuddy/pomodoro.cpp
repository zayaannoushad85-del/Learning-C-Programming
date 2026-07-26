/*
 * pomodoro.cpp
 * Desk Buddy — Pomodoro Timer Logic (implementation)
 */

#include "pomodoro.h"

PomodoroManager::PomodoroManager()
  : _phase(PomodoroPhase::IDLE),
    _lastPhase(PomodoroPhase::IDLE),
    _workMins(POMODORO_WORK_MIN),
    _shortBreakMins(POMODORO_SHORT_BREAK),
    _longBreakMins(POMODORO_LONG_BREAK),
    _completedCycles(0),
    _totalCycles(POMODORO_CYCLES_TO_LONG),
    _phaseDurationSec(0),
    _elapsedSec(0),
    _lastTickMs(0),
    _phaseChanged(false)
{}

void PomodoroManager::begin() {
  reset();
}

void PomodoroManager::update() {
  // Clear the one-shot flag from the previous cycle
  _phaseChanged = false;

  if (_phase == PomodoroPhase::IDLE   ||
      _phase == PomodoroPhase::PAUSED ||
      _phase == PomodoroPhase::DONE) {
    return;
  }

  unsigned long now = millis();

  // Guard against millis() overflow (~49 day edge case)
  if (now < _lastTickMs) {
    _lastTickMs = now;
    return;
  }

  // Accumulate whole seconds only
  if ((now - _lastTickMs) >= 1000) {
    _elapsedSec++;
    _lastTickMs += 1000;   // step by exactly 1000ms to avoid drift

    if (_elapsedSec >= _phaseDurationSec) {
      _advancePhase();
    }
  }
}

// ============================================================
// CONTROLS
// ============================================================

void PomodoroManager::start() {
  if (_phase == PomodoroPhase::IDLE) {
    _completedCycles = 0;
    _startPhase(PomodoroPhase::WORK);
  } else if (_phase == PomodoroPhase::PAUSED) {
    resume();
  }
}

void PomodoroManager::pause() {
  if (_phase == PomodoroPhase::WORK       ||
      _phase == PomodoroPhase::SHORT_BREAK ||
      _phase == PomodoroPhase::LONG_BREAK) {
    _lastPhase = _phase;
    _phase = PomodoroPhase::PAUSED;
    _phaseChanged = true;
  }
}

void PomodoroManager::resume() {
  if (_phase == PomodoroPhase::PAUSED) {
    _phase = _lastPhase;
    _lastTickMs = millis();   // reset tick so we don't count paused time
    _phaseChanged = true;
  }
}

void PomodoroManager::reset() {
  _phase            = PomodoroPhase::IDLE;
  _lastPhase        = PomodoroPhase::IDLE;
  _completedCycles  = 0;
  _elapsedSec       = 0;
  _phaseDurationSec = 0;
  _lastTickMs       = 0;
  _phaseChanged     = true;
}

void PomodoroManager::skip() {
  if (_phase != PomodoroPhase::IDLE && _phase != PomodoroPhase::DONE) {
    _advancePhase();
  }
}

// ============================================================
// STATE QUERIES
// ============================================================

PomodoroPhase PomodoroManager::getPhase()       { return _phase; }
bool PomodoroManager::isWorkSession()           { return _phase == PomodoroPhase::WORK; }
bool PomodoroManager::isBreak()                 { return _phase == PomodoroPhase::SHORT_BREAK || _phase == PomodoroPhase::LONG_BREAK; }
bool PomodoroManager::isPaused()                { return _phase == PomodoroPhase::PAUSED; }
bool PomodoroManager::isIdle()                  { return _phase == PomodoroPhase::IDLE; }
bool PomodoroManager::isDone()                  { return _phase == PomodoroPhase::DONE; }
bool PomodoroManager::phaseJustChanged()        { return _phaseChanged; }
uint8_t  PomodoroManager::getCompletedCycles()  { return _completedCycles; }
uint8_t  PomodoroManager::getTotalCycles()      { return _totalCycles; }

uint32_t PomodoroManager::getRemainingSeconds() {
  if (_phaseDurationSec <= _elapsedSec) return 0;
  return _phaseDurationSec - _elapsedSec;
}

uint32_t PomodoroManager::getElapsedSeconds() {
  return _elapsedSec;
}

uint32_t PomodoroManager::getPhaseDurationSeconds() {
  return _phaseDurationSec;
}

PomodoroPhase PomodoroManager::getDisplayPhase() {
  // pause() always saves the phase-before-pause into _lastPhase right
  // before switching _phase to PAUSED, so this is safe — _lastPhase is
  // guaranteed to be WORK/SHORT_BREAK/LONG_BREAK here, never IDLE/DONE.
  return (_phase == PomodoroPhase::PAUSED) ? _lastPhase : _phase;
}

// ============================================================
// SETTINGS
// ============================================================

void PomodoroManager::setWorkMinutes(uint8_t mins) {
  _workMins = (mins > 0) ? mins : 1;
}

void PomodoroManager::setShortBreakMinutes(uint8_t mins) {
  _shortBreakMins = (mins > 0) ? mins : 1;
}

void PomodoroManager::setLongBreakMinutes(uint8_t mins) {
  _longBreakMins = (mins > 0) ? mins : 1;
}

// ============================================================
// PRIVATE HELPERS
// ============================================================

uint32_t PomodoroManager::_phaseDuration(PomodoroPhase phase) {
  switch (phase) {
    case PomodoroPhase::WORK:        return (uint32_t)_workMins       * 60;
    case PomodoroPhase::SHORT_BREAK: return (uint32_t)_shortBreakMins * 60;
    case PomodoroPhase::LONG_BREAK:  return (uint32_t)_longBreakMins  * 60;
    default: return 0;
  }
}

void PomodoroManager::_startPhase(PomodoroPhase phase) {
  _lastPhase        = _phase;
  _phase            = phase;
  _elapsedSec       = 0;
  _phaseDurationSec = _phaseDuration(phase);
  _lastTickMs       = millis();
  _phaseChanged     = true;
}

void PomodoroManager::_advancePhase() {
  switch (_phase) {
    case PomodoroPhase::WORK:
      _completedCycles++;
      if (_completedCycles >= _totalCycles) {
        // Enough cycles done — long break
        _startPhase(PomodoroPhase::LONG_BREAK);
      } else {
        _startPhase(PomodoroPhase::SHORT_BREAK);
      }
      break;

    case PomodoroPhase::SHORT_BREAK:
      // Back to work after a short break
      _startPhase(PomodoroPhase::WORK);
      break;

    case PomodoroPhase::LONG_BREAK:
      // Full cycle complete
      _completedCycles = 0;
      _phase = PomodoroPhase::DONE;
      _phaseChanged = true;
      break;

    default:
      break;
  }
}
