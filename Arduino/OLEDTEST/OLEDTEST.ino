// #include <Wire.h>

// #define SDA_PIN 8
// #define SCL_PIN 9

// void setup() {
//   Serial.begin(115200);
//   delay(1000);

//   Wire.begin(SDA_PIN, SCL_PIN);

//   Serial.println("\n=== I2C Scanner ===");
// }

// void loop() {
//   byte error, address;
//   int devicesFound = 0;

//   Serial.println("Scanning...");

//   for (address = 1; address < 127; address++) {
//     Wire.beginTransmission(address);
//     error = Wire.endTransmission();

//     if (error == 0) {
//       Serial.printf("I2C device found at address 0x%02X\n", address);
//       devicesFound++;
//     }
//   }

//   if (devicesFound == 0) {
//     Serial.println("No I2C devices found. Check wiring!");
//   } else {
//     Serial.printf("Done. %d device(s) found.\n", devicesFound);
//   }

//   Serial.println("====================\n");
//   delay(300);
// }






#include <Wire.h>
#include <U8g2lib.h>

#define SDA_PIN 8
#define SCL_PIN 9

// Uncomment ONE of these based on your chip:
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);
// U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

void setup() {
  Serial.begin(115200);
  delay(1000);

  Wire.begin(SDA_PIN, SCL_PIN);

  u8g2.begin();
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB10_tr); // a nice bold font
  u8g2.drawStr(0, 20, "Hello!");
  u8g2.setFont(u8g2_font_6x10_tr);
  u8g2.drawStr(0, 40, "Desk Buddy is alive");
  u8g2.sendBuffer();

  Serial.println("OLED test complete");
}

void loop() {
  // nothing needed here
}