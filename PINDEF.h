/**
 * Pin definition file
 * 
 * NOTE: If the pin layout changes, the definitions in the file will need to be updated.
 */

#ifndef __PINDEF_H
#define __PINDEF_H

#ifndef PROTOTYPE_VERSION
#define PROTOTYPE_VERSION 2
#endif

// All pins defined on PORTA so they can be read and written in one operation

#if PROTOTYPE_VERSION == 1
/**
 * Prototype V1
 */
#define INPUT_PIN_UP           4 // D4/PA8
#define INPUT_PIN_DOWN         3 // D3/PA11
#define INPUT_PIN_LEFT         1 // D1/PA4
#define INPUT_PIN_RIGHT        2 // D2/PA10
#define OUTPUT_PIN_UP          5 // D5/PA9
#define OUTPUT_PIN_DOWN       10 // D10/PA6
#define OUTPUT_PIN_LEFT        8 // D8/PA7
#define OUTPUT_PIN_RIGHT       9 // D9/PA5
#define DIP1_PIN               7 // D7/PB9
#define DIP2_PIN               6 // D6/PB8

#define INPUT_PORT_PIN_UP      8 // D4/PA8
#define INPUT_PORT_PIN_DOWN   11 // D3/PA11
#define INPUT_PORT_PIN_LEFT    4 // D1/PA4
#define INPUT_PORT_PIN_RIGHT  10 // D2/PA10
#define OUTPUT_PORT_PIN_UP     9 // D5/PA9
#define OUTPUT_PORT_PIN_DOWN   6 // D10/PA6
#define OUTPUT_PORT_PIN_LEFT   7 // D8/PA7
#define OUTPUT_PORT_PIN_RIGHT  5 // D9/PA5
#define DIP1_PORT_PIN          9 // D7/PB9
#define DIP2_PORT_PIN          8 // D6/PB8

#endif

#if PROTOTYPE_VERSION >= 2
/**
 * Prototype V2
 */
#define INPUT_PIN_UP           3 // D3/PA11
#define INPUT_PIN_DOWN         2 // D2/PA10
#define INPUT_PIN_LEFT         0 // D0/PA2
#define INPUT_PIN_RIGHT        1 // D1/PA4
#define OUTPUT_PIN_UP         10 // D10/PA6
#define OUTPUT_PIN_DOWN        9 // D9/PA5
#define OUTPUT_PIN_LEFT        4 // D4/PA8
#define OUTPUT_PIN_RIGHT       8 // D8/PA7
#define DIP1_PIN               6 // D6/PB8
#define DIP2_PIN               5 // D5/PA9

#define INPUT_PORT_PIN_UP     11 // D3/PA11
#define INPUT_PORT_PIN_DOWN   10 // D2/PA10
#define INPUT_PORT_PIN_LEFT    2 // D0/P
#define INPUT_PORT_PIN_RIGHT   4 // D1/PA4
#define OUTPUT_PORT_PIN_UP     6 // D10/PA6
#define OUTPUT_PORT_PIN_DOWN   5 // D9/PA5
#define OUTPUT_PORT_PIN_LEFT   8 // D4/PA8
#define OUTPUT_PORT_PIN_RIGHT  7 // D8/PA7
#define DIP1_PORT_PIN          8 // D6/PB8
#define DIP2_PORT_PIN          9 // D5/PA9

#endif

#endif