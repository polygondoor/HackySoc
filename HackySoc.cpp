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

int step_SMTP = 0;

String from = "hahanet_1@polygondoor.com.au";

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

// maximum number of attempts for each step in sending email
int max_atempts = 8;
int attempts;

bool HackySoc::sendMessage(String recipient, String subject, String body) {

  bool try_again = true;

  Serial.println(F(" >>>>> SENDING MESSAGE"));

  // Attempt to create TCP
  Serial.print("create TCP");
  do {
    wifi->createTCP(SMTP_HOST, SMTP_PORT) ? try_again = false : attempts++;
    if ( attempts > max_atempts ) {
      Serial.println(F("Error creating TCP connection"));
      return false;
    } Serial.print(".");
  } while (try_again);
  
  try_again = true; attempts = 0;

  // EHLO
  Serial.println(""); Serial.print("Write EHLO");
  do {
    wifi->sendAndCheck("EHLO localhost", F("250")) ? try_again = false : attempts++;
    if ( attempts > max_atempts ) {
      Serial.println(F("Error sending 'EHLO'"));
      return false;
    } Serial.print(".");
  } while (try_again);
  Serial.println("");
  try_again = true; attempts = 0;
   
  // AUTH LOGIN
  Serial.println(""); Serial.print("AUTH LOGIN");
  do {
    wifi->sendAndCheck(F("AUTH LOGIN"),F("334 VXNlcm5hbWU6")) ? try_again = false : attempts++;
    if ( attempts > max_atempts ) {
      Serial.println(F("Error calling AUTH LOGIN"));
      return false;
    } Serial.print(".");
  } while (try_again);
  Serial.println("");
  try_again = true; attempts = 0;

  // Username
  Serial.println(""); Serial.print("Send Username");
  do {
    wifi->sendAndCheck(EMAIL_FROM_BASE64,F("334 UGFzc3dvcmQ6")) ? try_again = false : attempts++;
    if ( attempts > max_atempts ) {
      Serial.println(F("Error sending User name"));
      return false;
    } Serial.print(".");
  } while (try_again);

  try_again = true; attempts = 0;

  // password
  Serial.println(""); Serial.print("Send password");
  do {
    wifi->sendAndCheck(EMAIL_PASSWORD_BASE64,F("235")) ? try_again = false : attempts++;
    if ( attempts > max_atempts ) {
      Serial.println(F("Error with password"));
      return false;
    } Serial.print(".");
  } while (try_again);

  try_again = true; attempts = 0;

  // FROM
  Serial.println(""); Serial.print("Send FROM");
  do {
    char mailFrom[50] = "MAIL FROM:<"; // If 50 is not long enough change it, do the same for the array in the other cases
    strcat(mailFrom,EMAIL_FROM);
    strcat(mailFrom,">");

    wifi->sendAndCheck(mailFrom,F("250")) ? try_again = false : attempts++;
    if ( attempts > max_atempts ) {
      Serial.println(F("Error setting FROM"));
      return false;
    } Serial.print(".");
  } while (try_again);

  try_again = true; attempts = 0;
        
  // TO
  Serial.println(""); Serial.print("Send TO");
  do {
    wifi->sendAndCheck( "RCPT TO:<" + recipient + ">" ,F("250")) ? try_again = false : attempts++;
    if ( attempts > max_atempts ) {
      Serial.println(F("Error setting TO"));
      return false;
    } Serial.print(".");
  } while (try_again);

  try_again = true; attempts = 0;

  // DATA
  Serial.println(""); Serial.print("Send DATA");
  do {
    wifi->sendAndCheck(F("DATA"),F("354")) ? try_again = false : attempts++;
    if ( attempts > max_atempts ) {
      Serial.println(F("Error sending data"));
      return false;
    } Serial.print(".");
  } while (try_again);

  try_again = true; attempts = 0;

  // FROM
  Serial.println(""); Serial.print("Send FROM");
  do {
    wifi->sendAndCheck("FROM: " +  from +  " <" +  from + ">" ) ? try_again = false : attempts++;
    if ( attempts > max_atempts ) {
      Serial.println(F("Error setting FROM"));
      return false;
    } Serial.print(".");
  } while (try_again);

  try_again = true; attempts = 0;

  // TO
  Serial.println(""); Serial.print("Send TO");
  do {
    wifi->sendAndCheck( "TO: " + recipient + "<" + recipient + ">" ) ? try_again = false : attempts++;
    if ( attempts > max_atempts ) {
      Serial.println(F("Error setting TO"));
      return false;
    } Serial.print(".");
  } while (try_again);

  try_again = true; attempts = 0;

  // SUBJECT
  Serial.println(""); Serial.print("Send SUBJECT");
  do {
    wifi->sendAndCheck( "SUBJECT: " + subject ) ? try_again = false : attempts++;
    if ( attempts > max_atempts ) {
      Serial.println(F("Error setting Subject"));
      return false;
    } Serial.print(".");
  } while (try_again);

  try_again = true; attempts = 0;

  // END of header
  Serial.println(""); Serial.print("Send end of header");
  do {
    wifi->sendAndCheck(F("\r\n")) ? try_again = false : attempts++;
    if ( attempts > max_atempts ) {
      Serial.println(F("Error marking end of header"));
      return false;
    } Serial.print(".");
  } while (try_again);

  try_again = true; attempts = 0;

  // CONTENT
  Serial.println(""); Serial.print("Send CONTENT");
  do {
    wifi->sendAndCheck(body) ? try_again = false : attempts++;
    if ( attempts > max_atempts ) {
      Serial.println(F("Error sending content"));
      return false;
    } Serial.print(".");
  } while (try_again);

  try_again = true; attempts = 0;

  // ENd of content
  Serial.println(""); Serial.print("CONTENT END");
  do {
    wifi->sendAndCheck("\r\n.") ? try_again = false : attempts++;
    if ( attempts > max_atempts ) {
      Serial.println(F("Error sending end of content"));
      return false;
    } Serial.print(".");
  } while (try_again);

  try_again = true; attempts = 0;

  // Quit TCP connection
  Serial.println(""); Serial.print("QUIT");
  do {
    wifi->sendAndCheck("QUIT") ? try_again = false : attempts++;
    if ( attempts > max_atempts ) {
      Serial.println(F("Could not say quit"));
      return false;
    } Serial.print(".");
  } while (try_again);

  try_again = true; attempts = 0;

  // Release TCP
  Serial.println(""); Serial.print("Close TCP :)");
  do {
    wifi->releaseTCP() ? try_again = false : attempts++;
    if ( attempts > max_atempts ) {
      Serial.println(F("Could not release TCP"));
      return false;
    } Serial.print(".");
  } while (try_again);

  Serial.println(F(" >>>>> SENDING MESSAGE DONE"));
  return true;
}