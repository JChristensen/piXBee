// Sketch to test XBee communication between Arduino and Raspberry Pi.
//
// Required hardware:
// (1) Arduino Uno
// (2) XBee ZB S2C part no. XB24CZ7WIT-004
// (2) Adafruit XBee Adapter, http://www.adafruit.com/products/126
// (1) Adafruit FTDI Friend, http://www.adafruit.com/product/284
//
// The Arduino node is called the "transmitter" since it transmits first.
// The XBee on the Arduino node is configured as a router.
// Connect as follows:
//   Arduino Pin 0 (RX) --> XBee Adapter TX pin *
//   Arduino Pin 1 (TX) --> XBee Adapter RX pin
//   Arduino Pin 7      --> XBee Adapter Reset pin (RST)
//   +5V and GND        --> XBee Adapter +5V and GND
//   Arduino Pin 8      --> Ground (indicates transmitter node)
//   Arduino Pin 13 LED used to indicate waiting for ack after transmit.
//
// * Be sure to disconnect Arduino pin 0 (RX) from the XBee when uploading this sketch.
//
// The Raspberry Pi node is called the "receiver" and is connected to an
// XBee configured as a coordinator via a USB port and FTDI Friend:
//   XBee --> XBee Adapter --> FTDI Friend --> Raspberry Pi USB port
//
// This sketch can also run on two Arduinos, for both transmitter and receiver.
// Ground pin 8 on the transmitter (router) node, and leave it unconnected
// on the receiver (coordinator) node.
//
// The transmitter sends a data packet every 10 seconds. The receiver can
// optionally reply (ack) with a data packet. See the global variables
// before setup() to adjust these values.
//
// The XBees use API firmware. All parameters are set to factory
// default values except the following:
//
// Coordinator (firmware version 4060)
// ID=20171116 (can be any value but must match the router)
// CE=1
// NI=ZC01 (can be any value)
// BD=9600 [3]
// AP=2
//
// Router (firmware version 4060)
// ID=20171116 (can be any value but must match the coordinator)
// CE=0
// NI=ZR01 (can be any value)
// BD=9600 [3]
// AP=2
//
// Jack Christensen 19Nov2017 CC BY-SA
//
// "Sketch to test XBee communication between Arduino and Raspberry Pi" by Jack Christensen is
// licensed under CC BY-SA 4.0, http://creativecommons.org/licenses/by-sa/4.0/

#include <Streaming.h>        // http://arduiniana.org/libraries/streaming/
#include <XBee.h>             // http://code.google.com/p/xbee-arduino/

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
                digitalWrite(LED_BUILTIN, LOW);
                Serial << ms << F(" XBee TX OK ") << ms - m_msTX << F("ms RETRY=");
                Serial << txRetry << F(" DSCY=") << dscyStat << endl;
                break;
            default:
                Serial << ms << F(" XBee TX FAIL ") << ms - m_msTX << F("ms RETRY=");
                Serial << txRetry << F(" DELY=") << delyStat << F(" DSCY=") << dscyStat << endl;
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
                        Serial << ms << F("Received data too long (") << dataLen << F("), truncated to ") << sizeof(m_payload) << endl;
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
                        Serial << endl << ms << F(" Received message seq ") << m_payload.seq << F(" sent at ") << m_payload.time;
                        Serial << F(" \"") << m_payload.message << F("\"") << endl;
                    }
                    else
                    {
                        Serial << endl << ms << F(" Received ack for seq ") << m_payload.ack << F(" sent at ") << m_payload.time;
                        Serial << F(" \"") << m_payload.message << F("\"") << endl;
                    }
                    break;
                }
            default:
                Serial << endl << ms << F(" XBee RX no ACK\n");  //packet received and not ACKed
                break;
            }
            break;

        case MODEM_STATUS_RESPONSE:                   //XBee administrative messages
            getResponse().getModemStatusResponse(m_zbMSR);
            modemStat = m_zbMSR.getStatus();
            Serial << ms << ' ';
            switch (modemStat)
            {
            case HARDWARE_RESET:
                Serial << F("XBee HW reset\n");
                break;
            case ASSOCIATED:
                Serial << F("XBee associated\n");
                break;
            case DISASSOCIATED:
                Serial << F("XBee disassociated\n");
                break;
            default:
                Serial << F("XBee modem status 0x") << _HEX(modemStat) << endl;
                break;
            }            
            break;

        default:                                           //something else we were not expecting
            Serial << ms << ' ' << F("XBee unexpected frame\n");
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
    digitalWrite(LED_BUILTIN, HIGH);
    Serial << endl << m_msTX << F(" XBee TX\n");
}

// global variables
myXBee xb;
const int32_t baudrate(9600);
const uint8_t xbeeReset(7);              // logic 0 on this pin resets the XBee
const uint8_t xbeeFunction(8);           // ground this pin for transmitter/router, leave open for receiver/coordinator
bool isTransmitter(false);

// these variables can be modified to adjust message timing and sending of ack packets by the receiver
const uint32_t txInterval(10000);        // transmission interval, milliseconds
const bool sendAckMessage(true);         // set to true for receiver to send ack packets
const uint32_t ackDelayTime(1000);       // delay time before sending ack packet

void setup(void)
{
    pinMode(xbeeFunction, INPUT_PULLUP);
    pinMode(LED_BUILTIN, OUTPUT);
    Serial.begin(baudrate);
    xb.begin(Serial);
    Serial << F( "\n" __FILE__ " " __DATE__ " " __TIME__ "\n" );
    isTransmitter = !digitalRead(xbeeFunction);
    if (isTransmitter)
    {
        Serial << F("\nTransmitter (XBee Router)\n");
    }
    else
    {
        Serial << F("\nReceiver (XBee Coordinator)\n");
    }
    pinMode(xbeeReset, OUTPUT);
    delay(10);
    digitalWrite(xbeeReset, HIGH);
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

