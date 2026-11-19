#pragma once
#include <Arduino.h>

// mDNS name (http://<name>.local): give each flasher on the bench its own, e.g. per env in
// platformio.ini with -DCC_MDNS_NAME=\"cccamera\".
#ifndef CC_MDNS_NAME
#define CC_MDNS_NAME "cc"
#endif

// Wi-Fi: src/secrets.h (local, git-ignored; copy src/secrets.h.example) sets STATIC_WIFI,
// WIFI_NAME and WIFI_PWD. Without it the flasher uses WiFiManager (access point AutoConnectAP).
#if __has_include("secrets.h")
#include "secrets.h"
#endif

#ifndef STATIC_WIFI
#define MANAGED_WIFI
#endif


// CC pins
#if defined(CONFIG_IDF_TARGET_ESP32C3)
#define CC_RST 1
#define CC_DC 3
#define CC_DD 4
#else
#define CC_RST 33
#define CC_DC 19
#define CC_DD 23
#endif

void set_firmware_file(String firmware, int file_size);
void set_firmware_dump_file(String firmware, int file_size);