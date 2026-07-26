#include "config.h"
#include "input.h"

InputManager input;

void setup() {
  Serial.begin(SERIAL_BAUD_RATE);
  input.begin();
}

void loop() {
  input.update();

  if (input.wasModePressed())   Serial.println("Mode pressed");
  if (input.wasSelectPressed()) Serial.println("Select pressed");
  if (input.wasBackPressed())   Serial.println("Back pressed");
  if (input.wasTouched())       Serial.println("Touch detected");
}