#ifndef EnOcean_h
#define EnOcean_h

#include <Arduino.h>

typedef void (*AfterReceivedTel)(uint8_t /*rorg*/, uint8_t* /* ID */, uint8_t* /* pl */, uint8_t /* dbm */);

#define START_BYTE 0x55 // Sync Byte
#define PACKET_HEADER_LEN 0x04

enum PACKET_HEADER
{
  //SYN,
  DATA_LENGTH1 = 0,
  DATA_LENGTH2,
  OPT_LENGTH,
  PACKET_TYPE,
  //CRC8H,
};

// Packet Types
enum PACKET_TYPE
{
  RADIO_ERP1=0x01,
  RESPONSE,
  RADIO_SUB_TEL,
  EVENT,
  COMMON_COMMAND,
  SMART_ACK_COMMAND,
  REMOTE_MAN_COMMAND,
  RESERVED_01,
  RADIO_MESSAGE,
  RADIO_ERP2,
  CONFIG_COMMAND,
  COMMAND_ACCEPTED,
  RESERVED_02,
  RESERVED_03,
  RESERVED_04,
  RADIO_802_15_4,
  COMMAND_2_4,
};

class EnOcean
{
public:
  EnOcean(AfterReceivedTel pAfterReceived);
  void begin(); // Esp begins to listen packets from enocean module via UART2
  uint8_t send(uint8_t packetType, uint8_t rorg, uint8_t *pl);
  uint8_t deepSleep(uint32_t sleepPeriod);

private:
};

#endif // EnOcean_h
