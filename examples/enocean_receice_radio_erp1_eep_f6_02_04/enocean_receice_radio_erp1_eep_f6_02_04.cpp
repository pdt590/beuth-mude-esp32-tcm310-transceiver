#include <Arduino.h>
#include "EnOceanProfile.h"
#include "EnOcean.h"

EnOceanProfile eep;

void callback(uint8_t rorg, uint8_t *id, uint8_t *data, uint8_t dBm)
{
  uint32_t preData = (data[0] << 24 && 0xFF000000) +
                     (data[1] << 16 && 0xFF000000) +
                     (data[2] << 8 && 0xFF000000) +
                     data[3];
  uint8_t switchStatus = eep.getSwitchStatus(EEP_F6_02_04, preData);
  Serial.println(switchStatus, HEX);
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