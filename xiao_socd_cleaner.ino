/**
 * Seeeduino XIAO SOCD Cleaner
 * 
 * Current loop times:
 * 
 * - Neutral: 219 cycles / 4.56μs
 * - Up Priority: 217 cycles / 4.52μs
 * - Second Input Priority: 239 cycles / 4.98μs
 */

#include "IOBUS.h"

// Uncomment this define to show loop time and input/output states via serial monitoring
// #define DEBUG

// Most controllers will use logic level LOW to trigger an input, however if you've separated your circuits with
// an optocoupler you will likely want to invert the output logic and trigger outputs with a HIGH value instead.
// Uncomment this define to invert the output logic.
#define INVERT_OUTPUT_LOGIC

#define digitalPinToIOPin(P) ((g_APinDescription[P].ulPort << 5) + g_APinDescription[P].ulPin)
#define getIOPinValue(state, pinPos) (state >> pinPos) & 1

// All pins defined on PORTA so they can be read and written in one operation
#define INPUT_PIN_UP     digitalPinToIOPin(4)  // PA8
#define INPUT_PIN_DOWN   digitalPinToIOPin(3)  // PA11
#define INPUT_PIN_LEFT   digitalPinToIOPin(1)  // PA4
#define INPUT_PIN_RIGHT  digitalPinToIOPin(2)  // PA10
#define OUTPUT_PIN_UP    digitalPinToIOPin(5)  // PA9
#define OUTPUT_PIN_DOWN  digitalPinToIOPin(10) // PA6
#define OUTPUT_PIN_LEFT  digitalPinToIOPin(8)  // PA7
#define OUTPUT_PIN_RIGHT digitalPinToIOPin(9)  // PA5

// Cache IOBUS pin positions so they aren't computed each loop
uint8_t inUpPinPosition     = IOBUS_PINPOS(INPUT_PIN_UP);
uint8_t inDownPinPosition   = IOBUS_PINPOS(INPUT_PIN_DOWN);
uint8_t inLeftPinPosition   = IOBUS_PINPOS(INPUT_PIN_LEFT);
uint8_t inRightPinPosition  = IOBUS_PINPOS(INPUT_PIN_RIGHT);
uint8_t outUpPinPosition    = IOBUS_PINPOS(OUTPUT_PIN_UP);
uint8_t outDownPinPosition  = IOBUS_PINPOS(OUTPUT_PIN_DOWN);
uint8_t outLeftPinPosition  = IOBUS_PINPOS(OUTPUT_PIN_LEFT);
uint8_t outRightPinPosition = IOBUS_PINPOS(OUTPUT_PIN_RIGHT);

// Create input masks and comparison values for all possible checks in SOCD methods
uint32_t maskU  = 0 | (1 << inUpPinPosition);
uint32_t maskD  = 0 | (1 << inDownPinPosition);
uint32_t maskL  = 0 | (1 << inLeftPinPosition);
uint32_t maskR  = 0 | (1 << inRightPinPosition);
uint32_t maskUD = 0 | (1 << inUpPinPosition) | (1 << inDownPinPosition);
uint32_t maskLR = 0 | (1 << inLeftPinPosition) | (1 << inRightPinPosition);

// Cache offsets for pins for comparisons
uint32_t outUpValue    = (1 << outUpPinPosition);
uint32_t outDownValue  = (1 << outDownPinPosition);
uint32_t outLeftValue  = (1 << outLeftPinPosition);
uint32_t outRightValue = (1 << outRightPinPosition);

enum class Direction {
  neutral = -1,
  up = 0,
  down = 1,
  left = 2,
  right = 3
};

#ifdef DEBUG
// Timer variables
volatile uint16_t overflowCount = 0;
uint16_t totalCycleCount = 0;
uint16_t totalOverflowCount = 0;
#endif

// Loop variables
uint32_t outClrState = 0;
uint32_t outSetState = 0;
bool outUp; bool outDown; bool outLeft; bool outRight;
Direction lastDirectionUD = Direction::neutral;
Direction lastDirectionLR = Direction::neutral;

void setup() {
#ifdef DEBUG
  Serial.begin(115200);
#endif

  // Set up the pins
  IOBUS::pinMode(INPUT_PIN_UP, INPUT_PULLUP);
  IOBUS::pinMode(INPUT_PIN_DOWN, INPUT_PULLUP);
  IOBUS::pinMode(INPUT_PIN_LEFT, INPUT_PULLUP);
  IOBUS::pinMode(INPUT_PIN_RIGHT, INPUT_PULLUP);
  IOBUS::pinMode(OUTPUT_PIN_UP, OUTPUT, true);
  IOBUS::pinMode(OUTPUT_PIN_DOWN, OUTPUT, true);
  IOBUS::pinMode(OUTPUT_PIN_LEFT, OUTPUT, true);
  IOBUS::pinMode(OUTPUT_PIN_RIGHT, OUTPUT, true);
  IOBUS::digitalWrite(OUTPUT_PIN_UP, LOW);
  IOBUS::digitalWrite(OUTPUT_PIN_DOWN, LOW);
  IOBUS::digitalWrite(OUTPUT_PIN_LEFT, LOW);
  IOBUS::digitalWrite(OUTPUT_PIN_RIGHT, LOW);

#ifdef DEBUG
  Serial.println("Setup complete, begin SOCD algorithm");
  configureTimer();
#endif
}

void loop() {
#ifdef DEBUG
  // Current loop time is around 5μs max
  overflowCount = 0;
  TC5->COUNT16.COUNT.reg = 0;
#endif

  // Read current inputs and run SOCD algorithm
  // 9 cycles / 0.19μs for the input register read
  runSecondInputPriority(PORT_IOBUS->Group[0].IN.reg);
  // runNeutral(PORT_IOBUS->Group[0].IN.reg);
  // runUpPriority(PORT_IOBUS->Group[0].IN.reg);

#ifdef INVERT_OUTPUT_LOGIC
  // Build output states
  // 93 cycles / 1.9375μs
  outClrState = 0 | (outUp ? outUpValue : 0) | (outDown ? outDownValue : 0) | (outLeft ? outLeftValue : 0) | (outRight ? outRightValue : 0);
  outSetState = 0 | (outUp ? 0 : outUpValue) | (outDown ? 0 : outDownValue) | (outLeft ? 0 : outLeftValue) | (outRight ? 0 : outRightValue);
#else
  // 93 cycles / 1.9375μs
  outClrState = 0 | (outUp ? 0 : outUpValue) | (outDown ? 0 : outDownValue) | (outLeft ? 0 : outLeftValue) | (outRight ? 0 : outRightValue);
  outSetState = 0 | (outUp ? outUpValue : 0) | (outDown ? outDownValue : 0) | (outLeft ? outLeftValue : 0) | (outRight ? outRightValue : 0);
#endif

  // Set output state
  // 14 cycles / 0.29μs
  PORT_IOBUS->Group[0].OUTCLR.reg = outClrState;
  PORT_IOBUS->Group[0].OUTSET.reg = outSetState;

#ifdef DEBUG
  // Log timing
  totalCycleCount = TC5->COUNT16.COUNT.reg + (overflowCount * 65535) - 1;
  Serial.print("Loop cycle count: ");
  Serial.println(totalCycleCount);
  Serial.print("Loop time (us): ");
  Serial.println(1 / 48e6 * totalCycleCount * 1e6);
  Serial.print("Parsed outputs: ");
  Serial.print(outUp); Serial.print(outDown); Serial.print(outLeft); Serial.println(outRight);
  Serial.print("inputState: ");
  Serial.println(PORT_IOBUS->Group[0].IN.reg, BIN);
  Serial.print("outClrState: ");
  Serial.println(outClrState, BIN);
  Serial.print("outSetState: ");
  Serial.println(outSetState, BIN);
  Serial.print("PORTA OUTCLR: ");
  Serial.println(PORT_IOBUS->Group[0].OUTCLR.reg, BIN);
  Serial.print("PORTA OUTSET: ");
  Serial.println(PORT_IOBUS->Group[0].OUTSET.reg, BIN);
#endif
}

// This method will set each axis to neutral when SOCD are pressed
// 102 cycles / 2.125μs with no inputs
void runNeutral(uint32_t inputState) {
  if ((inputState & maskUD) == 0) {
    outUp = 1;
    outDown = 1;
  } else {
    outUp = inputState & maskU;
    outDown = inputState & maskD;
  }

  if ((inputState & maskLR) == 0) {
    outLeft = 1;
    outRight = 1;
  } else {
    outLeft = inputState & maskL;
    outRight = inputState & maskR;
  }
}

// This method will set the horizontal axis to neutral and priortize Up for the vertical when SOCD are pressed
// 101 cycles / 2.1μs with no inputs
void runUpPriority(uint32_t inputState) {
  if ((inputState & maskUD) == 0) {
    outUp = 0;
    outDown = 1;
  } else {
    outUp = inputState & maskU;
    outDown = inputState & maskD;
  }

  if ((inputState & maskLR) == 0) {
    outLeft = 1;
    outRight = 1;
  } else {
    outLeft = inputState & maskL;
    outRight = inputState & maskR;
  }
}

// This method tracks the last input for each axis to allow the latest input for that axis to pass through
// 123 cycles / 2.56μs with no inputs
void runSecondInputPriority(uint32_t inputState) {
  if ((inputState & maskUD) == 0) {
    switch (lastDirectionUD) {
      case Direction::up:
        outUp = 1;
        outDown = 0;
        break;
      case Direction::down:
        outUp = 0;
        outDown = 1;
        break;
      case Direction::neutral:
        outUp = 1;
        outDown = 1;
        break;
    }
  } else {
    if ((inputState & maskU) == 0) {
      lastDirectionUD = Direction::up;
      outUp = 0;
      outDown = 1;
    } else if ((inputState & maskD) == 0) {
      lastDirectionUD = Direction::down;
      outUp = 1;
      outDown = 0;
    } else {
      lastDirectionUD = Direction::neutral;
      outUp = 1;
      outDown = 1;
    }
  }

  if ((inputState & maskLR) == 0) {
    switch (lastDirectionLR) {
      case Direction::left:
        outLeft = 1;
        outRight = 0;
        break;
      case Direction::right:
        outLeft = 0;
        outRight = 1;
        break;
      case Direction::neutral:
        outLeft = 1;
        outRight = 1;
        break;
    }
  } else {
    if ((inputState & maskL) == 0) {
      lastDirectionLR = Direction::left;
      outLeft = 0;
      outRight = 1;
    } else if ((inputState & maskR) == 0) {
      lastDirectionLR = Direction::right;
      outLeft = 1;
      outRight = 0;
    } else {
      lastDirectionLR = Direction::neutral;
      outLeft = 1;
      outRight = 1;
    }
  }
}

#ifdef DEBUG
void TC5_Handler(void) {
  overflowCount++;
  TC5->COUNT16.INTFLAG.bit.MC0 = 1; // Clear the interrupt flag
}

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
  NVIC_DisableIRQ(TC5_IRQn);
  NVIC_ClearPendingIRQ(TC5_IRQn);
  NVIC_SetPriority(TC5_IRQn, 0);
  NVIC_EnableIRQ(TC5_IRQn);
  TC5->COUNT16.INTENSET.bit.MC0 = 1;          // Enable the TC5 Match/Compare 0 interrupt
  while (TC5->COUNT16.STATUS.bit.SYNCBUSY);

  // Start counter
  TC5->COUNT16.CTRLA.reg |= TC_CTRLA_ENABLE;  // Enable TC5
  while (TC5->COUNT16.STATUS.bit.SYNCBUSY);
}
#endif