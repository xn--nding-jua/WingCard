const char* versionstring = "v0.0.1";
const char compile_date[] = __DATE__ " " __TIME__;

#define WING_EVENTOUT 7

#include <SercomSPISlave.h>
#include "Ticker.h"

/*
Arduino SAMD21 SERCOM usage:
SERCOM0: I2C      <- 
SERCOM1: SPI      <- 
SERCOM2: SPI1     <- 
SERCOM3: SPISlave <-> WING
SERCOM4: unused (Pins 4/5)
SERCOM5: unused (Pins 16/17)

SerialUSB: USB
Serial1:   X32
*/

Sercom3SPISlave SPISlave;
#define SPI_BUFFER_LEN 60  // longest SPI-command is 7*8 = 56 bytes long
uint8_t spiTxBuffer[SPI_BUFFER_LEN];
uint8_t spiRxBuffer[SPI_BUFFER_LEN];
volatile uint8_t spiRxBufferPointer = 0;
volatile int8_t spiTxBufferPointer = -1; // -1 = no data to send
volatile uint8_t spiTxLen = 0;
volatile bool spiRxBufferNewData = false;

uint8_t ledCounter = 10; // 500ms
uint8_t wingAliveCounter = 59; // preload to 5 seconds (for ticker with 85ms)
uint8_t wingStartupCounter = 100; // 10 Sekunden
uint8_t wingStartupCounterB = 0;
bool wingPlayback = false;
