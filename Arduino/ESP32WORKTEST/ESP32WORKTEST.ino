// ESP32-S3-Zero-M test sketch
// Verifies serial comms, chip info, and onboard WS2812 RGB LED

#include <Adafruit_NeoPixel.h>

#define LED_PIN 21
#define NUM_LEDS 1

Adafruit_NeoPixel pixel(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n=== ESP32-S3-Zero-M Test ===");
  Serial.printf("Chip model: %s\n", ESP.getChipModel());
  Serial.printf("Chip revision: %d\n", ESP.getChipRevision());
  Serial.printf("CPU cores: %d\n", ESP.getChipCores());
  Serial.printf("CPU freq: %d MHz\n", ESP.getCpuFreqMHz());
  Serial.printf("Flash size: %d MB\n", ESP.getFlashChipSize() / (1024 * 1024));
  Serial.printf("PSRAM size: %d bytes\n", ESP.getPsramSize());
  Serial.printf("Free heap: %d bytes\n", ESP.getFreeHeap());
  Serial.println("=============================");

  pixel.begin();
  pixel.setBrightness(50);
}

void loop() {
  pixel.setPixelColor(0, pixel.Color(255, 0, 0)); // Red
  pixel.show();
  Serial.println("LED: RED");
  delay(500);

  pixel.setPixelColor(0, pixel.Color(0, 255, 0)); // Green
  pixel.show();
  Serial.println("LED: GREEN");
  delay(500);

  pixel.setPixelColor(0, pixel.Color(0, 0, 255)); // Blue
  pixel.show();
  Serial.println("LED: BLUE");
  delay(500);
}