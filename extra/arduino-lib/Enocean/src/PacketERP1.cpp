#include <Arduino.h>
#include "CRC8.h"
#include "SerialCommunication.h"
#include "EnOcean.h"
#include "PacketERP1.h"

/*
** BEGIN
** Send packet to enocean module 
*/

static SerialCommunication SerialCom;
static void serialSendHeader(uint16_t dataLen, uint8_t optDataLen);
static void serialSendOptData(uint8_t *pPacketOptData);
static void serialSendCRC8D(uint8_t *pPacketData, uint8_t *pPacketOptData, uint16_t dataLen, uint8_t optDataLen);

static void serialSendHeader(uint16_t dataLen, uint8_t optDataLen)
{
  SerialCom.sendByte(START_BYTE); // SENT SYN

  uint8_t pPacketHeader[PACKET_HEADER_LEN];
  pPacketHeader[DATA_LENGTH1] = dataLen >> 8;
  pPacketHeader[DATA_LENGTH2] = dataLen;
  pPacketHeader[OPT_LENGTH] = optDataLen;
  pPacketHeader[PACKET_TYPE] = RADIO_ERP1;
  SerialCom.sendBuffer((const char *)pPacketHeader, sizeof(pPacketHeader)); // SENT HEADER

  uint8_t packetCRC8H = getCRC8(0, (const uint8_t *)pPacketHeader, PACKET_HEADER_LEN);
  SerialCom.sendByte(packetCRC8H); // SENT CRC8H
}

static void serialSendOptData(uint8_t *pPacketOptData)
{
  pPacketOptData[SUBTEL_NUM] = 0x00; // TODO 0x03
  pPacketOptData[DST_ID_1] = 0xFF;   // Broadcast
  pPacketOptData[DST_ID_2] = 0xFF;
  pPacketOptData[DST_ID_3] = 0xFF;
  pPacketOptData[DST_ID_4] = 0xFF;
  pPacketOptData[DBM] = 0xFF;                                                         // Send case
  pPacketOptData[SEC_LEVEL] = 0x00;                                                   // Not process sec
  SerialCom.sendBuffer((const char *)pPacketOptData, PACKET_RADIO_ERP1_OPT_DATA_LEN); // SENT OPTIONAL DATA
}

static void serialSendCRC8D(uint8_t *pPacketData, uint8_t *pPacketOptData, uint16_t dataLen, uint8_t optDataLen)
{
  uint8_t packetCRC8D = getCRC8(0, (const uint8_t *)pPacketData, dataLen);
  packetCRC8D = getCRC8(packetCRC8D, (const uint8_t *)pPacketOptData, optDataLen);

  SerialCom.sendByte(packetCRC8D);
}

uint8_t PacketERP1::sendPacket(uint8_t rorg, uint8_t *pl)
{
  uint8_t *pPacketData, *pPacketOptData;

  switch (rorg)
  {
  case RORG_RPS:
    serialSendHeader(PACKET_RADIO_ERP1_RPS_DATA_LEN, PACKET_RADIO_ERP1_OPT_DATA_LEN); // SENT HEADER

    pPacketData = (uint8_t *)malloc(PACKET_RADIO_ERP1_RPS_DATA_LEN * sizeof(uint8_t));
    pPacketOptData = (uint8_t *)malloc(PACKET_RADIO_ERP1_OPT_DATA_LEN * sizeof(uint8_t));
    pPacketData[RPS_RORG] = RORG_RPS;
    pPacketData[RPS_PAYLOAD] = *pl;
    pPacketData[RPS_SENDER_1] = 0xF0; // Should not be 0xFFFFFFFF. It is set by enocean module
    pPacketData[RPS_SENDER_2] = 0xF1;
    pPacketData[RPS_SENDER_3] = 0xF2;
    pPacketData[RPS_SENDER_4] = 0xF3;
    pPacketData[RPS_STATUS] = 0xFF;                                                  // Random value
    SerialCom.sendBuffer((const char *)pPacketData, PACKET_RADIO_ERP1_RPS_DATA_LEN); // SENT DATA

    serialSendOptData(pPacketOptData); // SENT OPT DATA

    serialSendCRC8D(pPacketData, pPacketOptData, PACKET_RADIO_ERP1_RPS_DATA_LEN, PACKET_RADIO_ERP1_OPT_DATA_LEN); // SENT CRC8D
  case RORG_1BS:
    serialSendHeader(PACKET_RADIO_ERP1_1BS_DATA_LEN, PACKET_RADIO_ERP1_OPT_DATA_LEN); // SENT HEADER

    pPacketData = (uint8_t *)malloc(PACKET_RADIO_ERP1_1BS_DATA_LEN * sizeof(uint8_t));
    pPacketOptData = (uint8_t *)malloc(PACKET_RADIO_ERP1_OPT_DATA_LEN * sizeof(uint8_t));
    pPacketData[ONEBS_RORG] = RORG_1BS;
    pPacketData[ONEBS_PAYLOAD] = *pl;
    pPacketData[ONEBS_SENDER_1] = 0xF0; // Should not be 0xFFFFFFFF. It is set by enocean module
    pPacketData[ONEBS_SENDER_2] = 0xF1;
    pPacketData[ONEBS_SENDER_3] = 0xF2;
    pPacketData[ONEBS_SENDER_4] = 0xF3;
    pPacketData[ONEBS_STATUS] = 0xFF;                                                // Random value
    SerialCom.sendBuffer((const char *)pPacketData, PACKET_RADIO_ERP1_1BS_DATA_LEN); // SENT DATA

    serialSendOptData(pPacketOptData); // SENT OPT DATA

    serialSendCRC8D(pPacketData, pPacketOptData, PACKET_RADIO_ERP1_1BS_DATA_LEN, PACKET_RADIO_ERP1_OPT_DATA_LEN); // SENT CRC8D
    break;
  case RORG_4BS:
    serialSendHeader(PACKET_RADIO_ERP1_4BS_DATA_LEN, PACKET_RADIO_ERP1_OPT_DATA_LEN); // SENT HEADER

    pPacketData = (uint8_t *)malloc(PACKET_RADIO_ERP1_4BS_DATA_LEN * sizeof(uint8_t));
    pPacketOptData = (uint8_t *)malloc(PACKET_RADIO_ERP1_OPT_DATA_LEN * sizeof(uint8_t));
    pPacketData[FOURBS_RORG] = RORG_4BS;
    pPacketData[FOURBS_PAYLOAD_1] = pl[0];
    pPacketData[FOURBS_PAYLOAD_2] = pl[1];
    pPacketData[FOURBS_PAYLOAD_3] = pl[2];
    pPacketData[FOURBS_PAYLOAD_4] = pl[3];
    pPacketData[FOURBS_SENDER_1] = 0xF0; // Should not be 0xFFFFFFFF. It is set by enocean module
    pPacketData[FOURBS_SENDER_2] = 0xF1;
    pPacketData[FOURBS_SENDER_3] = 0xF2;
    pPacketData[FOURBS_SENDER_4] = 0xF3;
    pPacketData[FOURBS_STATUS] = 0xFF;                                               // Random value
    SerialCom.sendBuffer((const char *)pPacketData, PACKET_RADIO_ERP1_4BS_DATA_LEN); // SENT DATA

    serialSendOptData(pPacketOptData); // SENT OPT DATA

    serialSendCRC8D(pPacketData, pPacketOptData, PACKET_RADIO_ERP1_4BS_DATA_LEN, PACKET_RADIO_ERP1_OPT_DATA_LEN); // SENT CRC8D
    break;
  }

  //free(pPacketData);
  //free(pPacketOptData);

  return 1;
}