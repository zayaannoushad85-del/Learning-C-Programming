/*
 * clock.cpp
 * Desk Buddy — NTP Clock Manager (implementation)
 */

#include "clock.h"

static const char* DAY_NAMES[] = {
  "Sunday", "Monday", "Tuesday", "Wednesday",
  "Thursday", "Friday", "Saturday"
};

static const char* MONTH_NAMES[] = {
  "", // index 0 unused — months are 1-based
  "January", "February", "March",     "April",
  "May",      "June",     "July",      "August",
  "September","October",  "November",  "December"
};

ClockManager::ClockManager()
  : _synced(false),
    _connected(false),
    _lastSyncMs(0)
{
  memset(&_timeinfo, 0, sizeof(_timeinfo));
}

// ============================================================
// BEGIN — WiFi connect + NTP sync
// ============================================================

void ClockManager::begin() {
  Serial.println("[Clock] Connecting to WiFi...");

  if (_connectWiFi()) {
    Serial.printf("[Clock] Connected to %s\n", WIFI_SSID);
    Serial.printf("[Clock] IP: %s\n", WiFi.localIP().toString().c_str());

    if (_syncNTP()) {
      Serial.println("[Clock] NTP sync successful");
      Serial.println("[Clock] Time: " + getFormattedDateTime());
    } else {
      Serial.println("[Clock] NTP sync failed — will retry in update()");
    }
  } else {
    Serial.println("[Clock] WiFi connection failed — clock unavailable");
    Serial.println("[Clock] Check SSID/password in wifi_credentials.h");
  }
}

// ============================================================
// UPDATE — periodic resync
// ============================================================

void ClockManager::update() {
  // If not connected, periodically retry WiFi
  if (!_connected) {
    static unsigned long _lastRetry = 0;
    if (millis() - _lastRetry > 30000) {   // retry every 30 seconds
      _lastRetry = millis();
      Serial.println("[Clock] Retrying WiFi...");
      _connectWiFi();
    }
    return;
  }

  // If connected but not synced, retry NTP
  if (!_synced) {
    static unsigned long _lastNTPRetry = 0;
    if (millis() - _lastNTPRetry > 15000) {   // retry every 15 seconds
      _lastNTPRetry = millis();
      Serial.println("[Clock] Retrying NTP sync...");
      _syncNTP();
    }
    return;
  }

  // Periodic resync every hour to prevent drift
  if ((millis() - _lastSyncMs) >= NTP_RESYNC_INTERVAL_MS) {
    Serial.println("[Clock] Hourly NTP resync...");
    _syncNTP();
  }

  // Refresh timeinfo from system clock every call
  _refreshTime();
}

// ============================================================
// TIME GETTERS
// ============================================================

bool ClockManager::_refreshTime() {
  return getLocalTime(&_timeinfo);
}

uint8_t  ClockManager::getHour()       { _refreshTime(); return (uint8_t)_timeinfo.tm_hour; }
uint8_t  ClockManager::getMinute()     { _refreshTime(); return (uint8_t)_timeinfo.tm_min; }
uint8_t  ClockManager::getSecond()     { _refreshTime(); return (uint8_t)_timeinfo.tm_sec; }
uint8_t  ClockManager::getDay()        { _refreshTime(); return (uint8_t)_timeinfo.tm_mday; }
uint8_t  ClockManager::getMonth()      { _refreshTime(); return (uint8_t)(_timeinfo.tm_mon + 1); }
uint16_t ClockManager::getYear()       { _refreshTime(); return (uint16_t)(_timeinfo.tm_year + 1900); }
uint8_t  ClockManager::getDayOfWeek()  { _refreshTime(); return (uint8_t)_timeinfo.tm_wday; }

const char* ClockManager::getDayName() {
  _refreshTime();
  return DAY_NAMES[_timeinfo.tm_wday];
}

const char* ClockManager::getMonthName() {
  _refreshTime();
  uint8_t m = getMonth();
  if (m < 1 || m > 12) return "";
  return MONTH_NAMES[m];
}

// ============================================================
// FORMATTED STRINGS
// ============================================================

String ClockManager::getFormattedTime(bool show24h) {
  if (!_synced) return "--:--:--";
  _refreshTime();

  char buf[12];
  if (show24h) {
    snprintf(buf, sizeof(buf), "%02d:%02d:%02d",
      _timeinfo.tm_hour,
      _timeinfo.tm_min,
      _timeinfo.tm_sec);
  } else {
    uint8_t h = _timeinfo.tm_hour;
    const char* ampm = (h >= 12) ? "PM" : "AM";
    if (h == 0)       h = 12;
    else if (h > 12)  h -= 12;
    snprintf(buf, sizeof(buf), "%02d:%02d %s",
      h, _timeinfo.tm_min, ampm);
  }
  return String(buf);
}

String ClockManager::getFormattedDate() {
  if (!_synced) return "--/--/----";
  _refreshTime();

  char buf[16];
  snprintf(buf, sizeof(buf), "%02d %s %04d",
    _timeinfo.tm_mday,
    MONTH_NAMES[_timeinfo.tm_mon + 1],
    _timeinfo.tm_year + 1900);
  return String(buf);
}

String ClockManager::getFormattedDateTime() {
  return getFormattedDate() + "  " + getFormattedTime();
}

// ============================================================
// STATUS
// ============================================================

bool ClockManager::isSynced()    { return _synced; }
bool ClockManager::isConnected() { return _connected; }

void ClockManager::resync() {
  Serial.println("[Clock] Manual resync requested");
  _syncNTP();
}

// ============================================================
// PRIVATE HELPERS
// ============================================================

bool ClockManager::_connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED) {
    if ((millis() - start) > WIFI_CONNECT_TIMEOUT_MS) {
      _connected = false;
      WiFi.disconnect(true);
      return false;
    }
    delay(250);
    Serial.print(".");
  }

  Serial.println();
  _connected = true;
  return true;
}

bool ClockManager::_syncNTP() {
  if (!_connected) return false;

  // Configure NTP — this sets the system clock automatically
  configTime(NTP_GMT_OFFSET_SEC, NTP_DAYLIGHT_OFFSET_SEC, NTP_SERVER);

  // Wait for sync
  struct tm timeinfo;
  unsigned long start = millis();
  while (!getLocalTime(&timeinfo)) {
    if ((millis() - start) > NTP_SYNC_TIMEOUT_MS) {
      Serial.println("[Clock] NTP timeout");
      return false;
    }
    delay(200);
  }

  _timeinfo   = timeinfo;
  _synced     = true;
  _lastSyncMs = millis();
  return true;
}
