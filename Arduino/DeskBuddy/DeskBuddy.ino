/*
 * DeskBuddy.ino
 * Desk Buddy — Main Sketch (FreeRTOS dual-core architecture)
 *
 * Core assignment:
 *   Core 1 — uiTask    : input, state machine, clock, weather, display
 *   Core 0 — audioTask : mic, speaker (when soldered — placeholder for now)
 *
 * Files in this sketch folder:
 *   config.h
 *   wifi_credentials.h
 *   weather_config.h
 *   input.h    / input.cpp
 *   led.h      / led.cpp
 *   pomodoro.h / pomodoro.cpp
 *   state.h    / state.cpp
 *   clock.h    / clock.cpp
 *   weather.h  / weather.cpp
 *   display.h  / display.cpp
 *
 * Board settings (Arduino IDE):
 *   Board          : ESP32S3 Dev Module
 *   USB CDC On Boot: Enabled
 *   Flash Size     : 4MB
 *   PSRAM          : QSPI PSRAM
 *   Partition      : Default 4MB with spiffs
 *   Upload Speed   : 115200
 */

#include <math.h>
#include <string.h>

#include "config.h"
#include "input.h"
#include "led.h"
#include "pomodoro.h"
#include "state.h"
#include "clock.h"
#include "weather.h"
#include "display.h"

// ============================================================
// GLOBAL INSTANCES
// All managers are global so both tasks can access them safely
// ============================================================
InputManager    input;
LEDManager      led;
PomodoroManager pomodoro;
StateManager    stateManager(input, led, pomodoro);
ClockManager    clk;
WeatherManager  weather;

// Owned by uiTask, rebuilt from the managers above every render tick.
// display.cpp never reaches into clock/weather/pomodoro/state itself —
// this struct is the only handoff point (see display.h).
DisplayData displayData;

// ~10Hz — matches the budget display.h was animation-tuned around.
// Going faster just burns I2C bandwidth without looking any smoother.
#define DISPLAY_RENDER_INTERVAL_MS  100

// FreeRTOS task handles — useful for debugging and task management
TaskHandle_t uiTaskHandle    = NULL;
TaskHandle_t audioTaskHandle = NULL;

// ============================================================
// DeskBuddyState -> ScreenID
//
// These are two DIFFERENT enums with different member order
// (DeskBuddyState: IDLE,CLOCK,POMODORO,WEATHER,LISTENING,SPEAKING,PAT,SETTINGS
//  ScreenID:       IDLE,PAT,CLOCK,POMODORO,WEATHER,SETTINGS)
// so this must always be an explicit switch, never a raw cast — e.g.
// DeskBuddyState::CLOCK is value 1, but ScreenID::CLOCK is value 2.
// A cast would silently render the wrong screen.
// ============================================================
static ScreenID mapStateToScreen(DeskBuddyState state) {
  switch (state) {
    case DeskBuddyState::IDLE:      return ScreenID::IDLE;
    case DeskBuddyState::PAT:       return ScreenID::PAT;
    case DeskBuddyState::CLOCK:     return ScreenID::CLOCK;
    case DeskBuddyState::POMODORO:  return ScreenID::POMODORO;
    case DeskBuddyState::WEATHER:   return ScreenID::WEATHER;
    case DeskBuddyState::SETTINGS:  return ScreenID::SETTINGS;
    // No dedicated screen yet for these (Phase 2 / audio pipeline) —
    // display_render() itself also falls back to the idle face for
    // anything unmatched, this just makes that agreement explicit.
    case DeskBuddyState::LISTENING:
    case DeskBuddyState::SPEAKING:
    default:                        return ScreenID::IDLE;
  }
}

// ============================================================
// Pull fresh values from every manager into displayData.
// Called once per render tick (~10Hz), not once per uiTask loop.
// ============================================================
static void refreshDisplayData() {
  displayData.screen = mapStateToScreen(stateManager.getState());
  displayData.animTick++;   // free-running, never reset — per display.h

  // --- Clock ---
  displayData.hours      = clk.getHour();
  displayData.minutes    = clk.getMinute();
  displayData.weekday    = clk.getDayOfWeek();
  displayData.dayOfMonth = clk.getDay();

  // --- Pomodoro ---
  // getDisplayPhase() / getPhaseDurationSeconds() are display-aware
  // getters added to pomodoro.h for this integration:
  //   - getDisplayPhase() so a PAUSED work session still reads as
  //     FOCUS instead of flipping to BREAK while paused
  //   - getPhaseDurationSeconds() so the progress bar is computed
  //     against the real phase length (25/5/15 min), not a hardcoded
  //     25 minutes that made breaks show an almost-empty bar
  displayData.pomodoroIsWork       = (pomodoro.getDisplayPhase() == PomodoroPhase::WORK);
  displayData.pomodoroSecondsLeft  = (uint16_t)pomodoro.getRemainingSeconds();
  displayData.pomodoroTotalSeconds = (uint16_t)pomodoro.getPhaseDurationSeconds();

  uint8_t cycle = pomodoro.getCompletedCycles() + 1;   // show 1-4, not 0-3
  if (cycle > POMODORO_CYCLES_TO_LONG) cycle = POMODORO_CYCLES_TO_LONG;
  displayData.pomodoroCycle = cycle;

  // --- Weather ---
  // Only overwrite once a fetch has actually succeeded, so the screen
  // doesn't flash "0°C" for the few seconds it takes WiFi + the API
  // call to complete after boot.
  if (weather.isAvailable()) {
    displayData.tempC       = (int8_t)lroundf(weather.getTemperature());
    displayData.humidityPct = weather.getHumidity();
    displayData.windKmh     = (uint8_t)lroundf(weather.getWindSpeed() * 3.6f); // m/s -> km/h

    strncpy(displayData.weatherDesc, weather.getDescription().c_str(),
      sizeof(displayData.weatherDesc) - 1);
    displayData.weatherDesc[sizeof(displayData.weatherDesc) - 1] = '\0';
  }

  // --- Settings ---
  displayData.settingsSelectedIndex = stateManager.getSettingsIndex();
}

// ============================================================
// UI TASK — Core 1
// Handles: input, state machine, clock, weather, LED, display
// ============================================================
void uiTask(void *pvParameters) {
  Serial.println("[UI Task] Started on Core 1");

  // All UI-related initialization goes here
  stateManager.begin();
  clk.begin();
  weather.begin();
  display_init();   // owns Wire.begin() for the OLED's I2C pins

  unsigned long lastDisplayRenderMs = 0;

  // Task loop — never returns
  for (;;) {
    stateManager.update();
    clk.update();
    weather.update();

    // Render at ~10Hz, deliberately slower than the state/input loop
    if (millis() - lastDisplayRenderMs >= DISPLAY_RENDER_INTERVAL_MS) {
      lastDisplayRenderMs = millis();
      refreshDisplayData();
      display_render(displayData);
    }

    // Print time every 60 seconds when synced
    static unsigned long lastTimePrint = 0;
    if (clk.isSynced() && (millis() - lastTimePrint > 60000)) {
      lastTimePrint = millis();
      Serial.println("[Clock] " + clk.getFormattedDateTime());
    }

    // Yield to other tasks — MAIN_LOOP_DELAY_MS defined in config.h
    vTaskDelay(pdMS_TO_TICKS(MAIN_LOOP_DELAY_MS));
  }
}

// ============================================================
// AUDIO TASK — Core 0
// Handles: mic capture, wake word, speaker output (all pending)
// This task is a placeholder until mic/speaker are soldered
// ============================================================
void audioTask(void *pvParameters) {
  Serial.println("[Audio Task] Started on Core 0 (placeholder)");

  // Audio-related initialization will go here:
  //   mic.begin();
  //   speaker.begin();

  // Task loop — never returns
  for (;;) {
    // Audio processing will go here:
    //   mic.update();
    //   speaker.update();

    // Yield — audio task runs at 10ms intervals
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

// ============================================================
// SETUP — runs on Core 1 before tasks start
// Keep this minimal — task init goes inside the task functions
// ============================================================
void setup() {
  Serial.begin(SERIAL_BAUD_RATE);
  delay(1000);

  Serial.println("\n================================");
  Serial.println("  Desk Buddy — FreeRTOS Build");
  Serial.printf ("  Chip  : %s rev%d\n", ESP.getChipModel(), ESP.getChipRevision());
  Serial.printf ("  Cores : %d @ %d MHz\n", ESP.getChipCores(), ESP.getCpuFreqMHz());
  Serial.printf ("  Flash : %d MB\n", ESP.getFlashChipSize() / (1024 * 1024));
  Serial.printf ("  PSRAM : %d KB\n", ESP.getPsramSize() / 1024);
  Serial.printf ("  Heap  : %d bytes free\n", ESP.getFreeHeap());
  Serial.println("================================\n");

  // Create UI task pinned to Core 1
  // Stack size 8192 bytes — enough for display + state machine
  xTaskCreatePinnedToCore(
    uiTask,           // task function
    "UI Task",        // task name (for debugging)
    8192,             // stack size in bytes
    NULL,             // parameters (none)
    1,                // priority (1 = normal)
    &uiTaskHandle,    // task handle
    1                 // core (1 = Core 1)
  );

  // Create Audio task pinned to Core 0
  // Stack size 4096 bytes for now — will increase when audio is added
  xTaskCreatePinnedToCore(
    audioTask,
    "Audio Task",
    4096,
    NULL,
    1,
    &audioTaskHandle,
    0                 // core (0 = Core 0)
  );

  Serial.println("[Main] Both tasks created");
  Serial.printf ("[Main] UI Task stack: %d bytes\n", 8192);
  Serial.printf ("[Main] Audio Task stack: %d bytes\n", 4096);
}

// ============================================================
// LOOP — intentionally empty
// All work happens inside FreeRTOS tasks, not here.
// Arduino's loop() runs on Core 1 at lowest priority —
// leaving it empty ensures it doesn't interfere with uiTask.
// ============================================================
void loop() {
  vTaskDelay(pdMS_TO_TICKS(1000));   // idle loop — yields to tasks
}
