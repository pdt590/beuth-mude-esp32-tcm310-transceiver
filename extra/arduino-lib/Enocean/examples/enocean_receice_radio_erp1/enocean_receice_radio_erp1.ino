#include <Arduino.h>
#include "EnOcean/EnOcean.h"

void callback(uint8_t rorg, uint8_t *id, uint8_t *data, uint8_t dBm)
{
  Serial.print("Data: ");
  Serial.print(data[0], HEX);
  Serial.print(" ");
  Serial.print(data[1], HEX);
  Serial.print(" ");
  Serial.print(data[2], HEX);
  Serial.print(" ");
  Serial.println(data[3], HEX);
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
  ;
}