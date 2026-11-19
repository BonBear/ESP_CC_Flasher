#include "serial_cmd.h"
#include "main.h"
#include "cc_interface.h"

#define SERIAL_MAX_IMAGE (32 * 1024)   // CC1110F32 flash size
#define SERIAL_RX_TIMEOUT_MS 5000

static char s_line[64];
static size_t s_len = 0;

static uint32_t crc32_zlib(const uint8_t *data, size_t len)
{
  uint32_t crc = 0xFFFFFFFFu;
  for (size_t i = 0; i < len; i++)
  {
    crc ^= data[i];
    for (int b = 0; b < 8; b++)
      crc = (crc >> 1) ^ (0xEDB88320u & (0u - (crc & 1u)));
  }
  return ~crc;
}

static void serial_flash(size_t len, uint32_t crc)
{
  if (len == 0 || len > SERIAL_MAX_IMAGE)
  {
    Serial.println("#ERR length");
    return;
  }
  uint8_t *buf = (uint8_t *)malloc(len);
  if (!buf)
  {
    Serial.println("#ERR memory");
    return;
  }
  Serial.println("#READY");
  size_t got = 0;
  unsigned long last = millis();
  while (got < len && millis() - last < SERIAL_RX_TIMEOUT_MS)
  {
    int n = Serial.available();
    if (n > 0)
    {
      if ((size_t)n > len - got)
        n = len - got;
      got += Serial.readBytes(buf + got, n);
      last = millis();
    }
    else
    {
      delay(1);
    }
  }
  if (got != len)
  {
    free(buf);
    Serial.printf("#ERR timeout %u/%u\n", (unsigned)got, (unsigned)len);
    return;
  }
  if (crc32_zlib(buf, len) != crc)
  {
    free(buf);
    Serial.println("#ERR crc");
    return;
  }
  Serial.println("#GOT");

  uint16_t device_id = cc.begin(CC_DC, CC_DD, CC_RST);
  Serial.printf("Device id: %04X\n", device_id);
  cc.erase_chip();
  bool ok = false;
  for (int retry = 3; retry > 0 && !ok; retry--)
  {
    cc.write_code_memory(0x0000, buf, len);
    ok = cc.verify_code_memory(0x0000, buf, len) == 0;
    Serial.println(ok ? "Verify successful" : "Verify failed");
  }
  free(buf);
  cc.reset_cc();
  Serial.println(ok ? "#DONE OK" : "#DONE VERIFY_FAIL");
}

// ---- wiring check: drive or read DD / DC / RST by hand (no debug protocol) ----
static int pin_by_name(const char *name)
{
  if (strcasecmp(name, "DD") == 0)
    return CC_DD;
  if (strcasecmp(name, "DC") == 0)
    return CC_DC;
  if (strcasecmp(name, "RST") == 0)
    return CC_RST;
  return -1;
}

static void print_pins()
{
  Serial.printf("#PINS DD=%d DC=%d RST=%d\n", digitalRead(CC_DD), digitalRead(CC_DC), digitalRead(CC_RST));
}

// PIN <DD|DC|RST> <0|1|Z|U|D>: drive low/high, or input floating/pull-up/pull-down
static void cmd_pin(char *args)
{
  char name[8] = {0}, mode = 0;
  if (sscanf(args, "%7s %c", name, &mode) != 2 || pin_by_name(name) < 0)
  {
    Serial.println("#ERR usage: PIN <DD|DC|RST> <0|1|Z|U|D>");
    return;
  }
  int p = pin_by_name(name);
  switch (toupper(mode))
  {
  case '0': pinMode(p, OUTPUT); digitalWrite(p, LOW); break;
  case '1': pinMode(p, OUTPUT); digitalWrite(p, HIGH); break;
  case 'Z': pinMode(p, INPUT); break;
  case 'U': pinMode(p, INPUT_PULLUP); break;
  case 'D': pinMode(p, INPUT_PULLDOWN); break;
  default: Serial.println("#ERR mode must be 0, 1, Z, U or D"); return;
  }
  delay(2);
  print_pins();
}

// BLINK <DD|DC|RST> <seconds>: 1 Hz square wave, so the pin can be found at J6 with a meter
static void cmd_blink(char *args)
{
  char name[8] = {0};
  unsigned s = 0;
  if (sscanf(args, "%7s %u", name, &s) != 2 || pin_by_name(name) < 0 || s == 0 || s > 120)
  {
    Serial.println("#ERR usage: BLINK <DD|DC|RST> <1..120 s>");
    return;
  }
  int p = pin_by_name(name);
  pinMode(p, OUTPUT);
  for (unsigned i = 0; i < 2 * s; i++)
  {
    digitalWrite(p, i & 1 ? LOW : HIGH);
    delay(500);
  }
  pinMode(p, INPUT);
  Serial.println("#BLINK done");
}

static void handle_line(char *line)
{
  if (strcmp(line, "PING") == 0)
  {
    Serial.println("#PONG");
  }
  else if (strcmp(line, "INIT") == 0)
  {
    uint16_t device_id = cc.begin(CC_DC, CC_DD, CC_RST);
    uint8_t status = cc.send_cc_cmdS(0x34);
    Serial.printf("#INIT %04X %02X\n", device_id, status);
  }
  else if (strcmp(line, "EXIT") == 0)
  {
    cc.reset_cc();
    Serial.println("#EXIT");
  }
  else if (strcmp(line, "PINS") == 0)
  {
    print_pins();
  }
  else if (strncmp(line, "PIN ", 4) == 0)
  {
    cmd_pin(line + 4);
  }
  else if (strncmp(line, "BLINK ", 6) == 0)
  {
    cmd_blink(line + 6);
  }
  else if (strncmp(line, "FLASH ", 6) == 0)
  {
    unsigned long len = 0;
    unsigned long crc = 0;
    if (sscanf(line + 6, "%lu %lx", &len, &crc) != 2)
    {
      Serial.println("#ERR usage: FLASH <len> <crc32 hex>");
      return;
    }
    serial_flash(len, (uint32_t)crc);
  }
  else if (line[0])
  {
    Serial.println("#ERR unknown command");
  }
}

void serial_cmd_begin()
{
  s_len = 0;
}

void serial_cmd_loop()
{
  while (Serial.available())
  {
    char c = (char)Serial.read();
    if (c == '\r')
      continue;
    if (c == '\n')
    {
      s_line[s_len] = '\0';
      handle_line(s_line);
      s_len = 0;
    }
    else if (s_len < sizeof(s_line) - 1)
    {
      s_line[s_len++] = c;
    }
  }
}
