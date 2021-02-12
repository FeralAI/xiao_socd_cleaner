/*
  IOBUS.h - Single Cycle IOBUS Library for Microchip ATSAMD21 (Cortex®-M0+)

  Copyright (c) 2020 Sasapea's Lab. All right reserved.

  [i/o pin number mapping]

    PA00 - PA31 ...  0 - 31 or IOBUS_PIN('A', 0) - IOBUS_PIN('A', 31)
    PB00 - PB31 ... 32 - 63 or IOBUS_PIN('B', 0) - IOBUS_PIN('B', 31)

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/
#ifndef __IOBUS_H
#define __IOBUS_H

#ifdef ARDUINO
  #include "Arduino.h"
#else
  #include <stdint.h>
  #include <stdbool.h>
  #include <sam.h>
  #define LOW             0
  #define HIGH            1
  #define INPUT           0
  #define OUTPUT          1
  #define INPUT_PULLUP    2
  #define INPUT_PULLDOWN  3
#endif

#define DISABLE -1 // GPIO disable mode

#define IOBUS_PID(port)      ((port) - 'A')
#define IOBUS_PIN(port, pos) ((IOBUS_PID(port) << 5) | (pos & 31))

#define IOBUS_PORTID(pin)    ((pin) >> 5)
#define IOBUS_PINPOS(pin)    ((pin) & 31)
#define IOBUS_PINVAL(pin)    (1 << IOBUS_PINPOS(pin))

//
// Port Define for Seeeduino XIAO
//
#ifdef SEEED_XIAO_M0
  #define IOBUS_PIN_13_LED IOBUS_PIN('A', 17) // YELLOW LED
  #define IOBUS_PIN_RX_LED IOBUS_PIN('A', 18) // BLUE   LED
  #define IOBUS_PIN_TX_LED IOBUS_PIN('A', 19) // BLUE   LED
  #define IOBUS_PIN_D0     IOBUS_PIN('A',  2) // INT/DAC
  #define IOBUS_PIN_D1     IOBUS_PIN('A',  4) // INT/AC0
  #define IOBUS_PIN_D2     IOBUS_PIN('A', 10) // INT/GCLK
  #define IOBUS_PIN_D3     IOBUS_PIN('A', 11) // INT/GCLK
  #define IOBUS_PIN_D4     IOBUS_PIN('A',  8) // NMI/SDA
  #define IOBUS_PIN_D5     IOBUS_PIN('A',  9) // INT/SCL
  #define IOBUS_PIN_D6     IOBUS_PIN('B',  8) // INT/TX
  #define IOBUS_PIN_D7     IOBUS_PIN('B',  9) // INT/RX
  #define IOBUS_PIN_D8     IOBUS_PIN('A',  7) // INT/AC3/SCK
  #define IOBUS_PIN_D9     IOBUS_PIN('A',  5) // INT/AC1/MISO
  #define IOBUS_PIN_D10    IOBUS_PIN('A',  6) // INT/AC2/MOSI
  #define IOBUS_PIN_A0     IOBUS_PIN_D0
  #define IOBUS_PIN_A1     IOBUS_PIN_D1
  #define IOBUS_PIN_A2     IOBUS_PIN_D2
  #define IOBUS_PIN_A3     IOBUS_PIN_D3
  #define IOBUS_PIN_A4     IOBUS_PIN_D4
  #define IOBUS_PIN_A5     IOBUS_PIN_D5
  #define IOBUS_PIN_A6     IOBUS_PIN_D6
  #define IOBUS_PIN_A7     IOBUS_PIN_D7
  #define IOBUS_PIN_A8     IOBUS_PIN_D8
  #define IOBUS_PIN_A9     IOBUS_PIN_D9
  #define IOBUS_PIN_A10    IOBUS_PIN_D10
  #define IOBUS_PIN_DAC    IOBUS_PIN_D0
  #define IOBUS_PIN_SDA    IOBUS_PIN_D4
  #define IOBUS_PIN_SCL    IOBUS_PIN_D5
  #define IOBUS_PIN_TX     IOBUS_PIN_D6
  #define IOBUS_PIN_RX     IOBUS_PIN_D7
  #define IOBUS_PIN_SCK    IOBUS_PIN_D8
  #define IOBUS_PIN_MISO   IOBUS_PIN_D9
  #define IOBUS_PIN_MOSI   IOBUS_PIN_D10
#endif

typedef enum {
  IOBUS_PMUX_A = PORT_PMUX_PMUXE_A_Val,
  IOBUS_PMUX_B = PORT_PMUX_PMUXE_B_Val,
  IOBUS_PMUX_C = PORT_PMUX_PMUXE_C_Val,
  IOBUS_PMUX_D = PORT_PMUX_PMUXE_D_Val,
  IOBUS_PMUX_E = PORT_PMUX_PMUXE_E_Val,
  IOBUS_PMUX_F = PORT_PMUX_PMUXE_F_Val,
  IOBUS_PMUX_G = PORT_PMUX_PMUXE_G_Val,
  IOBUS_PMUX_H = PORT_PMUX_PMUXE_H_Val,
  IOBUS_PMUX_DISABLE = PORT_PMUX_PMUXE_H_Val + 1,
} IOBUS_PMUX;

#define IOBUS_PGROUP(pin) PORT_IOBUS->Group[(pin) >> 5]
#define IOBUS_DIRSET(pin) IOBUS_PGROUP(pin).DIRSET.reg
#define IOBUS_DIRCLR(pin) IOBUS_PGROUP(pin).DIRCLR.reg
#define IOBUS_DIRTGL(pin) IOBUS_PGROUP(pin).DIRTGL.reg
#define IOBUS_OUTSET(pin) IOBUS_PGROUP(pin).OUTSET.reg
#define IOBUS_OUTCLR(pin) IOBUS_PGROUP(pin).OUTCLR.reg
#define IOBUS_OUTTGL(pin) IOBUS_PGROUP(pin).OUTTGL.reg
#define IOBUS_IN(pin)     IOBUS_PGROUP(pin).IN.reg

class IOBUS {
  public:

    static void pinMode(uint32_t pin, uint32_t mode, bool drvstr = false) {
      PortGroup *PG = &IOBUS_PGROUP(pin);
      uint32_t cfg = IOBUS_PINPOS(pin);
      pin = IOBUS_PINVAL(pin);
      switch(mode) {
        case INPUT:
          PG->PINCFG[cfg].reg = (uint8_t)(PORT_PINCFG_INEN);
          PG->DIRCLR.reg = pin;
          PG->CTRL.reg  |= pin;
          break ;
        case INPUT_PULLUP:
          PG->PINCFG[cfg].reg = (uint8_t)(PORT_PINCFG_INEN | PORT_PINCFG_PULLEN);
          PG->DIRCLR.reg = pin;
          PG->OUTSET.reg = pin;
          PG->CTRL.reg  |= pin;
          break ;
        case INPUT_PULLDOWN:
          PG->PINCFG[cfg].reg = (uint8_t)(PORT_PINCFG_INEN | PORT_PINCFG_PULLEN);
          PG->DIRCLR.reg = pin;
          PG->OUTCLR.reg = pin;
          PG->CTRL.reg  |= pin;
          break ;
        case OUTPUT:
          PG->PINCFG[cfg].reg = (uint8_t)(drvstr ? PORT_PINCFG_DRVSTR | PORT_PINCFG_INEN : PORT_PINCFG_INEN);
          PG->DIRSET.reg = pin;
          PG->CTRL.reg  |= pin;
          break ;
        default:
          PG->PINCFG[cfg].reg = 0;
          PG->DIRCLR.reg = pin;
          PG->CTRL.reg  &= ~pin;
          break ;
      }
    }

    static void digitalWrite(uint32_t pin, uint32_t val) {
      if (val)
        IOBUS_OUTSET(pin) = IOBUS_PINVAL(pin);
      else
        IOBUS_OUTCLR(pin) = IOBUS_PINVAL(pin);
    }

    static int digitalRead(uint32_t pin) {
      return (IOBUS_IN(pin) >> IOBUS_PINPOS(pin)) & 1;
    }

    static void toggleOutput(uint32_t pin) {
      IOBUS_OUTTGL(pin) = IOBUS_PINVAL(pin);
    }

    static void toggleDirection(uint32_t pin) {
      IOBUS_DIRTGL(pin) = IOBUS_PINVAL(pin);
    }

    static void multiplexing(uint32_t pin, IOBUS_PMUX mux) {
      PortGroup *PG = &IOBUS_PGROUP(pin);
      uint32_t cfg = IOBUS_PINPOS(pin);
      if (mux == IOBUS_PMUX_DISABLE) {
        PG->PINCFG[cfg].bit.PMUXEN = 0;
      } else {
        PG->PMUX[cfg >> 1].reg = (PG->PMUX[cfg >> 1].reg & (pin & 1 ? 0x0F : 0xF0)) | (pin & 1 ? mux << 4 : mux);
        PG->PINCFG[cfg].bit.PMUXEN = 1;
      }
    }
};

#endif