/**
 * @example HTTPGET.ino
 * @brief The HTTPGET demo of library WeeESP8266. 
 * @author Wu Pengfei<pengfei.wu@itead.cc> 
 * @date 2015.03
 * 
 * @par Copyright:
 * Copyright (c) 2015 ITEAD Intelligent Systems Co., Ltd. \n\n
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version. \n\n
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <ESP8266.h>
#include <SoftwareSerial.h>
#include <Adafruit_SSD1306.h>
#include <HackySoc.h>

#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);

#define SSID        "electricityD6AA"
#define PASSWORD    "babaganoush12"

SoftwareSerial mySerial(10, 11); /* RX:D3, TX:D2 */ // This tells us which pins the ESP8266 is plugged into
ESP8266 wifi(mySerial); // This manages WIFI communications
HackySoc hackySoc(wifi); // This manages sending / receiving emails over the WIFI connection

bool send_flag = false; // we'll use this flag to know when to send the email commands

void setup(void)
{
    Serial.begin(9600);
    while (!Serial) { Serial.println(F(".")); }; // wait for serial to get going

    // Set up the screen
    // tell the Arduino that our little OLED screen is 128 x 64 pixels
    display.begin(SSD1306_SWITCHCAPVCC, 0x3C); // its the 0x3C that says "128 * 64"
    
    // set text size and other things
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setTextWrap(1);
    display.setCursor(0, 0);

    // try to connect to WIFI
    display.clearDisplay();
    display.print( F("Trying to connect to WIFI") );
    display.display();

    bool stillTryingToConnect = true;
    int tries = 0;

    while (stillTryingToConnect) {
      if ( hackySoc.connectToAP(SSID, PASSWORD) ) 
      {
        stillTryingToConnect = false;
        send_flag = true;
      } else {
        tries = tries + 1;
        if (tries > 20) {
          stillTryingToConnect = false;
          Serial.println(F("Cant connect to WIFI, I tried over 20 times!"));
        }
      }
    }

    display.clearDisplay();
    display.print( F("WIFI CONNECTED!") );
    display.display();
}

void loop() {

    if(send_flag){ // the send_flat is set, this means we are or need to start sending SMTP commands
      // hackySoc.sendMessage( "edeleflie@gmail.com", "[HACKYSOC] Hi there ", "this is the body of the email");

      display.clearDisplay();
      display.print( F("Counting Emails") );
      display.display();

      int count = hackySoc.countInbox();

      display.clearDisplay();
      display.setCursor(10, 10);
      display.print( count );
      display.display();
      
      send_flag = false;
    }

}

/*
void pingTimeAPI() {
    uint8_t buffer[512] = {0};  
  
  char *request = "GET /utc/now HTTP/1.1\r\nHost: www.timeapi.org\r\nAccept-Encoding: gzip,deflate\r\nUser-Agent:ESP8266\r\nContent-Type:application/json\r\n\r\n";
/
  // send request
  if (wifi.createTCP(HOST_NAME, HOST_PORT)) {
    if (wifi.send((const uint8_t*)request, strlen(request))) {
      Serial.println(F("Send data [OK]"));
    }
    else {
      Serial.println(F("Send data [ERR]"));
    }
  }
  else {
    Serial.println(F("Create tcp [ERR] [reboot SP8266 here?]"));
  }

  // read response
  len = wifi.recv(buffer, sizeof(buffer), 10000);
  if (len > 0) {
    Serial.print(F("Received:["));
    for (uint32_t i = 0; i < len; i++) {
      Serial.print((char)buffer[i]);
    }
    Serial.print(F("]\r\n"));
  }
 
  if (wifi.releaseTCP()) {
    Serial.println(F("Release tcp [OK]"));
  } else {
    Serial.println(F("Release tcp [ERR]"));
  }
  delay(20000);
}
*/


     
