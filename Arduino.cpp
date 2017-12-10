#include "Arduino.h"

// constructor saves the start time
ArduinoTiming::ArduinoTiming()
{
    m_startTime = std::chrono::steady_clock::now();
}

// return the time since the program began running in milliseconds
uint32_t ArduinoTiming::millis()
{
    std::chrono::steady_clock::time_point nowTime = std::chrono::steady_clock::now();
    return static_cast<uint32_t>(std::chrono::duration_cast<std::chrono::milliseconds>(nowTime - m_startTime).count());
}

// return the time since the program began running in microseconds
uint32_t ArduinoTiming::micros()
{
    std::chrono::steady_clock::time_point nowTime = std::chrono::steady_clock::now();
    return static_cast<uint32_t>(std::chrono::duration_cast<std::chrono::microseconds>(nowTime - m_startTime).count());
}

ArduinoTiming arduinoTiming;

// sleep for ms milliseconds
void delay(uint32_t ms)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

// sleep for us microseconds
void delayMicroseconds(uint32_t us)
{
    std::this_thread::sleep_for(std::chrono::microseconds(us));
}

// return the time since the program began running in milliseconds
uint32_t millis()
{
    return arduinoTiming.millis();
}

// return the time since the program began running in microseconds
uint32_t micros()
{
    return arduinoTiming.micros();
}
