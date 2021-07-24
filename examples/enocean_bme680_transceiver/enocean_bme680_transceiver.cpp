#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include "EnOcean.h"
#include "PacketERP1.h"
#include <Adafruit_Sensor.h>
#include <Adafruit_BME680.h>

void callback(uint8_t rorg, uint8_t *id, uint8_t *data, uint8_t dBm)
{
  Serial.print("Temp: ");
  Serial.println(data[0]);
  Serial.print("Pressure: ");
  Serial.println(data[1]);
  Serial.print("Hum: ");
  Serial.println(data[2]);
  Serial.print("Gas: ");
  Serial.println(data[3]);
};

EnOcean Enocean(callback);

// Initialize BME680
#define SEALEVELPRESSURE_HPA (1013.25)
Adafruit_BME680 bme;
uint8_t data[4];

void setup()
{
  // Open serial communications and wait for port to open:
  Serial.begin(57600); // Init UART0
  while (!Serial)
  {
    ; // wait for serial port to connect.
  }
  Enocean.begin();

  // BME680 supports 0x77 (default) or 0x76 address
  if (!bme.begin(0x77))
  {
    while (1)
      yield();
  }

  // Set up BME680 (oversampling and filter initialization)
  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);
  bme.setPressureOversampling(BME680_OS_4X);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
  bme.setGasHeater(320, 150); // 320*C for 150 ms
}

void loop()
{
  data[0] = bme.temperature; //temp
  data[1] = bme.pressure / 100.0; //pressure
  data[2] = bme.humidity; //humidity
  data[3] = bme.gas_resistance / 1000.0; //gas

  Enocean.send(RADIO_ERP1, RORG_4BS, data);
  delay(3000);
}