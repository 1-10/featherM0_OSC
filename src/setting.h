/******************** Mode ************************/
// Debug mode settings: see debug.h for detail Following functions are implemented
#include "utils/Debug.h"
#define DEBUG_LEVEL DEBUG_GENERAL | DEBUG_OSC

/******************** Machine Config ************************/
#define INPUT_SWITCH 5
//update rate inside void loop()
#define UPDATE_RATE_ms 10

#define MY_NUM 1

#define PIN_PIXELS 8
#define NUMPIXELS 2

struct LED{
    uint8_t r, g, b, w;
};


/******************** OSC Protocol settings ************************/
#define OSC_ADDRESS_START "start"
#define OSC_ADDRESS_BUTTON "button"
#define OSC_ADDRESS_NEOPIXEL "neo"
#define OSC_ADDRESS_LED "led"

#define OSC_JAPANESE 0
#define OSC_ENGLISH 1
#define OSC_CHINESE 2

enum STATUS {STARTED,BUTTON_PUSHED, NONE};
enum OSC_MESSAGE {LED_ON, LED_OFF, OSC_NEOPIXEL, ACTION_NONE};
enum OSC_DATA_TYPE {OSC_INT32, OSC_STR, OSC_FLOAT};

#define OSC_MAX_DATA_COUNTS 4
// #define INT32_PACK

// Data container for the incoming osc data
// dataType, intData
struct OSC_DATA{
    OSC_DATA_TYPE dataType;
    int32_t intData;
    // String strData;
    // float floatData;
};

// Data packet structure
// data counts, message: osc message in OSC_MESSAGE structure, dataContent: message content as OSC_DATA structure
struct OSC_DATA_PACKET{
    uint8_t dataCounts = OSC_MAX_DATA_COUNTS;
    OSC_MESSAGE message;
    OSC_DATA dataContent[OSC_MAX_DATA_COUNTS];
};


/************** SETTINGS_ETHERNET ***************/
#define CS_PIN 10

// Host port No.
// #define HOST_IP_ADDRESS_1 192
// #define HOST_IP_ADDRESS_2 168
// #define HOST_IP_ADDRESS_3 0
// #define HOST_IP_ADDRESS_4 2

const byte HOST_IP_ADDRESS[4] = {192, 168, 0, 2};

#define HOST_PORT 8889

// Put your MAC address here:
// #define MAC_ADDRESS_1 0x98
// #define MAC_ADDRESS_2 0x76
// #define MAC_ADDRESS_3 0xB6
// #define MAC_ADDRESS_4 0x11
// #define MAC_ADDRESS_5 0xab
// #define MAC_ADDRESS_6 0xd5
const byte MAC_ADDRESS[6] = {0x98, 0x76, 0x86, 0x11, 0xab, 0xd5};

// And your IP address
// #define IP_ADDRESS_1 192
// #define IP_ADDRESS_2 168
// #define IP_ADDRESS_3 0
// #define IP_ADDRESS_4 3
const byte IP_ADDRESS[4] = {192, 168, 0, 3};

// Local port No.
#define LOCAL_PORT 8888