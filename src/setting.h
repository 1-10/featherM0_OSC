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
#define NUMPIXELS 1

struct LED{
    uint8_t r, g, b, w;
};

/******************** OSC Protocol settings ************************/
#define OSC_ADDRESS_START "start"
#define OSC_ADDRESS_BUTTON "button"
#define OSC_ADDRESS_NEOPIXEL "neo"
#define OSC_ADDRESS_LED "led"

#define OSC_JAPANESE 0
#define OSC_ENGLISH 1S
#define OSC_CHINESE 2

// Currently not implemented, but feel free to mod!
// enum OSC_PACKET_TYPE {OSC_INT32, OSC_FLOAT32, OSC_STRING, OSC_NONE};
enum STATUS {STARTED,BUTTON_PUSHED, NONE};
enum ACTION {LED_ON, LED_OFF, NEOPIXEL, ACTION_NONE};


/************** SETTINGS_ETHERNET ***************/
#define CS_PIN 10

// Host port No.
#define HOST_IP_ADDRESS_1 192
#define HOST_IP_ADDRESS_2 168
#define HOST_IP_ADDRESS_3 0
#define HOST_IP_ADDRESS_4 2

#define HOST_PORT 8889

// Put your MAC address here:
#define MAC_ADDRESS_1 0x98
#define MAC_ADDRESS_2 0x76
#define MAC_ADDRESS_3 0xB6
#define MAC_ADDRESS_4 0x11
#define MAC_ADDRESS_5 0x83
#define MAC_ADDRESS_6 0xb3

// And your IP address
#define IP_ADDRESS_1 192
#define IP_ADDRESS_2 168
#define IP_ADDRESS_3 0
#define IP_ADDRESS_4 3

// Local port No.
#define LOCAL_PORT 8888