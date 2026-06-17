#pragma once

#include <Arduino.h>
#include <RFM69.h>         //get it here: https://www.github.com/lowpowerlab/rfm69
#include <RFM69_ATC.h> 
#include <List.hpp>
#include <cstdint>


//Match frequency to the hardware version of the radio on your Moteino (uncomment one):
//#define FREQUENCY     RF69_433MHZ
//#define FREQUENCY     RF69_868MHZ
#define FREQUENCY     RF69_868MHZ
//#define ENCRYPTKEY    "DasistmeineVersch" //has to be same 16 characters/bytes on all nodes, not more not less!
#define ENCRYPTKEY    "sampleEncryptKey" //has to be same 16 characters/bytes on all nodes, not more not less!

#define ENABLE_ATC    //comment out this line to disable AUTO TRANSMISSION CONTROL
//*********************************************************************************************
#ifndef NETWORKID
    #define NETWORKID   100
#endif

#ifndef GATEWAYID
    #define GATEWAYID   1
#endif

#ifndef STATSINTERVAL
  #define STATSINTERVAL 60 /* Aktualiesierungsintervallfür Uptime, Signalstärke etc. */
#endif

#define HOMIEVERSION "4.0.0"
#define MAX_PAYLOAD_SIZE 56 

#ifndef SERIALPORT
  #define SERIALPORT 1
#endif

#if (SERIALPORT == 0)
  #define mySerialPrint(...) Serial.print(__VA_ARGS__)
  #define mySerialPrintln(...) Serial.println(__VA_ARGS__)
  #define mySerialPrintf(...) Serial.printf(__VA_ARGS__)
#else if (SERIALPORT == 1)
  #define mySerialPrint(...) Serial1.print(__VA_ARGS__)
  #define mySerialPrintln(...) Serial1.println(__VA_ARGS__)
  #define mySerialPrintf(...) Serial1.printf(__VA_ARGS__)
#endif
#else if (SERIALPORT == 2)
  #define mySerialPrint(...) Serial2.print(__VA_ARGS__)
  #define mySerialPrintln(...) Serial2.println(__VA_ARGS__)
  #define mySerialPrintf(...) Serial2.printf(__VA_ARGS__)
#endif


typedef struct 
{
  uint16_t   myNodes;         // es gibt bis zu 1023 Nodes
  uint8_t   myProperties;
  uint8_t   myKeyWordNumber;
  char      myNachricht[MAX_PAYLOAD_SIZE];
} DTransfer;

// uint8_t bla = sizeof(DTransfer);


const char myKeyWords[][MAX_PAYLOAD_SIZE] ={"",
                            "$name",                                                                                                                                                             // allgemeine Topics
                            "$homie",  "$state",  "$localip", "$mac", "$fw/name", "$fw/version", "$nodes", "$implementation", "$stats", "$stats/interval",  // Topics f. devices
                           "$stats/uptime", "$stats/signal", "$stats/battery",                                                                                                         // Topics f. devices
                           "$type", "$properties", "$array", "$extensions",                                                                                                                         // Topics f. Nodes
                           "$settable", "$unit", "$datatype", "$format", "Zeit"                                                                                                          // Topics f. Properties
                            } ;

enum keywordnummern   {  myVALUE = 0,
                         myNAME,                                                                                                                                                    // allgemeine Topics
                         myHOMIE, mySTATE, myLOCALIP, myMAC, myFWNAME, myFWVERSION, myNODES, myIMPLEMENTATION, mySTATS, mySTATSINTERVAL,  mySTATSUPTIME, mySTATSSIGNAL, mySTATSBATTERY, // Topics f. devices
                         myTYPE, myPROPERTIES, myARRAY, myEXTENSIONS,                                                                                                                                 // Topics f. Nodes
                         mySETTABLE, myUNIT, myDATATYPE, myFORMAT, myZEIT                                                                                                                       // Topics f. Properties
                      };

const String myDatentypen[] = {"integer", "float", "boolean", "string", "enum", "color"};
const String myStati[] = {"init", "ready", "disconnected", "sleeping", "lost", "alert"};
/*
    
    string: This channel can show the received text on the given topic and can send text to a given topic.
    number: This channel can show the received number on the given topic and can send a number to a given topic. It can have a min, max and step values.
    dimmer: This channel handles numeric values as percentages. It can have min, max and step values.
    contact: This channel represents an open/close state of a given topic.
    switch: This channel represents an on/off state of a given topic and can send an on/off value to a given topic.
    colorRGB: This channel handles color values in RGB format. (Deprecated)
    colorHSB: This channel handles color values in HSB format. (Deprecated)
    color: This channel handles color values in HSB, RGB or xyY (x,y,brightness) formats.
    location: This channel handles a location.
    image: This channel handles binary images in common java supported formats (bmp,jpg,png).
    datetime: This channel handles date/time values.
    rollershutter: This channel is for rollershutters.

#
*/

/*
Einheiten :


    °C: Degree Celsius
    °F: Degree Fahrenheit
    °: Degree
    L: Liter
    gal: Galon
    V: Volts
    W: Watt
    A: Ampere
    %: Percent
    m: Meter
    ft: Feet
    Pa: Pascal
    psi: PSI
    #: Count or Amount


*/


bool incomingMessage(uint16_t, DTransfer*);

struct myHomeProperty
{
  uint8_t propertyID;
  uint16_t propertyNodeID;
  char propertyName[MAX_PAYLOAD_SIZE];
  char propertyUnit[MAX_PAYLOAD_SIZE];
  char propertyDataType[MAX_PAYLOAD_SIZE];
  char propertyFormat[MAX_PAYLOAD_SIZE];
  bool propertySettable;
};

struct myHomeNode
{
  char nodeName[MAX_PAYLOAD_SIZE];              // ist hier gleich Nodenummer
  char nodeType[MAX_PAYLOAD_SIZE];              // Enthaelt den Namen
  uint16_t nodeID;
  char nodeProperties[MAX_PAYLOAD_SIZE];        // ID's der Properties, durch Komma getrennt
};

class myHomeSachen
{
  using m_cb = void (*)(uint8_t, DTransfer*); //alias function pointer
  private: 
    m_cb action;
    DTransfer internMessage;
    RFM69_ATC *myRadio;
    char deviceName[MAX_PAYLOAD_SIZE], deviceLocalIp[MAX_PAYLOAD_SIZE], deviceMac[MAX_PAYLOAD_SIZE], deviceFwName[MAX_PAYLOAD_SIZE], deviceFwVersion[MAX_PAYLOAD_SIZE], deviceNodesIDs[MAX_PAYLOAD_SIZE], deviceImplementation[MAX_PAYLOAD_SIZE], 
        deviceStats[MAX_PAYLOAD_SIZE], deviceStatsInterval[MAX_PAYLOAD_SIZE], deviceStatsUptime[MAX_PAYLOAD_SIZE], deviceStatsSignal[MAX_PAYLOAD_SIZE], deviceStatsBattery[MAX_PAYLOAD_SIZE], deviceExtensions[MAX_PAYLOAD_SIZE];
    uint8_t deviceStatus, deviceStatsRFM69Temperatur;


  public:
    myHomeSachen( const char[] ); 
    bool initRadio(uint8_t, uint8_t, uint16_t, bool );
    bool sendToNode(uint16_t); // Zielnode, interner Struktur wird versand
    bool sendNodeinfoToNode(uint16_t, uint16_t, uint8_t, char[]); // Zielnode, NodenID, Type, Nachricht
    bool sendToNode(uint16_t, uint16_t, uint8_t, uint8_t, const char[]);// Zielnode, Node, Properties, KeyWordNumber, Nachricht
    bool getRadioData();
    void loop(void);
    bool sendDeviceInfo(void);
    bool sendNodeInfo(void);
    bool sendPropertiesInfo(void);
    uint8_t getRFM69Temp(void); // Temperatur des RFM69
    bool addNode(const char[], const char[], uint16_t); // name, Nodenummer > 0
    bool addProperty( uint8_t, uint16_t, const char[], const char[], const char[], bool, const char[] ); // Property Nummer, Node Nummer, Property Name, Unit Nummer, Datentyp Nummer, Setzen möglich, Format
    bool sendCharPropertyValue(uint8_t, uint16_t, const char[] ); // Property Nummer, Node Nummer, Value
    bool sendIntPropertyValue(uint8_t, uint16_t, int16_t ); // Property Nummer, Node Nummer, Value
    bool sendFloatPropertyValue(uint8_t, uint16_t, float ); // Property Nummer, Node Nummer, Value
    void printInternMessage(void);
    void setDeviceFwName(const char[]);
    void setDeviceFwVersion(const char[]);
    void setDeviceImplementation(const char[]);
    

};
