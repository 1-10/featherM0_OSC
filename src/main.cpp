#include <Arduino.h>

// Settings are stored here
#include <setting.h>

// debug function
#include <utils/Debug.h>
Debug debug(&Serial, DEBUG_LEVEL);


// /********************************** ETHERNET SETTINGS *************************************/
#include <Ethernet.h>
#include <EthernetUdp.h>

#include "FeatherOSC.h"

// The IP and MAC address conitainers, the indivisual addresses are defined in the setting.h
byte mac[] = {
  MAC_ADDRESS[0], MAC_ADDRESS[1], MAC_ADDRESS[2], MAC_ADDRESS[3], MAC_ADDRESS[4], MAC_ADDRESS[5]
};
  // MAC_ADDRESS_1, MAC_ADDRESS_2, MAC_ADDRESS_3, MAC_ADDRESS_4, MAC_ADDRESS_5, MAC_ADDRESS_6

IPAddress ip(IP_ADDRESS[0], IP_ADDRESS[1], IP_ADDRESS[2], IP_ADDRESS[3]);
// IPAddress ip(IP_ADDRESS_1, IP_ADDRESS_2, IP_ADDRESS_3, IP_ADDRESS_4);
unsigned int localPort = LOCAL_PORT;      // local port to listen on

//host device ip and port
IPAddress hostIP(HOST_IP_ADDRESS[0], HOST_IP_ADDRESS[1], HOST_IP_ADDRESS[2], HOST_IP_ADDRESS[3]);
// IPAddress hostIP(HOST_IP_ADDRESS_1, HOST_IP_ADDRESS_2, HOST_IP_ADDRESS_3, HOST_IP_ADDRESS_4);
unsigned int hostPort = HOST_PORT;

// buffers for receiving and sending data. This contiainer is pre-fixed to 24 bytes, might be possible to extend the length, but have't tested yet
// Path: .pio/libdeps/adafruit_feather_m0_express/Ethernet/src/Ethernet.h
// 2200924: Changed to 64 bytes
char packetBuffer[UDP_TX_PACKET_MAX_SIZE];

// An EthernetUDP instance to let us send and receive packets over UDP
EthernetUDP Udp;

FeatherOSC OSC(&Udp);

/********************************** LED SETTINGS **************************************************/
#include <Adafruit_NeoPixel.h>

Adafruit_NeoPixel pixels(NUMPIXELS, PIN_PIXELS, NEO_GRB + NEO_KHZ800);
struct LED led = {0x01,0,0,0};

/********************************** VARIABLES **************************************************/
bool buttonState = false;
uint32_t updateTimeStamp_ms = 0;

void turnOn_LED(){
  digitalWrite(LED_BUILTIN, HIGH);
  }

void turnOff_LED(){
  digitalWrite(LED_BUILTIN, LOW);
  }


/**********************************************************
 * @bref checks input switch
 * ********************************************************/
void checkInputSwitch(){
  int switchState = digitalRead(INPUT_SWITCH);
  if(switchState == 0 && buttonState == 1){
    debug.println("Input Switch pressed!", DEBUG_ACTION);
  }
  buttonState = switchState;
}

/***************************************
 * @bref set color and control neopixel
 * **************************************/
void controlNeoPixel(){
  for (uint8_t i = 0; i < NUMPIXELS; i++) {
    pixels.setPixelColor(i, pixels.Color(led.r, led.g, led.b));
    }
  pixels.show();
}

/***************************************
 * @bref set color and control neopixel
 * **************************************/
void controlNeoPixel(uint8_t r, uint8_t g, uint8_t b)
{
  for (uint8_t i = 0; i < NUMPIXELS; i++)
  {
    pixels.setPixelColor(i, pixels.Color(r, g, b));
  }
  pixels.show();
}

/***************************************
 * @bref turn on and off LED indicator
 * **************************************/
void controlLED(enum OSC_MESSAGE action){
  switch(action){
    case LED_ON:
      debug.ledOn();
      break;
    case LED_OFF:
      debug.ledOff();
      break;
    case OSC_NEOPIXEL:
      controlNeoPixel();
      break;
    default:
      break;
  }
}

/***************************************
 * @bref turn on and off LED indicator
 * **************************************/
void controlLED(OSC_DATA_PACKET oscInstruction)
{
  switch (oscInstruction.message)
  {
  case LED_ON:
    debug.ledOn();
    break;
  
  case LED_OFF:
    debug.ledOff();
    break;
  
  case OSC_NEOPIXEL:
    uint8_t rgb[3];
    rgb[0] = (uint8_t)oscInstruction.dataContent[0].intData;
    rgb[1] = (uint8_t)oscInstruction.dataContent[1].intData;
    rgb[2] = (uint8_t)oscInstruction.dataContent[2].intData;
    // controlNeoPixel(oscInstruction);
    controlNeoPixel(rgb[0], rgb[1], rgb[2]);
    break;
  
  default:
    debug.println("could not find message type", DEBUG_OSC);
    break;
  }
}

enum STATUS checkBtnState(){
  enum STATUS stat = NONE;
  if(digitalRead(INPUT_SWITCH) == LOW && !buttonState){
    #if DEBUG_LEVEL & DEBUG_OSC
    debug.ledOn();
    #endif
    buttonState = true;
    stat = BUTTON_PUSHED;
  }
  else if(digitalRead(INPUT_SWITCH) == HIGH && buttonState){
    #if DEBUG_LEVEL & DEBUG_OSC
    debug.ledOff();
    #endif
    buttonState = false;  
  }
  return stat;
}

void setup() {

    Ethernet.init(CS_PIN);
    Ethernet.begin(mac, ip);

    Serial.begin(9600);
    delay(2000);
    debug.println("serial up", DEBUG_GENERAL);

    // Check for Ethernet hardware present
    if (Ethernet.hardwareStatus() == EthernetNoHardware) {
      debug.println("Ethernet shield was not found.  Sorry, bro. Can't run without hardware. :(", DEBUG_GENERAL);
      while (true) {
        delay(1); // do nothing, no point running without Ethernet hardware
      }
    }
    if (Ethernet.linkStatus() == LinkOFF) {
      debug.println("Ethernet cable is not connected.", DEBUG_GENERAL);
    }

    // start UDP
    Udp.begin(localPort);
    OSC.init(hostIP, hostPort);
    debug.println("udp init", DEBUG_GENERAL);

    // onboard LED setting
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(INPUT_SWITCH, INPUT_PULLUP);
    pixels.begin();
    pixels.clear();

    updateTimeStamp_ms = millis();

    // Start up message to PC
    OSC.sendOSCstatus(STARTED);
    // OSC.sendOSCstatus(STARTED, hostIP);
    controlLED(OSC_NEOPIXEL);
    debug.println("osc sent out", DEBUG_GENERAL);
}

void loop() {
    if (millis() - updateTimeStamp_ms > UPDATE_RATE_ms){

    // Check incoming message
    struct OSC_DATA_PACKET oscData = OSC.checkOSCpackets();

    // Do some actions here
    if (oscData.message != ACTION_NONE) {
        controlLED(oscData);
    }

    // Status Response
    enum STATUS machineStatus = checkBtnState();
    if (machineStatus != NONE) OSC.sendOSCstatus(machineStatus);

    // Update timestamp
    updateTimeStamp_ms = millis();
    }
}
