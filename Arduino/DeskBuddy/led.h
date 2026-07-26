/*
 * led.h
 * Desk Buddy — Onboard WS2812 RGB LED Manager
 *
 * Wraps the single onboard WS2812 RGB LED (GPIO21) and maps it
 * to Desk Buddy's application states for visual feedback while
 * the OLED display is not yet available.
 *
 * Supported effects:
 *   - Solid color (instant)
 *   - Slow pulse (breathing effect)
 *   - Fast blink (attention / alert)
 *
 * Usage:
 *   #include "led.h"
 *   LEDManager led;
 *
 *   void setup() { led.begin(); }
 *
 *   void loop() {
 *     led.update();            // must be called every loop
 *     led.setState(DeskBuddyState::POMODORO);
 *   }
 */

#ifndef LED_H
#define LED_H

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include "config.h"

// Effect types
enum class LEDEffect {
  SOLID,
  PULSE,    // smooth breathing
  BLINK     // on/off flash
};

class LEDManager {
  public:
    LEDManager();

    // Call once in setup()
    void begin();

    // Call every loop() — handles animations
    void update();

    // Set color + effect directly
    void setColor(uint8_t r, uint8_t g, uint8_t b, LEDEffect effect = LEDEffect::SOLID);

    // Set LED appearance based on app state
    void setState(DeskBuddyState state);

    // Turn off LED
    void off();

  private:
    Adafruit_NeoPixel _pixel;

    uint8_t _r, _g, _b;
    LEDEffect _effect;

    // Pulse animation state
    float _brightness;
    float _pulseStep;
    bool  _pulseRising;
    unsigned long _lastUpdateTime;

    // Blink animation state
    bool _blinkOn;
    unsigned long _lastBlinkTime;
    uint16_t _blinkIntervalMs;

    void applyColor(uint8_t r, uint8_t g, uint8_t b);
};

#endif // LED_H
