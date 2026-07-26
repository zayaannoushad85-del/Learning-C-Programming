/*
 * pomodoro.h
 * Desk Buddy — Pomodoro Timer Logic
 *
 * Handles all Pomodoro timing, session tracking, and phase
 * transitions. Completely independent of display or audio —
 * other modules (display.cpp, led.cpp) read state from here
 * and react accordingly.
 *
 * Phases:
 *   IDLE        → timer not started yet
 *   WORK        → 25 min focus session (default)
 *   SHORT_BREAK → 5 min break after each work session
 *   LONG_BREAK  → 15 min break after every 4 work sessions
 *   PAUSED      → timer paused mid-session
 *   DONE        → all cycles complete
 *
 * Usage:
 *   #include "pomodoro.h"
 *   PomodoroManager pomodoro;
 *
 *   void setup() { pomodoro.begin(); }
 *
 *   void loop() {
 *     pomodoro.update();
 *
 *     if (pomodoro.isWorkSession())  { // show red LED }
 *     if (pomodoro.isBreak())        { // show green LED }
 *     if (pomodoro.phaseJustChanged()) { // play chime }
 *
 *     uint32_t remaining = pomodoro.getRemainingSeconds();
 *   }
 */

#ifndef POMODORO_H
#define POMODORO_H

#include <Arduino.h>
#include "config.h"

enum class PomodoroPhase {
  IDLE,
  WORK,
  SHORT_BREAK,
  LONG_BREAK,
  PAUSED,
  DONE
};

class PomodoroManager {
  public:
    PomodoroManager();

    // Call once in setup()
    void begin();

    // Call every loop() — advances the timer
    void update();

    // Controls
    void start();    // start from IDLE or resume from PAUSED
    void pause();    // pause mid-session
    void resume();   // resume from PAUSED
    void reset();    // reset everything back to IDLE
    void skip();     // skip current phase, jump to next

    // State queries
    PomodoroPhase getPhase();
    bool isWorkSession();
    bool isBreak();
    bool isPaused();
    bool isIdle();
    bool isDone();

    // True only for one update() cycle when a phase transition occurs
    // Use this to trigger a chime or animation
    bool phaseJustChanged();

    // Timer info
    uint32_t getRemainingSeconds();   // seconds left in current phase
    uint32_t getElapsedSeconds();     // seconds into current phase
    uint32_t getPhaseDurationSeconds(); // total length of current phase (25/5/15 min) — use this instead of hardcoding a duration when computing a progress bar
    uint8_t  getCompletedCycles();    // how many full work sessions done
    uint8_t  getTotalCycles();        // POMODORO_CYCLES_TO_LONG from config.h

    // Returns the phase to treat as "active" for display purposes.
    // While PAUSED, returns the phase that was paused (WORK/SHORT_BREAK/
    // LONG_BREAK) instead of PAUSED itself, so a paused work session
    // doesn't get mislabeled as a break on screen.
    PomodoroPhase getDisplayPhase();

    // Settings (can be updated from settings screen later)
    void setWorkMinutes(uint8_t mins);
    void setShortBreakMinutes(uint8_t mins);
    void setLongBreakMinutes(uint8_t mins);

  private:
    PomodoroPhase _phase;
    PomodoroPhase _lastPhase;

    uint8_t  _workMins;
    uint8_t  _shortBreakMins;
    uint8_t  _longBreakMins;
    uint8_t  _completedCycles;
    uint8_t  _totalCycles;

    uint32_t _phaseDurationSec;   // total duration of current phase in seconds
    uint32_t _elapsedSec;         // seconds elapsed in current phase
    unsigned long _lastTickMs;    // last millis() when we incremented elapsed

    bool _phaseChanged;

    void _startPhase(PomodoroPhase phase);
    uint32_t _phaseDuration(PomodoroPhase phase);
    void _advancePhase();
};

#endif // POMODORO_H
