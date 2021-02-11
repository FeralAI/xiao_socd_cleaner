# Seeeduino XIAO SOCD Cleaner

DIY SOCD cleaner module using a Seeeduino XIAO Arduino board.

Pin mappings aren't optimal since I've separated my input and output circuits using a PC847 optocoupler. I did this mostly to protect the output circuit.

The PC847 takes about 4μs to trigger an output. The current "last win" SOCD method takes about 10μs to run the logic. Combined that's about 15μs for a full loop. That shouldn't introduce any real added latency, however I'm still looking to optimize the logic. Some possible optimizations:

* Use a better/faster optocoupler, one that switches in the range of nanoseconds instead of microseconds
* Pin operations are optimized at an individual level using IOBUS, though this may be optimized by setting the register once for all pins instead of per pin.
