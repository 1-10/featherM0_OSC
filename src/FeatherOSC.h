#ifndef _FEATHEROSC_H_
#define _FEATHEROSC_H_

#include <Arduino.h>
#include <Ethernet.h>
#include <EthernetUdp.h>

/******************** Ethernet and OSC Protocol settings ************************/
// CS pin for the Feather Ethernet board
#define CS_PIN 10

#define OSC_ADDRESS_START "start"
#define OSC_ADDRESS_BUTTON "button"
#define OSC_ADDRESS_NEOPIXEL "neo"
#define OSC_ADDRESS_LED "led"

enum STATUS
{
    STARTED,
    BUTTON_PUSHED,
    NONE
};
enum OSC_MESSAGE
{
    LED_ON,
    LED_OFF,
    OSC_NEOPIXEL,
    ACTION_NONE
};
enum OSC_DATA_TYPE
{
    OSC_INT32,
    OSC_STR,
    OSC_FLOAT
};

#define OSC_MAX_DATA_COUNTS 4
// #define INT32_PACK

// Data container for the incoming osc data
// dataType, intData
struct OSC_DATA
{
    OSC_DATA_TYPE dataType;
    int32_t intData;
    String strData;
    // float floatData;
};

// Data packet structure
// data counts, message: osc message in OSC_MESSAGE structure, dataContent: message content as OSC_DATA structure
struct OSC_DATA_PACKET
{
    uint8_t dataCounts = OSC_MAX_DATA_COUNTS;
    OSC_MESSAGE message;
    OSC_DATA dataContent[OSC_MAX_DATA_COUNTS];
};


class FeatherOSC{

    public:
        FeatherOSC(EthernetUDP *u); // Intializer
        void init(IPAddress hostIP, unsigned int hostPort);
        OSC_DATA_PACKET checkOSCpackets();
        void sendOSCstatus(STATUS);

    private : 
        bool fullCompareAddress(String, String);
        uint8_t parseMachineNo(String, String);
        uint8_t *unpackint32(int32_t);
        byte mac[6];
        // IPAddress ip;
        // unsigned int localPort;
        unsigned int hostPort;
        IPAddress hostIP;
        char *packetBuffer;
        EthernetUDP *udp;

};

#endif