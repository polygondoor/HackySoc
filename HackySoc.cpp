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
#define EMAIL_PASSWORD_BASE64 "base64_encoded_email_password"  // 

#define FROM_STRING "FROM: " EMAIL_FROM  " <" EMAIL_FROM ">" 
#define USER_STRING "USER " EMAIL_FROM
#define PASS_STRING "PASS " PASS

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

// maximum number of attempts for each step in sending email
int max_atempts = 8;
int attempts = 0;

int HackySoc::countInbox(void){
  Serial.println(F(" HackySoc >>>>> COUNT INBOX"));
  bool try_again = true;

  Serial.print(F("IP status: "));
  String status = wifi->getIPStatus(); // 2: Got IP, 3: Connected, 4: Disconnected
  Serial.println( status );

  if ( status == "STATUS:3") {
    // TCP connection still open! close it
    Serial.print(F("QUIT ing existing connection"));
    wifi->sendAndCheck( F("QUIT"), F("+OK"));
    // wifi->releaseTCP();
  }

  // Attempt to create TCP
  Serial.print(F("create TCP"));
  do {
    wifi->createTCP(POP3_HOST, POP3_PORT) ? try_again = false : attempts++;
    if ( attempts > max_atempts ) {
      Serial.println(F("Error creating TCP connection"));
      return -1;
    } Serial.print(".");
  } while (try_again);
  
  try_again = true; attempts = 0;

  // Send User name
  Serial.println(F("")); Serial.print(F("Send user name"));
  do {
    // wifi->sendAndCheck( strcat("USER ", EMAIL_FROM), F("+OK")) ? try_again = false : attempts++;
    wifi->sendAndCheck( USER_STRING, F("+OK")) ? try_again = false : attempts++;
    if ( attempts > max_atempts ) {
      Serial.println(F("Sending username not accepted"));
      return -1;
    } Serial.print(".");
  } while (try_again);
  
  try_again = true; attempts = 0;

  // Sending pass word
  Serial.println(F("")); Serial.print(F("Send Password"));
  do {
    wifi->sendAndCheck( PASS_STRING, F("+OK")) ? try_again = false : attempts++;
    if ( attempts > max_atempts ) {
      Serial.println(F("Password not accepted"));
      return -1;
    } Serial.print(".");
  } while (try_again);

  try_again = true; attempts = 0;

  // prepare input buffer
  uint8_t inputBuffer[4096] = {0};
  int length = sizeof(inputBuffer);

  // send STAT command
  Serial.println(F("Sending STAT command"));
  wifi->sendAndReceive(inputBuffer, length, F("STAT"));

  // process input
  // Turn response into String
  String response((char*) &inputBuffer);

  // Lose the "+OK "
  String messageCountString = response.substring(4);

  // find index of space after message count
  int space = messageCountString.indexOf(' ');
  messageCountString = messageCountString.substring(0,space);
  int messageCount = messageCountString.toInt();
  Serial.print(F("Message count: "));
  Serial.print( messageCount);
  Serial.println("\r\n");

  //////////////////// Go THROUGH EMAILS ////////////
  for (int i = 1 ; i < (messageCount+1) ; i++){
    String command = "RETR ";
    command.concat(i);
    // String command = "RETR " + i;
    wifi->sendAndReceive(inputBuffer, length, command ) ;
    Serial.println((char*) &inputBuffer);
  }

  // Quit the POP3 server
  Serial.println(F("Sending QUIT"));
  wifi->sendAndCheck( F("QUIT"), F("+OK")); // this seems to close the TCP connection
  // wifi->releaseTCP();;

  return messageCount;

}

bool HackySoc::sendMessage(String recipient, String subject, String body) {

  bool try_again = true;

  Serial.println(F(" HackySoc >>>>> SENDING MESSAGE"));

  // Attempt to create TCP
  Serial.print(F("create TCP"));
  do {
    wifi->createTCP(SMTP_HOST, SMTP_PORT) ? try_again = false : attempts++;
    if ( attempts > max_atempts ) {
      Serial.println(F("Error creating TCP connection"));
      return false;
    } Serial.print(F("."));
  } while (try_again);
  
  try_again = true; attempts = 0;

  // EHLO
  Serial.println(F("")); Serial.print(F("Write EHLO"));
  do {
    wifi->sendAndCheck(F("EHLO localhost"), F("250")) ? try_again = false : attempts++;
    if ( attempts > max_atempts ) {
      Serial.println(F("Error sending 'EHLO'"));
      return false;
    } Serial.print(F("."));
  } while (try_again);

  try_again = true; attempts = 0;
   
  // AUTH LOGIN
  Serial.println(F("")); Serial.print(F("AUTH LOGIN"));
  do {
    wifi->sendAndCheck(F("AUTH LOGIN"),F("334 VXNlcm5hbWU6")) ? try_again = false : attempts++;
    if ( attempts > max_atempts ) {
      Serial.println(F("Error calling AUTH LOGIN"));
      return false;
    } Serial.print(".");
  } while (try_again);

  try_again = true; attempts = 0;

  // Username
  Serial.println(F("")); Serial.print(F("Send Username"));
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
    wifi->sendAndCheck( FROM_STRING, F("OK\r\n") ) ? try_again = false : attempts++;
    if ( attempts > max_atempts ) {
      Serial.println(F("Error setting FROM"));
      return false;
    } Serial.print(".");
  } while (try_again);

  try_again = true; attempts = 0;

  // TO
  Serial.println(""); Serial.print("Send TO");
  do {
    wifi->sendAndCheck( "TO: " + recipient + "<" + recipient + ">", F("OK\r\n") ) ? try_again = false : attempts++;
    if ( attempts > max_atempts ) {
      Serial.println(F("Error setting TO"));
      return false;
    } Serial.print(".");
  } while (try_again);

  try_again = true; attempts = 0;

  // SUBJECT
  Serial.println(""); Serial.print("Send SUBJECT");
  do {
    wifi->sendAndCheck( "SUBJECT: " + subject, F("OK\r\n") ) ? try_again = false : attempts++;
    if ( attempts > max_atempts ) {
      Serial.println(F("Error setting Subject"));
      return false;
    } Serial.print(".");
  } while (try_again);

  try_again = true; attempts = 0;

  // END of header
  Serial.println(""); Serial.print("Send end of header");
  do {
    wifi->sendAndCheck(F("\r\n"), F("OK\r\n")) ? try_again = false : attempts++;
    if ( attempts > max_atempts ) {
      Serial.println(F("Error marking end of header"));
      return false;
    } Serial.print(".");
  } while (try_again);

  try_again = true; attempts = 0;

  // CONTENT
  Serial.println(""); Serial.print("Send CONTENT");
  do {
    wifi->sendAndCheck(body, F("OK\r\n")) ? try_again = false : attempts++;
    if ( attempts > max_atempts ) {
      Serial.println(F("Error sending content"));
      return false;
    } Serial.print(".");
  } while (try_again);

  try_again = true; attempts = 0;

  // ENd of content
  Serial.println(""); Serial.print("CONTENT END");
  do {
    wifi->sendAndCheck("\r\n.", F("OK\r\n")) ? try_again = false : attempts++;
    if ( attempts > max_atempts ) {
      Serial.println(F("Error sending end of content"));
      return false;
    } Serial.print(".");
  } while (try_again);

  try_again = true; attempts = 0;

  // Quit TCP connection
  Serial.println(""); Serial.print("QUIT");
  do {
    wifi->sendAndCheck("QUIT", F("OK\r\n")) ? try_again = false : attempts++;
    if ( attempts > max_atempts ) {
      Serial.println(F("Could not say quit"));
      return false;
    } Serial.print(".");
  } while (try_again);

  try_again = true; attempts = 0;

  // Release TCP  
  Serial.println(""); Serial.print("Close TCP :)");
  // Note: this is returning FALSE ... so there is some kind of bug in 
  // the ESP8266 that either doesn't close the TCP connection,
  // or doesn't report it correctly.
  wifi->releaseTCP();

  Serial.println(F(" END HackySoc >>>>> SENDING MESSAGE"));
  return true;
}