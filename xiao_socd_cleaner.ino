/**
 * Seeeduino XIAO SOCD Cleaner
 * 
 * | SOCD Method           | Loop Minimum        | Loop Maximum        |
 * | --------------------- | ------------------- | ------------------- |
 * | Neutral               |  12 cycles / 0.25μs |  45 cycles / 0.94μs |
 * | Up Priority           |  12 cycles / 0.25μs |  46 cycles / 0.96μs |
 * | Second Input Priority |  12 cycles / 0.25μs |  62 cycles / 1.29μs |
 * 
 */

/**
 *  User Defines
 */

// Uncomment this define to show loop time and input/output states via serial monitoring
// #define DEBUG

// Set your selected SOCD cleaning method here: 0 - Neutral, 1 - Up Priority, 2 - Second Input Priority
#define SOCD_METHOD 2

// Set the version of the prototype board being used - this controls the pin definitions in PINDEF.h
#define PROTOTYPE_VERSION 2

// Most controllers will use logic level LOW to trigger an input, however if you've separated your circuits with
// an optocoupler you will likely want to invert the output logic and trigger outputs with a HIGH value instead.
// Uncomment this define to invert the output logic.
// #define INVERT_OUTPUT_LOGIC

/**
 *  End User Defines
 */

#include "IOBUS.h"
#include "IODEF.h"

#define COMPARE_VALUE 65535

// Loop variables
#if SOCD_METHOD == 2
Direction lastDirectionUD = Direction::neutral;
Direction lastDirectionLR = Direction::neutral;
#endif

void setup() {
#ifdef DEBUG
  Serial.begin(115200);
#endif
  configurePins();
#ifdef DEBUG
  configureTimer();
  Serial.println("Setup complete, begin SOCD algorithm");
#endif
}

void loop() {
#ifdef DEBUG
  TC5->COUNT16.COUNT.reg = 0;
#endif
  // Filter to just the inputs we care about from the port
  uint32_t maskedInput = PORT_IOBUS->Group[0].IN.reg ^ InputMasks::maskUDLR;
  uint32_t outputState = 0;

  if (maskedInput) {
    switch (maskedInput & InputMasks::maskUD) {
      case InputMasks::maskU:
        outputState |= InputMasks::valueU;
#if SOCD_METHOD == 2
        lastDirectionUD = Direction::up;
#endif
        break;
      case InputMasks::maskD:
        outputState |= InputMasks::valueD;
#if SOCD_METHOD == 2
        lastDirectionUD = Direction::down;
#endif
        break;
      case InputMasks::maskUD:
#if SOCD_METHOD == 1
        outputState |= InputMasks::valueU;
#elif SOCD_METHOD == 2
        outputState |= (lastDirectionUD == Direction::up) ? InputMasks::valueD : InputMasks::valueU;
#endif
        break;
    }

    switch (maskedInput & InputMasks::maskLR) {
      case InputMasks::maskL:
        outputState |= InputMasks::valueL;
#if SOCD_METHOD == 2
        lastDirectionLR = Direction::left;
#endif
        break;
      case InputMasks::maskR:
        outputState |= InputMasks::valueR;
#if SOCD_METHOD == 2
        lastDirectionLR = Direction::right;
#endif
        break;
#if SOCD_METHOD == 2
      case InputMasks::maskLR:
        outputState |= (lastDirectionLR == Direction::left) ? InputMasks::valueR : InputMasks::valueL;
        break;
#endif
    }
  }

#ifdef INVERT_OUTPUT_LOGIC
  PORT_IOBUS->Group[0].OUTCLR.reg = 0 | (outputState ^ InputMasks::valueUDLR);
  PORT_IOBUS->Group[0].OUTSET.reg = 0 | outputState;
#else
  PORT_IOBUS->Group[0].OUTCLR.reg = 0 | outputState;
  PORT_IOBUS->Group[0].OUTSET.reg = 0 | (outputState ^ InputMasks::valueUDLR);
#endif

#ifdef DEBUG
  // Log timing
  // Takes 8 cycles to reset timer
  uint16_t totalCycleCount = TC5->COUNT16.COUNT.reg - 8;
  double duration = 1 / 48e6 * totalCycleCount * 1e6;
  Serial.print(totalCycleCount);
  Serial.print(" cycles / ");
  Serial.print(duration);
  Serial.println("μs");
  // Serial.print("maskedInput: ");
  // Serial.println(maskedInput, BIN);
  // Serial.print("outputState: ");
  // Serial.println(outputState, BIN);
#endif
}

void configurePins() {
  IOBUS::pinMode(INPUT_IOPIN_UP, INPUT_PULLUP);
  IOBUS::pinMode(INPUT_IOPIN_DOWN, INPUT_PULLUP);
  IOBUS::pinMode(INPUT_IOPIN_LEFT, INPUT_PULLUP);
  IOBUS::pinMode(INPUT_IOPIN_RIGHT, INPUT_PULLUP);
  IOBUS::pinMode(OUTPUT_IOPIN_UP, OUTPUT, true);
  IOBUS::pinMode(OUTPUT_IOPIN_DOWN, OUTPUT, true);
  IOBUS::pinMode(OUTPUT_IOPIN_LEFT, OUTPUT, true);
  IOBUS::pinMode(OUTPUT_IOPIN_RIGHT, OUTPUT, true);
  IOBUS::digitalWrite(OUTPUT_IOPIN_UP, LOW);
  IOBUS::digitalWrite(OUTPUT_IOPIN_DOWN, LOW);
  IOBUS::digitalWrite(OUTPUT_IOPIN_LEFT, LOW);
  IOBUS::digitalWrite(OUTPUT_IOPIN_RIGHT, LOW);
}

#ifdef DEBUG
void configureTimer() {
  // Enable generic clock for Timer/Counter 4 and 5
  GCLK->CLKCTRL.reg = GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK0 | GCLK_CLKCTRL_ID(GCM_TC4_TC5);
  while (GCLK->STATUS.bit.SYNCBUSY);

  // Perform software reset
  TC5->COUNT16.CTRLA.reg = TC_CTRLA_SWRST;
  while (TC5->COUNT16.STATUS.reg & TC_STATUS_SYNCBUSY);
  while (TC5->COUNT16.CTRLA.bit.SWRST);

  // Configure TC5
  TC5->COUNT16.CTRLA.reg =
    TC_CTRLA_MODE_COUNT16 |               // Counter of 16 bits
    TC_CTRLA_WAVEGEN_MFRQ |               // Match frequency
    TC_CTRLA_PRESCALER_DIV1;              // Prescaler of 1 (no division), 1 / 48000000 = 0.0000000208333 = 20.833ns/cycle | ~1.365ms window
  TC5->COUNT16.CC[0].reg = COMPARE_VALUE; // uint16_t max value - 1
  while (TC5->COUNT16.STATUS.bit.SYNCBUSY);

  // Start counter
  TC5->COUNT16.CTRLA.reg |= TC_CTRLA_ENABLE; // Enable TC5
  while (TC5->COUNT16.STATUS.bit.SYNCBUSY);
}
#endif