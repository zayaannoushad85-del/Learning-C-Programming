/*
 * input.cpp
 * Desk Buddy — Touch & Button Input Handling (implementation)
 */

#include "input.h"

InputManager::InputManager() {
  // Members get properly initialized in begin(), this constructor
  // just exists so the class can be instantiated globally.
}

void InputManager::begin() {
  // Buttons: tactile switches wired to GND, using internal pull-up.
  // Pressed = LOW.
  initInput(_mode,   BUTTON_MODE_PIN,   true,  true);
  initInput(_select, BUTTON_SELECT_PIN, true,  true);
  initInput(_back,   BUTTON_BACK_PIN,   true,  true);

  // Touch sensor: TTP223B outputs HIGH when touched, no pull-up needed.
  initInput(_touch,  TOUCH_PIN,         false, false);
}

void InputManager::initInput(DebouncedInput &input, uint8_t pin, bool activeLow, bool usePullup) {
  input.pin = pin;
  input.activeLow = activeLow;
  input.lastChangeTime = millis();

  pinMode(pin, usePullup ? INPUT_PULLUP : INPUT);
  delay(10);   // let pull-up resistor settle before reading

  // Seed state from actual pin reading so boot-time levels
  // don't register as a false press on the first update()
  int reading = digitalRead(pin);
  bool pressedNow = activeLow ? (reading == LOW) : (reading == HIGH);
  input.rawState        = pressedNow;
  input.stableState     = pressedNow;
  input.lastStableState = pressedNow;
}

void InputManager::update() {
  updateInput(_mode);
  updateInput(_select);
  updateInput(_back);
  updateInput(_touch);
}

void InputManager::updateInput(DebouncedInput &input) {
  int reading = digitalRead(input.pin);
  bool pressedNow = input.activeLow ? (reading == LOW) : (reading == HIGH);

  if (pressedNow != input.rawState) {
    // Input changed — reset the debounce timer
    input.rawState = pressedNow;
    input.lastChangeTime = millis();
  }

  if ((millis() - input.lastChangeTime) > BUTTON_DEBOUNCE_MS) {
    // Signal has been stable long enough, commit it
    input.lastStableState = input.stableState;
    input.stableState = input.rawState;
  }
}

bool InputManager::wasPressed(DebouncedInput &input) {
  // True only on the transition from not-pressed -> pressed
  return (input.stableState == true && input.lastStableState == false);
}

bool InputManager::isHeld(DebouncedInput &input) {
  return input.stableState;
}

bool InputManager::wasModePressed()   { return wasPressed(_mode); }
bool InputManager::wasSelectPressed() { return wasPressed(_select); }
bool InputManager::wasBackPressed()   { return wasPressed(_back); }
bool InputManager::wasTouched()       { return wasPressed(_touch); }

bool InputManager::isModeHeld()   { return isHeld(_mode); }
bool InputManager::isSelectHeld() { return isHeld(_select); }
bool InputManager::isBackHeld()   { return isHeld(_back); }
bool InputManager::isTouchHeld()  { return isHeld(_touch); }
