#include <Arduino.h>

// Settings are stored here
#include <setting.h>

// debug function
#include <utils/Debug.h>
Debug debug(&Serial, DEBUG_LEVEL);

/********************************** ETHERNET SETTINGS *************************************/
#include <Ethernet.h>
#include <EthernetUdp.h>


// The IP and MAC address conitainers, the indivisual addresses are defined in the setting.h
byte mac[] = {
  MAC_ADDRESS_1, MAC_ADDRESS_2, MAC_ADDRESS_3, MAC_ADDRESS_4, MAC_ADDRESS_5, MAC_ADDRESS_6
};

IPAddress ip(IP_ADDRESS_1, IP_ADDRESS_2, IP_ADDRESS_3, IP_ADDRESS_4);
unsigned int localPort = LOCAL_PORT;      // local port to listen on

//host device ip and port
IPAddress host_IP(HOST_IP_ADDRESS_1,HOST_IP_ADDRESS_2,HOST_IP_ADDRESS_3,HOST_IP_ADDRESS_4);
unsigned int host_Port = HOST_PORT;

// buffers for receiving and sending data. This contiainer is pre-fixed to 24 bytes, might be possible to extend the length, but have't tested yet
char packetBuffer[UDP_TX_PACKET_MAX_SIZE];

// An EthernetUDP instance to let us send and receive packets over UDP
EthernetUDP Udp;

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
 * @bref turn on and off LED indicator
 * **************************************/
void controlLED(enum ACTION action){
  switch(action){
    case LED_ON:
      debug.ledOn();
      break;
    case LED_OFF:
      debug.ledOff();
      break;
    case NEOPIXEL:
      controlNeoPixel();
      break;
    default:
      break;
  }
}

/*************************************
 * @bref  Receives and parses UDP-OSC packet.
 *        Currently only 32bit int is implemented. It won't be hard to
 *        handle float and strings though.
 *        see detailed OSC string formatting at : 
*         http://veritas-vos-liberabit.com/trans/OSC/OSC-spec-1_0_examples.html#OSCstrings
 * Input: OSC message
 * Output: LEDColor
 * *************************************/
enum ACTION checkOSCpackets(){

  enum ACTION OSC_Action = ACTION_NONE;
  // if there's data available, read a packet
  uint8_t packetSize = Udp.parsePacket();
  if(packetSize)
    {
    debug.print("Received packet of size ", DEBUG_OSC);
    debug.println(packetSize, DEBUG_OSC);
    debug.print("From ", DEBUG_OSC);
    IPAddress remote_IP = Udp.remoteIP();
    for(int i =0; i < 4; i++)
    {
      debug.print(String(remote_IP[i]), DEBUG_OSC);
      if (i < 3)
      {
        debug.print(".", DEBUG_OSC);
      }
    }
    debug.print(", port ", DEBUG_OSC);
    debug.println(Udp.remotePort(), DEBUG_OSC);
    
    // read the packet into packetBufffer
    Udp.read(packetBuffer, UDP_TX_PACKET_MAX_SIZE);

#if DEBUG_LEVEL & DEBUG_OSC
    // debug print function
    debug.println("Contents:", DEBUG_OSC);
    debug.println(packetBuffer, DEBUG_OSC);
    uint8_t j = 0;
    for(int i =0; i < UDP_TX_PACKET_MAX_SIZE; i++){
      debug.printNumHex(packetBuffer[i], DEBUG_OSC);
      debug.print("/", DEBUG_OSC);
      debug.print((uint8_t)packetBuffer[i], DEBUG_OSC);
      j ++;
      if (j >= 4){
        j = 0;
        debug.println("", DEBUG_OSC);
      }
      else debug.print(" ", DEBUG_OSC);
    }
    debug.println("", DEBUG_OSC);
#endif

    String address;
    bool addressFound = false;
    uint8_t dataCount = 0;
    // OSC message container
    int value_OSC_i[4] = {0,0,0,0};
    // check the first charactor of every four bytes
    for(int i = 0; i < packetSize; i+=4){
        if (packetBuffer[i] == '/' && !addressFound){
          for (uint8_t j = i + 1; j< packetSize; j ++){
            if (packetBuffer[j] == ','){  // Looks up next address terminator
              addressFound = true;
              break;
            }
            else address += packetBuffer[j];     // Append the buffer content to the address String
          }
        }
        // Check the OSC tag which starts from ','
        else if(packetBuffer[i] == ','){
          debug.println("tag found", DEBUG_OSC);
          // If the packet type is int32
          if(packetBuffer[i + 1] == 'i'){
            debug.println("int tag", DEBUG_OSC);
            
            for (uint8_t j = i + 1; j < i + 4 + 1; j ++){
              if(packetBuffer[j] == 'i'){
                dataCount ++;
              }
            }
            for (uint8_t j = 0; j < dataCount; j ++){
              int value = 0;
              for (uint8_t k = 0; k < 4; k ++){
                debug.printlnNumHex((uint8_t)packetBuffer[i + 4 + k], DEBUG_OSC);
                value |= packetBuffer[i + 4 + k] << (3 - k)*8;
              }
              debug.println((uint16_t)value, DEBUG_OSC);
              value_OSC_i[j] = value;
            }
          }
        }
    }
    if (addressFound){
      debug.print("address found: ", DEBUG_OSC);
      debug.print(dataCount, DEBUG_OSC);
      debug.println(address, DEBUG_OSC);
      // Check address and dev number
      if (address.startsWith(OSC_ADDRESS_NEOPIXEL) && dataCount == 1){
        uint8_t tagSize = sizeof(OSC_ADDRESS_NEOPIXEL) -1;
        uint8_t ballNum = uint8_t(address.charAt(tagSize)) - '0'; // converting char -> ascii -> uint8
        debug.println(ballNum, DEBUG_OSC);
        if (ballNum == MY_NUM){
          OSC_Action = NEOPIXEL;
          debug.println("matched", DEBUG_OSC);
          led.r = (value_OSC_i[0] >> 24) & 0xFF;
          led.g = (value_OSC_i[0] >> 16) & 0xFF;
          led.b = (value_OSC_i[0] >> 8) & 0xFF;
          led.w = value_OSC_i[0] & 0xFF;

          debug.printNumHex(led.r, DEBUG_OSC);
          debug.print(" ", DEBUG_OSC);
          debug.printNumHex(led.g, DEBUG_OSC);
          debug.print(" ", DEBUG_OSC);
          debug.printNumHex(led.b, DEBUG_OSC);
          debug.print(" ", DEBUG_OSC);
          debug.printlnNumHex(led.w, DEBUG_OSC);
        }
      }
      if (address.startsWith(OSC_ADDRESS_LED) && dataCount == 1){
        uint8_t tagSize = sizeof(OSC_ADDRESS_NEOPIXEL) -1;
        uint8_t standbyNum = uint8_t(address.charAt(tagSize)) - '0'; // converting char -> ascii -> uint8
        if (standbyNum == MY_NUM){
          uint8_t val = value_OSC_i[0] & 0x1;
          debug.println("LED", DEBUG_OSC);

          if (val) OSC_Action = LED_ON;
          else OSC_Action = LED_OFF;

        }
      }  
    }  
  }
  // return gotOSCMessage;
  return OSC_Action;
}

/*************************************
 * Answer back a reply to the host
 * This fuction checks following variables
 * - (brightmode)
 * - pushButton
 * 
 * *************************************/
  void sendOSCstatus(enum STATUS stat){
  // #if DEBUG_LEVEL & DEBUG_OSC
  // debug.ledOn();
  // #endif

  for (uint8_t i = 0; i < UDP_TX_PACKET_MAX_SIZE; i ++) packetBuffer[i] = 0;
  String OSC_message = "/";
  uint8_t osc_responce = 0;
  switch (stat){
    case STARTED:
      OSC_message += OSC_ADDRESS_START;
      osc_responce = 1;
    break;
    case BUTTON_PUSHED:
      OSC_message += OSC_ADDRESS_BUTTON;
      osc_responce = 1;
    break;
    case NONE:
      OSC_message = "";
      osc_responce = 0;
    break;
    default:
      OSC_message = "";
      osc_responce = 0;
    return;
    break;
  }

  if (0 < sizeof(OSC_message)){
    OSC_message += String(MY_NUM);
    snprintf(packetBuffer, OSC_message.length() + 1, OSC_message.c_str());
    uint8_t tabPointer = (OSC_message.length()/4 + 1) * 4;
    debug.println(tabPointer, DEBUG_OSC);
    packetBuffer[tabPointer++] = ',';
    packetBuffer[tabPointer++] = 'i';
    tabPointer = (tabPointer/4 + 1)*4 + 3;

    packetBuffer[tabPointer++] = static_cast<char>(osc_responce);

    debug.println(packetBuffer, DEBUG_OSC);
    for (uint8_t i = 0; i < UDP_TX_PACKET_MAX_SIZE; i ++){
      debug.printNumHex((uint8_t)packetBuffer[i], DEBUG_OSC);
      
      if (i%4 == 3) debug.print("\n", DEBUG_OSC);
      else debug.print(" ", DEBUG_OSC);
    }

    Udp.beginPacket(host_IP, host_Port);
    Udp.write(packetBuffer, UDP_TX_PACKET_MAX_SIZE);
    Udp.endPacket();
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

  // onboard LED setting
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(INPUT_SWITCH, INPUT_PULLUP);
  pixels.begin();
  pixels.clear();

  updateTimeStamp_ms = millis();

  // Start up message to PC
  sendOSCstatus(STARTED);
  // controlLED(LED_ON);
  controlLED(NEOPIXEL);
}

void loop() {
  if (millis() - updateTimeStamp_ms > UPDATE_RATE_ms){

    // Check incoming message
    enum ACTION actionLED = checkOSCpackets();

    // Do some actions here
    if (actionLED != ACTION_NONE) {
      controlLED(actionLED);
    }

    // Status Response
    enum STATUS machineStatus = checkBtnState();
    if (machineStatus != NONE) sendOSCstatus(machineStatus);

    // Update timestamp
    updateTimeStamp_ms = millis();
  }
}
