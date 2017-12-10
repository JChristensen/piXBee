#include "HardwareSerial.h"

HardwareSerial::HardwareSerial()
{
}

int HardwareSerial::begin(const char *device, const speed_t baudrate, const int config)
{
    // open the port
    m_fd = open(device, O_RDWR | O_NOCTTY | O_NDELAY);
    if(m_fd == -1)
    {
        return -1;
    }

    struct termios portAttrib;

    if (tcgetattr(m_fd, &portAttrib) < 0)
    {
        std::cerr << "Error getting port attributes" << std::endl;
    }
    cfmakeraw(&portAttrib);
    cfsetspeed(&portAttrib, baudrate);
    portAttrib.c_cc[VMIN] = 0;
    portAttrib.c_cc[VTIME] = 0;
    tcflush(m_fd, TCIOFLUSH);
    int attrStatus = tcsetattr(m_fd, TCSAFLUSH, &portAttrib);
    if (attrStatus) std::cerr << "Set attr status = " << attrStatus << std::endl;
    tcflush(m_fd, TCIOFLUSH);

    return 0;
}

int HardwareSerial::end()
{
    return close(m_fd);
}

HardwareSerial Serial;
