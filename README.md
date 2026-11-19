# ESP CC flasher
This software brings you the possibility to Read and Write the internal Flash of the Texas Instruments CC 8051 series with an ESP32 using the CC interface.

The following CC Microcontroller are supported right now: CC1110, CC2430(up to 64kb), CC2431(up to 64kb), CC2510 and CC2511

To flash an CC Microcontroller connect the following pins:
CC-DebugData to ESP-23
CC-DebugClock to ESP-19
CC-Reset to ESP-33
GND to GND
3.3V to 3.3V
Or take a look at this basic schematic:
https://github.com/atc1441/ESP_CC_Flasher/blob/main/esp32_cc_flasher_connections.jpg

For an ESP32-C3 Super Mini, use GPIO 4 for CC-DebugData, GPIO 3 for
CC-DebugClock, and GPIO 1 for CC-Reset (GPIO 2, 8 and 9 are strapping pins).
Connect GND as above; 3.3V only if the target has no supply of its own.

The PCB Project can be found here: https://easyeda.com/lolerino/epd_esp32

This repo is made together with this explanation video:(click on it)


[![YoutubeVideo](https://img.youtube.com/vi/1mIrL0A4vOM/0.jpg)](https://www.youtube.com/watch?v=1mIrL0A4vOM)




### Needed Software

- Visual Studio Code https://code.visualstudio.com/
- Platform IO https://platformio.org/
- ESP32 core Library https://github.com/espressif/arduino-esp32
- This version of the WifiManager https://github.com/tzapu/WiFiManager/tree/feature_asyncwebserver
- AsyncTCP https://github.com/me-no-dev/AsyncTCP
- ESPAsyncWebServer https://github.com/me-no-dev/ESPAsyncWebServer


### HowTo:

Install Visual Studio Code and PlatformIO
Download this repo and open it in Visual Studio Code

In the Project:
For an ESP32-C3 Super Mini, select the `esp32-c3-supermini` PlatformIO
environment and upload it to the COM port assigned to the board. For the
original ESP32 DevKit, use `esp32doit-devkit-v1` and update its COM port in
`platformio.ini`.

Wi-Fi: copy `src/secrets.h.example` to `src/secrets.h` (git-ignored) and fill in the network.
Without it the flasher opens WiFiManager's access point `AutoConnectAP`. The C3 Super Mini
transmits at 8.5 dBm: at full power these boards often never connect.

mDNS: the flasher answers at `http://cc.local`; the `esp32-c3-supermini` environment sets
`-DCC_MDNS_NAME=\"cccamera\"`, so two flashers on one network do not collide.

USB serial commands (115200 baud, one per line, answers start with `#`): `PING`, `INIT`
(debug mode, prints the chip ID and status), `EXIT` (reset the target and let it run),
`PINS`, `PIN <n> <0|1|Z|U|D>`, `BLINK <n>` and `FLASH <len> <crc32 hex>` followed by the
image bytes (the flasher erases, writes, verifies and resets the target).
Enter your Wifi Credentials or leave it on Managed and Connect to the Created Wifi Netowrk after Upload to config the Wifi

Connect to http://cc.local/edit and upload the index.htm file to the esp32, open http://cc.local and start using it :)


