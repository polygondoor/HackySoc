/**
 * Written by Etienne Deleflie for Polygon Door's HackySoc
 * 2015
 * 1. Configure Wireless SSID and password here.
 * 2. Go into HackySoc.h and change SMTP / POP3 settings (you'll need to Base64 encode a couple of things)
 * 3. Configure Adafruit_SSD1306.h to point to 64 pixel high OLED
 */

#include <ESP8266.h>
#include <SoftwareSerial.h>
#include <HackySoc.h>

#define SSID        "electricityD6AA"
#define PASSWORD    "passwordhere"

SoftwareSerial mySerial(10, 11); /* RX:D3, TX:D2 */ // This tells us which pins the ESP8266 is plugged into
ESP8266 wifi(mySerial); // This manages WIFI communications
HackySoc hackySoc(wifi); // This manages sending / receiving emails over the WIFI connection

void setup(void)
{
    Serial.begin(9600);
    while (!Serial) {}; // wait for serial to get going

    // Connect to WIFI Access Point
    Serial.println();
    Serial.println("Connecting to WIFI...");
    if (hackySoc.connectToAP(SSID, PASSWORD)) {
      
      // Send an email
      hackySoc.sendMessage( "someone@somewhere.com", "[HACKYSOC] Yiihah! ", "I'm connected to WIFI and I can send emails!");
    }
}

void loop() {

    // Check that we are still connected to WIFI Access Point
    if ( !wifi.checkAP() ) {
      Serial.println("RE-connecting to WIFI...");
      hackySoc.connectToAP(SSID, PASSWORD);
    }

    if ( hackySoc.getNewMessage() ) {
      Serial.println(hackySoc.message);
    } else {
      Serial.println("There are no new messages");
    }

    delay(5000);
}
     
