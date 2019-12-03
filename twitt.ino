#include <M5StickC.h>
#include <Wire.h>
#include <WiFi.h>
#include <IFTTTWebhook.h>
#include "tweet.h"

#define WIFI_SSID ""
#define WIFI_PASSWORD ""

#define IFTTT_API_KEY ""
#define IFTTT_EVENT_NAME "send_tweet"
#define IFTTT_WEBHOOK_DEBUG

#define CARDKB_ADDR 0x5F
int x = 1;
int y = 20;
int char_x = 6;
int char_y = 9;
String message = "";
void setup() {
  M5.begin();
  Serial.begin(115200);
  Wire.begin();
  //pinMode(5, INPUT);
  //digitalWrite(5, HIGH);
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setRotation(3);
  M5.Lcd.setCursor(1, 1);
  M5.Lcd.setTextColor(BLUE);
  M5.Lcd.setTextSize(1);
  M5.Lcd.printf("Tweeter un message");
  M5.Lcd.println();
  M5.Lcd.setTextColor(WHITE);
  //M5.Lcd.printf(">>");
  Serial.println();

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  setClock();
}
void loop() {
  Wire.begin();
  Wire.requestFrom(CARDKB_ADDR, 1);
  while (Wire.available()) {
    char c = Wire.read(); // receive a byte as characterif
    if (c != 0) {
      if (c == 8) { // Erase characters
        if ((x == 1) && (y == 20)) { // If this isn't the beginning of the text
          Serial.println("Can't ERASED");
        } else {
          if ((y != 20) && (x == 1)) { //If this is the beginning but not the last line
            y = y - char_y;
            x = 157;
          }
          Serial.println("ERASED");
          M5.Lcd.fillRect(x - char_x, y - char_y, char_x, char_y, BLACK);
          x = x - char_x;
          M5.Lcd.setCursor(x, y - char_y);
          Serial.print("X:");
          Serial.println(x);
          Serial.print("Y:");
          Serial.println(y);
          message = message.substring(0, message.length() - 1);
          Serial.print("Message:");
          Serial.println(message);
        }
      } else {
        if ( (c != 27) && (c != 9) && (c != 181) && (c != 182) && (c != 180) && (c != 183) && (c != 13) ) { //ESC/TAB/UP/DOWN/LEFT/RIGHT/ENTER
          if (message.length()  <= 128) {
            if (x == 157) {
              y = y + char_y;
              x = 1;
            }
            M5.Lcd.setCursor(x, y - char_y);
            M5.Lcd.printf("%c", c);
            Serial.print("CHAR:");
            Serial.println(c, DEC);
            message = message + c;
            x = x + char_x;
            Serial.print("X:");
            Serial.println(x);
            Serial.print("Y:");
            Serial.println(y);
            M5.Lcd.setCursor(x, y - char_y);
            Serial.print("Message:");
            Serial.println(message);
          }
        }
        else {
          if (c == 13) { //ENTER
            Serial.println("Send tweet");
            M5.Lcd.pushImage(0, 0, 160, 80, (uint16_t *)tweet);
            Serial.print("Message:");
            Serial.println(message);
            Serial.print("URLENCODED:");
            Serial.println(urlencode(message));
            String msg = urlencode(message);
            const char* msg2 = msg.c_str();
            Serial.print("msg:");
            Serial.println(msg2);
            IFTTTWebhook wh(IFTTT_API_KEY, IFTTT_EVENT_NAME);
            wh.trigger(msg2);
            //Serial.println(http_code);
            ESP.restart();
          }
        }
      }
    }
  }
}

void setClock() {
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");

  Serial.print(F("Waiting for NTP time sync: "));
  time_t nowSecs = time(nullptr);
  while (nowSecs < 8 * 3600 * 2) {
    delay(500);
    Serial.print(F("."));
    yield();
    nowSecs = time(nullptr);
  }

  Serial.println();
  struct tm timeinfo;
  gmtime_r(&nowSecs, &timeinfo);
  Serial.print(F("Current time: "));
  Serial.print(asctime(&timeinfo));
}

String urlencode(String str)
{
    Serial.print("STR:");
    Serial.println(str);
    String encodedString="";
    char c;
    char code0;
    char code1;
    char code2;
    for (int i =0; i < str.length(); i++){
      c=str.charAt(i);
      if (c == ' '){
        encodedString+= '+';
      } else if (isalnum(c)){
        encodedString+=c;
      } else{
        code1=(c & 0xf)+'0';
        if ((c & 0xf) >9){
            code1=(c & 0xf) - 10 + 'A';
        }
        c=(c>>4)&0xf;
        code0=c+'0';
        if (c > 9){
            code0=c - 10 + 'A';
        }
        code2='\0';
        encodedString+='%';
        encodedString+=code0;
        encodedString+=code1;
        encodedString+=code2;
      }
      yield();
    }
    Serial.print("ENCODED STRING ENDED:");
    Serial.println(encodedString);
    return encodedString;
}
