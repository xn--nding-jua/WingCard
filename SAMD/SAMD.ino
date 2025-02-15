/*
  Demo-Program to communicate with Behringer WING via SPI-Protocol
  v0.0.1 built on 15.02.2025
  Infos: https://www.github.com/xn--nding-jua/WingCard
  Author: Christian Nöding
  
  Target-Board: Arduino Vidor 4000 (Atmel SAMD21 32-bit Cortex M0+ controller)
  Board-package: http://downloads.arduino.cc/Hourly/samd/package_samd-hourly-build_index.json

  Description:
  =====================================================================
  This Arduino-Project communicates with the Behringer WING and initializes as a WING-LIVE
  expansion Card. This project is in an early stage and only a proof-of-concept.

  Used libraries:
  =====================================================================
  SercomSPISlave v0.2.0 by lenvm
  Ticker v4.4.0 by Stefan Staub
  
  License information:
  =====================================================================
  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <https://www.gnu.org/licenses/>.

  Pin information:
  =====================================================================
  +------------+------------------+--------+--------+-----------------+--------+-----------------------+---------+---------+--------+--------+----------+----------+-----------------------------------------------------------+
  | Pin number |  MKR  Board pin  |  PIN   |  FPGA  | Notes           | Peri.A |     Peripheral B      | Perip.C | Perip.D | Peri.E | Peri.F | Periph.G | Periph.H | Usage for Audioplayer                                     |
  |            |                  |        |        |                 |   EIC  | ADC |  AC | PTC | DAC | SERCOMx | SERCOMx |  TCCx  |  TCCx  |    COM   | AC/GLCK  |                                                           |
  |            |                  |        |        |                 |(EXTINT)|(AIN)|(AIN)|     |     | (x/PAD) | (x/PAD) | (x/WO) | (x/WO) |          |          |                                                           |
  +------------+------------------+--------+--------+-----------------+--------+-----+-----+-----+-----+---------+---------+--------+--------+----------+----------+-----------------------------------------------------------+
  | 00         | D0               |  PA22  |  G1    |                 |  *06   |     |     | X10 |     |   3/00  |   5/00  |* TC4/0 | TCC0/4 |          | GCLK_IO6 |                                                           |
  | 01         | D1               |  PA23  |  N3    |                 |  *07   |     |     | X11 |     |   3/01  |   5/01  |* TC4/1 | TCC0/5 | USB/SOF  | GCLK_IO7 |                                                           |
  | 02         | D2               |  PA10  |  P3    |                 |   10   | *18 |     | X02 |     |   0/02  |   2/02  |*TCC1/0 | TCC0/2 | I2S/SCK0 | GCLK_IO4 |                                                           |
  | 03         | D3               |  PA11  |  R3    |                 |   11   | *19 |     | X03 |     |   0/03  |   2/03  |*TCC1/1 | TCC0/3 | I2S/FS0  | GCLK_IO5 |                                                           |
  | 04         | D4               |  PB10  |  T3    |                 |  *10   |     |     |     |     |         |   4/02  |* TC5/0 | TCC0/4 | I2S/MCK1 | GCLK_IO4 |                                                           |
  | 05         | D5               |  PB11  |  T2    |                 |  *11   |     |     |     |     |         |   4/03  |* TC5/1 | TCC0/5 | I2S/SCK1 | GCLK_IO5 |                                                           |
  | 06         | D6               |  PA20  |  G16   |                 |  *04   |     |     | X08 |     |   5/02  |   3/02  |        |*TCC0/6 | I2S/SCK0 | GCLK_IO4 | -> SPI NSS from WING                                      |
  | 07         | D7               |  PA21  |  G15   |                 |  *05   |     |     | X09 |     |   5/03  |   3/03  |        |*TCC0/7 | I2S/FS0  | GCLK_IO5 | -> EVENTOUT to WING                                       |
  +------------+------------------+--------+--------+-----------------+--------+-----+-----+-----+-----+---------+---------+--------+--------+----------+----------+-----------------------------------------------------------+
  |            |       SPI        |        |        |                 |        |     |     |     |     |         |         |        |        |          |          |                                                           |
  | 08         | MOSI             |  PA16  |  F16   |                 |  *00   |     |     | X04 |     |  *1/00  |   3/00  |*TCC2/0 | TCC0/6 |          | GCLK_IO2 | -> SPI-MOSI from WING                                     |
  | 09         | SCK              |  PA17  |  F15   |                 |  *01   |     |     | X05 |     |  *1/01  |   3/01  | TCC2/1 | TCC0/7 |          | GCLK_IO3 | -> SPI-SCK from WING                                      |
  | 10         | MISO             |  PA19  |  C16   |                 |   03   |     |     | X07 |     |  *1/03  |   3/03  |* TC3/1 | TCC0/3 | I2S/SD0  | AC/CMP1  | -> SPI-MISO to WING                                       |
  +------------+------------------+--------+--------+-----------------+--------------------+-----+-----+---------+---------+--------+--------+----------+----------+-----------------------------------------------------------+
  |            |       Wire       |        |        |                 |        |     |     |     |     |         |         |        |        |          |          |                                                           | 
  | 11         | SDA              |  PA08  |  C15   |                 |   NMI  | *16 |     | X00 |     |  *0/00  |   2/00  | TCC0/0 | TCC1/2 | I2S/SD1  |          |                                                           |
  | 12         | SCL              |  PA09  |  B16   |                 |   09   | *17 |     | X01 |     |  *0/01  |   2/01  | TCC0/1 | TCC1/3 | I2S/MCK0 |          |                                                           |
  +------------+------------------+--------+--------+-----------------+--------+-----+-----+-----+-----+---------+---------+--------+--------+----------+----------+-----------------------------------------------------------+
  |            |      Serial1     |        |        |                 |        |     |     |     |     |         |         |        |        |          |          |                                                           |
  | 13         | RX               |  PB23  |  C11   |                 |   07   |     |     |     |     |         |  *5/03  |        |        |          | GCLK_IO1 |                                                           |
  | 14         | TX               |  PB22  |  A13   |                 |   06   |     |     |     |     |         |  *5/02  |        |        |          | GCLK_IO0 |                                                           |
  +------------+------------------+--------+--------+-----------------+--------+-----+-----+-----+-----+---------+---------+--------+--------+----------+----------+-----------------------------------------------------------+
  | 15         | A0 / DAC0        |  PA02  |  C2    |                 |   02   | *00 |     | Y00 | OUT |         |         |        |        |          |          |                                                           |
  | 16         | A1               |  PB02  |  C3    |                 |  *02   | *10 |     | Y08 |     |         |   5/00  |        |        |          |          |                                                           |
  | 17         | A2               |  PB03  |  C6    |                 |  *03   | *11 |     | Y09 |     |         |   5/01  |        |        |          |          |                                                           |
  | 18         | A3               |  PA04  |  D1    |                 |   04   | *04 |  00 | Y02 |     |         |   0/00  |*TCC0/0 |        |          |          |                                                           |
  | 19         | A4               |  PA05  |  D3    |                 |   05   | *05 |  01 | Y03 |     |         |   0/01  |*TCC0/1 |        |          |          |                                                           |
  | 20         | A5               |  PA06  |  F3    |                 |   06   | *06 |  02 | Y04 |     |         |   0/02  | TCC1/0 |        |          |          |                                                           |
  | 21         | A6               |  PA07  |  G2    |                 |   07   | *07 |  03 | Y05 |     |         |   0/03  | TCC1/1 |        | I2S/SD0  |          |                                                           |
  +------------+------------------+--------+--------+-----------------+--------+-----+-----+-----+-----+---------+---------+--------+--------+----------+----------+-----------------------------------------------------------+
  |            |       USB        |        |        |                 |        |     |     |     |     |         |         |        |        |          |          |                                                           |
  | 22         |                  |  PA24  |        | USB N           |   12   |     |     |     |     |   3/02  |   5/02  |  TC5/0 | TCC1/2 | USB/DM   |          | -> Serial Rx/Tx via USB                                   |
  | 23         |                  |  PA25  |        | USB P           |   13   |     |     |     |     |   3/03  |   5/03  |  TC5/1 | TCC1/3 | USB/DP   |          | -> Serial Rx/Tx via USB                                   |
  | 24         |                  |  PA18  |        | USB ID          |   02   |     |     | X06 |     |   1/02  |   3/02  |  TC3/0 | TCC0/2 |          | AC/CMP0  |                                                           |
  +------------+------------------+--------+--------+-----------------+--------+-----+-----+-----+-----+---------+---------+--------+--------+----------+----------+-----------------------------------------------------------+
  | 25         | AREF             |  PA03  |  B1    |                 |   03   |  01 |     | Y01 |     |         |         |        |        |          |          |                                                           |
  +------------+------------------+--------+--------+-----------------+--------+-----+-----+-----+-----+---------+---------+--------+--------+----------+----------+-----------------------------------------------------------+
  |            | FPGA             |        |        |                 |        |     |     |     |     |         |         |        |        |          |          |                                                           |
  | 26         |                  |  PA12  |  H4    | FPGA TDI        |   12   |     |     |     |     |  *2/00  |   4/00  | TCC2/0 | TCC0/6 |          | AC/CMP0  | -> [JTAG]                                                 |
  | 27         |                  |  PA13  |  H3    | FPGA TCK        |   13   |     |     |     |     |  *2/01  |   4/01  | TCC2/1 | TCC0/7 |          | AC/CMP1  | -> [JTAG]                                                 |
  | 28         |                  |  PA14  |  J5    | FPGA TMS        |   14   |     |     |     |     |   2/02  |   4/02  |  TC3/0 | TCC0/4 |          | GCLK_IO0 | -> [JTAG]                                                 |
  | 29         |                  |  PA15  |  J4    | FPGA TDO        |   15   |     |     |     |     |  *2/03  |   4/03  |  TC3/1 | TCC0/5 |          | GCLK_IO1 | -> [JTAG]                                                 |
  | 30         |                  |  PA27  |  E2    | FPGA CLK        |   15   |     |     |     |     |         |         |        |        |          | GCLK_IO0 | -> 48 MHz Clock -> FPGA                                   |
  +------------+------------------+--------+--------+-----------------+--------+-----+-----+-----+-----+---------+---------+--------+--------+----------+----------+-----------------------------------------------------------+
  | 31         |                  |  PA28  |  L16   | FPGA MB INT     |   08   |     |     |     |     |         |         |        |        |          | GCLK_IO0 | -> NINA RESET_N                                           |
  | 32         |                  |  PB08  |        | LED_RED_BUILTIN |   08   |  02 |     | Y14 |     |         |   4/00  |  TC4/0 |        |          |          | -> LED                                                    |
  | 33         |                  |  PB09  |        | SAM_INT_OUT     |  *09   |  03 |     | Y15 |     |         |   4/01  |  TC4/1 |        |          |          |                                                           |
  +------------+------------------+--------+--------+-----------------+--------+-----+-----+-----+-----+---------+---------+--------+--------+----------+----------+-----------------------------------------------------------+
  |            | 32768Hz Crystal  |        |        |                 |        |     |     |     |     |         |         |        |        |          |          |                                                           |
  | 34         |                  |  PA00  |        | XIN32           |   00   |     |     |     |     |         |   1/00  | TCC2/0 |        |          |          |                                                           |
  | 35         |                  |  PA01  |        | XOUT32          |   01   |     |     |     |     |         |   1/01  | TCC2/1 |        |          |          |                                                           |
  +------------+------------------+--------+--------+-----------------+--------+-----+-----+-----+-----+---------+---------+--------+--------+----------+----------+-----------------------------------------------------------+
*/

#include "SAMD.h"

void ticker50msFcn() {
  ledCounter -= 1;
  if (ledCounter == 0) {
    ledCounter = 10; // preload to 500ms
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
  }
}
Ticker ticker50ms(ticker50msFcn, 50, 0, MILLIS);

void ticker85msFcn() {
  wingAliveCounter--;
  if (wingAliveCounter == 0) {
    wingAliveCounter = 59; // preload to 5 seconds

    wingAliveCommand();
  }

  if (wingStartupCounter > 0) {
    wingStartupCounter--;

    if (wingStartupCounter == 0) {
      // tell the WING, that we have a card
      wingAnnounceCardPart1();
    }
  }

  if (wingStartupCounterB > 0) {
    wingStartupCounterB--;

    if (wingStartupCounterB == 0) {
      // close announcement
      wingAnnounceCardPart2();
    }
  }

  if (wingPlayback) {
    // send current sample-position every 85ms if playing
    uint32_t wingPlaybackPosition = 4.2 * 48000.0f;//playerinfo.duration * playerinfo.progress * (48000.0f/100.0f); // calculate sample-position based on 48kHz, duration and current process
    wingSend("*1s1    *", 0xb5, true, 2);
    spiTxBuffer[6] = wingPlaybackPosition & 0xFF; // LSB
    spiTxBuffer[7] = (wingPlaybackPosition >> 8) & 0xFF;
    spiTxBuffer[8] = (wingPlaybackPosition >> 16) & 0xFF;;
    spiTxBuffer[9] = (wingPlaybackPosition >> 24) & 0xFF;; // MSB
  }
}
Ticker ticker85ms(ticker85msFcn, 85, 0, MILLIS);


void setup() {
  Serial.begin(115200);
  
  wingInit();
}

void loop() {
  wingHandleCommunication();
}