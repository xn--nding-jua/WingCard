void wingInit() {
  pinMode(WING_EVENTOUT, OUTPUT);
  digitalWrite(WING_EVENTOUT, false);

  /*
    SercomSPISlave-library has specific pins. We are using Sercom3 as it
    has good-matching PINs for the Arduino MKR Vidor 4000 board:
    enum MOSI_Pins {PA16, PA22};
    enum SCK_Pins {PA17, PA23};
    enum SS_Pins {PA18, PA20, PA24};
    enum MISO_Pins {PA19, PA21, PA25};
  */
  SPISlave.SercomInit(SPISlave.MOSI_Pins::PA16, SPISlave.SCK_Pins::PA17, SPISlave.SS_Pins::PA20, SPISlave.MISO_Pins::PA19); // MOSI_Pins MOSI_Pin, SCK_Pins SCK_Pin, SS_Pins SS_Pin, MISO_Pins MISO_Pin
}

void wingHandleCommunication() {
  if (spiRxBufferPointer >= 8) {
    // we received at least 8 bytes. Check command and decide if we have to wait for more data depending on command

    if (spiRxBuffer[0] != 0x2a) {
      spiResetRx();
      return;
    }else{
      // seems to be a valid command: check the type of command
      if (spiRxBuffer[1] == 'H') { // check for *H*
        // WING wants like to know us
        wingSend("*hWING-LIVE:A:038-0-g52bc66c7:4EDC5534*", 0xb4, false, 0); // send String without EventOut-Signal
        wingStartupCounterB = 25; // 2.5 Sekunden
      }else if ((spiRxBuffer[1] == '1') && (spiRxBuffer[2] == 'S')) { // check for *1S*
        // set number of channels to record for Card #1
      }else if ((spiRxBuffer[1] == '1') && (spiRxBuffer[2] == 'Q')) {
        // format Card #1
      }else if ((spiRxBuffer[1] == '1') && (spiRxBuffer[2] == 'H')) {
        // record Card #1
        // new session-name is transmitted on spiRxBuffer[3...6], spiRxBuffer[7..8] should be 0x2000
      }else if ((spiRxBuffer[1] == '1') && (spiRxBuffer[2] == 'D') && (spiRxBufferPointer >= 16)) {
        // play Card #1
        // spiRxBuffer[3...11] seems to have some options for an index(?)
      }else if ((spiRxBuffer[1] == '1') && (spiRxBuffer[2] == 'E')) { // TODO: check this command
        // pause Card #1
      }else if ((spiRxBuffer[1] == '1') && (spiRxBuffer[2] == 'F')) { // TODO: check this command
        // stop Card #1
      }else if ((spiRxBuffer[1] == '1') && (spiRxBuffer[2] == 'O')) {
        // select session for Card #1
        // session name is transmitted in spiRxBuffer[3...6] as 4-byte timeCode
      }else if ((spiRxBuffer[1] == '1') && (spiRxBuffer[2] == 'M')) {
        // set index for Card #1
        // index is transmitted in spiRxBuffer[3...6] as time-Index
      }
    }
  }
}

void wingSend(String data, uint8_t checksum, bool setEventOut, uint8_t offset) {
  // copy command to spiTxBuffer and calculate the checksum-byte
  // set spiTxBufferPointer to 0 to start communication


/*
  We want to write the following data here:
        	~  ~  ~  ~  ~  ~  ~  ~ 
    WING	00 00 00 00 00 00 00 00
    CARD	2a 31 65 2a e4 00 00 00
    	    *  1  e  *  nA ~  ~  ~ 
*/

  // flush buffer with zeros
  for (uint8_t i=0; i<SPI_BUFFER_LEN; i++) {
    spiTxBuffer[i] = 0x00;
  }

  // write desired data to buffer
  uint8_t dataLen = data.length();
  data.toCharArray((char*)spiTxBuffer[offset], dataLen);
  spiTxBuffer[dataLen + offset] = checksum; // TODO: check calculation and implement it instead of setting it manually
  spiTxLen = findMultipliersByEight(dataLen + offset + 1);

  // check last two bytes to be 0x00. If not, add another 8 bytes to transmit
  if ((spiTxBuffer[spiTxLen-2] != 0x00) || (spiTxBuffer[spiTxLen-1] != 0x00)) {
    spiTxLen += 8; // add another 8 bytes to meet requirements of WING to have two following 0x00 at end of transmission
  }

  spiTxBufferPointer = 0; // begin transaction
  SERCOM3->SPI.DATA.reg = spiTxBuffer[0]; // copy first array-element to SPI-output-register
  if (setEventOut) {
    digitalWrite(WING_EVENTOUT, true); // tell the WING that we have new data and like to talk
  }
}

void wingAnnounceCardPart1() {
  wingSend("*1e*", 0xe4, true, 0); // send "noCard"
  delayMicroseconds(500); // we transmit 64 bit at 2MHz, so wait 64*0.5us = 32us + additional time that WING takes to talk to us
  wingSend("*2e*", 0xe4, true, 0); // send "noCard"
  delayMicroseconds(500);
}

void wingAnnounceCardPart2() {
  wingSend("*BE*", 0xc4, true, 0);
  delayMicroseconds(500); // we transmit 64 bit at 2MHz, so wait 64*0.5us = 32us + additional time that WING takes to talk to us
  wingSend("*1e*", 0xe4, true, 1); // send "noCard"
  delayMicroseconds(500); // we transmit 64 bit at 2MHz, so wait 64*0.5us = 32us + additional time that WING takes to talk to us
  wingSend("*2e*", 0xe4, true, 2); // send "noCard"
  delayMicroseconds(500); // we transmit 64 bit at 2MHz, so wait 64*0.5us = 32us + additional time that WING takes to talk to us
}

void wingAnnounceSdCardPresence(uint8_t card, uint8_t sizeInGb) {
  wingSend("*1e*", 0xe4, true, 2);
  delay(1);

  wingSend("*1S0", 0x00, true, 0);
  spiTxBuffer[8] = 0x5e; // TODO: convert sizeInGb to this numbers
  spiTxBuffer[9] = 0xdb; // TODO: convert sizeInGb to this numbers
  spiTxBuffer[10] = 0x04; // TODO: convert sizeInGb to this numbers
  spiTxBuffer[11] = 0x00; // TODO: convert sizeInGb to this numbers
  spiTxBuffer[12] = 0x2a; // *
  spiTxBuffer[13] = 0xca;
  spiTxBuffer[14] = 0x00;
  spiTxBuffer[15] = 0x00;
  spiTxLen = 16;
  delay(1);
}

void wingAliveCommand() {
  wingSend("*BE*", 0xc4, true, 0);
}

String wingTimecodeToString(String timecodeHex) {
  uint32_t timecode = hexToInt(timecodeHex);

  uint16_t year = (timecode >> 25) + 1980;
  uint8_t month =  (timecode & 0x1FFFFFF) >> 21;
  uint8_t day = (timecode & 0x1FFFFF) >> 16;
  uint8_t hour = (timecode & 0xFFFF) >> 11;
  uint8_t minute =  (timecode & 0x7FF) >> 5;
  uint8_t second =  (timecode & 0x1F) << 1;

  return String(day) + "." + String(month) + "." + String(year) + " " + String(hour) + ":" + String(minute) + ":" + String(second);
}

uint8_t wingTimecodeToTracknumber(String timecodeHex) {
  uint32_t timecode = hexToInt(timecodeHex);
  uint8_t minute =  (timecode & 0x7FF) >> 5;
  return minute;
}

String wingDateToTimecode(uint8_t day, uint8_t month, uint16_t year, uint8_t hour, uint8_t minute, uint8_t second) {
  uint32_t timecode = (uint32_t)((year - 1980)) << 25;
  timecode += (uint32_t)month << 21;
  timecode += (uint32_t)day << 16;
  timecode += (uint32_t)hour << 11;
  timecode += (uint32_t)minute << 5;
  timecode += (uint32_t)second << 1;

  return intToHex(timecode, 8);
}

String wingCreateDummyTOC() {
  String toc;

  for (uint8_t i=0; i<10; i++) { // TODO: fixed Number of entries here
    toc = toc + wingDateToTimecode(1, 1, 2025, 0, i, 0) + "|";
  }

  return toc;
}

void SERCOM3_Handler() {
  // Information see AN2465 SAM D21 SERCOM SPI Configuration
  // https://ww1.microchip.com/downloads/aemDocuments/documents/OTH/ProductDocuments/LegacyCollaterals/00002465A.pdf

  // =================================================
  // Functions to receive data from WING
  // =================================================
  uint8_t data = 0;
  data = (uint8_t)SERCOM3->SPI.DATA.reg;
  uint8_t interrupts = SERCOM3->SPI.INTFLAG.reg; // Read SPI interrupt register

  // Slave Select Low interrupt
  if (interrupts & (1 << 3)) // 1000 = bit 3 = SSL // page 503
  {
    SERCOM3->SPI.INTFLAG.bit.SSL = 1; // Clear Slave Select Low interrupt
  }
  
  // Data Received Complete interrupt: this is where the data is received, which is used in the main loop
  if (interrupts & (1 << 2)) // 0100 = bit 2 = RXC // page 503
  {
    data = SERCOM3->SPI.DATA.reg; // Read data register
    SERCOM3->SPI.INTFLAG.bit.RXC = 1; // Clear Receive Complete interrupt
  }
  
  // Data Transmit Complete interrupt
  if (interrupts & (1 << 1)) // 0010 = bit 1 = TXC // page 503
  {
    SERCOM3->SPI.INTFLAG.bit.TXC = 1; // Clear Transmit Complete interrupt
  }
  
  // Data Register Empty interrupt
  if (interrupts & (1 << 0)) // 0001 = bit 0 = DRE // page 503
  {
    SERCOM3->SPI.DATA.reg = 0xAA;
  }
  
  // Write data to spiRxBuffer
  spiRxBuffer[spiRxBufferPointer++] = data; // write to current element and increment bufferPointer
  if (spiRxBufferPointer >= 60) {
    spiRxBufferPointer = 0;
  }
  spiRxBufferNewData = true;


  // =================================================
  // Functions to send data to WING
  // =================================================
  // prepare output-data from spiTxBuffer for next SPI-transmission
  if (spiTxBufferPointer > -1) {
    if (spiTxBufferPointer >= spiTxLen) {
      // we have no more data -> output zeros to WING
      SERCOM3->SPI.DATA.reg = 0x00;
      spiTxBufferPointer = -1;
    }else{
      // we have still data to transmit
      SERCOM3->SPI.DATA.reg = spiTxBuffer[++spiTxBufferPointer]; // go to next array-element as we've already sent the first byte
      if (spiTxBufferPointer == (spiTxLen - 1)) {
        // last byte will be sent on next SPI-transmission -> turnOff EVENTOUT
        digitalWrite(WING_EVENTOUT, false);
      }
    }
  }else{
    // nothing to send -> output zeros to WING
    SERCOM3->SPI.DATA.reg = 0x00;
  }
}

uint8_t findMultipliersByEight(uint8_t bytes) {
  return ((bytes/8) + 1) * 8; // caution: we need an integer-division here!
}

void spiResetRx() {
  // not a valid command for us
  spiRxBufferPointer = 0;

  // flush buffer with zeros
  for (uint8_t i=0; i<SPI_BUFFER_LEN; i++) {
    spiRxBuffer[i] = 0x00;
  }
}