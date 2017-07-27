#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include "gmt.h"
#include "TimeLib.h"

#define pin_resert 16
#define TIME_HEADER  "T"   // Header tag for serial time sync message
#define TIME_REQUEST  7    // ASCII bell character requests a time sync message 

// WiFi information
const char WIFI_SSID[] = "pega-sp";
const char WIFI_PSK[] = "pega@6789";

String serialReceive = "";
boolean serialComplete = false;
bool inHelp = false, inSetTime = false;
bool sig = false;
bool sttled = true;

int lastsecond = 0;

//gmt object
gmt capnhat;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  delay(200);
  // reserve 50 bytes for the serialReceive:
  serialReceive.reserve(50);
  Serial.println("\nESP8266 CLOCK");
  Serial.print("Connecting to "); Serial.println(WIFI_SSID);
  pinMode(13, OUTPUT);// buzzer
  pinMode(12, OUTPUT);// led
  pinMode(pin_resert, OUTPUT);// hard reset
  digitalWrite(pin_resert, HIGH);// hard reset
  // Connect to WiFi
  connectWiFi();
  tit();
  // update GMT time
  while (!capnhat.sync("sangseu.github.io", "/gmt/index.html", 80));
  capnhatthoigian();
  tit();
  printTime();
  setSyncProvider( requestSync);  //set function to call when sync required
  lastsecond = second();
}
void loop() {
  sig = true;
  // alarm even ==============================================
  if (at(6, 0, 0)) {
    buzz();
    if (capnhat.sync("sangseu.github.io", "/gmt/index.html", 80))
      capnhatthoigian();
  }
  else if (at(8, 0, 0)) {
    buzz(); delay(800);
    buzz(); delay(800);
    buzz(); delay(800);
    buzz(); delay(800);
    buzz();
  }
  else if (at(12, 0, 0)) {
    buzz(); delay(800);
    buzz(); delay(800);
    buzz(); delay(800);
    buzz(); delay(800);
    buzz();
  }
  else if (at(13, 30, 0)) {
    buzz(); delay(800);
    buzz(); delay(800);
    buzz(); delay(800);
    buzz(); delay(800);
    buzz();
  }
  // alarm even ==============================================

  if (second() != lastsecond) {
    lastsecond = second();
    if (second() % 5) Serial.print(".");
    else Serial.println(".");

    if ((second() == 0) && ( minute() == 0 || minute() == 30) && sig) {
      tit();
      printTime();
    }
  }

  // serial even ==============================================
  serialEvent();
  if (serialComplete) {// have some Input
    // do somthing
    if (serialReceive.substring(0, 4) == "help") {
      inHelp = true;
      Serial.println("\n__by MiG-0__");
      Serial.println("Simple clock project. Clock just play alarm with buzzer.");
      Serial.println("Type 's' to set time");
      Serial.println("Type 't' to get time");
      Serial.println("Type 'a' to get alarm list");
      Serial.println("Type 'u' to update time");
      Serial.println("Type 'r' to hardreset");
      Serial.println("Type 'q' to quit help");
    }
    else if (inHelp) {
      char* temp = &serialReceive[0];
      switch (temp[0]) {
        case 'u': {
            Serial.println("Update time");
            if (capnhat.sync("sangseu.github.io", "/gmt/index.html", 80))
              capnhatthoigian();
            printTime();
            break;
          }
        case 's': {
            Serial.println("Format: hours:minutes:seconds");
            Serial.println("Exp: 13:06:50");
            inSetTime = true;
            break;
          }
        case 't': {
            printTime();
            break;
          }
        case 'r': {
            Serial.println("Reseting...");
            hardreset();
            break;
          }
        case 'q': {
            inHelp = false;
            Serial.println("Quit help menu");
            break;
          }
        default: break;
      }
    }
    else Serial.println("Type \"help\"");

    if (inSetTime) {
      if (serialReceive.length() == 9) {
        Serial.println("have input");
        // do something
        inSetTime = false;
        inHelp = false;
      }
    }

    // end do somthing

    // clear the string:
    serialReceive = "";
    serialComplete = false;
  }
  // serial even ==============================================

}

void capnhatthoigian() {
  setTime(capnhat.gio(), capnhat.phut(), capnhat.giay(), 23, 2, 2017);
}

// Attempt to connect to WiFi
void connectWiFi() {

  // Set WiFi mode to station (client)
  WiFi.mode(WIFI_STA);

  // Initiate connection with SSID and PSK
  WiFi.begin(WIFI_SSID, WIFI_PSK);

  // Wait for WiFi connection
  while ( WiFi.status() != WL_CONNECTED ) {
    delay(100);
    Serial.print(".");
  }
}

void buzz() {// tone E7
  tone(13, 2637.02);
  delay(100);
  noTone(13);
  delay(50);
  tone(13, 2637.02);
  delay(100);
  noTone(13);
  sig = false;
}
void tit() {
  tone(13, 2637.02);
  delay(100);
  noTone(13);
  delay(50);
}

bool at(int gio, int phut, int giay) {
  if (gio == hour() && phut == minute() && giay == second()) return true;
  else return false;

}

void led() {
  sttled = !sttled;
  digitalWrite(12, !digitalRead(12));
}

void serialEvent() {
  while (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read();
    // add it to the serialString:
    serialReceive += inChar;
    // if the incoming character is a newline, set a flag
    // so the main loop can do something about it:
    if (inChar == '\n') {
      serialComplete = true;
    }
  }
}

void printTime() {
  Serial.print("The time is:\t");
  Serial.print(hour());
  Serial.print(":");
  Serial.print(minute());
  Serial.print(":");
  Serial.println(second());
}

void hardreset() {
  digitalWrite(pin_resert, LOW);
  delay(10);
}

time_t requestSync()
{
  Serial.write(TIME_REQUEST);
  return 0; // the time will be sent later in response to serial mesg
}
