/**
 * I/O definition file
 * 
 * NOTE: If the pin layout changes, the definitions in the file will need to be updated.
 */
#ifndef __IODEF_H
#define __IODEF_H

#include "PINDEF.h"

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
    static const uint16_t PROGMEM maskD    = 0 | (1 << INPUT_PORT_PIN_DOWN);
    static const uint16_t PROGMEM maskL    = 0 | (1 << INPUT_PORT_PIN_LEFT);
    static const uint16_t PROGMEM maskR    = 0 | (1 << INPUT_PORT_PIN_RIGHT);
    static const uint16_t PROGMEM maskU    = 0 | (1 << INPUT_PORT_PIN_UP);
    static const uint16_t PROGMEM maskUD   = maskU | maskD;
    static const uint16_t PROGMEM maskLR   = maskL | maskR;
    static const uint16_t PROGMEM maskUDLR = maskU | maskD | maskL | maskR;
 PROGMEM
    // Cached output bit  PROGMEMflags
    static const uint16_t PROGMEM valueD    = 0 | (1 << OUTPUT_PORT_PIN_DOWN);
    static const uint16_t PROGMEM valueL    = 0 | (1 << OUTPUT_PORT_PIN_LEFT);
    static const uint16_t PROGMEM valueR    = 0 | (1 << OUTPUT_PORT_PIN_RIGHT);
    static const uint16_t PROGMEM valueU    = 0 | (1 << OUTPUT_PORT_PIN_UP);
    static const uint16_t PROGMEM valueLR   = valueL | valueR;
    static const uint16_t PROGMEM valueUD   = valueU | valueD;
    static const uint16_t PROGMEM valueUDLR = valueU | valueD | valueL | valueR;
};

#endif