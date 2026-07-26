/*
 * weather.h
 * Desk Buddy — OpenWeatherMap Weather Module
 *
 * Fetches current weather for the configured city using the
 * OpenWeatherMap Current Weather API (free tier, no key limits
 * beyond 60 calls/min). Refreshes every 10 minutes by default.
 *
 * Requires:
 *   - WiFi already connected (ClockManager handles this)
 *   - ArduinoJson library (Sketch → Library Manager → "ArduinoJson" by bblanchon)
 *   - HTTPClient (built into ESP32 Arduino core, no install needed)
 *
 * Usage:
 *   #include "weather.h"
 *   WeatherManager weather;
 *
 *   void setup() { weather.begin(); }
 *
 *   void loop() {
 *     weather.update();
 *     if (weather.isAvailable()) {
 *       Serial.println(weather.getDescription());
 *       Serial.printf("%.1f C, %d%% humidity\n",
 *         weather.getTemperature(),
 *         weather.getHumidity());
 *     }
 *   }
 */

#ifndef WEATHER_H
#define WEATHER_H

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "config.h"
#include "weather_config.h"

// Weather condition categories (mapped from OWM condition codes)
// Used by display and LED to pick icons/colors
enum class WeatherCondition {
  UNKNOWN,
  CLEAR,
  CLOUDY,
  RAIN,
  DRIZZLE,
  THUNDERSTORM,
  SNOW,
  MIST
};

struct WeatherData {
  float       temperature;      // °C
  float       feelsLike;        // °C
  float       tempMin;          // °C
  float       tempMax;          // °C
  uint8_t     humidity;         // %
  float       windSpeed;        // m/s
  String      description;      // e.g. "light rain"
  String      cityName;         // confirmed city name from API
  WeatherCondition condition;   // simplified category
  unsigned long fetchedAt;      // millis() when last fetched
};

class WeatherManager {
  public:
    WeatherManager();

    // Call once in setup() — does first fetch if WiFi is up
    void begin();

    // Call every loop() — fetches new data on interval
    void update();

    // Force an immediate refresh
    void refresh();

    // True once at least one successful fetch has completed
    bool isAvailable();

    // True if currently fetching (async-friendly flag)
    bool isFetching();

    // Weather data getters
    float       getTemperature();    // current temp in °C
    float       getFeelsLike();
    float       getTempMin();
    float       getTempMax();
    uint8_t     getHumidity();       // %
    float       getWindSpeed();      // m/s
    String      getDescription();    // "light rain", "clear sky" etc.
    String      getCityName();
    WeatherCondition getCondition();

    // Formatted strings for display
    String getFormattedTemp();       // "28.5°C"
    String getFormattedHumidity();   // "72%"
    String getFormattedWind();       // "3.2 m/s"
    String getSummaryLine();         // "28°C  Light Rain  72%"

    // Minutes since last successful fetch (for display staleness)
    uint32_t getMinutesSinceUpdate();

  private:
    WeatherData   _data;
    bool          _available;
    bool          _fetching;
    unsigned long _lastFetchMs;

    bool _fetchWeather();
    WeatherCondition _mapConditionCode(int code);
};

#endif // WEATHER_H
