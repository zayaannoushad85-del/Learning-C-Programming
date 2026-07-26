/*
 * state.h
 * Desk Buddy — Application State Machine Manager
 *
 * Central coordinator that:
 *   - Owns the current DeskBuddyState
 *   - Reads input events from InputManager
 *   - Drives LED color via LEDManager
 *   - Controls PomodoroManager
 *   - Will later drive display and audio when those modules are ready
 *
 * State transition map:
 *
 *   Any state + MODE button  → cycles to next state
 *   Any state + BACK button  → returns to IDLE
 *
 *   IDLE       → tap touch   → PAT
 *   CLOCK      → (passive, just shows clock)
 *   POMODORO   → SELECT      → start / pause / resume
 *              → BACK        → reset + go IDLE
 *   SETTINGS   → SELECT      → confirm setting
 *              → MODE        → next setting item
 *              → BACK        → exit to IDLE
 *   WEATHER    → (passive display)
 *   LISTENING  → (entered programmatically by audio module)
 *   SPEAKING   → (entered programmatically by audio module)
 *   PAT        → auto-returns to IDLE after PAT_DURATION_MS
 *
 * Usage:
 *   #include "state.h"
 *   StateManager stateManager(input, led, pomodoro);
 *
 *   void setup() { stateManager.begin(); }
 *
 *   void loop() {
 *     stateManager.update();
 *   }
 */

#ifndef STATE_H
#define STATE_H

#include <Arduino.h>
#include "config.h"
#include "input.h"
#include "led.h"
#include "pomodoro.h"

// How long the PAT state stays before returning to IDLE (ms)
#define PAT_DURATION_MS     2000

// How many MODE presses to cycle through (matches DeskBuddyState count
// minus LISTENING and SPEAKING which are entered programmatically)
#define STATE_CYCLE_COUNT   6   // IDLE, CLOCK, POMODORO, WEATHER, PAT, SETTINGS

// Number of rows in the settings menu — must match the ITEMS[] array
// size in display.cpp's drawSettingsScreen()
#define SETTINGS_ITEM_COUNT 4

class StateManager {
  public:
    // Pass references to the already-constructed managers
    StateManager(InputManager &input, LEDManager &led, PomodoroManager &pomodoro);

    // Call once in setup()
    void begin();

    // Call every loop() — processes input and updates state
    void update();

    // Read current state from outside (display, audio modules will use this)
    DeskBuddyState getState();

    // Allow audio module to push LISTENING / SPEAKING states
    void setState(DeskBuddyState newState);

    // Debug: print current state to Serial
    void printState();

    // Which settings row is currently highlighted (0 to SETTINGS_ITEM_COUNT-1).
    // SELECT cycles through rows while in SETTINGS state.
    uint8_t getSettingsIndex();

  private:
    InputManager    &_input;
    LEDManager      &_led;
    PomodoroManager &_pomodoro;

    DeskBuddyState  _state;
    DeskBuddyState  _previousState;
    unsigned long   _stateEnteredAt;   // millis() when we entered current state
    unsigned long   _bootGuardUntil;   // ignore all inputs until this timestamp
    uint8_t         _settingsIndex;    // currently highlighted settings row

    void _transitionTo(DeskBuddyState newState);
    void _handleIdle();
    void _handleClock();
    void _handlePomodoro();
    void _handleWeather();
    void _handlePat();
    void _handleSettings();

    // Cycles IDLE → CLOCK → POMODORO → WEATHER → PAT → SETTINGS → IDLE
    DeskBuddyState _nextCycleState(DeskBuddyState current);

    const char* _stateName(DeskBuddyState state);
};

#endif // STATE_H
