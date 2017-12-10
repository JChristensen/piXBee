#include "Stream.h"

Stream::Stream()
{
}

int Stream::available()
{
    readAll();
    return m_buf.available();
}

int Stream::read()
{
    readAll();

    char c;
    int stat = m_buf.get(c);
    if (stat < 0)
    {
        return stat;
    }
    else if (stat > 0)
    {
        std::cerr << "<Lost " << stat << ">";
    }
    return c;
}

void Stream::flush()
{
    tcdrain(m_fd);
}

size_t Stream::write(const uint8_t ch)
{
    tcdrain(m_fd);  // ensure any prior characters are sent before trying to send more
    return ::write(m_fd, &ch, 1);
}

// read all available characters from the serial device into our buffer.
// return false if buffer fills up, else return true;
bool Stream::readAll()
{
    int nchar;
    char c[8];

    do
    {
        nchar = ::read(m_fd, c, 1);
        if (nchar > 0)
        {
            if (!m_buf.put(c[0]))
            {
                //std::cerr << "Serial buffer overrun!" << std::endl;
                std::cerr << "Lost char " << static_cast<int>(c[0]) << " (" << c[0] << ")" << std::endl;
                return false;
            }
        }
    }
    while (nchar > 0);

    return true;
}
