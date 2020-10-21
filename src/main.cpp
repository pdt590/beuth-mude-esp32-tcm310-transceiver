#include <Arduino.h>
#include "EnOcean.h"

uint8_t data[4] = {0x12,0x34,0x56,0x78};

void callback(uint8_t rorg, uint32_t id, uint32_t data , uint8_t dBm) {
};

EnOcean parser(callback);

void setup()
{
  // Open serial communications and wait for port to open:
  Serial.begin(57600); // Init UART0
  while (!Serial) {
    ; // wait for serial port to connect.
  }
  parser.begin();
}

void loop()
{
  parser.sendPacket(RADIO_ERP1, RORG_4BS, data);
  Serial.println();
  Serial.println("Packet sent");
  delay(3000);
}