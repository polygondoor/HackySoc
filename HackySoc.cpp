/*
  HackySoc.h
  Copyright (c) 2015 Etienne Deleflie.  All right reserved.
*/

// include the ESP8266 code
// TODO: change the name and pay respect to source
#include <ESP8266.h>
#include <SoftwareSerial.h>

// include this library's description file
#include "HackySoc.h"

// SMTP parameters
#define SMTP_HOST "mail.polygondoor.com.au"     // Your email provider's SMTP outgoing server name for unencrypted email
#define SMTP_PORT 587                     // Your email provider's SMTP outgoing port for unencrypted email

#define POP3_HOST "mail.polygondoor.com.au"
#define POP3_PORT 110

#define EMAIL_FROM "hahanet_1@polygondoor.com.au"
#define PASS "Jh~@u~qyLnc!"
// We'll need your EMAIL_FROM and its EMAIL_PASSWORD base64 encoded, you can use https://www.base64encode.org/
#define EMAIL_FROM_BASE64 "aGFoYW5ldF8xQHBvbHlnb25kb29yLmNvbS5hdQ=="  // etienne@polygondoor.com.au
#define EMAIL_PASSWORD_BASE64 "Smh+QHV+cXlMbmMh"  // 

char EMAIL_TO[]  = "edeleflie@gmail.com";
char SUBJECT[]  = "A polygon door has opened";
char EMAIL_CONTENT[]  = "Hello,\r\n This seems to have worked OK! :)";

int step_SMTP = 0;

// Constructor /////////////////////////////////////////////////////////////////
// Function that handles the creation and setup of instances

HackySoc::HackySoc(ESP8266 &theWifi): wifi(&theWifi)
{
  // do whatever is required to initialize the library
}

bool HackySoc::connectToAP(String ssid, String pwd)
{
  // First check if we are already connected.
  if ( wifi->checkAP() )
  {
    Serial.println(F(" ALREADY CONNECTED !"));
    return true;
  } 

  Serial.println(F("Starting Arduino to WiFi chip communication..."));
  wifi->restart();

  // Uncomment this to get ESP8266 version
  Serial.print(F("FW Version:"));
  Serial.println(wifi->getVersion().c_str());

  if (wifi->setOprToStation()) {
  // if (wifi->setOprToSoftAP()) {
  // if (wifi->setOprToStationSoftAP()) {
      // Serial.print(F("to station + softap [ok] \r\n"));
  } else {
      Serial.print(F("Can't set ESP8266 to right mode (station + softap) [err] \r\n"));
      return false;
  }   

  //delay(2000); // un required delay to let WIFI find 

  // Uncomment this to print out available wireless networks
  Serial.println( wifi->getAPList());  

  Serial.println(F("Trying to join WiFi network..."));
  // Now connect to wireless network
  if (wifi->joinAP(ssid, pwd)) {
    // Serial.println(F("Join AP success"));

    delay(2000); // this delay seems to help IP number reporting

    if (wifi->disableMUX()) {
      
      Serial.print(F("IP:"));
      Serial.println( wifi->getLocalIP().c_str());
      Serial.println(F("WiFi connected [OK]"));

      return true;

    } else {
        Serial.println(F("WiFi setup [MUX error]"));
    }

  } else {
      Serial.println(F("WiFi setup [Join AP error]"));
  }

  return false;

}

bool HackySoc::sendMessage(String recipient, String subject, String body) {
  Serial.println(F(" >>>>> SEND MESSAGE CALLED"));
  while (step_SMTP != -1) {
    if(do_next_SMP_step()){ // execute the next command
      step_SMTP++; // increment the count so that the next command will be executed next time.
    }
  }

  return true;

}

// Private Methods /////////////////////////////////////////////////////////////
// Functions only available to other functions in this library

int HackySoc::countInbox(void){
  Serial.println(F("CountInBox called"));

  Serial.println(F("Connecting..."));
  if ( wifi->createTCP(POP3_HOST, POP3_PORT) ) {
    // we have a TCP session open

    Serial.println(F("Sending user name"));
    //if ( wifi->sendAndCheck( strcat("USER ",EMAIL_FROM), F("+OK"))) {
    if ( wifi->sendAndCheck( F("USER hahanet_1@polygondoor.com.au"), F("+OK"))) {
      // User is good

      Serial.println(F("Sending pass"));
      if ( wifi->sendAndCheck( strcat("PASS ",PASS), F("+OK"))) {
        // password is good

        // now read some input
        uint8_t inputBuffer[512] = {0};
        int length = sizeof(inputBuffer);
        wifi->sendAndReceive(inputBuffer, length, F("STAT"));
        Serial.println(F("====================="));
        /*
        for (uint32_t i = 0; i < length; i++) {
            m_puart->write(buffer[i]);
            //Serial.print( &buffer[i]);
        }
        */
        Serial.write( inputBuffer, length);

        // try to parse result and return number of emails available:
        // try this code: http://stackoverflow.com/questions/29320434/convert-array-of-uint8-t-to-string-in-c
        // String s = String( inputBuffer, length );

        Serial.println(F("====================="));

      } else Serial.println(F("Sending PASS name. no go"));

    } else Serial.println(F("Sending user name. no go"));

    wifi->sendAndCheck( F("QUIT"), F("+OK"));
  } else Serial.println(F("Could not create TCP!"));

}

// do_next executes the SMTP command in the order required.
bool HackySoc::do_next_SMP_step(void)
{
    switch(step_SMTP){ 
    case 0:
        Serial.println(F("Connecting..."));
        return wifi->createTCP(SMTP_HOST, SMTP_PORT);
        break;
        
    case 1:
      Serial.println(F("Sending 'EHLO'..."));
        return wifi->sendAndCheck("EHLO localhost", F("250"));
        break;
   
    case 2:
      Serial.println(F("AUTH LOGIN ..."));
        return wifi->sendAndCheck(F("AUTH LOGIN"),F("334 VXNlcm5hbWU6"));
        break;

    case 3:
      Serial.println(F("Sending User name"));
        return wifi->sendAndCheck(EMAIL_FROM_BASE64,F("334 UGFzc3dvcmQ6")); 
        break;
              
    case 4:
      Serial.println(F("Sending passowrd User name"));
        return wifi->sendAndCheck(EMAIL_PASSWORD_BASE64,F("235"));
        break;
          
    case 5:{
        Serial.println(F("Setting FROM ... "));
        char mailFrom[50] = "MAIL FROM:<"; // If 50 is not long enough change it, do the same for the array in the other cases
        strcat(mailFrom,EMAIL_FROM);
        strcat(mailFrom,">");

        return wifi->sendAndCheck(mailFrom,F("250"));
        break;
    }  
    case 6:{
        Serial.println(F("Setting TO ... "));
        char rcptTo[50] = "RCPT TO:<";
        strcat(rcptTo,EMAIL_TO);
        strcat(rcptTo,">");
        return wifi->sendAndCheck(rcptTo,F("250"));
        break;
    }  
    case 7:
        Serial.println(F("Call DATA command... "));
        // Send "DATA"  command, the server will reply with something like "334 end message with \r\n.\r\n."
        return wifi->sendAndCheck(F("DATA"),F("354"));
        break;
        
    case 8:{
        Serial.println(F("Apply FROM header ... "));
        // apply "FROM: from_name <from_email@domain.com>" header
        char from[100] = "FROM: ";
        strcat(from,EMAIL_FROM);
        strcat(from," ");
        strcat(from,"<");
        strcat(from,EMAIL_FROM);
        strcat(from,">");
        return wifi->sendAndCheck(from);  
        break;
    }   
    case 9:{
        Serial.println(F("Apply TO header ... "));
        // apply TO header 
        char to[100] = "TO: ";
        strcat(to,EMAIL_TO);
        strcat(to,"<");
        strcat(to,EMAIL_TO);
        strcat(to,">");
        return wifi->sendAndCheck(to);  
        break;
    }
    case 10:{
        Serial.println(F("Apply SUBJECT header ... "));
        // apply SUBJECT header
        char subject[50] = "SUBJECT: ";
        strcat(subject,SUBJECT);
        return wifi->sendAndCheck(subject);
        break;
    }
    case 11:
        Serial.println(F("Mark end of Header ... "));
        return wifi->sendAndCheck(F("\r\n"));
        break;
    case 12:
        Serial.println(F("Send CONTENT ... "));
        return wifi->sendAndCheck(EMAIL_CONTENT);
        break;
    case 13:
        return wifi->sendAndCheck("\r\n."); 
        break;
    case 14:
        Serial.println(F("SAY QUIT ... "));
        return wifi->sendAndCheck("QUIT");
        break;
    case 15:
        Serial.println(F("Release TCP ... "));
        wifi->releaseTCP();
        return true;
        break;
    case 16:
        Serial.println(F("Done!"));
        step_SMTP = -1;
        return false; // we don't want to increment the count
        break;
    default:
        break;
        }
}

