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
  MAC_ADDRESS[0], MAC_ADDRESS[1], MAC_ADDRESS[2], MAC_ADDRESS[3], MAC_ADDRESS[4], MAC_ADDRESS[5]
};
  // MAC_ADDRESS_1, MAC_ADDRESS_2, MAC_ADDRESS_3, MAC_ADDRESS_4, MAC_ADDRESS_5, MAC_ADDRESS_6

IPAddress ip(IP_ADDRESS[0], IP_ADDRESS[1], IP_ADDRESS[2], IP_ADDRESS[3]);
// IPAddress ip(IP_ADDRESS_1, IP_ADDRESS_2, IP_ADDRESS_3, IP_ADDRESS_4);
unsigned int localPort = LOCAL_PORT;      // local port to listen on

//host device ip and port
IPAddress host_IP(HOST_IP_ADDRESS[0], HOST_IP_ADDRESS[1], HOST_IP_ADDRESS[2], HOST_IP_ADDRESS[3]);
// IPAddress host_IP(HOST_IP_ADDRESS_1, HOST_IP_ADDRESS_2, HOST_IP_ADDRESS_3, HOST_IP_ADDRESS_4);
unsigned int host_Port = HOST_PORT;

// buffers for receiving and sending data. This contiainer is pre-fixed to 24 bytes, might be possible to extend the length, but have't tested yet
// 2200924: Changed to 32 bytes
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

uint8_t* unpackint32(int32_t int32packet)
{
  static uint8_t retVal[4] = {0};
  retVal[0] = (int32packet >> 24) & 0xFF;
  retVal[1] = (int32packet >> 16) & 0xFF;
  retVal[2] = (int32packet >> 8) & 0xFF;
  retVal[3] = int32packet & 0xFF;
  debug.printNumHex(retVal[0], DEBUG_OSC);
  debug.print(" ", DEBUG_OSC);
  debug.printNumHex(retVal[1], DEBUG_OSC);
  debug.print(" ", DEBUG_OSC);
  debug.printNumHex(retVal[2], DEBUG_OSC);
  debug.print(" ", DEBUG_OSC);
  debug.printlnNumHex(retVal[3], DEBUG_OSC);
  return retVal;
}

uint8_t parseMachineNo(String addr, String matchingText)
{
  uint8_t machineNo = 0;
  if (addr.startsWith(matchingText))
  {
    uint8_t tagSize = sizeof(OSC_ADDRESS_NEOPIXEL) - 1;
    machineNo = uint8_t(addr.charAt(tagSize)) - '0'; // converting char -> ascii -> uint8
    debug.print("machineNo: ", DEBUG_OSC);
    debug.println(machineNo, DEBUG_OSC);
  }
  return machineNo;
}

bool fullCompareAddress(String addr, String matchingText)
{
  if (addr.startsWith(matchingText))
  {
    debug.print("compare address tags: ", DEBUG_OSC);
    debug.println((uint8_t)addr.length(), DEBUG_OSC);
    debug.println((uint8_t)matchingText.length(), DEBUG_OSC);
    if (addr.length() == matchingText.length()) return true;
  }
  return false;
}

/*************************************
 * @bref  Receives and parses UDP-OSC packet.
 *        Currently only 32bit int is implemented. It won't be hard to
 *        handle float and strings though.
 *        see detailed OSC string formatting at : 
*         http://veritas-vos-liberabit.com/trans/OSC/OSC-spec-1_0_examples.html#OSCstrings
 * Input: OSC message
 * Output: LEDColor
 * TODO:  Add multiple number sort system
 *        Implement address container system
 *        Implement string
 *        Implement float32
 * *************************************/
OSC_DATA_PACKET checkOSCpackets(){
// enum OSC_MESSAGE checkOSCpackets(){
  OSC_DATA_PACKET oscData;
  oscData.message = ACTION_NONE;
for(uint8_t i = 0; i < 4; i ++){
    oscData.dataContent[i].dataType = OSC_INT32;
    oscData.dataContent[i].intData = 0;
}

  // enum OSC_MESSAGE OSC_Action = ACTION_NONE;
  // OSC_DATA data[OSC_MAX_DATA_COUNTS] = {NEOPIXEL, OSC_INT32, 0};  // probably wrong way to initialize array of structs

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
    // uint8_t j = 0;
    for(uint8_t i = 0; i < UDP_TX_PACKET_MAX_SIZE; i++){
      debug.printNumHex(packetBuffer[i], DEBUG_OSC);
      debug.print("/", DEBUG_OSC);
      debug.print((uint8_t)packetBuffer[i], DEBUG_OSC);
      debug.print("/", DEBUG_OSC);
      debug.print((char)packetBuffer[i], DEBUG_OSC);
      // prettify the debug out put ...
      if ((i + 1) % 4 == 0){
        debug.println("", DEBUG_OSC);
      }
      else debug.print(" ", DEBUG_OSC);
    }
    debug.println("", DEBUG_OSC);
#endif

    String address = "";
    bool addressFound = false;
    bool dataCountDone = false;
    uint8_t dataCount = 0;

    // OSC message container
    int32_t valInt32_OSC_i[4] = {0, 0, 0, 0};
    uint8_t valInt32Ptr = 0;
    // check the first charactor of every four bytes
    for(uint8_t parsedHeaderPosition = 0; parsedHeaderPosition < packetSize; parsedHeaderPosition += 4){
      debug.print("Header: ", DEBUG_OSC);
      debug.println(parsedHeaderPosition, DEBUG_OSC);
      /****************** address extraction ******************************/
      if (packetBuffer[parsedHeaderPosition] == '/' && !addressFound) // Looks up first '/'
      {
        for (uint8_t j = parsedHeaderPosition + 1; j< packetSize; j ++){     // Read the address/container name after the "/"
          debug.print("j: ", DEBUG_OSC);
          debug.print(j, DEBUG_OSC);
          debug.print(" ", DEBUG_OSC);
          debug.println(packetBuffer[j], DEBUG_OSC);

          if(!addressFound)
          {
            // Append the buffer content to the address String
            if (packetBuffer[j] != char(0) && packetBuffer[j] != ',')
            {
              address += packetBuffer[j];
            }
            // Looks up next address terminator, leave when the ',' is found
            else if (packetBuffer[j] == ',')
            {
              addressFound = true;
              debug.print("Address: ", DEBUG_OSC);
              debug.println(address, DEBUG_OSC);
            }
          }

      /****************** Tag extraction ******************************/
          if(addressFound && !dataCountDone){
            // Check the OSC tag which starts from ','
            // if(packetBuffer[j] == ','){
            //   debug.print("tag found: ", DEBUG_OSC);
            // }

            // Int tag extraction
            if(packetBuffer[j] == 'i'){
              // debug.println("int32", DEBUG_OSC);
              dataCount ++;
            }
            // End condition of the loop
            else if(packetBuffer[j] == char(0)){
              dataCountDone = true;
              debug.print("data count: ", DEBUG_OSC);
              debug.println(dataCount, DEBUG_OSC);
              parsedHeaderPosition = (j / 4 + 1) * 4;
              debug.print("Header: ", DEBUG_OSC);
              debug.println(parsedHeaderPosition, DEBUG_OSC);

              break;  // j's for loop
            }
          }
        }
      }

      /****************** Data extraction ******************************/
      if(dataCountDone)
      {
        int32_t valInt32 = 0;
        for (uint8_t k = 0; k < 4; k++)
        {
          debug.printNumHex((uint8_t)packetBuffer[parsedHeaderPosition + k], DEBUG_OSC);
          debug.print(" ", DEBUG_OSC);
          valInt32 |= packetBuffer[parsedHeaderPosition + k] << (3 - k) * 8;
        }
        debug.print(" /", DEBUG_OSC);
        debug.println((uint16_t)valInt32, DEBUG_OSC);
        valInt32_OSC_i[valInt32Ptr++] = valInt32;
        debug.println(valInt32Ptr, DEBUG_OSC);
      }
    }

  // Store data to return data container
    if (addressFound){
      debug.print("address found: ", DEBUG_OSC);
      debug.println(address, DEBUG_OSC);

      if (fullCompareAddress(address, OSC_ADDRESS_NEOPIXEL))
      {
        debug.println("matched", DEBUG_OSC);

        oscData.message = OSC_NEOPIXEL;
        for (uint8_t i = 0; i < dataCount; i++)
        {
          oscData.dataContent[i].intData = valInt32_OSC_i[i];
          }
      }
#ifdef INT32PACK
      // This sequence extracts packed four uint_8_t in single int32 to the original four int8_t s 
      if (parseMachineNo(address, OSC_ADDRESS_NEOPIXEL) == MY_NUM){
        oscData.message = OSC_NEOPIXEL;
        debug.println("matched", DEBUG_OSC);
        uint8_t* unpackValuePtr = unpackint32(value_OSC_i[0]);

        oscData.dataContent[0].intData = *unpackValuePtr;
        oscData.dataContent[1].intData = *(unpackValuePtr + 1);
        oscData.dataContent[2].intData = *(unpackValuePtr + 2);
        oscData.dataContent[3].intData = *(unpackValuePtr + 3);
      }
      #endif
    }
  }
  return oscData;
}

/*************************************
 * Answer back a reply to the host
 * This fuction checks following variables
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
  controlLED(OSC_NEOPIXEL);
}

void loop() {
  if (millis() - updateTimeStamp_ms > UPDATE_RATE_ms){

    // Check incoming message
    struct OSC_DATA_PACKET oscData = checkOSCpackets();
    // enum OSC_MESSAGE actionLED = checkOSCpackets();

    // Do some actions here
    if (oscData.message != ACTION_NONE) {
      controlLED(oscData);
    }

    // Status Response
    enum STATUS machineStatus = checkBtnState();
    if (machineStatus != NONE) sendOSCstatus(machineStatus);

    // Update timestamp
    updateTimeStamp_ms = millis();
  }
}
