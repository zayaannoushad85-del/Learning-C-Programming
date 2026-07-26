/*
 * led.cpp
 * Desk Buddy — Onboard WS2812 RGB LED Manager (implementation)
 *
 * State → Color mapping:
 *   IDLE        → soft white pulse
 *   CLOCK       → calm blue pulse
 *   POMODORO    → red solid (work) / green solid (break) — handled externally by pomodoro.cpp
 *   WEATHER     → cyan solid
 *   LISTENING   → purple pulse
 *   SPEAKING    → orange pulse
 *   PAT         → pink solid
 *   SETTINGS    → yellow blink
 */

#include "led.h"

// Pulse speed: smaller = slower breathing
#define PULSE_STEP_AMOUNT   1.5f
#define PULSE_UPDATE_MS     15

// Blink defaults
#define BLINK_INTERVAL_MS   400

LEDManager::LEDManager()
  : _pixel(ONBOARD_LED_COUNT, ONBOARD_LED_PIN, NEO_GRB + NEO_KHZ800),
    _r(0), _g(0), _b(0),
    _effect(LEDEffect::SOLID),
    _brightness(0.0f),
    _pulseStep(PULSE_STEP_AMOUNT),
    _pulseRising(true),
    _lastUpdateTime(0),
    _blinkOn(false),
    _lastBlinkTime(0),
    _blinkIntervalMs(BLINK_INTERVAL_MS)
{}

void LEDManager::begin() {
  _pixel.begin();
  _pixel.setBrightness(80);   // 0-255, keep reasonable to not blind
  _pixel.clear();
  _pixel.show();
}

void LEDManager::update() {
  unsigned long now = millis();

  switch (_effect) {

    case LEDEffect::SOLID:
      // Nothing to animate — color already applied in setColor()
      break;

    case LEDEffect::PULSE: {
      if ((now - _lastUpdateTime) < PULSE_UPDATE_MS) break;
      _lastUpdateTime = now;

      if (_pulseRising) {
        _brightness += _pulseStep;
        if (_brightness >= 255.0f) {
          _brightness = 255.0f;
          _pulseRising = false;
        }
      } else {
        _brightness -= _pulseStep;
        if (_brightness <= 0.0f) {
          _brightness = 0.0f;
          _pulseRising = true;
        }
      }

      float scale = _brightness / 255.0f;
      applyColor(
        (uint8_t)(_r * scale),
        (uint8_t)(_g * scale),
        (uint8_t)(_b * scale)
      );
      break;
    }

    case LEDEffect::BLINK: {
      if ((now - _lastBlinkTime) < _blinkIntervalMs) break;
      _lastBlinkTime = now;
      _blinkOn = !_blinkOn;
      applyColor(
        _blinkOn ? _r : 0,
        _blinkOn ? _g : 0,
        _blinkOn ? _b : 0
      );
      break;
    }
  }
}

void LEDManager::setColor(uint8_t r, uint8_t g, uint8_t b, LEDEffect effect) {
  _r = r;
  _g = g;
  _b = b;
  _effect = effect;

  // Reset animation state on every color change
  _brightness  = 0.0f;
  _pulseRising = true;
  _blinkOn     = false;

  if (effect == LEDEffect::SOLID) {
    applyColor(r, g, b);
  } else if (effect == LEDEffect::PULSE) {
    // Start mid-brightness so pulse is immediately visible
    _brightness = 50.0f;
  }
}

void LEDManager::setState(DeskBuddyState state) {
  switch (state) {
    case DeskBuddyState::IDLE:
      setColor(200, 200, 200, LEDEffect::PULSE);   // soft white pulse
      break;

    case DeskBuddyState::CLOCK:
      setColor(0, 80, 255, LEDEffect::PULSE);      // calm blue pulse
      break;

    case DeskBuddyState::POMODORO:
      setColor(255, 20, 0, LEDEffect::SOLID);      // red solid (work session)
      // pomodoro.cpp can call setColor() directly to switch to green on break
      break;

    case DeskBuddyState::WEATHER:
      setColor(0, 220, 220, LEDEffect::SOLID);     // cyan
      break;

    case DeskBuddyState::LISTENING:
      setColor(140, 0, 255, LEDEffect::PULSE);     // purple pulse
      break;

    case DeskBuddyState::SPEAKING:
      setColor(255, 100, 0, LEDEffect::PULSE);     // orange pulse
      break;

    case DeskBuddyState::PAT:
      setColor(255, 60, 120, LEDEffect::SOLID);    // pink
      break;

    case DeskBuddyState::SETTINGS:
      setColor(255, 200, 0, LEDEffect::BLINK);     // yellow blink
      break;

    default:
      off();
      break;
  }
}

void LEDManager::off() {
  _effect = LEDEffect::SOLID;
  applyColor(0, 0, 0);
}

void LEDManager::applyColor(uint8_t r, uint8_t g, uint8_t b) {
  _pixel.setPixelColor(0, _pixel.Color(r, g, b));
  _pixel.show();
}
