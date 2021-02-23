# Seeeduino XIAO SOCD Cleaner

DIY SOCD cleaner module for an arcade stick, using a Seeeduino XIAO Arduino board.

## Description

The goal with this sketch is to create a standalone SOCD cleaner that runs as fast as possible with the given hardware:

* Use of Cortex®-M0+ Single Cycle IOBUS for all I/O - This allows for querying pins via direct CPU to port/register access, which is much faster than using the Arduino `digitalRead` and `digitalWrite` functions, and even faster than typical direct port manipulation! I discovered this through the [Seeeduino XIAO by Nanase](https://wiki.seeedstudio.com/Seeeduino-XIAO-by-Nanase/#use-single-cycle-iobus) page, which linked to [Sasapea's Lab](https://lab.sasapea.mydns.jp/2020/03/16/seeeduino-xiao/) for the `IOBUS.h` library.
* Caching - RAM is an afterthought when CPU cycles are on the line. Anything that can be pre-calculated (pin positions, pin values, binary offsets for compare, etc.) is stored in variables or PROGMEM prior to executing the main loop.
* Stack vs. Heap variables - Stack variables take fewer processor cycles to access.  Prioritize local variables instead of global variables.
* `Release` build optimizations - Removing the DEBUG define will exclude all benchmark and logging code, allowing for optimal performance.

### SOCD Cleaning

There are two SOCD cleaning methods to choose from, they are:

* `Neutral` - Holding SOCD will give you a neutral input on either axis.
* `Second Input Priority` - Commonly known as the `Last Win` method, this passes through the latest input for a given axis while maintaining the pressed state of the other direction for that axis. For example if you are holding left then press right, you will get a right input. If you then release and re-press left while still holding right, you will then get a left input. Finally, releasing left will give you a right input because right is still pressed. If both inputs are activated for an axis in the same input sampling window, you will receive neutral, though this is highly unlikely due to the extremely small window for sampling inputs. This is the default method for this sketch.

Each SOCD cleaning method can be modified to make the `Up` direction take priority, known as `Up Priority`. When selecting the `Neutral` method with `Up Priority` enabled, this mimics the default SOCD cleaning of the Hit Box controller. `Up Priority` can also be used with `Second Input Priority` to override the vertical axis behavior.

The 2 DIP switches in the schematic will allow selection of SOCD cleaning method (DIP 1) and Up Priority enable/disable (DIP 2). The SOCD cleaning takes from 12 cycles / 0.25μs up to 80 cycles / 1.67μs depending on the inputs and SOCD options.

## Building the XIAO SOCD Cleaner

### Pin Mapping

![Seeedino XIAO pinout](/assets/Seeeduino-XIAO-pinout.jpg)

The default pin mapping for this sketch is:

| Digital Pin # | PORT Pin | INPUT/OUTPUT | Direction |
| ------------- | -------- | ------------ | --------- |
| D0            | PA2      | INPUT        | LEFT      |
| D1            | PA4      | INPUT        | RIGHT     |
| D2            | PA10     | INPUT        | DOWN      |
| D3            | PA11     | INPUT        | UP        |
| D4            | PA8      | OUTPUT       | LEFT      |
| D5            | PA9      | INPUT        | DIP2      |
| D6            | PB8      | INPUT        | DIP1      |
| D8            | PA7      | OUTPUT       | RIGHT     |
| D9            | PA5      | OUTPUT       | DOWN      |
| D10           | PA6      | OUTPUT       | UP        |

### Schematic

The Seeeduino XIAO runs on 3.3v, and can be directly hooked up to encoder boards that provide 3.3v and use a common ground:

![XIAO SOCD Schematic 3.3v common ground](/assets/XIAO%20SOCD%20Cleaner_schem_direct.png)

> NOTE: The XIAO can be powered via the 5v pin instead of the 3.3v pin, however the pin logic will still run on 3.3v.

A photocoupler/optocoupler can also be used to electrically isolate the input and output circuits for better compatibility with pad hacks and retail encoders:

![XIAO SOCD Schematic w/optocoupler](/assets/XIAO%20SOCD%20Cleaner_schem_v2.png)

The input/output wire coloring on the schematic follows the typical Sanwa JLF wiring scheme with the 5-pin connector facing the buttons, as seen in the bottom-right corner of this image:

![Sanwa JLF wiring diagram](/assets/sanwa_wiring.jpg)

### Prototype

<div style="width: 100%">
  <img src="assets/xiao_socd_proto2_front.jpg" alt="XIAO SOCD Prototype Front" style="display: inline-block; width: 49%" />
  <img src="assets/xiao_socd_proto2_back.jpg" alt="XIAO SOCD Prototype Back" style="display: inline-block; width: 49%" /> 
</div>

Parts used for the prototype:

* 1x Seeeduino XIAO
* 1x [Mini Perma Proto Breadboard](https://www.amazon.com/gp/product/B085WPTQ9B)
* 1x LTV847/PC847 4-channel photocoupler (rise/fall time for trigger is ~4μs)
* 4x 100Ω resistors (limits the forward voltage to the expected 1.2v for PC847 operation)
* 2x 5-pin JST-XH male connectors
* 1x 2-pin JST-XH male connector
* 1x 2-position DIP switch

### Notes

This sketch uses logic level LOW to detect an input is pressed, which is quite common for controllers and encoders. Since this prototype uses the LTV847 photocoupler, the output logic needs to be inverted to trigger a press on logic level HIGH. The `INVERT_OUTPUT_LOGIC` define is used in the sketch to, well, invert the output logic to work with the photocoupler.

### Performance

All SOCD cleaning methods take up to **2μs** for a full loop. With the **4μs** the LTV847 takes to trigger the outputs, that's about **6μs** max, or **.006ms**, of additional latency per input. I would consider that imperceptible to a human.

## TODOs

A bulk of the optimizations to be had are already done, mostly around pin and register access. There are still a few more minor optimizations to be had with the sketch and prototype:

* Evaluate logic for optimizations (inline calcs, flip boolean logic, bitwise operations, etc.)
* Use a faster photo/optocoupler, one that switches in the range of nanoseconds instead of microseconds

## Resources

* [INTRODUCTION TO SOCD AND RESOLUTIONS](https://www.hitboxarcade.com/blogs/faq/what-is-an-socd)
* [Seeeduino XIAO Wiki](https://wiki.seeedstudio.com/Seeeduino-XIAO/)
* [Seeeduino XIAO by Nanase](https://wiki.seeedstudio.com/Seeeduino-XIAO-by-Nanase/)
* [Seeeduino XIAO Schematic](https://files.seeedstudio.com/wiki/Seeeduino-XIAO/res/Seeeduino-XIAO-v1.0-SCH-191112.pdf)
* [Sasapea's Lab (IOBUS.h library)](https://lab.sasapea.mydns.jp/2020/03/16/seeeduino-xiao/)
* [LTV847 Datasheet](https://www.mouser.com/datasheet/2/239/LTV-8X7_series_201610_-1544776.pdf), [PC847 Datasheet](https://datasheet.octopart.com/PC847-Sharp-Microelectronics-datasheet-101325.pdf)
