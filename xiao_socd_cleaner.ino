/**
 * Seeeduino XIAO SOCD Cleaner
 * 
 * Current loop times:
 * 
 * Neutral - min: 99 cycles / 2.06μs, max: 181 cycles / 3.77μs
 * Up Priority - min: 99 cycles / 2.06μs, max: 185 cycles / 3.85μs
 * Second Input Priority: min: 131 cycles / 2.73μs, max: 182 cycles / 3.79μs
 */

#include "IOBUS.h"

/**
 *  User Defines
 */

// Uncomment this define to show loop time and input/output states via serial monitoring
#define DEBUG

// Set your selected SOCD cleaning method here: 0 - Neutral, 1 - Up Priority, 2 - Second Input Priority
#define SOCD_METHOD 2

// Most controllers will use logic level LOW to trigger an input, however if you've separated your circuits with
// an optocoupler you will likely want to invert the output logic and trigger outputs with a HIGH value instead.
// Uncomment this define to invert the output logic.
// #define INVERT_OUTPUT_LOGIC

/**
 *  End User Defines
 */

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
uint32_t inputState = 0;
uint32_t outClrState = 0;
uint32_t outSetState = 0;
Direction lastDirectionUD = Direction::neutral;
Direction lastDirectionLR = Direction::neutral;


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
  // Current loop time is around 5μs max
  overflowCount = 0;
  TC5->COUNT16.COUNT.reg = 0;
#endif

  // Read current inputs and run SOCD algorithm
  // 8 cycles / 0.167μs for the input register read + 4 cycles / 0.083μs to set var
  inputState = PORT_IOBUS->Group[0].IN.reg;
  // These are 4 cycles / 0.083μs each, or when run sequentially come to 7 cycles / 0.146μs
  outClrState = 0;
  outSetState = 0;

#if SOCD_METHOD == 0 // Neutral
  // This method will set each axis to neutral when SOCD are pressed
  // 145 cycles / 3.021μs with no inputs
  if ((inputState & maskUD) == 0) {
    outSetState = outSetState | outUpValue | outDownValue;
  } else {
    outClrState = outClrState | ((inputState & maskU) == 0 ? outUpValue : 0) | ((inputState & maskD) == 0 ? outDownValue : 0);
    outSetState = outSetState | ((inputState & maskU) == 0 ? 0 : outUpValue) | ((inputState & maskD) == 0 ? 0 : outDownValue);
  }

  if ((inputState & maskLR) == 0) {
    outSetState = outSetState | outLeftValue | outRightValue;
  } else {
    outClrState = outClrState | ((inputState & maskL) == 0 ? outLeftValue : 0) | ((inputState & maskR) == 0 ? outRightValue : 0);
    outSetState = outSetState | ((inputState & maskL) == 0 ? 0 : outLeftValue) | ((inputState & maskR) == 0 ? 0 : outRightValue);
  }
#elif SOCD_METHOD == 1 // Up Priority
  // This method will set the horizontal axis to neutral and priortize Up for the vertical when SOCD are pressed
  // 149 cycles / 3.104μs with no inputs
  if ((inputState & maskUD) == 0) {
    outClrState = outClrState | outUpValue;
    outSetState = outSetState | outDownValue;
  } else {
    outClrState = outClrState | ((inputState & maskU) == 0 ? outUpValue : 0) | ((inputState & maskD) == 0 ? outDownValue : 0);
    outSetState = outSetState | ((inputState & maskU) == 0 ? 0 : outUpValue) | ((inputState & maskD) == 0 ? 0 : outDownValue);
  }

  if ((inputState & maskLR) == 0) {
    outSetState = outSetState | outLeftValue | outRightValue;
  } else {
    outClrState = outClrState | ((inputState & maskL) == 0 ? outLeftValue : 0) | ((inputState & maskR) == 0 ? outRightValue : 0);
    outSetState = outSetState | ((inputState & maskL) == 0 ? 0 : outLeftValue) | ((inputState & maskR) == 0 ? 0 : outRightValue);
  }
#elif SOCD_METHOD == 2 // Second Input Priority (Last Win)
  // This method tracks the last input for each axis to allow the latest input for that axis to pass through
  // 149 cycles / 3.104μs with no inputs (max run time)
  if ((inputState & maskUD) == 0) {
    switch (lastDirectionUD) {
      case Direction::up:
        outClrState = outClrState | outDownValue;
        outSetState = outSetState | outUpValue;
        break;
      case Direction::down:
        outClrState = outClrState | outUpValue;
        outSetState = outSetState | outDownValue;
        break;
      case Direction::neutral:
        outSetState = outSetState | outUpValue | outDownValue;
        break;
    }
  } else if ((inputState & maskU) == 0) {
    lastDirectionUD = Direction::up;
    outClrState = outClrState | outUpValue;
    outSetState = outSetState | outDownValue;
  } else if ((inputState & maskD) == 0) {
    lastDirectionUD = Direction::down;
    outClrState = outClrState | outDownValue;
    outSetState = outSetState | outUpValue;
  } else {
    lastDirectionUD = Direction::neutral;
    outSetState = outSetState | outUpValue | outDownValue;
  }

  if ((inputState & maskLR) == 0) {
    switch (lastDirectionLR) {
      case Direction::left:
        outClrState = outClrState | outRightValue;
        outSetState = outSetState | outLeftValue;
        break;
      case Direction::right:
        outClrState = outClrState | outLeftValue;
        outSetState = outSetState | outRightValue;
        break;
      case Direction::neutral:
        outSetState = outSetState | outLeftValue | outRightValue;
        break;
    }
  } else if ((inputState & maskL) == 0) {
    lastDirectionLR = Direction::left;
    outClrState = outClrState | outLeftValue;
    outSetState = outSetState | outRightValue;
  } else if ((inputState & maskR) == 0) {
    lastDirectionLR = Direction::right;
    outClrState = outClrState | outRightValue;
    outSetState = outSetState | outLeftValue;
  } else {
    lastDirectionLR = Direction::neutral;
    outSetState = outSetState | outLeftValue | outRightValue;
  }
#endif

  // Set output state
  // 14 cycles / 0.29μs
#ifdef INVERT_OUTPUT_LOGIC
  PORT_IOBUS->Group[0].OUTCLR.reg = outSetState;
  PORT_IOBUS->Group[0].OUTSET.reg = outClrState;
#else
  PORT_IOBUS->Group[0].OUTCLR.reg = outClrState;
  PORT_IOBUS->Group[0].OUTSET.reg = outSetState;
#endif

#ifdef DEBUG
  // Log timing
  totalCycleCount = TC5->COUNT16.COUNT.reg + (overflowCount * 65535) - 1;
  Serial.print("Loop cycle count: ");
  Serial.println(totalCycleCount);
  Serial.print("Loop time (us): ");
  Serial.println(1 / 48e6 * totalCycleCount * 1e6);
  // Serial.print("Parsed outputs: ");
  // Serial.print(outUp); Serial.print(outDown); Serial.print(outLeft); Serial.println(outRight);
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
  delay(100);
#endif
}

void configurePins() {
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

void TC5_Handler(void) {
  overflowCount++;
  TC5->COUNT16.INTFLAG.bit.MC0 = 1; // Clear the interrupt flag
}
#endif