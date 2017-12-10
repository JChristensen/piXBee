#ifndef ARDUINO_H_INCLUDED
#define ARDUINO_H_INCLUDED

#define ARDUINO 100

#include <chrono>
#include <thread>
#include <stdint.h>
#include <stdio.h> // for size_t
#include <stddef.h>
#include <termios.h>
#include "Stream.h"
#include "HardwareSerial.h"

uint32_t millis();
uint32_t micros();
void delay(uint32_t ms);
void delayMicroseconds(uint32_t us);

class ArduinoTiming
{
public:
    ArduinoTiming();
    uint32_t millis();
    uint32_t micros();

private:
    std::chrono::steady_clock::time_point m_startTime;  //program start time
};

#endif // ARDUINO_H_INCLUDED
