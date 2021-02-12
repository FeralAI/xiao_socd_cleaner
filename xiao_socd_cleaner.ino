/**
 * Seeeduino XIAO SOCD Cleaner
 */

#include "IOBUS.h"

// Uncomment this define to show loop time and input/output states via serial monitoring
#define DEBUG

// Most controllers will use logic level LOW to trigger an input, however if you've separated your circuits with
// an optocoupler you will likely want to invert the output logic and trigger outputs with a HIGH value instead.
// Uncomment this define to invert the output logic.
// #define INVERT_OUTPUT_LOGIC

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

// Cache pin values for getting inputs and setting outputs
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

// Declare loop variables
#ifdef DEBUG
uint32_t currentTime;
uint32_t loopTime;
#endif
uint32_t inputState = 0;
uint32_t outClrState = 0;
uint32_t outSetState = 0;
uint8_t inUp; uint8_t inDown; uint8_t inLeft; uint8_t inRight;
uint8_t outUp; uint8_t outDown; uint8_t outLeft; uint8_t outRight;
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
#endif
}

void loop() {
#ifdef DEBUG
  // Current loop time is around 7-8μs
  currentTime = micros();
#endif
 
  // Read current inputs
  inputState = PORT_IOBUS->Group[0].IN.reg;
  inUp    = getIOPinValue(inputState, inUpPinPosition);
  inDown  = getIOPinValue(inputState, inDownPinPosition);
  inLeft  = getIOPinValue(inputState, inLeftPinPosition);
  inRight = getIOPinValue(inputState, inRightPinPosition);

  // Run SOCD algorithm
  runSecondInputPriority();
  // runNeutral();
  // runUpPriority();

  // Build output states
  outClrState = 0;
  outSetState = 0;

#ifdef INVERT_OUTPUT_LOGIC
  if (outUp)
    outClrState |= outUpValue;
  else
    outSetState |= outUpValue;

  if (outDown)
    outClrState |= outDownValue;
  else
    outSetState |= outDownValue;

  if (outLeft)
    outClrState |= outLeftValue;
  else
    outSetState |= outLeftValue;

  if (outRight)
    outClrState |= outRightValue;
  else
    outSetState |= outRightValue;
#else
  if (outUp)
    outSetState |= outUpValue;
  else
    outClrState |= outUpValue;

  if (outDown)
    outSetState |= outDownValue;
  else
    outClrState |= outDownValue;

  if (outLeft)
    outSetState |= outLeftValue;
  else
    outClrState |= outLeftValue;

  if (outRight)
    outSetState |= outRightValue;
  else
    outClrState |= outRightValue;
#endif
  
  // Set output state
  PORT_IOBUS->Group[0].OUTCLR.reg = outClrState;
  PORT_IOBUS->Group[0].OUTSET.reg = outSetState;

#ifdef DEBUG
  // Log timing
  loopTime = micros() - currentTime;
  Serial.print("Loop time: ");
  Serial.println(loopTime);
  Serial.print("Parsed inputs: ");
  Serial.print(inUp); Serial.print(inDown); Serial.print(inLeft); Serial.println(inRight);
  Serial.print("Parsed outputs: ");
  Serial.print(outUp); Serial.print(outDown); Serial.print(outLeft); Serial.println(outRight);
  Serial.print("inputState:  ");
  Serial.println(inputState, BIN);
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
void runNeutral() {
  if (!(inUp | inDown)) {
    outUp = 1;
    outDown = 1;
  } else {
    outUp = inUp;
    outDown = inDown;
  }

  if (!(inLeft | inRight)) {
    outLeft = 1;
    outRight = 1;
  } else {
    outLeft = inLeft;
    outRight = inRight;
  }
}

// This method will set the horizontal axis to neutral and priortize Up for the vertical when SOCD are pressed
void runUpPriority() {
  if (!(inUp | inDown)) {
    outUp = 0;
    outDown = 1;
  } else {
    outUp = inUp;
    outDown = inDown;
  }

  if (!(inLeft | inRight)) {
    outLeft = 1;
    outRight = 1;
  } else {
    outLeft = inLeft;
    outRight = inRight;
  }
}

// This method tracks the last input for each axis to allow the latest input for that axis to pass through
void runSecondInputPriority() {
  if (!(inUp | inDown)) {
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
    outUp = inUp;
    outDown = inDown;
    if (!outUp)
      lastDirectionUD = Direction::up;
    else if (!outDown)
      lastDirectionUD = Direction::down;
    else
      lastDirectionUD = Direction::neutral;
  }

  if (!(inLeft | inRight)) {
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
    outLeft = inLeft;
    outRight = inRight;
    if (!outLeft)
      lastDirectionLR = Direction::left;
    else if (!outRight)
      lastDirectionLR = Direction::right;
    else
      lastDirectionLR = Direction::neutral;
  }
}
