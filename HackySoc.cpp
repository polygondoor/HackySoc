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

// POP3 parameters
#define POP3_HOST "mail.polygondoor.com.au"
#define POP3_PORT 110

// Email account details, and their BASE64 versions
// Use services like https://www.base64encode.org/ to create the Base64 version of your email details
#define EMAIL_FROM "someone@somewhere.com"
#define PASS "youremailpassword"
#define EMAIL_FROM_BASE64 "aGFoYW5ldF8xQHwerwer5kb29yLmNvbS5hdQ=="
#define EMAIL_PASSWORD_BASE64 "Smh+werwerXlMbmMh"

#define FROM_STRING "FROM: " EMAIL_FROM  " <" EMAIL_FROM ">" 
#define USER_STRING "USER " EMAIL_FROM
#define PASS_STRING "PASS " PASS

// Constructor /////////////////////////////////////////////////////////////////
// Function that handles the creation and setup of instances

HackySoc::HackySoc(ESP8266 &theWifi): wifi(&theWifi)
{
  // By default, HackySoc contains a message that is just "-"
  message[0] = '-';
  message[1] = '\0';
}

bool HackySoc::connectToAP(String ssid, String pwd)
{
  // First check if we are already connected.
  if ( wifi->checkAP() )
  {
    // Serial.println(F(" ALREADY CONNECTED !"));
    return true;
  } 

  wifi->restart();

  // Uncomment this to get ESP8266 version
  // Serial.print(F("FW Version:"));
  // Serial.println(wifi->getVersion().c_str());

  if (wifi->setOprToStation()) {
  // if (wifi->setOprToSoftAP()) {
  // if (wifi->setOprToStationSoftAP()) {
      // Serial.print(F("to station + softap [ok] \r\n"));
  } else {
      Serial.print(F("Can't set ESP8266 to right mode (station + softap) [err] \r\n"));
      return false;
  }   

  // Uncomment this to print out available wireless networks
  Serial.println( wifi->getAPList());

  // Now connect to wireless network
  if (wifi->joinAP(ssid, pwd)) {
    // Serial.println(F("Join AP success"));

    delay(2000); // this delay seems to help IP number reporting

    if (wifi->disableMUX()) {
      
      // Uncomment this to get the IP
      Serial.println( wifi->getLocalIP().c_str());

      return true;

    } else {
        Serial.println(F("WiFi setup [MUX error]"));
    }

  } else {
      Serial.println(F("WiFi setup [Join AP error]"));
  }

  return false;

}

// maximum number of attempts for each step in sending email
int max_atempts = 8;
int attempts = 0;

/*
* Returns the number of emails available
* 
*/

int HackySoc::countInbox(void){
  bool try_again = true;

  // Serial.print(F("IP status: "));
  String status = wifi->getIPStatus(); // 2: Got IP, 3: Connected, 4: Disconnected
  // Serial.println( status );

  if ( status.startsWith("STATUS:3") ) {
    // TCP connection still open! close it
    wifi->sendAndCheck( F("QUIT"), F("+OK"));
    // wifi->releaseTCP();
  }

  // Attempt to create TCP
  do {
    wifi->createTCP(POP3_HOST, POP3_PORT) ? try_again = false : attempts++;
    if ( attempts > max_atempts ) {
      Serial.println(F("Error creating TCP connection"));
      return -1;
    };
  } while (try_again);
  
  try_again = true; attempts = 0;

  // Send User name
  do {
    // wifi->sendAndCheck( strcat("USER ", EMAIL_FROM), F("+OK")) ? try_again = false : attempts++;
    wifi->sendAndCheck( USER_STRING, F("+OK")) ? try_again = false : attempts++;
    if ( attempts > max_atempts ) {
      Serial.println(F("Sending username not accepted"));
      return -1;
    } ;
  } while (try_again);
  
  try_again = true; attempts = 0;

  // Sending pass word
  do {
    wifi->sendAndCheck( PASS_STRING, F("+OK")) ? try_again = false : attempts++;
    if ( attempts > max_atempts ) {
      Serial.println(F("Password not accepted"));
      return -1;
    } ;
  } while (try_again);

  try_again = true; attempts = 0;

  // prepare input buffer
  uint8_t inputBuffer[64] = {0};
  int length = sizeof(inputBuffer);

  // send STAT command
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

  if (messageCount) return messageCount;

  return 0;
}

/* 
* Retrieves first email in queue
*
*
*/

bool HackySoc::getNewMessage(void){
  bool try_again = true;

  // Serial.print(F("IP status: "));
  String status = wifi->getIPStatus(); // 2: Got IP, 3: Connected, 4: Disconnected
  // Serial.println( status );

  if ( status.startsWith("STATUS:3") ) {
    // TCP connection still open! close it
    // Serial.print(F("QUIT ing existing connection"));
    wifi->sendAndCheck( F("QUIT"), F("+OK"));
    // wifi->releaseTCP();
  }

  // Attempt to create TCP
  // Serial.print(F("create TCP"));
  do {
    wifi->createTCP(POP3_HOST, POP3_PORT) ? try_again = false : attempts++;
    if ( attempts > max_atempts ) {
      Serial.println(F("Error creating TCP connection"));
      return false;
    };
  } while (try_again);
  
  try_again = true; attempts = 0;

  // Send User name
  // Serial.println(F("")); Serial.print(F("Send user name"));
  do {
    // wifi->sendAndCheck( strcat("USER ", EMAIL_FROM), F("+OK")) ? try_again = false : attempts++;
    wifi->sendAndCheck( USER_STRING, F("+OK")) ? try_again = false : attempts++;
    if ( attempts > max_atempts ) {
      Serial.println(F("Sending username not accepted"));
      return false;
    };
  } while (try_again);
  
  try_again = true; attempts = 0;

  // Sending pass word
  // Serial.println(F("")); Serial.print(F("Send Password"));
  do {
    wifi->sendAndCheck( PASS_STRING, F("+OK")) ? try_again = false : attempts++;
    if ( attempts > max_atempts ) {
      Serial.println(F("Password not accepted"));
      return false;
    };
  } while (try_again);

  // sort out structures that we are going to use
  char* email_contents[] = { this->from, this->subject, this->message };
  size_t sizes[3] = { sizeof(this->from), sizeof(this->subject), sizeof(this->message) };

  // prepare input buffer
  uint8_t inputBufferBody[128] = {0};
  int length = sizeof(inputBufferBody);

  // send STAT command
  // Serial.println(F("Sending STAT command"));
  wifi->sendAndReceive(inputBufferBody, length, F("STAT"));

  // process input
  // Turn response into String
  String response((char*) &inputBufferBody);

  // Lose the "+OK "
  String messageCountString = response.substring(4);

  // find index of space after message count
  int space = messageCountString.indexOf(' ');
  messageCountString = messageCountString.substring(0,space);
  int messageCount = messageCountString.toInt();

  //////////////////// JUST GET THE FIRST MESSAGE! ////////////
  if (messageCount > 0) {

    String command = F("RETR 1");
    wifi->sendAndReceiveEmail(email_contents, sizes, command ) ;

    // Then DELETE that message
    wifi->sendAndCheck( "DELE 1", F("+OK"));

    // Quit the POP3 server
    wifi->sendAndCheck( F("QUIT"), F("+OK")); // this seems to close the TCP connection
    // wifi->releaseTCP();;

  } else {
    // there are no new messages

    // Quit the POP3 server
    wifi->sendAndCheck( F("QUIT"), F("+OK")); // this seems to close the TCP connection

    // wifi->releaseTCP();
    return false;

  }

  return true;

}

bool HackySoc::sendMessage(String recipient, String subject, String body) {

  bool try_again = true;

  // Attempt to create TCP
  do {
    wifi->createTCP(SMTP_HOST, SMTP_PORT) ? try_again = false : attempts++;
    if ( attempts > max_atempts ) {
      Serial.println(F("Error creating TCP connection"));
      return false;
    }
  } while (try_again);
  
  try_again = true; attempts = 0;

  // EHLO
  do {
    wifi->sendAndCheck(F("EHLO localhost"), F("250")) ? try_again = false : attempts++;
    if ( attempts > max_atempts ) {
      Serial.println(F("Error sending 'EHLO'"));
      return false;
    } ;
  } while (try_again);

  try_again = true; attempts = 0;
   
  // AUTH LOGIN
  do {
    wifi->sendAndCheck(F("AUTH LOGIN"),F("334 VXNlcm5hbWU6")) ? try_again = false : attempts++;
    if ( attempts > max_atempts ) {
      Serial.println(F("Error calling AUTH LOGIN"));
      return false;
    } 
  } while (try_again);

  try_again = true; attempts = 0;

  // Username
  do {
    wifi->sendAndCheck(EMAIL_FROM_BASE64,F("334 UGFzc3dvcmQ6")) ? try_again = false : attempts++;
    if ( attempts > max_atempts ) {
      Serial.println(F("Error sending User name"));
      return false;
    }
  } while (try_again);

  try_again = true; attempts = 0;

  // password
  do {
    wifi->sendAndCheck(EMAIL_PASSWORD_BASE64,F("235")) ? try_again = false : attempts++;
    if ( attempts > max_atempts ) {
      Serial.println(F("Error with password"));
      return false;
    }
  } while (try_again);

  try_again = true; attempts = 0;

  // FROM
  do {
    char mailFrom[50] = "MAIL FROM:<"; // If 50 is not long enough change it, do the same for the array in the other cases
    strcat(mailFrom,EMAIL_FROM);
    strcat(mailFrom,">");

    wifi->sendAndCheck(mailFrom,F("250")) ? try_again = false : attempts++;
    if ( attempts > max_atempts ) {
      Serial.println(F("Error setting FROM"));
      return false;
    }
  } while (try_again);

  try_again = true; attempts = 0;
        
  // TO
  do {
    wifi->sendAndCheck( "RCPT TO:<" + recipient + ">" ,F("250")) ? try_again = false : attempts++;
    if ( attempts > max_atempts ) {
      Serial.println(F("Error setting TO"));
      return false;
    }
  } while (try_again);

  try_again = true; attempts = 0;

  // DATA
  do {
    wifi->sendAndCheck(F("DATA"),F("354")) ? try_again = false : attempts++;
    if ( attempts > max_atempts ) {
      Serial.println(F("Error sending data"));
      return false;
    }
  } while (try_again);

  try_again = true; attempts = 0;

  // FROM
  do {
    wifi->sendAndCheck( FROM_STRING, F("OK\r\n") ) ? try_again = false : attempts++;
    if ( attempts > max_atempts ) {
      Serial.println(F("Error setting FROM"));
      return false;
    }
  } while (try_again);

  try_again = true; attempts = 0;

  // TO
  do {
    wifi->sendAndCheck( "TO: " + recipient + "<" + recipient + ">", F("OK\r\n") ) ? try_again = false : attempts++;
    if ( attempts > max_atempts ) {
      Serial.println(F("Error setting TO"));
      return false;
    }
  } while (try_again);

  try_again = true; attempts = 0;

  // SUBJECT
  do {
    wifi->sendAndCheck( "SUBJECT: " + subject, F("OK\r\n") ) ? try_again = false : attempts++;
    if ( attempts > max_atempts ) {
      Serial.println(F("Error setting Subject"));
      return false;
    }
  } while (try_again);

  try_again = true; attempts = 0;

  // END of header
  do {
    wifi->sendAndCheck(F("\r\n"), F("OK\r\n")) ? try_again = false : attempts++;
    if ( attempts > max_atempts ) {
      Serial.println(F("Error marking end of header"));
      return false;
    }
  } while (try_again);

  try_again = true; attempts = 0;

  // CONTENT
  do {
    wifi->sendAndCheck(body, F("OK\r\n")) ? try_again = false : attempts++;
    if ( attempts > max_atempts ) {
      Serial.println(F("Error sending content"));
      return false;
    }
  } while (try_again);

  try_again = true; attempts = 0;

  // ENd of content
  do {
    wifi->sendAndCheck("\r\n.", F("OK\r\n")) ? try_again = false : attempts++;
    if ( attempts > max_atempts ) {
      Serial.println(F("Error sending end of content"));
      return false;
    }
  } while (try_again);

  try_again = true; attempts = 0;

  // Quit TCP connection
  do {
    wifi->sendAndCheck("QUIT", F("OK\r\n")) ? try_again = false : attempts++;
    if ( attempts > max_atempts ) {
      Serial.println(F("Could not say quit"));
      return false;
    }
  } while (try_again);

  try_again = true; attempts = 0;

  // Release TCP  
  // Note: this is returning FALSE ... so there is some kind of bug in 
  // the ESP8266 that either doesn't close the TCP connection,
  // or doesn't report it correctly.
  wifi->releaseTCP();

  return true;
}

// Private Methods /////////////////////////////////////////////////////////////
// Functions only available to other functions in this library
