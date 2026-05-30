#include "mytransmit.h"
//#include <List.hpp>
#include "HardwareSerial.h"
#include "RFM69.h"
#include "WSerial.h"
#include "WString.h"
#include "pgmspace.h"
#include "wiring_time.h"
#include <cstdint>
#include <cstdio>
#include <cstring>

/*
ToDo:
  properties automatisch erzeugen
  nodes automatisch erzeugen
  nodeinfo senden
  iteminfo senden
  Anpassen Item struct -> Unit in char statt int

*/

List<myHomeProperty> myHomieProperies;  // Liste fuer die Properties
List<myHomeNode> myHomieNodes;          // Liste für die Nodes


myHomeSachen::myHomeSachen( const char myName[] )
{
  action = (m_cb)incomingMessage;
  
  if(snprintf(deviceName,MAX_PAYLOAD_SIZE,"%s",myName) == MAX_PAYLOAD_SIZE)
    deviceName[MAX_PAYLOAD_SIZE-1] = '\0';
  
  strncpy(deviceNodesIDs,"",MAX_PAYLOAD_SIZE);
  //strncpy(deviceLocalIp,"",MAX_PAYLOAD_SIZE);
  //strncpy(deviceMac,"",MAX_PAYLOAD_SIZE);
  //strncpy(deviceFwName,"dummy Device",MAX_PAYLOAD_SIZE);
  //strncpy(deviceFwVersion,"0.0.1",MAX_PAYLOAD_SIZE);
  strncpy(deviceImplementation,"STM32F103c8t6 mini",MAX_PAYLOAD_SIZE);
  //strncpy(deviceStatsUptime,"0",MAX_PAYLOAD_SIZE);
  //strncpy(deviceStatsSignal,"0",MAX_PAYLOAD_SIZE);
  //strncpy(deviceStatsBattery,"0",MAX_PAYLOAD_SIZE);
  //snprintf(deviceStatsInterval, MAX_PAYLOAD_SIZE, "%d",STATSINTERVAL);
  strncpy(deviceExtensions," ",MAX_PAYLOAD_SIZE);

}

bool myHomeSachen::initRadio(uint8_t ssPin, uint8_t irqPin, uint16_t nodeID, bool myIsHW )
{
  Serial1.println("Entering initRadio");

  myRadio = new RFM69_ATC(ssPin, irqPin, myIsHW);
  //myRadio = new RFM69_ATC();
  if( !myRadio->initialize(FREQUENCY,nodeID,NETWORKID) )
    return false;
  else{
    myRadio->encrypt(ENCRYPTKEY);
    
    #ifdef IS_RFM69HW_HCW
    Serial1.println("Setting HigPower");
    myRadio->setHighPower(); //must include this only for RFM69HW/HCW!
    #endif

    return true;
  }
}


bool myHomeSachen::sendToNode(uint8_t destNode)
{

  Serial1.print("Sending to Gateway : ");
  Serial1.println(destNode);
  Serial1.print("Sending ...");
  if (myRadio->sendWithRetry(destNode, (const void*)(&internMessage), sizeof(DTransfer),2 , 100))
  {
    Serial1.println("\nok.\n");
    return true;
  }
  else
  {
    Serial1.println("\nfehlgeschlagen!!\n");
    return false;
  }
}

bool myHomeSachen::sendNodeinfoToNode(uint8_t destNode, uint8_t myNodeID, uint8_t myType, char myNachricht[]) // gateway, Nodenummer, Name, Type, Nachricht
{
  bool ergebnis;
  uint8_t laenge;

  internMessage.myNodes = myNodeID;
  internMessage.myProperties = 0;
  internMessage.myKeyWordNumber = myType;
  if(snprintf(internMessage.myNachricht,MAX_PAYLOAD_SIZE,"%s",myNachricht) >= MAX_PAYLOAD_SIZE)
    internMessage.myNachricht[MAX_PAYLOAD_SIZE-1] = '\0';

  Serial1.println(("In sendNodeinfoToNode"));
  printInternMessage();
  
  if(sendToNode(destNode))
    ergebnis = true;
  else
    ergebnis = false;  
  

  return ergebnis;
}

bool myHomeSachen::sendToNode(uint8_t destNode, uint8_t myDummyChanel, uint8_t myDummyItem, uint8_t myDummyKeyWordNumber, const char myDummyNachricht[])
{
  internMessage.myNodes = myDummyChanel;
  internMessage.myProperties = myDummyItem;
  internMessage.myKeyWordNumber = myDummyKeyWordNumber;
 
  if(snprintf(internMessage.myNachricht,MAX_PAYLOAD_SIZE,"%s",myDummyNachricht) >= MAX_PAYLOAD_SIZE)
    internMessage.myNachricht[MAX_PAYLOAD_SIZE-1] = '\0';
 

  Serial1.println(("In SendToNode"));
  printInternMessage();

  if(sendToNode(destNode))
    return true;
  else
    return false;
}

void myHomeSachen::loop(void)
{
  uint8_t  temperatur;
     if(myRadio->receiveDone() && myRadio->DATALEN == sizeof(DTransfer))
    {
        Serial1.println("Message received");
        memcpy(&internMessage, myRadio->DATA, sizeof(internMessage)); 
        if (myRadio->ACKRequested())
        {
            myRadio->sendACK();
            Serial1.println(" - ACK sent");
            delay(10);
        }  
        deviceStatsRFM69Temperatur = myRadio->readTemperature();

        action(myRadio->getAddress(), &internMessage);
    } 
}

bool myHomeSachen::sendDeviceInfo(void)
{
  
  String myBoolean;
  bool sendenok = true;

  Serial1.println("\nSending Device infos...\n");
  if(!sendToNode(GATEWAYID, 0, 0, myHOMIE, HOMIEVERSION))                   // $homie
    sendenok  = false;
  if(!sendToNode(GATEWAYID, 0, 0, myNAME, deviceName))                      // $name
    sendenok  = false;
  if(!sendToNode(GATEWAYID, 0, 0, mySTATE, "init"))                         // $state
    sendenok  = false;
/*  
    if(!sendToNode(GATEWAYID, 0, 0, myLOCALIP, ""))                           // $localip
    sendenok  = false;
  if(!sendToNode(GATEWAYID, 0, 0, myMAC, ""))                               // $mac
    sendenok  = false;
  if(!sendToNode(GATEWAYID, 0, 0, myFWNAME, deviceFwName))                  // $fw/name
    sendenok  = false;
  if(!sendToNode(GATEWAYID, 0, 0, myFWVERSION, deviceFwVersion))            // $fw/version
    sendenok  = false;
*/  
  if(!sendToNode(GATEWAYID, 0, 0, myNODES, deviceNodesIDs))
    {                 // $nodes
      sendenok  = false;
    }
  if(!sendToNode(GATEWAYID, 0, 0, myEXTENSIONS, deviceExtensions))                 // $nodes
      sendenok  = false;
/*  
    if(!sendToNode(GATEWAYID, 0, 0, myIMPLEMENTATION, deviceImplementation))  // $implementation
    sendenok  = false;
  if(!sendToNode(GATEWAYID, 0, 0, mySTATS, deviceStats))                    // $stats
    sendenok  = false;
  if(!sendToNode(GATEWAYID, 0, 0, mySTATSINTERVAL, deviceStatsInterval))    // $stats/interval
    sendenok  = false;
  if(!sendToNode(GATEWAYID, 0, 0, mySTATSUPTIME, deviceStatsUptime))        // $stats/uptime
    sendenok  = false;
  if(!sendToNode(GATEWAYID, 0, 0, mySTATSSIGNAL, deviceStatsSignal))        // $stats/signal
    sendenok  = false;
  if(!sendToNode(GATEWAYID, 0, 0, mySTATSBATTERY, deviceStatsBattery))      // $stats/battery
    sendenok  = false;
*/
  return sendenok;
}

bool myHomeSachen::sendNodeInfo(void)
{
  myHomeNode  pNode;
  bool sendenok = true;

  if(!myHomieNodes.isEmpty())
  {
    Serial1.println("\nSending Node infos...");
    Serial1.print("Number of Nodes : ");
    Serial1.println(myHomieNodes.getSize());
    Serial1.println();

    for( int lauf = 0; lauf < myHomieNodes.getSize(); lauf++)
    {
      pNode = myHomieNodes[lauf];

      // sendToNode(uint8_t destNode, uint8_t myNodeNummer, String myType, String myEigenschaften) // gateway, Nodenummer, Type, Nachricht
      if(!sendNodeinfoToNode(GATEWAYID, pNode.nodeID, myNAME, pNode.nodeName ))
        sendenok  = false;

      if(!sendNodeinfoToNode(GATEWAYID, pNode.nodeID,myTYPE, pNode.nodeType ))
        sendenok  = false;

      if(!sendNodeinfoToNode(GATEWAYID, pNode.nodeID, myPROPERTIES, pNode.nodeProperties ))
        sendenok  = false;

    }
  }

  return sendenok;
}

bool myHomeSachen::sendPropertiesInfo(void)
{
  bool sendenok = true;
  myHomeProperty pProperty;

  if(!myHomieProperies.isEmpty())
  {
    Serial1.println("\nSending Properties infos...\n");
    for( int lauf = 0; lauf < myHomieProperies.getSize(); lauf++)
    {
      pProperty = myHomieProperies[lauf];
      internMessage.myNodes = pProperty.propertyNodeID;
      internMessage.myProperties = pProperty.propertyID;
      internMessage.myKeyWordNumber = myNAME;
      strcpy(internMessage.myNachricht,pProperty.propertyName);

      if(!sendToNode(GATEWAYID))
        sendenok  = false;

      internMessage.myKeyWordNumber = myUNIT;
      strcpy(internMessage.myNachricht,pProperty.propertyUnit);
      if(!sendToNode(GATEWAYID))
        sendenok  = false;

      internMessage.myKeyWordNumber = myDATATYPE;
      strcpy(internMessage.myNachricht,pProperty.propertyDataType);
      if(!sendToNode(GATEWAYID))
        sendenok  = false;

      internMessage.myKeyWordNumber = mySETTABLE;
      if(pProperty.propertySettable == true)
        strcpy(internMessage.myNachricht,"true");    
      else
        strcpy(internMessage.myNachricht,"false");

      if(!sendToNode(GATEWAYID))
        sendenok  = false;

      if(strlen(pProperty.propertyFormat) > 0)
      {
        internMessage.myKeyWordNumber = myFORMAT;
        strcpy(internMessage.myNachricht,pProperty.propertyFormat);
        if(!sendToNode(GATEWAYID))
          sendenok  = false;
      }

    }
  }
  return sendenok;
}

bool myHomeSachen::addNode(const char myName[], const char myType[], uint8_t Nummer)
{
  myHomeNode dummyNode;
  dummyNode.nodeProperties[0] = '\0';
  char buffer[MAX_PAYLOAD_SIZE];

  Serial1.print("Adding Node : ");
  Serial1.print( myName );
  Serial1.print(" with Number : ");
  Serial1.println(Nummer);

  if(Nummer > 0) // Nodenummern müsser größer als 0 sein
  {

    if(strlen(deviceNodesIDs) == 0)  // erster Node
    {
      sprintf(deviceNodesIDs,"%d",Nummer);
    }
    else
    { 
        sprintf(buffer, ",%d",Nummer);
        Serial.printf("Bufferlaenge : %d, NodeiDslaenge %d\n",strlen(buffer),strlen(deviceNodesIDs));
        if(strlen(buffer) + strlen(deviceNodesIDs) < MAX_PAYLOAD_SIZE)
          strcat(deviceNodesIDs, buffer);
    
    }

    Serial1.println(String("myNodes : ") + deviceNodesIDs);

    dummyNode.nodeID = Nummer;

    if(snprintf(dummyNode.nodeType, MAX_PAYLOAD_SIZE,"%s",myType) == MAX_PAYLOAD_SIZE)
      dummyNode.nodeType[MAX_PAYLOAD_SIZE -1] = '\0';

    dummyNode.nodeProperties[0] = '\0';
    if(snprintf(dummyNode.nodeName, MAX_PAYLOAD_SIZE,"%s",myName) == MAX_PAYLOAD_SIZE)
      dummyNode.nodeName[MAX_PAYLOAD_SIZE -1] = '\0';
    

    myHomieNodes.add(dummyNode);
    return true;
  }
  else
    return false;
}

bool myHomeSachen::addProperty( uint8_t pNummer, uint8_t nNummer, const char pName[], const char pUnit[], const char pDataType[], bool pSettable, const char pFormat[]) // Property Nummer, Node Nummer, Property Name, Unit Nummer, Datentyp Nummer, Setzen möglich
{
  myHomeProperty hProperty;
  myHomeNode hNode;

  Serial1.printf("Adding Property : %d\n",pNummer);
  Serial1.printf("Nodenummer      : %d\n",nNummer);
  Serial1.printf("Propertyname    : %s\n", pName);
  Serial1.printf("Propertyunit    : %s\n", pUnit);
  Serial1.printf("Propertydatatype: %s\n", pDataType);
  Serial1.printf("Propertysettable: %d\n",pSettable); 
  


  if(myHomieNodes.getSize() > 0 && strlen(pName) > 0) // Name muss Zeichen enthalten und die Nodeliste darf nicht leer sein
  {
    uint8_t lauf = 0, stringlaenge;
    bool fertig = false;
    char bNummerChar[4], buffer[MAX_PAYLOAD_SIZE];

    while (fertig == false && lauf < myHomieNodes.getSize()) 
    {
      hNode = myHomieNodes[lauf];
      if( hNode.nodeID == nNummer)
      {

        if (hNode.nodeProperties[0] == '\0') 
        {
          sprintf(hNode.nodeProperties,"%d",pNummer);
        }
        else if(strlen(hNode.nodeProperties) + 5 <= MAX_PAYLOAD_SIZE)
        {
          sprintf(buffer,",%d",pNummer);
          stringlaenge = strlen(hNode.nodeProperties);
          strncat(hNode.nodeProperties,buffer, MAX_PAYLOAD_SIZE - stringlaenge);
        }
        else
          lauf = myHomieNodes.getSize(); // zu viele Properties
        
        Serial1.printf("nodeProperties : %s", hNode.nodeProperties);
        fertig = true;
        myHomieNodes.add(hNode);  // geänderten Node hinzufügen
        myHomieNodes.remove(lauf); // alten Node löschen
          
      }
      lauf += 1;
    }
    if(fertig == true)
    {
      hProperty.propertyNodeID = nNummer;
      hProperty.propertyID = pNummer;
      hProperty.propertySettable = pSettable;
      if(snprintf(hProperty.propertyUnit, MAX_PAYLOAD_SIZE,"%s",pUnit)>=MAX_PAYLOAD_SIZE)
        hProperty.propertyUnit[MAX_PAYLOAD_SIZE-1]='\0';

      if(snprintf(hProperty.propertyName, MAX_PAYLOAD_SIZE,"%s",pName)>=MAX_PAYLOAD_SIZE)
        hProperty.propertyName[MAX_PAYLOAD_SIZE-1]='\0';

      if(snprintf(hProperty.propertyDataType, MAX_PAYLOAD_SIZE,"%s",pDataType)>=MAX_PAYLOAD_SIZE)
        hProperty.propertyDataType[MAX_PAYLOAD_SIZE-1]='\0';

      if(snprintf(hProperty.propertyFormat, MAX_PAYLOAD_SIZE,"%s",pFormat)>=MAX_PAYLOAD_SIZE)
        hProperty.propertyFormat[MAX_PAYLOAD_SIZE-1]='\0';
      
      myHomieProperies.add(hProperty);
      return true;
    }
  }
  else
    return false;

}

bool myHomeSachen::sendCharPropertyValue(uint8_t propertyNummer, uint8_t nodeNummer, const char propertyValue[])
{
  internMessage.myNodes = nodeNummer;
  internMessage.myProperties = propertyNummer;
  internMessage.myKeyWordNumber = 0;
  strcpy(internMessage.myNachricht,propertyValue);
  if(!sendToNode(GATEWAYID))
    return false;
  else
    return true;
}

bool myHomeSachen::sendIntPropertyValue(uint8_t propertyNummer, uint8_t nodeNummer, int16_t propertyValue)
{
  internMessage.myNodes = nodeNummer;
  internMessage.myProperties = propertyNummer;
  internMessage.myKeyWordNumber = 0;
  sprintf(internMessage.myNachricht,"%d",propertyValue);
  if(!sendToNode(GATEWAYID))
    return false;
  else
    return true;
}

bool myHomeSachen::sendFloatPropertyValue(uint8_t propertyNummer, uint8_t nodeNummer, float propertyValue)
{
  internMessage.myNodes = nodeNummer;
  internMessage.myProperties = propertyNummer;
  internMessage.myKeyWordNumber = 0;
  sprintf(internMessage.myNachricht,"%f",propertyValue);
  if(!sendToNode(GATEWAYID))
    return false;
  else
    return true;
}

void myHomeSachen::printInternMessage(void)
{
  Serial1.print("Gateway : ");
  Serial1.print(GATEWAYID);
  Serial1.print(" Nodenummer : ");
  Serial1.print(internMessage.myNodes);
  Serial1.print(" Propertynummer : ");
  Serial1.print(internMessage.myProperties);
  Serial1.print(" Topic : ");
  Serial1.print(myKeyWords[internMessage.myKeyWordNumber]);
  Serial1.print(" Nachricht : ");
  Serial1.println(internMessage.myNachricht);
}

void myHomeSachen::setDeviceFwName(const char FwName[])
{

   if(  snprintf(deviceFwName,MAX_PAYLOAD_SIZE-1,"%s",FwName) >= MAX_PAYLOAD_SIZE)
    deviceFwName[MAX_PAYLOAD_SIZE-1]='\0';

}

void myHomeSachen::setDeviceFwVersion(const char FwVersion[])
{
  
  if( snprintf(deviceFwVersion,MAX_PAYLOAD_SIZE-1,"%s", FwVersion) >= MAX_PAYLOAD_SIZE)
    deviceFwVersion[MAX_PAYLOAD_SIZE-1]='\0';
  
}
void myHomeSachen::setDeviceImplementation(const char implementation[])
{
   
  if( snprintf(deviceImplementation,MAX_PAYLOAD_SIZE-1,"%s", implementation) >= MAX_PAYLOAD_SIZE)
    deviceImplementation[MAX_PAYLOAD_SIZE-1] = '\0';
}


uint8_t myHomeSachen::getRFM69Temp( void ) // RFM69 Temperatur
{
  return deviceStatsRFM69Temperatur;
}