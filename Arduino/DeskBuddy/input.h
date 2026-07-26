/*
 * input.h
 * Desk Buddy — Touch & Button Input Handling
 *
 * Wraps the TTP223B touch sensor and the three tactile buttons
 * (Mode / Select / Back) with debouncing, and exposes simple
 * "was this just pressed" style checks for the rest of the app.
 *
 * Usage:
 *   #include "input.h"
 *   InputManager input;
 *
 *   void setup() {
 *     input.begin();
 *   }
 *
 *   void loop() {
 *     input.update();  // call this every loop, as often as possible
 *
 *     if (input.wasModePressed())   { ... }
 *     if (input.wasSelectPressed()) { ... }
 *     if (input.wasBackPressed())   { ... }
 *     if (input.wasTouched())       { ... }
 *   }
 */

#ifndef INPUT_H
#define INPUT_H

#include <Arduino.h>
#include "config.h"

class InputManager {
  public:
    InputManager();

    // Call once in setup()
    void begin();

    // Call every loop() — reads pins and updates debounced state
    void update();

    // Edge-triggered checks: true only once per press, on the
    // transition from "not pressed" to "pressed"
    bool wasModePressed();
    bool wasSelectPressed();
    bool wasBackPressed();
    bool wasTouched();

    // Level checks: true for as long as the input is held down
    bool isModeHeld();
    bool isSelectHeld();
    bool isBackHeld();
    bool isTouchHeld();

  private:
    // Raw + debounced state for each input
    struct DebouncedInput {
      uint8_t pin;
      bool activeLow;        // true if pressed == LOW (buttons w/ pull-up)
      bool rawState;
      bool stableState;
      bool lastStableState;
      unsigned long lastChangeTime;
    };

    DebouncedInput _mode;
    DebouncedInput _select;
    DebouncedInput _back;
    DebouncedInput _touch;

    void initInput(DebouncedInput &input, uint8_t pin, bool activeLow, bool usePullup);
    void updateInput(DebouncedInput &input);
    bool wasPressed(DebouncedInput &input);
    bool isHeld(DebouncedInput &input);
};

#endif // INPUT_H