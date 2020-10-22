#include <Arduino.h>
#include "EnOcean.h"

uint8_t data[4] = {0x12,0x34,0x56,0x78};

void callback(uint8_t rorg, uint32_t id, uint8_t* data , uint8_t dBm) {
  Serial.print("Data: ");
  Serial.print(data[0], HEX);
  Serial.print(" ");
  Serial.print(data[1], HEX);
  Serial.print(" ");
  Serial.print(data[2], HEX);
  Serial.print(" ");
  Serial.println(data[3], HEX);
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
  delay(3000);
}