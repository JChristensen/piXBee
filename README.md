# XBee Interface for Raspberry Pi #
https://github.com/JChristensen/piXBee  
ReadMe file  
Jack Christensen Dec 2017

## Be forewarned
This is alpha quality, proof-of-concept demo code. It works for me but needs some review, tuning, general cleanup, etc. etc. Use at your own risk!

## Background
Over the last few years, I have deployed several XBee wireless sensor networks based on Arduino (ATmega328P and ATmega1284P). While the XBee modules form mesh networks, logically the data flow is that of a star network. The node with the XBee coordinator is the data concentrator that all sensor nodes send their data to. The coordinator node has an Ethernet interface and forwards the data to an online service like GroveStreams, ThingSpeak, etc.

This actually works remarkably well but the data concentrator function does seem like a lot to ask of a small microcontroller.

Thus there was a desire for a more powerful data concentrator and while a Raspberry Pi seemed like a suitable platform, I could not find an XBee interface that would serve similar to [Andrew Rapp's excellent XBee-Arduino library](https://github.com/andrewrapp/xbee-arduino) that I use with my Arduino projects.

## Approach
After studying the XBee-Arduino library a bit, it seemed that the library could be interfaced to a C++ program running on Linux. This turned out to be remarkably straightforward, only requiring a few classes to emulate a small subset of the Arduino environment. Mr Rapp gets more kudos here because had the library not been very well written in the first place, then this could have been quite messy. As it was, each class only required a few member functions that were quite simple in most cases. My goal was to use the XBee-Arduino library without any changes and I was quite happy to realize that goal.

## Running the demonstration
`main.cpp` is example code that will run on a Raspberry Pi, but it should work most anywhere that GCC is installed. It was developed and tested with [Code::Blocks](http://www.codeblocks.org/) on a Linux PC but is not difficult to compile and build from the command line; included is a bash script `(build.sh)` to build the executable.

The `examples` folder contains an Arduino sketch that will communicate with a Raspberry Pi that has an XBee connected to a USB port via an FTDI interface. Read the comments in `piXBee.ino` for circuit connection information. The `main.cpp` file that runs on Linux is basically just a copy of this sketch, modified to use `cout` in place of `Serial` writes, and removing Arduino GPIO calls.

#### To deploy the example code: ####
1. Create a directory on the Raspberry Pi and from this repo, copy `*.cpp, *.h` and `build.sh` to it.
2. From the [XBee library](https://github.com/andrewrapp/xbee-arduino), copy the `XBee.cpp` and `XBee.h` files to the directory created above. Alternately, if the XBee library already exists elsewhere, set the `xbeeLib` variable in the `build.sh` script accordingly.
3. Run the `build.sh` script to create the executable file `piXBee`.
4. Load the example sketch on an Arduino. In a terminal on the Pi, run `piXBee`. Observe the communication there and in the Arduino serial monitor.
