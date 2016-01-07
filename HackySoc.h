/*
  HackySoc.h
  Copyright (c) 2015 Etienne Deleflie.  All right reserved.
*/

// ensure this library description is only included once
#ifndef HackySoc_h
#define HackySoc_h

// library interface description
class HackySoc
{

  // user-accessible "public" interface
  public:
    HackySoc(ESP8266 &wifi);
    char from[128];
    char subject[128];
    char message[2048];

    /**
     * Connect to the  
     * 
     * @return the string of version. 
     */
    bool connectToAP(String ssid, String pwd);

    bool sendMessage(String recipient, String subject, String body);

    int  countInbox(void);

    bool getNewMessage(void);

  // library-accessible "private" interface
  private:
    /* The UART to communicate with ESP8266 */
    ESP8266 *wifi; 
    void setESPBaudrate(SoftwareSerial&);
   
};

#endif

