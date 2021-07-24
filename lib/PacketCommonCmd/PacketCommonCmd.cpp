#include <Arduino.h>
#include "Util.h"
#include "CRC8.h"
#include "SerialCommunication.h"
#include "EnOcean.h"
#include "PacketCommonCmd.h"

/*
** BEGIN
** Send packet to enocean module 
*/

static SerialCommunication SerialCom;
static void serialSendHeader(uint16_t dataLen, uint8_t optDataLen);
static void serialSendOptData(uint8_t code, uint8_t *pPacketOptData, uint8_t packetOptDataLen);
static void serialSendCRC8D(uint8_t *pPacketData, uint8_t *pPacketOptData, uint16_t dataLen, uint8_t optDataLen);

static void serialSendHeader(uint16_t dataLen, uint8_t optDataLen)
{
  SerialCom.sendByte(START_BYTE); // SENT SYN

  uint8_t pPacketHeader[PACKET_HEADER_LEN];
  pPacketHeader[DATA_LENGTH1] = dataLen >> 8;
  pPacketHeader[DATA_LENGTH2] = dataLen;
  pPacketHeader[OPT_LENGTH] = optDataLen;
  pPacketHeader[PACKET_TYPE] = COMMON_COMMAND;
  SerialCom.sendBuffer((const char *)pPacketHeader, sizeof(pPacketHeader)); // SENT HEADER

  uint8_t packetCRC8H = getCRC8(0, (const uint8_t *)pPacketHeader, PACKET_HEADER_LEN);
  SerialCom.sendByte(packetCRC8H); // SENT CRC8H
}

static void serialSendOptData(uint8_t code, uint8_t *pPacketOptData, uint8_t packetOptDataLen)
{
  switch (code)
  {
  case CO_WR_SLEEP:
  case CO_WR_RESET:
    break;

  default:
    break;
  }
}

static void serialSendCRC8D(uint8_t *pPacketData, uint8_t *pPacketOptData, uint16_t dataLen, uint8_t optDataLen)
{
  uint8_t packetCRC8D = getCRC8(0, (const uint8_t *)pPacketData, dataLen);
  packetCRC8D = getCRC8(packetCRC8D, (const uint8_t *)pPacketOptData, optDataLen);

  SerialCom.sendByte(packetCRC8D);
}

uint8_t PacketCommonCmd::sendPacket(uint8_t code) // TODO send for all codes
{
  uint8_t *pPacketData, *pPacketOptData;

  switch (code)
  {
  case CO_WR_RESET:
    serialSendHeader(PACKET_COMMON_CMD_WR_RESET_DATA_LEN, PACKET_COMMON_CMD_WR_RESET_OPT_DATA_LEN); // SENT HEADER

    pPacketData = (uint8_t *)malloc(PACKET_COMMON_CMD_WR_RESET_DATA_LEN * sizeof(uint8_t));
    pPacketOptData = (uint8_t *)malloc(PACKET_COMMON_CMD_WR_RESET_OPT_DATA_LEN * sizeof(uint8_t));
    pPacketData[CMD_CODE_WR_RESET] = code;
    SerialCom.sendBuffer((const char *)pPacketData, PACKET_COMMON_CMD_WR_RESET_DATA_LEN); // SENT DATA

    serialSendOptData(CO_WR_RESET, pPacketOptData, PACKET_COMMON_CMD_WR_RESET_OPT_DATA_LEN); // SENT OPT DATA

    serialSendCRC8D(pPacketData, pPacketOptData, PACKET_COMMON_CMD_WR_RESET_DATA_LEN, PACKET_COMMON_CMD_WR_RESET_OPT_DATA_LEN); // SENT CRC8D
    break;

  default:
    break;
  }

  //free(pPacketData);
  //free(pPacketOptData);

  return 1;
}

uint8_t PacketCommonCmd::sendPacket(uint8_t code, uint32_t sleepPeriod) // TODO send for all codes
{
  uint8_t *pPacketData, *pPacketOptData;

  switch (code)
  {
  case CO_WR_SLEEP:
    serialSendHeader(PACKET_COMMON_CMD_WR_SLEEP_DATA_LEN, PACKET_COMMON_CMD_WR_SLEEP_OPT_DATA_LEN); // SENT HEADER

    pPacketData = (uint8_t *)malloc(PACKET_COMMON_CMD_WR_SLEEP_DATA_LEN * sizeof(uint8_t));
    pPacketOptData = (uint8_t *)malloc(PACKET_COMMON_CMD_WR_SLEEP_OPT_DATA_LEN * sizeof(uint8_t));
    pPacketData[CMD_CODE_WR_SLEEP] = code;
    pPacketData[DEEP_SLEEP_1] = sleepPeriod >> 24;
    pPacketData[DEEP_SLEEP_2] = sleepPeriod >> 16;
    pPacketData[DEEP_SLEEP_3] = sleepPeriod >> 8;
    pPacketData[DEEP_SLEEP_4] = sleepPeriod;
    SerialCom.sendBuffer((const char *)pPacketData, PACKET_COMMON_CMD_WR_SLEEP_DATA_LEN); // SENT DATA

    serialSendOptData(CMD_CODE_WR_SLEEP, pPacketOptData, PACKET_COMMON_CMD_WR_SLEEP_OPT_DATA_LEN); // SENT OPT DATA

    serialSendCRC8D(pPacketData, pPacketOptData, PACKET_COMMON_CMD_WR_SLEEP_DATA_LEN, PACKET_COMMON_CMD_WR_SLEEP_OPT_DATA_LEN); // SENT CRC8D
    break;

  default:
    break;
  }

  //free(pPacketData);
  //free(pPacketOptData);

  return 1;
}