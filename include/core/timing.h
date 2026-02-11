#ifndef TIMING_H
#define TIMING_H

#include <types.h>

void static wait(uint32_t milliseconds) {
    const uint32_t iterations_per_ms = 1000000;  // Only a rough estimate
    volatile uint32_t count = milliseconds * iterations_per_ms;
    while (count--) {
        __asm__ volatile("nop");
    }
}

// GMT/UTC Timezone Offsets (in hours)
// Offset is added to UTC.
// Negative values = behind UTC, Positive = ahead of UTC.
#define TZ_BAKER_ISLAND -12    // GMT-12:00  Baker Island
#define TZ_SAMOA -11           // GMT-11:00  American Samoa
#define TZ_HAWAII -10          // GMT-10:00  Hawaii
#define TZ_ALASKA -9           // GMT-09:00  Alaska
#define TZ_PST -8              // GMT-08:00  Pacific (US/Canada)
#define TZ_MST -7              // GMT-07:00  Mountain (US/Canada)
#define TZ_CST -6              // GMT-06:00  Central (US/Canada)
#define TZ_EST -5              // GMT-05:00  Eastern (US/Canada)
#define TZ_AST -4              // GMT-04:00  Atlantic
#define TZ_ARGENTINA -3        // GMT-03:00  Argentina, Brazil
#define TZ_MID_ATLANTIC -2     // GMT-02:00  Mid-Atlantic
#define TZ_AZORES -1           // GMT-01:00  Azores, Cape Verde
#define TZ_UTC 0               // GMT+00:00  UTC / London / Lisbon
#define TZ_CET 1               // GMT+01:00  Central Europe (Paris, Berlin)
#define TZ_EET 2               // GMT+02:00  Eastern Europe (Cairo, Helsinki)
#define TZ_MSK 3               // GMT+03:00  Moscow, Nairobi
#define TZ_GULF 4              // GMT+04:00  Dubai, Baku
#define TZ_IST_INDIA 5         // GMT+05:00  Pakistan (use +5:30 for India)
#define TZ_SRI_LANKA 5         // GMT+05:30  Sri Lanka, India (handled as +5, add 30 min)
#define TZ_BANGLADESH 6        // GMT+06:00  Bangladesh, Bhutan
#define TZ_INDOCHINA 7         // GMT+07:00  Bangkok, Jakarta
#define TZ_CHINA 8             // GMT+08:00  China, Singapore, Perth
#define TZ_JAPAN 9             // GMT+09:00  Japan, Korea
#define TZ_AEST 10             // GMT+10:00  Sydney, Melbourne
#define TZ_PACIFIC_ISLANDS 11  // GMT+11:00  Solomon Islands
#define TZ_NZST 12             // GMT+12:00  New Zealand, Fiji

// Half-hour offset zones
#define TZ_INDIA_MINUTES 30  // India/Sri Lanka: GMT+05:30
#define TZ_IRAN_OFFSET 3     // Iran: GMT+03:30
#define TZ_IRAN_MINUTES 30
#define TZ_NEPAL_OFFSET 5  // Nepal: GMT+05:45
#define TZ_NEPAL_MINUTES 45
#define TZ_MYANMAR_OFFSET 6  // Myanmar: GMT+06:30
#define TZ_MYANMAR_MINUTES 30
#define TZ_CHATHAM_OFFSET 12  // Chatham Islands: GMT+12:45
#define TZ_CHATHAM_MINUTES 45

// Active Timezone
#define TIMEZONE_HOURS TZ_SRI_LANKA
#define TIMEZONE_MINUTES TZ_INDIA_MINUTES

#endif  // TIMING_H
