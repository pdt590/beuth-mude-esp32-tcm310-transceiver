#include <Arduino.h>
#include "EnOcean.h"
#include "EnOcean/PacketERP1.h"

uint8_t data[4] = {0x12, 0x34, 0x56, 0x78};

void callback(uint8_t rorg, uint8_t *id, uint8_t *data, uint8_t dBm)
{ // 4 bytes id, 4 bytes data
};

EnOcean Enocean(callback);

void setup()
{
  // Open serial communications and wait for port to open:
  Serial.begin(57600); // Init UART0
  while (!Serial)
  {
    ; // wait for serial port to connect.
  }
  Enocean.begin();
}

void loop()
{
  Enocean.send(RADIO_ERP1, RORG_4BS, data);
  delay(3000);
}