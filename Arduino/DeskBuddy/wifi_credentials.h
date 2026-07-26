/*
 * wifi_credentials.h
 * Desk Buddy — WiFi Credentials
 *
 * Fill in your WiFi SSID and password below.
 * Keep this file private — do NOT share or commit it to git.
 *
 * If you ever use version control (GitHub etc.), add this filename
 * to your .gitignore file so credentials are never uploaded.
 */

#ifndef WIFI_CREDENTIALS_H
#define WIFI_CREDENTIALS_H

#define WIFI_SSID     "Zayaan's S25"
#define WIFI_PASSWORD "zanuzayaan13"

// NTP server — uses Google's public time server by default
// Alternatives: "pool.ntp.org", "time.cloudflare.com"
#define NTP_SERVER    "time.google.com"

// Timezone: India Standard Time (UTC+5:30)
// If you're in a different timezone, adjust gmtOffset_sec:
//   UTC+5:30 (India)     = 19800
//   UTC+0    (London)    = 0
//   UTC-5    (New York)  = -18000
//   UTC+8    (Singapore) = 28800
#define NTP_GMT_OFFSET_SEC      19800   // UTC+5:30 (IST)
#define NTP_DAYLIGHT_OFFSET_SEC 0       // India has no daylight saving

#endif // WIFI_CREDENTIALS_H
