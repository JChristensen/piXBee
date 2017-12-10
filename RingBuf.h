#ifndef RINGBUF_H_INCLUDED
#define RINGBUF_H_INCLUDED

// A simple ring (circular) buffer class for character data.
// J.Christensen 15Nov2017

#include <iostream>

class RingBuf
{
    public:
        // constructor creates a ring buffer with the given size.
        RingBuf(const int bufSize = 1024);

        // get the next character from the buffer.
        // returns -1 if none available, else returns the number of
        // characters lost since the last get().
        int get(char &ch);

        // store a new character in the buffer. returns true for success,
        // false if the character could not be stored because the buffer was full.
        bool put(const char ch);

        // return the number of characters in the buffer.
        int available();

        // destructor de-allocates the dynamically allocated ring buffer.
        ~RingBuf();

    private:
        char *m_ringBuf;    // pointer to the start of the buffer
        char *m_nextWrite;  // pointer to the next position to put a character
        char *m_nextRead;   // pointer to the next position to get a character
        int m_bufSize;      // number of characters the buffer can hold
        int m_nChars;       // number of characters currently in the buffer
        int m_nLost;        // number of characters lost because the buffer was full
};

#endif // RINGBUF_H_INCLUDED
