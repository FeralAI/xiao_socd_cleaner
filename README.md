# Seeeduino XIAO SOCD Cleaner

DIY SOCD cleaner module using a Seeeduino XIAO Arduino board.

## Description

The goal with this sketch is to create a standalone SOCD cleaner that runs as fast as possible with the given hardware:

* Use of Cortex®-M0+ Single Cycle IOBUS for all I/O - This allows for querying pins via direct CPU to port/register access, which is much faster than using the Arduino `digitalRead` and `digitalWrite` functions, and even faster than typical direct port manipulation! I discovered this through the [Seeeduino XIAO by Nanase](https://wiki.seeedstudio.com/Seeeduino-XIAO-by-Nanase/#use-single-cycle-iobus) page, which linked to [Sasapea's Lab](https://lab.sasapea.mydns.jp/2020/03/16/seeeduino-xiao/) for the `IOBUS.h` library.
* Caching - RAM is an afterthought when CPU cycles are on the line. Anything that can be pre-calculated (pin positions, pin values, binary offsets for compare, etc.) is stored in variables prior to executing the main loop.
* `Release` build optimizations - Removing the DEBUG define will exclude all benchmark and logging code, allowing for optimal performance.

### SOCD Cleaning

There are three SOCD cleaning methods available in the sketch:

* `Neutral` - Holding SOCD will give you a neutral input on either axis.
* `Up Priority` - Like `Neutral`, except Up takes priority on the vertical axis. This is the default SOCD cleaning method of the Hit Box controllers.
* `Second Input Priority` - Commonly known as the `Last Win` method, this passes through the latest input for a given axis while maintaining the pressed state of the other direction for that axis. For example if you are holding left then press right, you will get a right input. If you then release and re-press left while still holding right, you will then get a left input. Finally, releasing left will give you a right input because right is still pressed. If both inputs are activated for an axis in the same input sampling window, you will receive neutral, though this is highly unlikely due to the extremely small window for sampling inputs. This is the default method for this sketch.

The only way to swap SOCD methods right now is to change the method call in `void loop()` and reupload the sketch. Adding logic around the PORTB pins would allow selecting between 4 different SOCD methods, and would be great for wiring up to DIP switches.

The current loop execution times are:

 | SOCD Method           | Loop Minimum        | Loop Maximum        |
 | --------------------- | ------------------- | ------------------- |
 | Neutral               |  32 cycles / 0.67μs |  80 cycles / 1.67μs |
 | Up Priority           |  29 cycles / 0.60μs |  73 cycles / 1.52μs |
 | Second Input Priority |  30 cycles / 0.62μs | 104 cycles / 2.17μs |

### Pin Mapping

![Seeedino XIAO pinout](/assets/Seeeduino-XIAO-pinout.jpg)

The default pin mapping for this sketch is:

| Digital Pin # | PORT Pin | INPUT/OUTPUT | Direction |
| ------------- | -------- | ------------ | --------- |
| D1            | PA4      | INPUT        | LEFT      |
| D2            | PA10     | INPUT        | RIGHT     |
| D3            | PA11     | INPUT        | DOWN      |
| D4            | PA8      | INPUT        | UP        |
| D5            | PA9      | OUTPUT       | UP        |
| D8            | PA7      | OUTPUT       | LEFT      |
| D9            | PA5      | OUTPUT       | RIGHT     |
| D10           | PA6      | OUTPUT       | DOWN      |

## Building the XIAO SOCD Cleaner

### Schematic

The XIAO runs on 3.3v, and can be directly hooked up to boards that support that voltage and a common ground. The provided schematic uses a photocoupler to isolate the input and output circuits for better compatibility with pad hacks and retail encoders.

![XIAO SOCD Schematic](/assets/XIAO%20SOCD%20Cleaner_schem.png)

The input/output wire coloring on the schematic follows the typical Sanwa JLF wiring scheme with the 5-pin connector facing the buttons, as seen in the bottom-right corner of this image:

![Sanwa JLF wiring diagram](/assets/sanwa_wiring.jpg)

### Prototype

![XIAO SOCD Prototype Front](/assets/xiao_socd_proto1_front.jpg)
Prototype layout almost matches schematic 1:1

![XIAO SOCD Prototype Back](/assets/xiao_socd_proto1_back.jpg)
Ignore the solder bridge that is touching the D0 pin (A4 via on the proto board), the female 2.54mm header was clipped on the top of the board to prevent always pulling D0 to ground.

Parts used for the prototype:

* 1x 4x6cm protoboard
* 1x Seeeduino XIAO w/headers
* 1x LTV847/PC847 4-channel photocoupler (rise/fall time for trigger is ~4μs)
* 4x 100Ω resistors (limits the forward voltage to the expected 1.2v for PC847 operation)
* 2x 5-pin JST-XH male connectors (common connector style for arcade joysticks, 1 pin for each direction and 1 ground)
* 1x 2-pin JST-XH male connector (to power the XIAO)
* 2x 8-pin 2.54mm female headers (makes XIAO pluggable)
* 1x 16-pin DIP socket (makes the photocoupler pluggable)

### Notes

This sketch uses logic level LOW to detect an input is pressed, which is quite common for controllers and encoders. Since this prototype uses the LTV847 photocoupler, the output logic needs to be inverted to trigger a press on logic level HIGH. The `INVERT_OUTPUT_LOGIC` define is used in the sketch to, well, invert the output logic to work with the photocoupler.

### Performance

All SOCD cleaning methods take **2μs** or less for a full loop. With the **4μs** the LTV847 takes to trigger the outputs, that's about **6μs** max, or **.006ms**, of additional latency per input. I would consider that imperceptible to a human.

## TODOs

A bulk of the optimizations to be had are already done, mostly around pin and register access. There are still a few more minor optimizations to be had with the sketch and prototype:

* Evaluate logic for optimizations (inline calcs, flip boolean logic, bitwise operations, etc.)
* Use a faster photo/optocoupler, one that switches in the range of nanoseconds instead of microseconds

## Resources

* [INTRODUCTION TO SOCD AND RESOLUTIONS](https://www.hitboxarcade.com/blogs/faq/what-is-an-socd)
* [Seeeduino XIAO Wiki](https://wiki.seeedstudio.com/Seeeduino-XIAO/)
* [Seeeduino XIAO by Nanase](https://wiki.seeedstudio.com/Seeeduino-XIAO-by-Nanase/)
* [Seeeduino XIAO Datasheet](https://files.seeedstudio.com/wiki/Seeeduino-XIAO/res/Seeeduino-XIAO-v1.0-SCH-191112.pdf)
* [Sasapea's Lab (IOBUS.h library)](https://lab.sasapea.mydns.jp/2020/03/16/seeeduino-xiao/)
* [LTV847 Datasheet](https://www.mouser.com/datasheet/2/239/LTV-8X7_series_201610_-1544776.pdf)
* [PC847 Datasheet](https://datasheet.octopart.com/PC847-Sharp-Microelectronics-datasheet-101325.pdf)
