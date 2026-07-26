/*
 * config.h
 * Desk Buddy — Central Configuration
 *
 * Board: ESP32-S3-Zero-M Mini (ESP32-S3FH4R2)
 *   - Dual-core LX7 @ 240MHz
 *   - 4MB Flash / 2MB PSRAM (Quad SPI)
 *   - 24 GPIO pins broken out
 *   - GPIO33-GPIO37 are NOT exposed on this board (internally reserved
 *     for Octal PSRAM signaling) — never assign these in software
 *   - GPIO43 = TX, GPIO44 = RX (default UART0, avoid reassigning)
 *   - GPIO19/GPIO20 = native USB D-/D+, avoid reassigning
 *   - GPIO0 = BOOT strap pin, avoid reassigning
 *   - Onboard WS2812 RGB LED on GPIO21
 *   - Native USB only, no USB-UART chip (USB CDC On Boot must be Enabled)
 *
 * This file should only contain #define / const values and the
 * global state enum. No logic, no function bodies.
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// BOARD / ONBOARD PERIPHERALS
#define ONBOARD_LED_PIN     21   // WS2812 RGB LED (built into the module)
#define ONBOARD_LED_COUNT   1

// I2C BUS — OLED Display
#define I2C_SDA_PIN         8
#define I2C_SCL_PIN         9
#define I2C_CLOCK_HZ        400000UL   // 400kHz fast mode

// Display geometry (SH1106 / SSD1306 both 128x64 on this project)
#define OLED_WIDTH          128
#define OLED_HEIGHT         64
#define OLED_I2C_ADDRESS    0x3C

// NOTE: exact chip (SH1106 vs SSD1306) confirmed separately —
// the constructor selection lives in display.h/.cpp, not here.


// TOUCH SENSOR — TTP223B (capacitive)
#define TOUCH_PIN           1

// TACTILE BUTTONS
#define BUTTON_MODE_PIN     2
#define BUTTON_SELECT_PIN   11
#define BUTTON_BACK_PIN     10

#define BUTTON_DEBOUNCE_MS  40

// I2S BUS 0 — Microphone (INMP441)
#define MIC_I2S_SCK_PIN     4    // bit clock
#define MIC_I2S_WS_PIN      5    // word select (L/R clock)
#define MIC_I2S_SD_PIN      6    // serial data (mic output)

#define MIC_SAMPLE_RATE     16000
#define MIC_BITS_PER_SAMPLE 32    // INMP441 outputs 24-bit data in a 32-bit frame


// I2S BUS 1 — Speaker (MAX98357A)
#define SPK_I2S_BCLK_PIN    12
#define SPK_I2S_LRC_PIN     13
#define SPK_I2S_DIN_PIN     7

#define SPK_SAMPLE_RATE     16000
#define SPK_BITS_PER_SAMPLE 16

// ============================================================
// RESERVED / DO NOT USE FOR PERIPHERALS
// ============================================================
// GPIO0          — BOOT strap pin
// GPIO19, GPIO20 — Native USB D-/D+
// GPIO33-GPIO37  — Not exposed on this board (internal Octal PSRAM)
// GPIO43, GPIO44 — Default UART0 TX/RX

// ============================================================
// SYSTEM / TIMING CONSTANTS
// ============================================================
#define SERIAL_BAUD_RATE        115200
#define MAIN_LOOP_DELAY_MS      10

// Pomodoro defaults (in minutes, adjustable later via settings menu)
#define POMODORO_WORK_MIN       25
#define POMODORO_SHORT_BREAK    5
#define POMODORO_LONG_BREAK     15
#define POMODORO_CYCLES_TO_LONG 4

// ============================================================
// APPLICATION STATE MACHINE
// ============================================================
enum class DeskBuddyState {
  IDLE,
  CLOCK,
  POMODORO,
  WEATHER,
  LISTENING,
  SPEAKING,
  PAT,
  SETTINGS
};

#endif // CONFIG_H