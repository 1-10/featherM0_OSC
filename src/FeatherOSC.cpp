#include "FeatherOSC.h"
#include "utils/Debug.h"
#include "setting.h"

extern Debug debug;

FeatherOSC::FeatherOSC(EthernetUDP *u):udp(u)
{
    // // buffers for receiving and sending data. This contiainer is pre-fixed to 24 bytes, might be possible to extend the length, but have't tested yet
    // // Path: .pio/libdeps/adafruit_feather_m0_express/Ethernet/src/Ethernet.h
    // // 2200924: Changed to 64 bytes
    packetBuffer = new char[UDP_TX_PACKET_MAX_SIZE];

    // macAddress = {MAC_ADDRESS_1, MAC_ADDRESS_2, MAC_ADDRESS_3, MAC_ADDRESS_4, MAC_ADDRESS_5, MAC_ADDRESS_6};
    macAddress[0] = MAC_ADDRESS_1;
    macAddress[1] = MAC_ADDRESS_2;
    macAddress[2] = MAC_ADDRESS_3;
    macAddress[3] = MAC_ADDRESS_4;
    macAddress[4] = MAC_ADDRESS_5;
    macAddress[5] = MAC_ADDRESS_6;
    localPort = LOCAL_PORT;
    hostPort = HOST_PORT;

    IPAddress hostIP(HOST_IP_ADDRESS_1, HOST_IP_ADDRESS_2, HOST_IP_ADDRESS_3, HOST_IP_ADDRESS_4);
    IPAddress localIP(IP_ADDRESS_1, IP_ADDRESS_2, IP_ADDRESS_3, IP_ADDRESS_4);

    _hostIP = hostIP;
    _localIP = localIP;

};

    // void FeatherOSC::init(IPAddress local_ip, unsigned int local_port, IPAddress host_ip, unsigned int host_port)
    void FeatherOSC::init()
    {

        Ethernet.init(CS_PIN);
        Ethernet.begin(macAddress, _localIP);
        // Check for Ethernet hardware present
        if (Ethernet.hardwareStatus() == EthernetNoHardware)
        {
            debug.println("Ethernet shield was not found.  Sorry, bro. Can't run without hardware. :(", DEBUG_GENERAL);
            while (true)
            {
                delay(1); // do nothing, no point running without Ethernet hardware
            }
        }
        if (Ethernet.linkStatus() == LinkOFF)
        {
            debug.println("Ethernet cable is not connected.", DEBUG_GENERAL);
        }

        // start UDP
        udp->begin(localPort);
}

bool FeatherOSC::fullCompareAddress(String addr, String matchingText)
{
    if (addr.startsWith(matchingText))
    {
        debug.print("compare address tags: ", DEBUG_OSC);
        debug.print((uint8_t)addr.length(), DEBUG_OSC);
        debug.print(" ", DEBUG_OSC);
        debug.println((uint8_t)matchingText.length(), DEBUG_OSC);

        if (addr.length() == matchingText.length()) return true;
    }
    return false;
}

uint8_t FeatherOSC::parseMachineNo(String addr, String matchingText)
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

uint8_t *FeatherOSC::unpackint32(int32_t int32packet)
{
    static uint8_t retVal[4] = {0};
    retVal[0] = (int32packet >> 24) & 0xFF;
    retVal[1] = (int32packet >> 16) & 0xFF;
    retVal[2] = (int32packet >> 8) & 0xFF;
    retVal[3] = int32packet & 0xFF;
    debug.printNumHex(retVal[0], DEBUG_OSC);
    debug.print("  ", DEBUG_OSC);
    debug.printNumHex(retVal[1], DEBUG_OSC);
    debug.print("  ", DEBUG_OSC);
    debug.printNumHex(retVal[2], DEBUG_OSC);
    debug.print("  ", DEBUG_OSC);
    debug.printlnNumHex(retVal[3], DEBUG_OSC);
    return retVal;
}

/*************************************
 * @bref  Receives and parses UDP-OSC packet.
 *        Currently only 32bit int is implemented. It won't be hard to
 *        handle float and strings though.
 *        see detailed OSC string formatting at : 
*         http://veritas-vos-liberabit.com/trans/OSC/OSC-spec-1_0_examples.html#OSCstrings
 * Input: OSC message over UDP
 * Output:data packet structure OSC_DATA_PACKET
 * TODO:  Implement string
 *        Implement float32
 * *************************************/
OSC_DATA_PACKET FeatherOSC::checkOSCpackets()
{
    OSC_DATA_PACKET oscData;
    oscData.message = ACTION_NONE;
    for (uint8_t i = 0; i < 4; i++)
    {
        oscData.dataContent[i].dataType = OSC_INT32;
        oscData.dataContent[i].intData = 0;
        oscData.dataContent[i].strData = "";
    }

    // if there's data available, read a packet
    uint8_t packetSize = udp->parsePacket();
    if (packetSize)
    {
        debug.print("Received packet of size ", DEBUG_OSC);
        debug.println(packetSize, DEBUG_OSC);
        debug.print("From ", DEBUG_OSC);
        IPAddress remote_IP = udp->remoteIP();
        for (int i = 0; i < 4; i++)
        {
            debug.print(String(remote_IP[i]), DEBUG_OSC);
            if (i < 3)
            {
                debug.print(".", DEBUG_OSC);
            }
        }
        debug.print(", port ", DEBUG_OSC);
        debug.println(udp->remotePort(), DEBUG_OSC);

        // read the packet into packetBufffer
        udp->read(packetBuffer, UDP_TX_PACKET_MAX_SIZE);

#if DEBUG_LEVEL & DEBUG_OSC
        // debug print function
        debug.println("Contents:", DEBUG_OSC);
        debug.println(packetBuffer, DEBUG_OSC);
        // uint8_t j = 0;
        for (uint8_t i = 0; i < UDP_TX_PACKET_MAX_SIZE; i++)
        {
            debug.printNumHex(packetBuffer[i], DEBUG_OSC);
            debug.print("/", DEBUG_OSC);
            debug.print((uint8_t)packetBuffer[i], DEBUG_OSC);
            debug.print("/", DEBUG_OSC);
            debug.print((char)packetBuffer[i], DEBUG_OSC);
            // prettify the debug out put ...
            if ((i + 1) % 4 == 0)
            {
                debug.println("", DEBUG_OSC);
            }
            else
                debug.print(" ", DEBUG_OSC);
        }
        debug.println("", DEBUG_OSC);
#endif

        String address = "";
        bool addressFound = false;
        bool dataCountDone = false;
        uint8_t dataCount = 0;
        String messageTags = "";

        // OSC message container
        int32_t valInt32_OSC_i[OSC_MAX_DATA_COUNTS] = {0};
        uint8_t valInt32Ptr = 0;
        String string_OSC[OSC_MAX_DATA_COUNTS] = {""};
        uint8_t string_OSC_ptr = 0;
        uint8_t dataPosition = 0;
        // check the first charactor of every four bytes
        for (uint8_t parsedHeaderPosition = 0; parsedHeaderPosition < packetSize; parsedHeaderPosition += 4)
        {
            // debug.print("Header: ", DEBUG_OSC);
            // debug.println(parsedHeaderPosition, DEBUG_OSC);
            /****************** address extraction ******************************/
            if (packetBuffer[parsedHeaderPosition] == '/' && !addressFound) // Looks up first '/'
            {
                for (uint8_t j = parsedHeaderPosition + 1; j < packetSize; j++)
                { // Read the address/container name after the "/"
                    // debug.print("j: ", DEBUG_OSC);
                    // debug.print(j, DEBUG_OSC);
                    // debug.print(" ", DEBUG_OSC);
                    // debug.println(packetBuffer[j], DEBUG_OSC);

                    if (!addressFound)
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
                    if (addressFound && !dataCountDone)
                    {
                        // Int tag extraction
                        if (packetBuffer[j] == 'i')
                        {
                            messageTags += 'i';
                            dataCount++;
                        }
                        // Int tag extraction
                        else if (packetBuffer[j] == 's')
                        {
                            messageTags += 's';
                            dataCount++;
                        }
                        // End condition of the loop
                        else if (packetBuffer[j] == char(0))
                        {
                            dataCountDone = true;
                            debug.print("data count: ", DEBUG_OSC);
                            debug.print(dataCount, DEBUG_OSC);
                            debug.print(", tags: ", DEBUG_OSC);
                            debug.println(messageTags, DEBUG_OSC);

                            parsedHeaderPosition = (j / 4 + 1) * 4;
                            // debug.print("Header: ", DEBUG_OSC);
                            // debug.println(parsedHeaderPosition, DEBUG_OSC);

                            dataPosition = parsedHeaderPosition;

                            break; // exit from j's for loop
                        }
                    }
                }
            }
            /****************** Data extraction ******************************/
            if (dataCountDone)
            {
                uint8_t t = (parsedHeaderPosition - dataPosition) / 4;
                // In case the message is Int32
                if (messageTags[t] == 'i')
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
                // In case the message is String
                else if (messageTags[t] == 's')
                {
                    String messageString = "";
                    uint8_t k = parsedHeaderPosition;
                    while (packetBuffer[k] != char(0))
                    {
                        debug.printNumHex((uint8_t)packetBuffer[k], DEBUG_OSC);
                        debug.print("/", DEBUG_OSC);
                        debug.print(packetBuffer[k], DEBUG_OSC);
                        debug.print(" ", DEBUG_OSC);
                        messageString += packetBuffer[k];
                        if (k >= packetSize)
                        {
                            debug.println("Oops, over run", DEBUG_OSC);
                            break;
                        }
                        k++;
                    }
                    if(messageString.length() > 0)
                    {
                        string_OSC[string_OSC_ptr++] = messageString;
                        debug.println("stored string", DEBUG_OSC);
                    }
                    debug.print(" /", DEBUG_OSC);
                    debug.println(messageString, DEBUG_OSC);
                    parsedHeaderPosition = (k / 4) * 4;
                    // parsedHeaderPosition = (k / 4 + 1) * 4;
                    debug.println(parsedHeaderPosition, DEBUG_OSC);
                }
            }
        }

        // Store data to return data container
        if (addressFound)
        {
            debug.print("address found: ", DEBUG_OSC);
            debug.println(address, DEBUG_OSC);

            if (fullCompareAddress(address, OSC_ADDRESS_NEOPIXEL))
            {
                debug.println("osc tag matched", DEBUG_OSC);

                oscData.message = OSC_NEOPIXEL;

                // store data to the oscData container according to the messageTags
                string_OSC_ptr = 0;
                valInt32Ptr = 0;
                for (uint8_t i = 0; i < dataCount; i++)
                {
                    if (messageTags[i] == 'i'){
                        oscData.dataContent[i].intData = valInt32_OSC_i[valInt32Ptr++];
                        oscData.dataContent[i].dataType = OSC_INT32;
                    }
                    else if (messageTags[i] == 's')
                    {
                        if (string_OSC[string_OSC_ptr].length() > 0)
                        {
                            // debug.println("stored string to oscData", DEBUG_OSC);
                            oscData.dataContent[i].strData = string_OSC[string_OSC_ptr++];
                            oscData.dataContent[i].dataType = OSC_STR;
                        }
                    }
                }
                oscData.dataCounts = dataCount;
            }
#ifdef INT32PACK
            // This sequence extracts packed four uint_8_t in single int32 to the original four int8_t s
            if (parseMachineNo(address, OSC_ADDRESS_NEOPIXEL) == MY_NUM)
            {
                oscData.message = OSC_NEOPIXEL;
                debug.println("matched", DEBUG_OSC);
                uint8_t *unpackValuePtr = unpackint32(value_OSC_i[0]);

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
 * Sends out OSC message to the host define wit hostIP and hostPort
 * 
 * *************************************/
void FeatherOSC::sendOSCstatus(enum STATUS stat)
// void FeatherOSC::sendOSCstatus(enum STATUS stat, IPAddress hostIP)
{

    for (uint8_t i = 0; i < UDP_TX_PACKET_MAX_SIZE; i++)
        packetBuffer[i] = 0;
    String OSC_message = "/";
    uint8_t osc_responce = 0;
    switch (stat)
    {
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

    if (0 < sizeof(OSC_message))
    {
        OSC_message += String(MY_NUM);
        snprintf(packetBuffer, OSC_message.length() + 1, OSC_message.c_str());
        uint8_t tabPointer = (OSC_message.length() / 4 + 1) * 4;
        debug.println(tabPointer, DEBUG_OSC);
        packetBuffer[tabPointer++] = ',';
        packetBuffer[tabPointer++] = 'i';
        tabPointer = (tabPointer / 4 + 1) * 4 + 3;

        packetBuffer[tabPointer++] = static_cast<char>(osc_responce);

        debug.println(packetBuffer, DEBUG_OSC);
        for (uint8_t i = 0; i < UDP_TX_PACKET_MAX_SIZE; i++)
        {
            debug.printNumHex((uint8_t)packetBuffer[i], DEBUG_OSC);

            if (i % 4 == 3)
                debug.print("\n", DEBUG_OSC);
            else
                debug.print(" ", DEBUG_OSC);
        }

        udp->beginPacket(_hostIP, hostPort);
        udp->write(packetBuffer, UDP_TX_PACKET_MAX_SIZE);
        udp->endPacket();
    }
}
