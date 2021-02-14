/**
 * Seeeduino XIAO SOCD Cleaner
 * 
 * | SOCD Method           | Loop Minimum        | Loop Maximum        |
 * | --------------------- | ------------------- | ------------------- |
 * | Neutral               |  27 cycles / 0.56μs |  74 cycles / 1.54μs |
 * | Up Priority           |  30 cycles / 0.62μs |  78 cycles / 1.63μs |
 * | Second Input Priority |  35 cycles / 0.73μs | 106 cycles / 2.21μs |
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
#define INVERT_OUTPUT_LOGIC

/**
 *  End User Defines
 */

#include "IOBUS.h"
#include "IODEF.h"

#ifdef DEBUG
// Timer variables
volatile uint16_t overflowCount = 0;
#endif

// Loop variables
uint32_t maskedInput = 0;
uint32_t outputState = 0;
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
  // Current loop time is around 4μs max
  overflowCount = 0;
  TC5->COUNT16.COUNT.reg = 0;
#endif
  // Filter to just the inputs we care about from the port
  maskedInput = PORT_IOBUS->Group[0].IN.reg ^ InputMasks::maskUDLR;

  if (maskedInput) {
    outputState = 0 | (((maskedInput >> INPUT_PORT_PIN_UP) & 1) << OUTPUT_PORT_PIN_UP)
                    | (((maskedInput >> INPUT_PORT_PIN_DOWN) & 1) << OUTPUT_PORT_PIN_DOWN)
                    | (((maskedInput >> INPUT_PORT_PIN_LEFT) & 1) << OUTPUT_PORT_PIN_LEFT)
                    | (((maskedInput >> INPUT_PORT_PIN_RIGHT) & 1) << OUTPUT_PORT_PIN_RIGHT);

    // Perform SOCD cleaning
    if ((outputState & InputMasks::valueUD) == InputMasks::valueUD) { // Has vertical SOCD?
#if SOCD_METHOD == 1
      outputState &= ~InputMasks::valueD;
#elif SOCD_METHOD == 2
      outputState &= ~((lastDirectionUD == Direction::up) ? InputMasks::valueU : InputMasks::valueD);
#else
      outputState &= ~InputMasks::valueUD;
#endif
    }
#if SOCD_METHOD == 2 
    else lastDirectionUD = (outputState & InputMasks::valueU) ? Direction::up : Direction::down;
#endif

    if ((outputState & InputMasks::valueLR) == InputMasks::valueLR) { // Has horizontal SOCD?
#if SOCD_METHOD == 2
      outputState &= ~((lastDirectionLR == Direction::left) ? InputMasks::valueL : InputMasks::valueR);
#else
      outputState &= ~InputMasks::valueLR;
#endif
    }
#if SOCD_METHOD == 2 
    else lastDirectionLR = (outputState & InputMasks::valueL) ? Direction::left : Direction::right;
#endif
  } else {
    outputState;
  }

  // Set output state - 14 cycles / 0.29μs
#ifdef INVERT_OUTPUT_LOGIC
  PORT_IOBUS->Group[0].OUTCLR.reg = 0 | (outputState ^ InputMasks::valueUDLR);
  PORT_IOBUS->Group[0].OUTSET.reg = 0 | outputState;
#else
  PORT_IOBUS->Group[0].OUTCLR.reg = 0 | outputState;
  PORT_IOBUS->Group[0].OUTSET.reg = 0 | (outputState ^ InputMasks::valueUDLR);
#endif

#ifdef DEBUG
  // Log timing
  // -2 for timer reset and stop
  uint16_t totalCycleCount = TC5->COUNT16.COUNT.reg - 2;
  Serial.print(totalCycleCount);
  Serial.print(" cycles / ");
  Serial.print(1 / 48e6 * totalCycleCount * 1e6);
  Serial.println("μs");
  Serial.print("maskedInput: ");
  Serial.println(maskedInput, BIN);
  Serial.print("outputState: ");
  Serial.println(outputState, BIN);
  Serial.print("PORTA: ");
  Serial.println(PORT_IOBUS->Group[0].OUTCLR.reg, BIN);
  // Delay a bit so we can read the logged output
  delay(100);
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
    TC_CTRLA_MODE_COUNT16 |       // Counter of 16 bits
    TC_CTRLA_WAVEGEN_MFRQ |       // Match frequency
    TC_CTRLA_PRESCALER_DIV1;      // Prescaler of 1 (no division), 1 / 48000000 = 0.0000000208333 = 20.833ns/cycle | ~1.365ms window
  TC5->COUNT16.CC[0].reg = 65355; // uint16_t max value - 1
  while (TC5->COUNT16.STATUS.bit.SYNCBUSY);

  // Configure interrupt
  // NVIC_DisableIRQ(TC5_IRQn);
  // NVIC_ClearPendingIRQ(TC5_IRQn);
  // NVIC_SetPriority(TC5_IRQn, 0);
  // NVIC_EnableIRQ(TC5_IRQn);
  // TC5->COUNT16.INTENSET.bit.MC0 = 1;          // Enable the TC5 Match/Compare 0 interrupt
  // while (TC5->COUNT16.STATUS.bit.SYNCBUSY);

  // Start counter
  TC5->COUNT16.CTRLA.reg |= TC_CTRLA_ENABLE;  // Enable TC5
  while (TC5->COUNT16.STATUS.bit.SYNCBUSY);
}

// void TC5_Handler(void) {
//   overflowCount++;
//   TC5->COUNT16.INTFLAG.bit.MC0 = 1; // Clear the interrupt flag
// }
#endif