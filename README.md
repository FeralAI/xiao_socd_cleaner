# Seeeduino XIAO SOCD Cleaner

![Seeedino XIAO pinout](/assets/Seeeduino-XIAO-pinout.jpg)

DIY SOCD cleaner module using a Seeeduino XIAO Arduino board.

## Introduction

The goal with this sketch is to be as fast as possible without resorting to writing assembly. The steps taken for speed are:

* Use of Cortex®-M0+ Single Cycle IOBUS for all I/O - This allows for single-cycle access to ports, which is much faster than using Arduino `digitalRead` and `digitalWrite` functions, and even faster than direct port manipulation! I discovered this through the [Seeeduino XIAO by Nanase](https://wiki.seeedstudio.com/Seeeduino-XIAO-by-Nanase/#use-single-cycle-iobus) page, which linked to [Sasapea's Lab](https://lab.sasapea.mydns.jp/2020/03/16/seeeduino-xiao/) for the `IOBUS.h` library.
* Excessive caching - RAM is an afterthought when CPU cycles are on the line. Anything that only needs to be calculated once (pin positions, pin values, binary offset values, etc.) is stored in a variable.
* `Release` build optimizations - Removing the DEBUG define will exclude all benchmark and logging code, allowing for optimal performance.

There are three SOCD methods available in the sketch:

* `Neutral` - Holding simultaneous opposite cardinal directions will give you a neutral input.
* `Up Priority` - Like `Neutral`, except Up takes priority on the vertical axis. This is the default SOCD method of the Hitbox controller.
* `Second Input Priority` - Commonly known as the `Last Win` method, this passes through the latest input for a given axis while maintaining the pressed state of the other direction for that axis. For example if you are holding left then press right, you will get a right input. If you then release and re-press left while still holding right, you will then get a left input. Finally, releasing left will give you a right input because right is still pressed. If both inputs are activated for an axis in the same input sampling window, you will receive neutral, though this is highly unlikely due to the extremely small window for sampling inputs. This is the default method for this sketch.

The only way to swap SOCD methods right now is to change the method call in `void loop()`. Adding logic around the PORTB pins would allow selecting between 4 different SOCD methods, and would be great for wiring up to DIP switches.

* The loop execution time using the `Neutral` or `Up Priority` method is **6-7μs**.
* The loop execution time using the `Last Win` method is **7-8μs**.

## Prototype

![XIAO SOCD Prototype Front](/assets/xiao_socd_proto1_front.jpg)
![XIAO SOCD Prototype Back](/assets/xiao_socd_proto1_back.jpg)

> Note: D0 is NOT connected to the 8-pin 2.54mm female header (via A4 on the proto board). I was going to leave that pin unused and shift the XIAO down, but too much of the other soldering was already done. I just clipped the base of the female header where D0 plugs in to prevent the ground connection. I'll eventually add a circuit diagram to make this all a bit clearer.

Parts used for my prototype:

* 1x 4x6cm protoboard
* 1x Seeeduino XIAO w/headers
* 1x PC847 4-channel photocoupler (completely isolates the input and output circuits at the cost of about 4μs)
* 4x 100Ω resistors (limits the forward voltage to the expected 1.2v for PC847 operation)
* 2x 5-pin JST-XH male connectors (common connector style for arcade joysticks, 1 pin for each direction and 1 ground)
* 1x 2-pin JST-XH male connector (to power the XIAO)
* 2x 8-pin 2.54mm female headers (makes XIAO pluggable)
* 1x 16-pin DIP socket (makes the photocoupler pluggable)

The current **"last win" SOCD** method takes about **8μs** to for a full loop. With the **4μs** the PC847 takes to trigger the outputs, that's about **12μs** max, or **.012ms**, of additional latency per input. I would consider that imperceptible to a human.

## Further Optimizations

A bulk of the gains to be had are already done, mostly around pin and register access. There are still a few more "micro" optimizations to be had with the sketch and prototype:

* Use a faster photo/optocoupler, one that switches in the range of nanoseconds instead of microseconds
* Evaluate SOCD logic for optimizations (flip boolean logic, bitwise operations, etc.)

## Resources

* [INTRODUCTION TO SOCD AND RESOLUTIONS](https://www.hitboxarcade.com/blogs/faq/what-is-an-socd)
* [Seeeduino XIAO Wiki](https://wiki.seeedstudio.com/Seeeduino-XIAO/)
* [Seeeduino XIAO by Nanase](https://wiki.seeedstudio.com/Seeeduino-XIAO-by-Nanase/)
* [Seeeduino XIAO Datasheet](https://files.seeedstudio.com/wiki/Seeeduino-XIAO/res/Seeeduino-XIAO-v1.0-SCH-191112.pdf)
* [Sasapea's Lab (IOBUS.h library)](https://lab.sasapea.mydns.jp/2020/03/16/seeeduino-xiao/)
* [PC827/PC847 Datasheet](https://datasheet.octopart.com/PC847-Sharp-Microelectronics-datasheet-101325.pdf)
