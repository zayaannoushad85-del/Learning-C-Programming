#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdint.h>
#include <U8g2lib.h>

// ============================================================================
// CHIP SELECT — flip this once the OLED is soldered and you've confirmed
// which controller it actually uses (SSD1306 and SH1106 look identical from
// outside; check the datasheet/silkscreen on the board, or just try one and
// see if the image is correct vs. shifted/garbled).
// ============================================================================
#define OLED_CHIP_SSD1306 1
// #define OLED_CHIP_SH1106 1

// I2C pins/address — these MUST match config.h. Pulled in directly here
// rather than assumed from a macro name I can't verify, since a wrong macro
// name would just fail to compile; wrong pin numbers would compile fine and
// fail silently on hardware, which is worse. Double check these three
// against your actual config.h before flashing.
#define DISPLAY_I2C_SDA_PIN   8
#define DISPLAY_I2C_SCL_PIN   9
#define DISPLAY_I2C_ADDRESS   0x3C
#define DISPLAY_I2C_CLOCK_HZ  400000UL

#define DISPLAY_WIDTH  128
#define DISPLAY_HEIGHT 64

#if defined(OLED_CHIP_SSD1306)
extern U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2;
#elif defined(OLED_CHIP_SH1106)
extern U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2;
#else
#error "Define exactly one of OLED_CHIP_SSD1306 or OLED_CHIP_SH1106 in display.h"
#endif

// ----------------------------------------------------------------------------
// Screens. Conceptually maps onto the DeskBuddyState enum in state.h, but
// the member ORDER here is deliberately not the same as DeskBuddyState's
// (this has no LISTENING/SPEAKING, and PAT sits in a different slot) — so
// never cast a DeskBuddyState straight to ScreenID. The values don't line
// up and it will silently render the wrong screen. Convert with an explicit
// switch statement in the integration layer (see mapStateToScreen() in
// DeskBuddy.ino). LISTENING and SPEAKING aren't handled here yet (Phase 2 /
// audio pipeline screens) — map them to ScreenID::IDLE on the caller side;
// display_render() also falls back to the idle face for anything unmatched,
// so the two layers agree.
// ----------------------------------------------------------------------------
enum class ScreenID : uint8_t {
  IDLE,
  PAT,
  CLOCK,
  POMODORO,
  WEATHER,
  SETTINGS
};

// ----------------------------------------------------------------------------
// Everything a frame needs to render, in one place. display.cpp does not
// reach into clock.h / weather.h / pomodoro.h / state.h itself - your uiTask
// fills this struct each frame from whatever those managers actually expose,
// then calls display_render(data). This is the only integration point:
// as long as this struct gets filled, the rendering code doesn't care what
// your other modules' function names are.
// ----------------------------------------------------------------------------
struct DisplayData {
  ScreenID screen = ScreenID::IDLE;

  // Free-running counter, incremented once per display_render() call.
  // Drives the idle breathing bob and the weather sun-icon rotation.
  // Just do animTick++ each frame in the caller - it never needs to be
  // reset or synced to anything.
  uint32_t animTick = 0;

  // --- CLOCK ---
  uint8_t hours = 0;        // 0-23 (IST, already local time)
  uint8_t minutes = 0;      // 0-59
  uint8_t weekday = 0;      // 0=Sun .. 6=Sat
  uint8_t dayOfMonth = 1;   // 1-31

  // --- POMODORO ---
  bool pomodoroIsWork = true;          // true=work/focus, false=break
  uint16_t pomodoroSecondsLeft = 0;    // seconds remaining in current phase
  uint16_t pomodoroTotalSeconds = 0;   // total length of current phase (25/5/15 min) — drives the progress bar. Fill from PomodoroManager::getPhaseDurationSeconds(), NOT a hardcoded constant, since work/short-break/long-break are different lengths.
  uint8_t pomodoroCycle = 1;           // 1-4, current cycle number

  // --- WEATHER ---
  int8_t tempC = 0;
  char weatherDesc[12] = "";  // short, will be upper-cased at draw time, e.g. "haze"
  uint8_t humidityPct = 0;
  uint8_t windKmh = 0;

  // --- SETTINGS ---
  uint8_t settingsSelectedIndex = 0;  // which menu row is highlighted (0-3)
};

// Call once from setup(), after Wire has NOT yet been begun elsewhere for
// these pins (display_init() calls Wire.begin() itself).
void display_init();

// Call once per UI tick from uiTask with a freshly-populated DisplayData.
// Handles clearing, drawing the right screen, and pushing the buffer over
// I2C. This is the only "expensive" part (I2C transfer) - budget for it
// when picking your uiTask loop rate; ~8-10Hz is what this was designed
// and animation-tuned around, going much faster just burns I2C bandwidth
// without looking any smoother.
void display_render(const DisplayData& data);

#endif // DISPLAY_H
