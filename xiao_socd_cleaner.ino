/**
 * Seeeduino XIAO SOCD Cleaner
 */
#include "IOBUS.h"

// Uncomment to log via Serial
// #define DEBUG

#define digitalPinToIOPin(P) ((g_APinDescription[P].ulPort << 5) + g_APinDescription[P].ulPin)

#define INPUT_PIN_UP digitalPinToIOPin(4)
#define INPUT_PIN_DOWN digitalPinToIOPin(3)
#define INPUT_PIN_LEFT digitalPinToIOPin(1)
#define INPUT_PIN_RIGHT digitalPinToIOPin(2)
#define OUTPUT_PIN_UP digitalPinToIOPin(5)
#define OUTPUT_PIN_DOWN digitalPinToIOPin(10)
#define OUTPUT_PIN_LEFT digitalPinToIOPin(8)
#define OUTPUT_PIN_RIGHT digitalPinToIOPin(9)

#define READ_UP IOBUS::digitalRead(INPUT_PIN_UP)
#define READ_DOWN IOBUS::digitalRead(INPUT_PIN_DOWN)
#define READ_LEFT IOBUS::digitalRead(INPUT_PIN_LEFT)
#define READ_RIGHT IOBUS::digitalRead(INPUT_PIN_RIGHT)

#define WRITE_UP_LOW IOBUS::digitalWrite(OUTPUT_PIN_UP, LOW);
#define WRITE_DOWN_LOW IOBUS::digitalWrite(OUTPUT_PIN_DOWN, LOW);
#define WRITE_LEFT_LOW IOBUS::digitalWrite(OUTPUT_PIN_LEFT, LOW);
#define WRITE_RIGHT_LOW IOBUS::digitalWrite(OUTPUT_PIN_RIGHT, LOW);

#define WRITE_UP_HIGH IOBUS::digitalWrite(OUTPUT_PIN_UP, HIGH);
#define WRITE_DOWN_HIGH IOBUS::digitalWrite(OUTPUT_PIN_DOWN, HIGH);
#define WRITE_LEFT_HIGH IOBUS::digitalWrite(OUTPUT_PIN_LEFT, HIGH);
#define WRITE_RIGHT_HIGH IOBUS::digitalWrite(OUTPUT_PIN_RIGHT, HIGH);

#define PIN_COUNT 4

enum class Direction {
  neutral = -1,
  up = 0,
  down = 1,
  left = 2,
  right = 3
};

uint32_t inUp; uint32_t inDown; uint32_t inLeft; uint32_t inRight;
uint32_t outUp; uint32_t outDown; uint32_t outLeft; uint32_t outRight;
Direction lastDirectionUD = Direction::neutral;
Direction lastDirectionLR = Direction::neutral;

#ifdef DEBUG
uint32_t currentTime;
uint32_t loopTime;
#endif

void setup() {
#ifdef DEBUG
  Serial.begin(115200);
#endif

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
  currentTime = micros();
#endif

  // Read current inputs
  inUp = READ_UP;
  inDown = READ_DOWN;
  inLeft = READ_LEFT;
  inRight = READ_RIGHT;

  // Run algorithm
  runLastWin();

  // Send outputs
  if (outUp)
    WRITE_UP_LOW
  else
    WRITE_UP_HIGH;

  if (outDown)
    WRITE_DOWN_LOW
  else
    WRITE_DOWN_HIGH;

  if (outLeft)
    WRITE_LEFT_LOW
  else
    WRITE_LEFT_HIGH;

  if (outRight)
    WRITE_RIGHT_LOW
  else
    WRITE_RIGHT_HIGH;

#ifdef DEBUG
  // Log timing
  loopTime = micros() - currentTime;
  Serial.print("Input state: ");
  Serial.print(inUp); Serial.print(inDown); Serial.print(inLeft); Serial.println(inRight);
  Serial.print("Output state: ");
  Serial.print(outUp); Serial.print(outDown); Serial.print(outLeft); Serial.println(outRight);
  Serial.print("Loop time: ");
  Serial.println(loopTime);
#endif
}

uint8_t runNeutral() {
  if (inUp == 0 && inDown == 0) {
    outUp = 1;
    outDown = 1;
  } else {
    outUp = inUp;
    outDown = inDown;
  }

  if (inLeft == 0 && inRight == 0) {
    outLeft = 1;
    outRight = 1;
  } else {
    outLeft = inLeft;
    outRight = inRight;
  }
}

uint8_t runLastWin() {
  if (inUp == 0 && inDown == 0) {
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
    if (outUp == 0)
      lastDirectionUD = Direction::up;
    else if (outDown == 0)
      lastDirectionUD = Direction::down;
    else
      lastDirectionUD = Direction::neutral;
  }

  if (inLeft == 0 && inRight == 0) {
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
    if (outLeft == 0)
      lastDirectionLR = Direction::left;
    else if (outRight == 0)
      lastDirectionLR = Direction::right;
    else
      lastDirectionLR = Direction::neutral;
  }
}