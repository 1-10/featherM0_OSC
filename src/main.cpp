#include <Arduino.h>

// Settings are stored here
#include <setting.h>

// debug function
#include <utils/Debug.h>
Debug debug(&Serial, DEBUG_LEVEL);


#include <Ethernet.h>
#include <EthernetUdp.h>

// Custom OSC lib
#include "FeatherOSC.h"

// An EthernetUDP instance to let us send and receive packets over UDP
EthernetUDP Udp;

FeatherOSC OSC(&Udp);

/********************************** LED SETTINGS **************************************************/
#include <Adafruit_NeoPixel.h>

Adafruit_NeoPixel pixels(NUMPIXELS, PIN_PIXELS, NEO_GRB + NEO_KHZ800);
// struct LED led = {0x01,0,0,0};

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
// void controlNeoPixel(){
//   for (uint8_t i = 0; i < NUMPIXELS; i++) {
//     pixels.setPixelColor(i, pixels.Color(led.r, led.g, led.b));
//     }
//   pixels.show();
// }

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
// void controlLED(enum OSC_MESSAGE action){
//   switch(action){
//     case LED_ON:
//       debug.ledOn();
//       break;
//     case LED_OFF:
//       debug.ledOff();
//       break;
//     case OSC_NEOPIXEL:
//       controlNeoPixel();
//       break;
//     default:
//       break;
//   }
// }

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

    Serial.begin(9600);

    // OSC.init(localIP, localPort, hostIP, hostPort);
    OSC.init();
    debug.println("udp osc init done", DEBUG_GENERAL);

    // onboard LED setting
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(INPUT_SWITCH, INPUT_PULLUP);
    pixels.begin();
    pixels.clear();

    updateTimeStamp_ms = millis();

    // Start up message to PC
    OSC.sendOSCstatus(STARTED);
    controlNeoPixel(LED_INIT_COLOR_R, LED_INIT_COLOR_G, LED_INIT_COLOR_B);
    debug.println("start msg sent out", DEBUG_GENERAL);
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
