/**
 * Seeeduino XIAO SOCD Cleaner
 * 
 * The SOCD algorithms take up to 80 cycles / 1.67μs to run. This can be reduced by removing the if statements and compiling
 * for a specific SOCD method.
 * 
 * DIP 1 sets the base operating mode between Neutral (OFF) and Second Input Priority (ON).
 * DIP 2 set to ON will make Up take priority.
 * 
 */

/**
 *  User Defines
 */

// Uncomment this define to show loop time and input/output states via serial monitoring
// #define DEBUG

// Set the version of the prototype board being used - this controls the pin definitions in PINDEF.h
#define PROTOTYPE_VERSION 2

// Most controllers will use logic level LOW to trigger an input, however if you've separated your circuits with
// an optocoupler you will likely want to invert the output logic and trigger outputs with a HIGH value instead.
// Uncomment this define to invert the output logic.
// #define INVERT_OUTPUT_LOGIC

/**
 *  End User Defines
 */

#include "IODEF.h"

#define COMPARE_VALUE 65535

// Loop variables
Direction lastDirectionUD = Direction::neutral;
Direction lastDirectionLR = Direction::neutral;
bool secondInputPriority = false;
bool upPriority = false;

void configureSOCD() {
  uint32_t portA = PORT_IOBUS->Group[PORTA].IN.reg;
  uint32_t portB = PORT_IOBUS->Group[PORTB].IN.reg;

#if PROTOTYPE_VERSION == 1
  secondInputPriority = (portB & (1 << DIP1_PORT_PIN)) == 0;
  upPriority = (portB & (1 << DIP2_PORT_PIN)) == 0;
#else
  secondInputPriority = (portB & (1 << DIP1_PORT_PIN)) == 0;
  upPriority = (portA & (1 << DIP2_PORT_PIN)) == 0;
#endif
#ifdef DEBUG
  if (secondInputPriority)
    Serial.print("Using SIP SOCD cleaning");
  else
    Serial.print("Using Neutral SOCD cleaning");
  
  if (upPriority)
    Serial.println(" with Up Priority");
  else
    Serial.println();
#endif
}

void setup() {
#ifdef DEBUG
  Serial.begin(115200);
  while (!Serial);
#endif
  configurePins();
  configureSOCD();
#ifdef DEBUG
  configureTimer();
  Serial.println("Setup complete, begin SOCD algorithm");
#endif
}

void loop() {
#ifdef DEBUG
  TC5->COUNT16.COUNT.reg = 0;
#endif
  uint32_t inputValues = ~PORT_IOBUS->Group[PORTA].IN.reg;
  uint32_t outputState = 0;

  switch (inputValues & InputMasks::maskUD) {
    case InputMasks::maskUD:
      if (upPriority)
        outputState |= OutputValues::valueU;
      else if (secondInputPriority)
        outputState |= (lastDirectionUD == Direction::up) ? OutputValues::valueD : OutputValues::valueU;
      break;
    case InputMasks::maskU:
      outputState |= OutputValues::valueU;
      lastDirectionUD = Direction::up;
      break;
    case InputMasks::maskD:
      outputState |= OutputValues::valueD;
      lastDirectionUD = Direction::down;
      break;
  }

  switch (inputValues & InputMasks::maskLR) {
    case InputMasks::maskLR:
      if (secondInputPriority)
        outputState |= (lastDirectionLR == Direction::left) ? OutputValues::valueR : OutputValues::valueL;
      break;
    case InputMasks::maskL:
      outputState |= OutputValues::valueL;
      lastDirectionLR = Direction::left;
      break;
    case InputMasks::maskR:
      outputState |= OutputValues::valueR;
      lastDirectionLR = Direction::right;
      break;
  }

#ifdef INVERT_OUTPUT_LOGIC
  PORT_IOBUS->Group[0].OUTCLR.reg = (outputState ^ OutputValues::valueUDLR);
  PORT_IOBUS->Group[0].OUTSET.reg = outputState;
#else
  PORT_IOBUS->Group[0].OUTCLR.reg = outputState;
  PORT_IOBUS->Group[0].OUTSET.reg = (outputState ^ OutputValues::valueUDLR);
#endif

#ifdef DEBUG
  // Log timing
  // Takes 8 cycles to reset timer + 1 to capture during print
  Serial.println(TC5->COUNT16.COUNT.reg - 9);
  delay(100);
  // Serial.print("maskedInput: ");
  // Serial.println(maskedInput, BIN);
  // Serial.print("outputState: ");
  // Serial.println(outputState, BIN);
#endif
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