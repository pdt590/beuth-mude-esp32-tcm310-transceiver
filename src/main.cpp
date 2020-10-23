#include <Arduino.h>
#include "EnOcean.h"
#include "PacketERP1.h"

uint8_t data[4] = {0x12, 0x34, 0x56, 0x78};

void callback(uint8_t rorg, uint8_t *id, uint8_t *data, uint8_t dBm)
{ // 4 bytes id, 4 bytes data
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
  while (!Serial)
  {
    ; // wait for serial port to connect.
  }
  parser.begin();
}

void loop()
{
  //parser.send(RADIO_ERP1, RORG_4BS, data);
  //delay(3000);
  parser.send(RADIO_ERP1, RORG_4BS, data);
  delay(2000);
  parser.deepSleep(1000);
  delay(3000);
}