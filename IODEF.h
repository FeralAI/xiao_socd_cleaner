/**
 * I/O definition file
 */

#ifndef __IODEF_H
#define __IODEF_H

#include "PINDEF.h"
#include "IOBUS.h"

#define digitalPinToIOPin(P) ((g_APinDescription[P].ulPort << 5) + g_APinDescription[P].ulPin)

// All pins defined on PORTA so they can be read and written in one operation
#define INPUT_IOPIN_UP     digitalPinToIOPin(INPUT_PIN_UP)
#define INPUT_IOPIN_DOWN   digitalPinToIOPin(INPUT_PIN_DOWN)
#define INPUT_IOPIN_LEFT   digitalPinToIOPin(INPUT_PIN_LEFT)
#define INPUT_IOPIN_RIGHT  digitalPinToIOPin(INPUT_PIN_RIGHT)
#define OUTPUT_IOPIN_UP    digitalPinToIOPin(OUTPUT_PIN_UP)
#define OUTPUT_IOPIN_DOWN  digitalPinToIOPin(OUTPUT_PIN_DOWN)
#define OUTPUT_IOPIN_LEFT  digitalPinToIOPin(OUTPUT_PIN_LEFT)
#define OUTPUT_IOPIN_RIGHT digitalPinToIOPin(OUTPUT_PIN_RIGHT)
#define DIP_IOPIN_1        digitalPinToIOPin(DIP1_PIN)
#define DIP_IOPIN_2        digitalPinToIOPin(DIP2_PIN)

void configurePins() {
  IOBUS::pinMode(DIP_IOPIN_1, INPUT_PULLUP);
  IOBUS::pinMode(DIP_IOPIN_2, INPUT_PULLUP);

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

enum class Direction {
  neutral = -1,
  up      = 0,
  down    = 1,
  left    = 2,
  right   = 3,
};

// This class predefines all the possible input masks and values for the pin definitions above,
// and is used to avoid on-the-fly calculations during the input cleaning process.
class InputMasks {
  public:
    // Set up input masks. Bit offsets are based on the port number, e.g. PA9 == (1 << 9).
    // If the input pins are changed, these values will need to be updated
    
    // Input mask permuatations
    static const uint32_t maskD    = (1 << INPUT_PORT_PIN_DOWN);
    static const uint32_t maskL    = (1 << INPUT_PORT_PIN_LEFT);
    static const uint32_t maskR    = (1 << INPUT_PORT_PIN_RIGHT);
    static const uint32_t maskU    = (1 << INPUT_PORT_PIN_UP);
    static const uint32_t maskUD   = maskU | maskD;
    static const uint32_t maskLR   = maskL | maskR;
    static const uint32_t maskUDLR = maskU | maskD | maskL | maskR;

    // Cached output bit enable flags
    static const uint32_t valueD    = (1 << OUTPUT_PORT_PIN_DOWN);
    static const uint32_t valueL    = (1 << OUTPUT_PORT_PIN_LEFT);
    static const uint32_t valueR    = (1 << OUTPUT_PORT_PIN_RIGHT);
    static const uint32_t valueU    = (1 << OUTPUT_PORT_PIN_UP);
    static const uint32_t valueLR   = valueL | valueR;
    static const uint32_t valueUD   = valueU | valueD;
    static const uint32_t valueUDLR = valueU | valueD | valueL | valueR;

    static const uint32_t dip1 = (1 << DIP1_PORT_PIN);
    static const uint32_t dip2 = (1 << DIP2_PORT_PIN);
};

#endif