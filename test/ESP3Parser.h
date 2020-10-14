#ifndef ESP3Parser_h
#define ESP3Parser_h

#include "Arduino.h"
#include "SerialCommunication.h"

typedef void (*AfterReceivedTel)(uint8_t/*rorg*/, uint32_t/* ID */, uint32_t/* data */, uint8_t/* rssi */);

#define START_BYTE 0x55
#define RORG_RPS 0xF6
#define RORG_1BS 0xD5
#define RORG_4BS 0xA5
#define RORG_VLD 0xD2

class ESP3Parser
{
public:
  ESP3Parser(AfterReceivedTel pAfterReceived);
  void initialization();

private:
};

#endif // ESP3Parser_h
