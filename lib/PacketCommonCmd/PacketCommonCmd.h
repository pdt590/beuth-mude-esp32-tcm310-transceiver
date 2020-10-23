#ifndef PacketCommonCmd_h
#define PacketCommonCmd_h

#include <Arduino.h>

enum PACKET_COMMON_CMD_CODE
{
  CO_WR_SLEEP = 0x01,
  CO_WR_RESET,
};

#define PACKET_COMMON_CMD_WR_SLEEP_DATA_LEN 0x0005
#define PACKET_COMMON_CMD_WR_SLEEP_OPT_DATA_LEN 0x00
enum PACKET_COMMON_CMD_WR_SLEEP_DATA
{
  //SYN,
  CMD_CODE_WR_SLEEP = 0,
  DEEP_SLEEP_1,
  DEEP_SLEEP_2,
  DEEP_SLEEP_3,
  DEEP_SLEEP_4,
  //CRC8H,
};

#define PACKET_COMMON_CMD_WR_RESET_DATA_LEN 0x0001
#define PACKET_COMMON_CMD_WR_RESET_OPT_DATA_LEN 0x00
enum PACKET_COMMON_CMD_WR_RESET_DATA
{
  //SYN,
  CMD_CODE_WR_RESET = 0,
  //CRC8H,
};

class PacketCommonCmd
{
public:
  uint8_t sendPacket(uint8_t code); // sleepPeriod from 0 to 16,777,215 x 10ms
  uint8_t sendPacket(uint8_t code, uint32_t sleepPeriod); // sleepPeriod from 0 to 16,777,215 x 10ms

private:
};

#endif // PacketCommonCmd_h