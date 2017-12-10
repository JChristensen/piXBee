#include <iostream>
#include <string.h>
#include "XBee.h"

using namespace std;

class myXBee : public XBee
{
public:
    myXBee( XBeeAddress64 dest = XBeeAddress64(0x0, 0x0) );     //default destination is the coordinator
    bool read(void);
    void transmit(void);
    XBeeAddress64 m_destAddr;     // destination address
    struct payload_t
    {
        uint16_t seq;             // sequence number
        uint16_t ack;             // if acknowledging a received packet, sender's sequence number, else zero
        uint32_t time;            // time of transmission
        char message[32];
    }
    m_payload;

private:
    ZBTxStatusResponse m_zbStat;
    ModemStatusResponse m_zbMSR;
    ZBRxResponse m_zbRX;
    ZBTxRequest m_zbTX;
    uint32_t m_msTX;              // time of last transmission
    uint16_t sendSeq;             // sending sequence number.
};

myXBee::myXBee(XBeeAddress64 dest)
{
    m_destAddr = dest;
}

//process incoming traffic from the XBee
bool myXBee::read()
{
    uint8_t delyStat;    //TX delivery status
    uint8_t dscyStat;    //TX discovery status
    uint8_t txRetry;     //TX retrycount
    uint8_t modemStat;   //modem status response
    bool packetReceived(false);
    uint32_t ms = millis();

    readPacket();
    if ( getResponse().isAvailable() )
    {
        switch (getResponse().getApiId())   //what kind of packet did we get?
        {
        case ZB_TX_STATUS_RESPONSE:         //transmit status for packets we've sent
            getResponse().getZBTxStatusResponse(m_zbStat);
            delyStat = m_zbStat.getDeliveryStatus();
            dscyStat = m_zbStat.getDiscoveryStatus();
            txRetry = m_zbStat.getTxRetryCount();
            switch (delyStat)
            {
            case SUCCESS:
                //digitalWrite(LED_BUILTIN, LOW);
                cout << ms << " XBee TX OK " << ms - m_msTX << "ms RETRY=";
                cout << static_cast<unsigned int>(txRetry) << " DSCY=" << static_cast<unsigned int>(dscyStat) << endl;
                break;
            default:
                cout << ms << " XBee TX FAIL " << ms - m_msTX << "ms RETRY=";
                cout << txRetry << " DELY=" << static_cast<unsigned int>(delyStat) << " DSCY=" << static_cast<unsigned int>(dscyStat) << endl;
                break;
            }
            break;

        case ZB_RX_RESPONSE:                               //rx data packet
            getResponse().getZBRxResponse(m_zbRX);         //get the received data
            switch (m_zbRX.getOption() & 0x01)             //check ack bit only
            {
            case ZB_PACKET_ACKNOWLEDGED:
                {
                    uint8_t dataLen = m_zbRX.getDataLength();
                    if ( dataLen > sizeof(m_payload) )
                    {
                        cout << ms << "Received data too long (" << dataLen << "), truncated to " << sizeof(m_payload) << endl;
                        dataLen = sizeof(m_payload);
                    }
                    uint8_t* p = (uint8_t*) &m_payload;
                    for (uint8_t i=0; i<dataLen; i++)      //copy the received data to our buffer
                    {
                        *p++ = m_zbRX.getData(i);
                    }
                    //process the received data
                    m_destAddr = m_zbRX.getRemoteAddress64();     //save the sender's address
                    packetReceived = true;
                    if (m_payload.ack == 0)
                    {
                        cout << endl << ms << " Received message seq " << m_payload.seq << " sent at " << m_payload.time;
                        cout << " \"" << m_payload.message << "\"" << endl;
                    }
                    else
                    {
                        cout << endl << ms << " Received ack for seq " << m_payload.ack << " sent at " << m_payload.time;
                        cout << " \"" << m_payload.message << "\"" << endl;
                    }
                    break;
                }
            default:
                cout << endl << ms << " XBee RX no ACK\n";  //packet received and not ACKed
                break;
            }
            break;

        case MODEM_STATUS_RESPONSE:                   //XBee administrative messages
            getResponse().getModemStatusResponse(m_zbMSR);
            modemStat = m_zbMSR.getStatus();
            cout << ms << " ";
            switch (modemStat)
            {
            case HARDWARE_RESET:
                cout << "XBee HW reset\n";
                break;
            case ASSOCIATED:
                cout << "XBee associated\n";
                break;
            case DISASSOCIATED:
                cout << "XBee disassociated\n";
                break;
            default:
                cout << "XBee modem status " << modemStat << endl;
                break;
            }
            break;

        default:                                           //something else we were not expecting
            cout << ms << " XBee unexpected frame\n";
            break;
        }
    }
    return packetReceived;
}

//Send an XBee data packet.
//Our data packet is defined as follows:
//Byte  0-3:  Transmission time from millis()
//Bytes 4-19: Text message.
void myXBee::transmit(void)
{
    m_zbTX.setAddress64(m_destAddr);          //build the tx request packet
    m_zbTX.setAddress16(0xFFFE);
    m_zbTX.setPayload((uint8_t*)&m_payload);
    m_zbTX.setPayloadLength(sizeof(m_payload));
    m_msTX = millis();
    m_payload.time = m_msTX;
    m_payload.seq = ++sendSeq;
    send(m_zbTX);
    //digitalWrite(LED_BUILTIN, HIGH);
    cout << endl << m_msTX << " XBee TX\n";
}

// global variables
myXBee xb;
const char *device = "/dev/ttyUSB0";
const speed_t baudrate(B9600);
//const uint8_t xbeeReset(7);              // logic 0 on this pin resets the XBee
//const uint8_t xbeeFunction(8);           // ground this pin for transmitter/router, leave open for receiver/coordinator
bool isTransmitter(false);

// these variables can be modified to adjust message timing and sending of ack packets by the receiver
const uint32_t txInterval(10000);        // transmission interval, milliseconds
const bool sendAckMessage(true);         // set to true for receiver to send ack packets
const uint32_t ackDelayTime(1000);       // delay time before sending ack packet

void setup(void)
{
    //pinMode(xbeeFunction, INPUT_PULLUP);
    //pinMode(LED_BUILTIN, OUTPUT);
    //Serial.begin(baudrate);
    int beginStatus = Serial.begin(device, baudrate, 0);
    if (beginStatus == 0)
    {
        cerr << "Serial device opened.\n";
    }
    else
    {
        cerr << "Serial device open failed, status: " << beginStatus << endl;
        exit(1);
    }
    xb.begin(Serial);

    //Serial << F( "\n" __FILE__ " " __DATE__ " " __TIME__ "\n" );
    //isTransmitter = !digitalRead(xbeeFunction);
    if (isTransmitter)
    {
        cout << "\nTransmitter (XBee Router)\n";
    }
    else
    {
        cout << "\nReceiver (XBee Coordinator)\n";
    }
    //pinMode(xbeeReset, OUTPUT);
    //delay(10);
    //digitalWrite(xbeeReset, HIGH);
}

void loop(void)
{
    static uint32_t msLast;
    static uint32_t msRecv;
    static bool ackPending(false);

    if (xb.read() && sendAckMessage)    // if we received a data packet, prepare to respond to it
    {
        msRecv = millis();
        ackPending = true;
    }

    if ( isTransmitter && (millis() - msLast >= txInterval) )
    {
        msLast += txInterval;
        xb.m_payload.ack = 0;   // transmitter message, not an ack
        strcpy(xb.m_payload.message, "Hello from Arduino!");
        xb.transmit();
    }
    else if (!isTransmitter)
    {
        if (ackPending && (millis() - msRecv >= ackDelayTime))
        {
            xb.m_payload.ack = xb.m_payload.seq;
            strcpy(xb.m_payload.message, "Received packet");
            xb.transmit();
            ackPending = false;
        }
    }
}

int g_signal(0);

void sigintHandler(int s)
{
    g_signal = s;
}

int main()
{
    // catch Ctrl-C (SIGINT)
    struct sigaction sig;
    sig.sa_handler = sigintHandler;
    sigaction(SIGINT, &sig, NULL);

    setup();
    while (!g_signal)
        loop();

    int endStatus = Serial.end();
    if (endStatus == 0)
    {
        cout << "\nSerial device closed." << endl;
    }
    else
    {
        cerr << "Serial device close failed, status: " << endStatus << endl;
    }
    return 0;
}
