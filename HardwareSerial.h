#ifndef HARDWARESERIAL_H_INCLUDED
#define HARDWARESERIAL_H_INCLUDED

#include <fcntl.h>
#include <iostream>
#include <signal.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>      // for size_t
#include <termios.h>
#include <unistd.h>
#include "Stream.h"

class HardwareSerial : public Stream
{
    public:
        HardwareSerial();
        int begin(const char *device, const speed_t baudrate, const int config);
        int end();

    private:
};

extern HardwareSerial Serial;

#endif // HARDWARESERIAL_H_INCLUDED
