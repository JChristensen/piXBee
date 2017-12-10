#ifndef STREAM_H_INCLUDED
#define STREAM_H_INCLUDED

#include <fcntl.h>
#include <iostream>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>      // for size_t
#include <termios.h>
#include <unistd.h>     // for write(2)
#include "RingBuf.h"

class Stream
{
    public:
        Stream();
        int available();
        int read();
        size_t write(const uint8_t);
        void flush();

    protected:
        bool readAll();
        int m_fd;
        RingBuf m_buf;
};

#endif // STREAM_H_INCLUDED
