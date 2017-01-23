//get time http://sangseu.github.io/writing/gmt/
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include "gmt.h"

#define pin_resert 16

// WiFi information
const char WIFI_SSID[] = "HKBike - Sanpham";
const char WIFI_PSK[] = "hkbike@6789";

String serialReceive = "";
boolean serialComplete = false;
bool inHelp = false, inSetTime = false;
bool sig = false;
bool sttled = true;

//gmt object
gmt capnhat;

unsigned long deltaMicroSeconds = 0;
unsigned long timeUpdate = 0;
unsigned long timeLast = 0;

//Time start Settings:

int seconds = 0;
int minutes = 0;
int hours = 0;
int days = 0;

int lastSeconds = 0;

//Accuracy settings

int dailyErrorFast = 0; // set the average number of milliseconds your microcontroller's time is fast on a daily basis
int dailyErrorBehind = 0; // set the average number of milliseconds your microcontroller's time is behind on a daily basis

int correctedToday = 1; // do not change this variable, one means that the time has already been corrected today for the error in your boards crystal. This is true for the first day because you just set the time when you uploaded the sketch.

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
  buzz();
  // update GMT time
  while (!capnhat.sync("sangseu.github.io", "/gmt/index.html", 80));
  capnhatthoigian();
  printTime();
}
void loop() {
  deltaMicroSeconds = (millis() - timeLast) / 1000;
  if (deltaMicroSeconds) {
    timeLast = millis();
    seconds += 1;
    Serial.println(seconds);
  }

  if (seconds == 60) {
    seconds = 0;
    minutes = minutes + 1;
  }

  // every second even ==============================================
  if (seconds != lastSeconds) {

    sig = true;

    //toogle led
    led();

    // buzz every 30 minutes
    if (lastSeconds == 59) {
      if (minutes == 0 || minutes == 30) {
        if (sig) buzz();
      }
      printTime();
    }

    //at second 0, check update, change counter hours
    if (seconds == 0)
    {
      // change counter
      if (minutes == 15) {
        if (capnhat.sync("sangseu.github.io", "/gmt/index.html", 80)) {
          capnhatthoigian();
          Serial.println("Updating time...");
        }
      }

      //if one minute has passed, start counting milliseconds from zero again and add one minute to the clock.
      if (minutes == 60) {
        minutes = 0;
        hours = hours + 1;
      }

      // if one hour has passed, start counting minutes from zero and add one hour to the clock
      if (hours == 24) {
        hours = 0;
      }

      if (hours == 6) {
        hardreset();
      }
    }

    lastSeconds = seconds;
  }
  // end every second even ==============================================

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
  else if (at(17, 30, 0)) {
    buzz(); delay(800);
    buzz(); delay(800);
    buzz(); delay(800);
    buzz(); delay(800);
    buzz();
  }
  // alarm even ==============================================

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
  hours = capnhat.gio();
  minutes = capnhat.phut();
  seconds = capnhat.giay();
  timeLast = millis();
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

bool at(int gio, int phut, int giay) {
  if (gio == hours && phut == minutes && giay == seconds) return true;
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
  Serial.print(hours);
  Serial.print(":");
  Serial.print(minutes);
  Serial.print(":");
  Serial.println(seconds);
}

void hardreset() {
  digitalWrite(pin_resert, LOW);
  delay(10);
}

