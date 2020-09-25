#ifndef _FEATHEROSC_H_
#define _FEATHEROSC_H_

#include <Arduino.h>
#include <Ethernet.h>
#include <EthernetUdp.h>

/******************** Ethernet and OSC Protocol settings ************************/
// CS pin for the Feather Ethernet board
#define CS_PIN 10

// Put your MAC address here:
#define MAC_ADDRESS_1 0x98
#define MAC_ADDRESS_2 0x76
#define MAC_ADDRESS_3 0xB6
#define MAC_ADDRESS_4 0x11
#define MAC_ADDRESS_5 0xab
#define MAC_ADDRESS_6 0xd5

// Local IP and PortNo.
#define IP_ADDRESS_1 192
#define IP_ADDRESS_2 168
#define IP_ADDRESS_3 0
#define IP_ADDRESS_4 3
#define LOCAL_PORT 8888

// Host IP and Port
#define HOST_IP_ADDRESS_1 192
#define HOST_IP_ADDRESS_2 168
#define HOST_IP_ADDRESS_3 0
#define HOST_IP_ADDRESS_4 2
#define HOST_PORT 8889

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
        // void init(IPAddress lcoalIP, unsigned int lcoalPort, IPAddress hostIP, unsigned int hostPort);
        void init();
        OSC_DATA_PACKET checkOSCpackets();
        void sendOSCstatus(STATUS);

    private : 
        bool fullCompareAddress(String, String);
        uint8_t parseMachineNo(String, String);
        uint8_t *unpackint32(int32_t);
        byte macAddress[6];
        IPAddress _localIP;
        IPAddress _hostIP;
        unsigned int localPort;
        unsigned int hostPort;
        char *packetBuffer;
        EthernetUDP *udp;

};

#endif