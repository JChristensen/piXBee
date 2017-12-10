#include "RingBuf.h"

RingBuf::RingBuf(const int bufSize) : m_bufSize(bufSize), m_nChars(0), m_nLost(0)
{
    m_ringBuf = new (std::nothrow) char[m_bufSize];
    if (!m_ringBuf)
    {
        std::cerr << "Could not allocate ring buffer!" << std::endl;
        exit(1);
    }
    m_nextWrite = m_ringBuf;
    m_nextRead = m_ringBuf;
}

RingBuf::~RingBuf()
{
    delete[] m_ringBuf;
    m_ringBuf = nullptr;
}

int RingBuf::get(char &ch)
{
    // are there any chars to get?
    if (m_nChars == 0)
    {
        return -1;      // no
    }
    else
    {
        ch = *m_nextRead++;     // yes, return it to the caller
        --m_nChars;             // decrement the character count

        // are we pointing past the end of the buffer
        if (m_nextRead >= m_ringBuf + m_bufSize)
        {
            m_nextRead = m_ringBuf;    // yes, point back to the start
        }

        // there's room for at least one character now, so zero the lost counter
        int nLost = m_nLost;
        m_nLost = 0;
        return nLost;
    }
}

bool RingBuf::put(const char ch)
{
    // is there room
    if (m_nChars >= m_bufSize)
    {
        ++m_nLost;          // no
        //std::cout << "RingBuf full, lost " << (static_cast<int>(ch) & 0xff) << " (" << ch << ")" << std::endl;
        return false;
    }
    else
    {
        *m_nextWrite++ = ch;  // yes, store the character
        ++m_nChars;           // count it
        //std::cout << "RingBuf put " << (static_cast<int>(ch) & 0xff) << " (" << ch << ") count=" << m_nChars << std::endl;

        // are we pointing past the end of the buffer
        if (m_nextWrite >= m_ringBuf + m_bufSize)
        {
            m_nextWrite = m_ringBuf;    // yes, point back to the start
        }
        return true;
    }
}

int RingBuf::available()
{
    return m_nChars;
}
