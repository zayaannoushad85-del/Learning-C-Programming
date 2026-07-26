/*
 * weather_config.h
 * Desk Buddy — OpenWeatherMap API Configuration
 *
 * Keep this file private — do NOT share or commit to git.
 * Add to .gitignore alongside wifi_credentials.h
 *
 * To regenerate your API key:
 *   openweathermap.org → Login → API Keys → Create new key
 */

#ifndef WEATHER_CONFIG_H
#define WEATHER_CONFIG_H

#define OWM_API_KEY   "d59f3229153e8a522fd16adc69af1d96"
#define OWM_CITY      "Pattambi,IN"
#define OWM_UNITS     "metric"   // "metric" = Celsius, "imperial" = Fahrenheit

// How often to fetch new weather data (ms) — every 10 minutes
#define WEATHER_FETCH_INTERVAL_MS   600000UL

#endif // WEATHER_CONFIG_H
