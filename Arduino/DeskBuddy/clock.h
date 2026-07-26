/*
 * clock.h
 * Desk Buddy — NTP Clock Manager
 *
 * Connects to WiFi, syncs time via NTP, and exposes current
 * time and date in a clean interface. Falls back gracefully
 * if WiFi is unavailable — isSynced() returns false and
 * getFormattedTime() returns "--:--:--" until sync succeeds.
 *
 * No external library needed — uses ESP32 Arduino core's
 * built-in configTime() and getLocalTime().
 *
 * Usage:
 *   #include "clock.h"
 *   ClockManager clk;
 *
 *   void setup() { clk.begin(); }
 *
 *   void loop() {
 *     clk.update();
 *     if (clk.isSynced()) {
 *       Serial.println(clk.getFormattedTime());
 *     }
 *   }
 */

#ifndef CLOCK_H
#define CLOCK_H

#include <Arduino.h>
#include <WiFi.h>
#include <time.h>
#include "config.h"
#include "wifi_credentials.h"

// How long to wait for WiFi connection before giving up (ms)
#define WIFI_CONNECT_TIMEOUT_MS   10000

// How often to re-sync NTP after initial sync (ms) — every 60 minutes
#define NTP_RESYNC_INTERVAL_MS    3600000UL

// How long to wait for NTP sync after WiFi connects (ms)
#define NTP_SYNC_TIMEOUT_MS       8000

class ClockManager {
  public:
    ClockManager();

    // Connect to WiFi and sync NTP — call once in setup()
    void begin();

    // Call every loop() — handles periodic NTP resync
    void update();

    // Time components
    uint8_t  getHour();      // 0-23
    uint8_t  getMinute();    // 0-59
    uint8_t  getSecond();    // 0-59

    // Date components
    uint8_t  getDay();       // 1-31
    uint8_t  getMonth();     // 1-12
    uint16_t getYear();      // e.g. 2026

    // Day of week: 0=Sunday, 1=Monday ... 6=Saturday
    uint8_t  getDayOfWeek();
    const char* getDayName();      // "Monday", "Tuesday" etc.
    const char* getMonthName();    // "January", "February" etc.

    // Formatted strings ready for display
    String getFormattedTime(bool show24h = true);   // "14:35:07" or "02:35:07 PM"
    String getFormattedDate();                       // "01 Jul 2026"
    String getFormattedDateTime();                   // "01 Jul 2026  14:35:07"

    // Status
    bool isSynced();        // true once NTP sync succeeds
    bool isConnected();     // true if WiFi is currently connected

    // Force an immediate NTP resync attempt
    void resync();

  private:
    bool          _synced;
    bool          _connected;
    unsigned long _lastSyncMs;
    struct tm     _timeinfo;

    bool _connectWiFi();
    bool _syncNTP();
    bool _refreshTime();   // updates _timeinfo from system clock
};

#endif // CLOCK_H
