#ifndef SPARKS_TIME_COUNT
#define SPARKS_TIME_COUNT

#include <time.h>
#include <NTPClient.h>
#include <WiFiUdp.h>



struct tm timeinfo;
const char *ntpServer = "pool.ntp.org";
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, ntpServer, 0);

String localTime()
{
    
// #if defined(ARDUINO_ARCH_ESP8266)
    if (timeClient.getEpochTime() == 0)
    {
        Serial.println("Failed to fetch time.");
    }
    String timestamp = timeClient.getFullFormattedTime();

// #elif defined(ARDUINO_ARCH_ESP32)
//     int year, month, day, hour, minute, sec;
//     if (!getLocalTime(&timeinfo))
//     {
//         Serial.println("Failed to fetch time.");
//     }
//     sec = timeinfo.tm_sec;
//     minute = timeinfo.tm_min;
//     hour = timeinfo.tm_hour;
//     day = timeinfo.tm_mday;
//     month = timeinfo.tm_mon;
//     year = timeinfo.tm_year;
//     String timestamp = String(year + 1900) + "-" + String(month + 1) + "-" + String(day) + " " + String(hour) + ":" + String(minute) + ":" + String(sec);

// #endif

    return timestamp;
}

#endif