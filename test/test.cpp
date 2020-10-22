  // 55 00 07 07 01 7A F6 00 FE FB FE 35 20 00 FF FF FF FF 39 00 5C // Test RADIO_ERP1_RORG_RPS packet
  //                         01 99 83 26    01             #     #  // Received packet at TCM310

  /*
  uint8_t test_packet[21] = {0x55, 0x00, 0x07, 0x07, 0x01, 0x7A, 0xF6, 0x00, 0xFE, 0xFB, 0xFE, 0x35, 0x20, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x39, 0x00, 0x5C};
  for(uint8_t i=0; i < sizeof(test_packet); i++ ) {
    SerialCom.sendByte(test_packet[i]);
  } 
  */

  /* 
  uint8_t pPacket[10]
  for (uint8_t i = 0; i < sizeof(pPacket); i++)
  {
    SerialCom.sendByte(pPacket[i]);
  } 
  */

  //SerialCom.sendBuffer((const char *)pPacket, sizeof(pPacket));