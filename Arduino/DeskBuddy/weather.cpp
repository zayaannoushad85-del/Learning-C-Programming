/*
 * weather.cpp
 * Desk Buddy — OpenWeatherMap Weather Module (implementation)
 *
 * API endpoint used:
 *   https://api.openweathermap.org/data/2.5/weather
 *   ?q=Pattambi,IN&units=metric&appid=<key>
 *
 * Sample response fields used:
 *   main.temp, main.feels_like, main.temp_min, main.temp_max,
 *   main.humidity, wind.speed, weather[0].description,
 *   weather[0].id, name
 */

#include "weather.h"

WeatherManager::WeatherManager()
  : _available(false),
    _fetching(false),
    _lastFetchMs(0)
{
  _data = {0, 0, 0, 0, 0, 0, "", "", WeatherCondition::UNKNOWN, 0};
}

// ============================================================
// BEGIN
// ============================================================

void WeatherManager::begin() {
  Serial.println("[Weather] Initializing...");

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("[Weather] WiFi up — fetching initial weather data");
    _fetchWeather();
  } else {
    Serial.println("[Weather] WiFi not ready — will fetch when connected");
  }
}

// ============================================================
// UPDATE — called every loop()
// ============================================================

void WeatherManager::update() {
  if (WiFi.status() != WL_CONNECTED) return;

  // Fetch on interval
  if ((millis() - _lastFetchMs) >= WEATHER_FETCH_INTERVAL_MS) {
    _fetchWeather();
  }
}

void WeatherManager::refresh() {
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("[Weather] Manual refresh requested");
    _fetchWeather();
  } else {
    Serial.println("[Weather] Cannot refresh — WiFi not connected");
  }
}

// ============================================================
// GETTERS
// ============================================================

bool    WeatherManager::isAvailable()     { return _available; }
bool    WeatherManager::isFetching()      { return _fetching; }
float   WeatherManager::getTemperature()  { return _data.temperature; }
float   WeatherManager::getFeelsLike()    { return _data.feelsLike; }
float   WeatherManager::getTempMin()      { return _data.tempMin; }
float   WeatherManager::getTempMax()      { return _data.tempMax; }
uint8_t WeatherManager::getHumidity()     { return _data.humidity; }
float   WeatherManager::getWindSpeed()    { return _data.windSpeed; }
String  WeatherManager::getDescription()  { return _data.description; }
String  WeatherManager::getCityName()     { return _data.cityName; }
WeatherCondition WeatherManager::getCondition() { return _data.condition; }

String WeatherManager::getFormattedTemp() {
  if (!_available) return "--°C";
  char buf[10];
  snprintf(buf, sizeof(buf), "%.1f°C", _data.temperature);
  return String(buf);
}

String WeatherManager::getFormattedHumidity() {
  if (!_available) return "--%";
  return String(_data.humidity) + "%";
}

String WeatherManager::getFormattedWind() {
  if (!_available) return "-- m/s";
  char buf[12];
  snprintf(buf, sizeof(buf), "%.1f m/s", _data.windSpeed);
  return String(buf);
}

String WeatherManager::getSummaryLine() {
  if (!_available) return "Weather unavailable";
  char buf[48];
  snprintf(buf, sizeof(buf), "%.0f°C  %s  %d%%",
    _data.temperature,
    _data.description.c_str(),
    _data.humidity);
  return String(buf);
}

uint32_t WeatherManager::getMinutesSinceUpdate() {
  if (!_available) return 0;
  return (millis() - _data.fetchedAt) / 60000UL;
}

// ============================================================
// PRIVATE — HTTP fetch + JSON parse
// ============================================================

bool WeatherManager::_fetchWeather() {
  _fetching = true;

  String url = "https://api.openweathermap.org/data/2.5/weather?q=";
  url += OWM_CITY;
  url += "&units=";
  url += OWM_UNITS;
  url += "&appid=";
  url += OWM_API_KEY;

  Serial.println("[Weather] Fetching: " + url);

  HTTPClient http;
  http.begin(url);
  http.setTimeout(8000);

  int httpCode = http.GET();

  if (httpCode != 200) {
    Serial.printf("[Weather] HTTP error: %d\n", httpCode);
    if (httpCode == 401) Serial.println("[Weather] Invalid API key — check weather_config.h");
    if (httpCode == 404) Serial.println("[Weather] City not found — check OWM_CITY in weather_config.h");
    http.end();
    _fetching = false;
    _lastFetchMs = millis();   // still update so we don't spam retries
    return false;
  }

  String payload = http.getString();
  http.end();

  // Parse JSON — JsonDocument size 1024 is enough for current weather
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, payload);

  if (error) {
    Serial.printf("[Weather] JSON parse error: %s\n", error.c_str());
    _fetching = false;
    _lastFetchMs = millis();
    return false;
  }

  // Extract fields
  _data.temperature  = doc["main"]["temp"].as<float>();
  _data.feelsLike    = doc["main"]["feels_like"].as<float>();
  _data.tempMin      = doc["main"]["temp_min"].as<float>();
  _data.tempMax      = doc["main"]["temp_max"].as<float>();
  _data.humidity     = doc["main"]["humidity"].as<uint8_t>();
  _data.windSpeed    = doc["wind"]["speed"].as<float>();
  _data.description  = doc["weather"][0]["description"].as<String>();
  _data.cityName     = doc["name"].as<String>();
  _data.condition    = _mapConditionCode(doc["weather"][0]["id"].as<int>());
  _data.fetchedAt    = millis();

  // Capitalise first letter of description
  if (_data.description.length() > 0) {
    _data.description[0] = toupper(_data.description[0]);
  }

  _available   = true;
  _fetching    = false;
  _lastFetchMs = millis();

  Serial.println("[Weather] Fetch successful:");
  Serial.println("[Weather] " + getSummaryLine());
  Serial.printf ("[Weather] Feels like: %.1f°C | Wind: %.1f m/s\n",
    _data.feelsLike, _data.windSpeed);
  Serial.printf ("[Weather] Min: %.1f°C | Max: %.1f°C\n",
    _data.tempMin, _data.tempMax);

  return true;
}

WeatherCondition WeatherManager::_mapConditionCode(int code) {
  // OWM condition code ranges:
  // 2xx = Thunderstorm, 3xx = Drizzle, 4xx = unused
  // 5xx = Rain, 6xx = Snow, 7xx = Atmosphere (mist/fog/haze)
  // 800 = Clear, 80x = Clouds
  if (code >= 200 && code < 300) return WeatherCondition::THUNDERSTORM;
  if (code >= 300 && code < 400) return WeatherCondition::DRIZZLE;
  if (code >= 500 && code < 600) return WeatherCondition::RAIN;
  if (code >= 600 && code < 700) return WeatherCondition::SNOW;
  if (code >= 700 && code < 800) return WeatherCondition::MIST;
  if (code == 800)               return WeatherCondition::CLEAR;
  if (code > 800)                return WeatherCondition::CLOUDY;
  return WeatherCondition::UNKNOWN;
}
