//get time http://sangseu.github.io/writing/gmt/
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include "gmt.h"

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

unsigned long timeNow = 0;
unsigned long timeUpdate = 0;
unsigned long timeLast = 0;

//Time start Settings:

int startingHour = 12; // set your starting hour here, not below at int hour. This ensures accurate daily correction of time

int seconds = 0;
int minutes = 33;
int hours = startingHour;
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
  // Connect to WiFi
  connectWiFi();
  buzz();
  // update GMT time
  while (!capnhat.sync("sangseu.github.io", "/writing/gmt/index.html", 80));
  capnhatthoigian();
}
void loop() {
  timeNow = capnhat.giay() + (millis() - timeUpdate) / 1000; // the number of milliseconds that have passed since capnhatthoigian()
  seconds = timeNow - timeLast;//the number of seconds that have passed since the last time 60 seconds was reached.

  if (seconds == 60) {
    timeLast = timeNow;// reset delta_second, delta_second 0->60
    minutes = minutes + 1;
  }

  //if one minute has passed, start counting milliseconds from zero again and add one minute to the clock.
  if (minutes == 60) {
    minutes = 0;
    hours = hours + 1;
  }

  // if one hour has passed, start counting minutes from zero and add one hour to the clock

  if (hours == 24) {
    hours = 0;
    days = days + 1;
  }

  //if 24 hours have passed , add one day

  if (hours == (24 - startingHour) && correctedToday == 0) {
    delay(dailyErrorFast * 1000);
    seconds = seconds + dailyErrorBehind;
    correctedToday = 1;
  }

  //every time 24 hours have passed since the initial starting time and it has not been reset this day before, add milliseconds or delay the progran with some milliseconds.
  //Change these varialbes according to the error of your board.
  // The only way to find out how far off your boards internal clock is, is by uploading this sketch at exactly the same time as the real time, letting it run for a few days
  // and then determine how many seconds slow/fast your boards internal clock is on a daily average. (24 hours).

  if (hours == 24 - startingHour + 2) {
    correctedToday = 0;
  }

  //let the sketch know that a new day has started for what concerns correction, if this line was not here the arduiono
  // would continue to correct for an entire hour that is 24 - startingHour.

  // alarm even ==============================================
  sig = true;
  if (at(6, 0, 0)) {
    buzz();
    if (capnhat.sync("sangseu.github.io", "/writing/gmt/index.html", 80))
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
      Serial.println("Type 'q' to quit help");
    }
    else if (inHelp) {
      char* temp = &serialReceive[0];
      switch (temp[0]) {
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

  // frequency even ==============================================
  if (seconds != lastSeconds) {
    led();
    // buzz every 30 minutes
    if (lastSeconds == 60) {
      printTime();
      if (minutes == 0 || minutes == 30) {
        buzz();
      }
    }
    lastSeconds = seconds;
  }
  // frequency even ==============================================
}

void capnhatthoigian() {
  hours = capnhat.gio();
  minutes = capnhat.phut();
  seconds = capnhat.giay();
  timeUpdate = millis();
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

void buzz() {// tone C7
  tone(13, 2093);
  delay(100);
  noTone(13);
  delay(50);
  tone(13, 2093);
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

