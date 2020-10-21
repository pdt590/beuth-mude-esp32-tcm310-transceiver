#ifndef CRC8_h
#define CRC8_h

#include <Arduino.h>

uint8_t getCRC8(uint8_t crc, uint8_t const *buf, uint16_t len);

#endif // CRC8_h