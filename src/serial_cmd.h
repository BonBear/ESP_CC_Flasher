#pragma once
#include <Arduino.h>

// Line commands over the USB serial port (115200 8N1), for flashing without WiFi.
// Answers start with '#'; everything else on the port is log output.
//   PING                 -> #PONG
//   INIT                 -> #INIT <device id hex> <status hex>   (enters debug mode)
//   EXIT                 -> #EXIT                                 (reset, run firmware)
//   READ <addr> <len>    -> #DATA <len> <crc32>, then lowercase hex lines (32 B/line),
//                           then #DONE OK (resets target); or #ERR <reason>.
//                           <addr> is hex, <len> is decimal (1..4096, within 32 KB flash).
//   FLASH <len> <crc32>  -> #READY, then send <len> raw bytes -> #GOT, then
//                           #DONE OK | #DONE VERIFY_FAIL, or #ERR <reason>
// <len> and <crc32> are decimal and hex; crc32 is the zlib CRC-32 of the image.
// Wiring check (J6: 2 = DD, 3 = DC, 5 = GND; RST is the reset line):
//   PINS                       -> #PINS DD=<0|1> DC=<0|1> RST=<0|1>   (levels read now)
//   PIN <DD|DC|RST> <0|1|Z|U|D> -> drive low/high, or input floating/pull-up/pull-down
//   BLINK <DD|DC|RST> <s>       -> 1 Hz square wave for s seconds (find the pin with a meter)
void serial_cmd_begin();
void serial_cmd_loop();
