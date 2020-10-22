#include <Arduino.h>
#include "CRC8.h"
#include "SerialCommunication.h"
#include "EnOceanProfile.h"
#include "EnOcean.h"

/*
** BEGIN
** Init UART and listen packets from enocean module 
*/

static SerialCommunication SerialCom;
// Data Length - 2 bytes - 0xnnnn
static uint16_t dataLength;
static uint8_t dataLength1;
static uint8_t dataLength2;
// Optional Length - 1 byte - 0x07 (in case of RADIO_ERP1 = 1)
static uint8_t optLength;
// Packet Type - 1 byte - 0xnn (RADIO_ERP1 = 1)
static uint8_t packetType;
// CRC8H - 1 byte
static uint8_t crc8h;

// Radio Telegram
// RORG - 1 byte
static uint8_t rorg;
// Data - 1 or 4 bytes (1BS or 4BS)
static uint8_t payload[4];
// Sender ID - 4 bytes
static uint8_t senderId[4];
// Status - 1 byte
static uint8_t status;

// Optional Data
// SubTelNum - 1 byte - 0xnn (Send: 3 / receive: 0)
static uint8_t subTelNum;
// Destination ID - 4 byte - 0xnnnnnnnn (Broadcast - 0xFFFFFFFF)
static uint8_t destinationId[4];
// dBm - 1 byte - 0xnn
static uint8_t dbm;
// Security Level - 1 byte - 0x0n
static uint8_t securityLevel;

// CRC8D - 1 byte - 0xnn
static uint8_t crc8d;
static AfterReceivedTel pReceivedOpe;

static void reset();
static uint8_t getRORG();
static uint16_t getDataLength();
static uint32_t getSenderId();
static uint32_t getPayload();
static void prettyPrint();

static uint8_t decodeSync(char aChar);
static uint8_t decodeDataLength1(char aChar);
static uint8_t decodeDataLength2(char aChar);
static uint8_t decodeOptLength(char aChar);
static uint8_t decodePacketType(char aChar);
static uint8_t decodeCrc8h(char aChar);

static uint8_t decodeROrg(char aChar);
static uint8_t decodePayload1(char aChar);
static uint8_t decodePayload2(char aChar);
static uint8_t decodePayload3(char aChar);
static uint8_t decodePayload4(char aChar);
static uint8_t decodeSenderId1(char aChar);
static uint8_t decodeSenderId2(char aChar);
static uint8_t decodeSenderId3(char aChar);
static uint8_t decodeSenderId4(char aChar);
static uint8_t decodeStatus(char aChar);

static uint8_t decodeSubTelNum(char aChar);
static uint8_t decodeDstId1(char aChar);
static uint8_t decodeDstId2(char aChar);
static uint8_t decodeDstId3(char aChar);
static uint8_t decodeDstId4(char aChar);
static uint8_t decodeDbm(char aChar);
static uint8_t decodeSecLevel(char aChar);
static uint8_t decodeCrc8d(char aChar);

enum STATE_ESP3_RADIO_ERP1
{
  STATE_SYNC = 0,
  STATE_DATA_LENGTH1,
  STATE_DATA_LENGTH2,
  STATE_OPT_LENGTH,
  STATE_PACKET_TYPE,
  STATE_CRC8H,

  STATE_RORG,
  STATE_PAYLOAD_1,
  STATE_PAYLOAD_2,
  STATE_PAYLOAD_3,
  STATE_PAYLOAD_4,
  STATE_SENDER_1,
  STATE_SENDER_2,
  STATE_SENDER_3,
  STATE_SENDER_4,
  STATE_STATUS,

  STATE_SUBTEL_NUM,
  STATE_DST_ID_1,
  STATE_DST_ID_2,
  STATE_DST_ID_3,
  STATE_DST_ID_4,
  STATE_DBM,
  STATE_SEC_LEVEL,
  STATE_CRC8D,
};

typedef uint8_t (*DecodeOpe)(char);

const DecodeOpe DecodeOpeSet[] = {
    // HEADER
    decodeSync,        /* STATE_SYNC */
    decodeDataLength1, /* STATE_DATA_LENGTH1 */
    decodeDataLength2, /* STATE_DATA_LENGTH2 */
    decodeOptLength,   /* STATE_OPT_LENGTH */
    decodpPacketType,  /* STATE_PACKET_TYPE */
    decodeCrc8h,       /* STATE_CRC8H */

    // RADIO_TELEGRAM
    decodeROrg,      /* STATE_RORG */
    decodePayload1,  /* STATE_PAYLOAD_1 */
    decodePayload2,  /* STATE_PAYLOAD_2 */
    decodePayload3,  /* STATE_PAYLOAD_3 */
    decodePayload4,  /* STATE_PAYLOAD_4 */
    decodeSenderId1, /* STATE_SENDER_1 */
    decodeSenderId2, /* STATE_SENDER_2 */
    decodeSenderId3, /* STATE_SENDER_3 */
    decodeSenderId4, /* STATE_SENDER_4 */
    decodeStatus,    /* STATE_STATUS */

    // OPTIONAL_DATA
    decodeSubTelNum, /* STATE_SUBTEL_NUM */
    decodeDstId1,    /* STATE_DST_ID_1 */
    decodeDstId2,    /* STATE_DST_ID_2 */
    decodeDstId3,    /* STATE_DST_ID_3 */
    decodeDstId4,    /* STATE_DST_ID_4 */
    decodeDbm,       /* STATE_DBM */
    decodeSecLevel,  /* STATE_SEC_LEVEL */
    decodeCrc8d      /* STATE_CRC8D */
};


/* typedef struct __attribute__((packed))
{
  uint8_t sync;
  uint16_t data_len;
  uint8_t optional_data_len;
  uint8_t packet_type;
  uint8_t crc8_h;
  uint32_t *data;
  uint32_t *optional_data;
  uint8_t crc8_d;
} enocean_packet_t;
 */
typedef struct __attribute__((packed))
{
  uint8_t sync;
  uint16_t data_len;
  uint8_t optional_data_len;
  uint8_t packet_type;
  uint8_t crc8_h;
} enocean_packet_header_t;

typedef struct __attribute__((packed))
{
  uint8_t rorg;
  uint8_t payload;
  uint32_t sender_id;
  uint8_t status;
} enocean_packet_data_radio_erp1_RPS_t;

typedef struct __attribute__((packed))
{
  uint8_t rorg;
  uint8_t payload;
  uint32_t sender_id;
  uint8_t status;
} enocean_packet_data_radio_erp1_1BS_t;

typedef struct __attribute__((packed))
{
  uint8_t rorg;
  uint32_t payload;
  uint32_t sender_id;
  uint8_t status;
} enocean_packet_data_radio_erp1_4BS_t;

typedef struct __attribute__((packed))
{
  uint8_t sub_tel_num;
  uint32_t dest_id;
  uint8_t dbm;
  uint8_t sec_level;
} enocean_packet_optdata_radio_erp1_t;

typedef uint8_t enocean_packet_crc8d_t;


EnOcean::EnOcean(AfterReceivedTel pAfterReceived)
{
  pReceivedOpe = pAfterReceived;
  reset();
}

void EnOcean::begin() // Esp begins to listen packets from enocean module via UART2
{
#ifdef DEBUG
  Serial.println("                                   ");
  Serial.println("                                   ");
  Serial.println("");
  Serial.println("    ID     R-ORG     Data       dBm");
  Serial.println("------------------------------------");
#endif

  SerialCom.init();
  SerialCom.setReceptOpe((ReceptionOpe *)DecodeOpeSet);
}

static void reset()
{
  memset(payload, 0, sizeof(payload));
  memset(senderId, 0, sizeof(senderId));
  dbm = 0;
}

/* static uint8_t getRORG() {
  return rorg;
} */

static uint16_t getDataLength()
{
  return ((uint16_t(dataLength1) << 8) & 0xff00) + (dataLength2 & 0xff);
}

static uint32_t getSenderId()
{
  uint32_t aResponse = ((uint32_t(senderId[0]) << 24) & 0xFF000000) + ((uint32_t(senderId[1]) << 16) & 0x00FF0000) + ((uint32_t(senderId[2]) << 8) & 0x0000FF00) + (uint32_t(senderId[3]) & 0x000000FF);
  return aResponse;
}

static uint32_t getPayload()
{
  uint32_t aResponse = ((uint32_t(payload[0]) << 24) & 0xFF000000) + ((uint32_t(payload[1]) << 16) & 0x00FF0000) + ((uint32_t(payload[2]) << 8) & 0x0000FF00) + (uint32_t(payload[3]) & 0x000000FF);
  return aResponse;
}

static void prettyPrint()
{
  uint8_t i;

  Serial.print(" ");
  for (i = 0; i < 4; i++)
  {
    if ((senderId[i] & 0xF0) == 0x00)
    {
      Serial.print((senderId[i] & 0xF0), HEX);
    }
    Serial.print(senderId[i], HEX);
  }

  switch (rorg)
  {
  case RORG_RPS: /* RPS Telegram */
    Serial.print("   RPS   ");
    if ((payload[0] & 0xF0) == 0x00)
    {
      Serial.print((payload[0] & 0xF0), HEX);
    }
    Serial.print(payload[0], HEX);
    Serial.print("          ");
    break;

  case RORG_1BS: /* 1BS Telegram */
    Serial.print("   1BS   ");
    if ((payload[0] & 0xF0) == 0x00)
    {
      Serial.print((payload[0] & 0xF0), HEX);
    }
    Serial.print(payload[0], HEX);
    Serial.print("          ");
    break;

  case RORG_4BS: /* 4BS Telegram */
    Serial.print("   4BS   ");
    for (i = 0; i < 4; i++)
    {
      if ((payload[i] & 0xF0) == 0x00)
      {
        Serial.print((payload[i] & 0xF0), HEX);
      }
      Serial.print(payload[i], HEX);
      Serial.print(" ");
    }
    break;
  }

  Serial.print("  -");
  Serial.println(dbm, DEC);
}

/* STATE_SYNC */
static uint8_t decodeSync(char aChar)
{
  uint8_t state;
  if (aChar == START_BYTE)
  {
    state = STATE_DATA_LENGTH1;
  }
  else
  {
    state = STATE_SYNC;
  }
  return state;
}

/* STATE_DATA_LENGTH1 */
static uint8_t decodeDataLength1(char aChar)
{
  dataLength1 = aChar;
  return STATE_DATA_LENGTH2;
}

/* STATE_DATA_LENGTH2 */
static uint8_t decodeDataLength2(char aChar)
{
  dataLength2 = aChar;
  //dataLength = getDataLength();
  return STATE_OPT_LENGTH;
}

/* STATE_OPT_LENGTH */
static uint8_t decodeOptLength(char aChar)
{
  optLength = aChar;
  return STATE_PACKET_TYPE;
}

/* STATE_PACKET_TYPE */
static uint8_t decodpPacketType(char aChar)
{
  packetType = aChar;
  return STATE_CRC8H;
}

/* STATE_CRC8H */
static uint8_t decodeCrc8h(char aChar)
{
  crc8h = aChar;
  return STATE_RORG;
}

/* STATE_RORG */
static uint8_t decodeROrg(char aChar)
{
  rorg = aChar;
  return STATE_PAYLOAD_1;
}

/* STATE_PAYLOAD_1 */
static uint8_t decodePayload1(char aChar)
{
  uint8_t state = 0;
  payload[0] = aChar;
  if ((rorg == RORG_1BS) || (rorg == RORG_RPS))
  {
    state = STATE_SENDER_1;
  }
  else if (rorg == RORG_4BS)
  {
    state = STATE_PAYLOAD_2;
  }
  return state;
}

/* STATE_PAYLOAD_2 */
static uint8_t decodePayload2(char aChar)
{
  payload[1] = aChar;
  return STATE_PAYLOAD_3;
}

/* STATE_PAYLOAD_3 */
static uint8_t decodePayload3(char aChar)
{
  payload[2] = aChar;
  return STATE_PAYLOAD_4;
}

/* STATE_PAYLOAD_4 */
static uint8_t decodePayload4(char aChar)
{
  payload[3] = aChar;
  return STATE_SENDER_1;
}

/* STATE_SENDER_1 */
static uint8_t decodeSenderId1(char aChar)
{
  senderId[0] = aChar;
  return STATE_SENDER_2;
}

/* STATE_SENDER_2 */
static uint8_t decodeSenderId2(char aChar)
{
  senderId[1] = aChar;
  return STATE_SENDER_3;
}

/* STATE_SENDER_3 */
static uint8_t decodeSenderId3(char aChar)
{
  senderId[2] = aChar;
  return STATE_SENDER_4;
}

/* STATE_SENDER_4 */
static uint8_t decodeSenderId4(char aChar)
{
  senderId[3] = aChar;
  return STATE_STATUS;
}

/* STATE_STATUS */
static uint8_t decodeStatus(char aChar)
{
  status = aChar;
  return STATE_SUBTEL_NUM;
}

/* STATE_SUBTEL_NUM */
static uint8_t decodeSubTelNum(char aChar)
{
  subTelNum = aChar;
  return STATE_DST_ID_1;
}

/* STATE_DST_ID_1 */
static uint8_t decodeDstId1(char aChar)
{
  destinationId[0] = aChar;
  return STATE_DST_ID_2;
}

/* STATE_DST_ID_2 */
static uint8_t decodeDstId2(char aChar)
{
  destinationId[1] = aChar;
  return STATE_DST_ID_3;
}

/* STATE_DST_ID_3 */
static uint8_t decodeDstId3(char aChar)
{
  destinationId[2] = aChar;
  return STATE_DST_ID_4;
}

/* STATE_DST_ID_4 */
static uint8_t decodeDstId4(char aChar)
{
  destinationId[3] = aChar;
  return STATE_DBM;
}

/* STATE_DBM */
static uint8_t decodeDbm(char aChar)
{
  dbm = aChar;
  return STATE_SEC_LEVEL;
}

/* STATE_SEC_LEVEL */
static uint8_t decodeSecLevel(char aChar)
{
  securityLevel = aChar;
  return STATE_CRC8D;
}

/* STATE_CRC8D */
static uint8_t decodeCrc8d(char aChar)
{
  crc8d = aChar;

  uint32_t senderId = getSenderId();
  uint32_t data = getPayload();

  if (pReceivedOpe != NULL)
  {
    (*pReceivedOpe)(rorg, senderId, data, dbm);
  }

#ifdef DEBUG
  if ((rorg == RORG_RPS) || (rorg == RORG_1BS) || (rorg == RORG_4BS) || (rorg == RORG_VLD))
  { // RPS 1BS 4BS
    prettyPrint();
  }
#endif

  reset();
  return STATE_SYNC;
}

/*
** END
** Init UART and listen packets from enocean module 
*/

/*
** BEGIN
** Send packet to enocean module 
*/
enocean_packet_header_t packetHeader;
enocean_packet_data_radio_erp1_RPS_t packetData_Erp1_RPS;
enocean_packet_data_radio_erp1_1BS_t packetData_Erp1_1BS;
enocean_packet_data_radio_erp1_4BS_t packetData_Erp1_4BS;
enocean_packet_optdata_radio_erp1_t packetOptData;
enocean_packet_crc8d_t packetCRC8D;

uint8_t EnOcean::sendPacket(uint8_t packetType, uint8_t rorg, uint8_t *pl)
{

  packetHeader.sync = START_BYTE;
  if (packetType == RADIO_ERP1)
  {
    switch (rorg)
    {
    case RORG_RPS:
    case RORG_1BS:
      packetHeader.data_len = 0x0007;
      break;
    case RORG_4BS:
      packetHeader.data_len = 0x000A;
      break;

    default:
      break;
    }
    packetHeader.optional_data_len = 0x07; // fixed 7 bytes
  }
  else
  {
    return 0;
  }

  packetHeader.packet_type = packetType;
  //pPacket.crc8_h = getCRC8(0, (const uint8_t *)&(pPacket.data_len), 4); // TODO
  packetHeader.crc8_h = 0x0F;

  SerialCom.sendBuffer((const char *)&packetHeader, sizeof(packetHeader));

  if (packetType == RADIO_ERP1)
  {
    switch (rorg)
    {
    case RORG_RPS:
      packetData_Erp1_RPS.rorg = RORG_RPS;
      packetData_Erp1_RPS.payload = *pl;
      packetData_Erp1_RPS.sender_id = 0xFF00FF00;
      packetData_Erp1_RPS.status = 0x01;
      SerialCom.sendBuffer((const char *)&packetData_Erp1_RPS, sizeof(packetData_Erp1_RPS));
      break;
    case RORG_1BS:
      packetData_Erp1_1BS.rorg = RORG_RPS;
      packetData_Erp1_1BS.payload = *pl;
      packetData_Erp1_1BS.sender_id = 0xFF00FF00;
      packetData_Erp1_1BS.status = 0x01;
      SerialCom.sendBuffer((const char *)&packetData_Erp1_1BS, sizeof(packetData_Erp1_1BS));
      break;
    case RORG_4BS:
      packetData_Erp1_4BS.rorg = RORG_RPS;
      packetData_Erp1_4BS.payload = *pl;
      packetData_Erp1_4BS.sender_id = 0xFF00FF00;
      packetData_Erp1_4BS.status = 0x01;
      SerialCom.sendBuffer((const char *)&packetData_Erp1_4BS, sizeof(packetData_Erp1_4BS));
      break;

    default:
      break;
    }
  }
  else
  {
    return 0;
  }

  packetOptData.sub_tel_num = 0x00;   // TODO 0x03
  packetOptData.dest_id = 0xFFFFFFFF; // Broadcast
  packetOptData.dbm = 0xFF;           // Send case
  packetOptData.sec_level = 0x00;     // Not process
  SerialCom.sendBuffer((const char *)&packetOptData, sizeof(packetOptData));

  //pPacket->crc8_d = getCRC8(0, (const uint8_t *)pPacket->data_buffer, (pPacket->data_len + (uint16_t)pPacket->optional_data_len)); // TODO
  packetCRC8D = 0x0F;
  SerialCom.sendByte(packetCRC8D);

  // 55 00 07 07 01 7A F6 00 FE FB FE 35 20 00 FF FF FF FF 39 00 5C // Test RADIO_ERP1_RORG_RPS packet
  //                         01 99 83 26    01             #     #  // Received packet at TCM310
  uint8_t test_packet[21] = {0x55, 0x00, 0x07, 0x07, 0x01, 0x7A, 0xF6, 0x00, 0xFE, 0xFB, 0xFE, 0x35, 0x20, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x39, 0x00, 0x5C};

  //Serial.write((const uint8_t *)&pPacket, sizeof(pPacket)); // Send packet via UART0
  //SerialCom.sendPacket((const char *)&test_packet, sizeof(test_packet));
  //SerialCom.sendBuffer((const char *)&pPacket, sizeof(pPacket));
  return 1;
}

/*
** END
** Send packet to enocean module 
*/