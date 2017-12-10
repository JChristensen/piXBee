#!/bin/bash
# Simple script to build piXBee project.

# Directory where the XBee library is stored
xbeeLib="../../Dropbox/sketchbook/libraries/XBee"

g++ -Wall -std=c++11 -fexceptions -DARDUINO=100 -O2 -std=c++11 -I$xbeeLib -I. -c $xbeeLib/XBee.cpp -o XBee.o
g++ -Wall -std=c++11 -fexceptions -DARDUINO=100 -O2 -std=c++11 -I$xbeeLib -I. -c Arduino.cpp -o Arduino.o
g++ -Wall -std=c++11 -fexceptions -DARDUINO=100 -O2 -std=c++11 -I$xbeeLib -I. -c HardwareSerial.cpp -o HardwareSerial.o
g++ -Wall -std=c++11 -fexceptions -DARDUINO=100 -O2 -std=c++11 -I$xbeeLib -I. -c main.cpp -o main.o
g++ -Wall -std=c++11 -fexceptions -DARDUINO=100 -O2 -std=c++11 -I$xbeeLib -I. -c RingBuf.cpp -o RingBuf.o
g++ -Wall -std=c++11 -fexceptions -DARDUINO=100 -O2 -std=c++11 -I$xbeeLib -I. -c Stream.cpp -o Stream.o
g++  -o piXBee XBee.o Arduino.o HardwareSerial.o main.o RingBuf.o Stream.o  -s
