#ifndef EnOcean_h
#define EnOcean_h

#include <Arduino.h>
#include "SerialCommunication.h"

typedef void (*AfterReceivedTel)(uint8_t /*rorg*/, uint32_t /* ID */, uint8_t* /* pl */, uint8_t /* dbm */);

// Sync Byte
#define START_BYTE 0x55

// Packet Types
#define RADIO_ERP1 0x01

// R-ORG
#define RORG_RPS 0xF6
#define RORG_1BS 0xD5
#define RORG_4BS 0xA5
#define RORG_VLD 0xD2

class EnOcean
{
public:
  EnOcean(AfterReceivedTel pAfterReceived);
  void begin(); // Esp begins to listen packets from enocean module via UART2
  uint8_t sendPacket(uint8_t packetType, uint8_t rorg, uint8_t *pl);

private:
};

#endif // EnOcean_h
