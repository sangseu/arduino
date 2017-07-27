#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>

#include <TimeLib.h>
#include <TimeAlarms.h>

#include "gmt.h"

#define pin_reset 16
#define pin_buzz 13
#define pin_led 12
#define pin_relay 4

//gmt object
gmt capnhat;

AlarmId id;

const byte DNS_PORT = 53;
IPAddress apIP(10, 10, 10, 10);
DNSServer dnsServer;
ESP8266WebServer webserver(80);

// WiFi information
const char WIFI_SSID[] = "pega-sp";
const char WIFI_PSK[] = "pega@6789";

const char *ssid = "pega.timer";
const char *password = "pega@6789";

const unsigned int defaultGio = 0;
const unsigned int defaultPhut = 0;
const unsigned int maxAlarm = 15;
const unsigned int EEPROMnumberAlarmsAddr = 1;
const unsigned int EEPROMstartAlarmInfoAddr = 2;
const unsigned int tryConnect = 60;
const unsigned int tryUpdate = 10;

static unsigned int numberAlarms = 0;
static unsigned int alarmInfo[4 * maxAlarm + 1];
static AlarmId ids[maxAlarm + 1];

/*
  alarm frame:
  on/off | gio | phut | do_dai
*/

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("\nESP8266 ALARM");

  initGPIO();

  ESP.eraseConfig();
  delay(500);
  /*Connect to WiFi*/
  connectWiFi();
  startServer();


  EEPROM.begin(512);
  delay(10);

  upDateTime();

  /* Load alarmm from eeprom or init eeprom at fisrt run */
  if (EEPROMreadInt(0) == 252) { // 0xAA
    Serial.println("Read config...");
    Serial.println("Alarm: ");
    // create the alarms, to trigger at specific times
    loadAlarms();
    Serial.println();
  }
  else {
    Serial.println("first run, write eeprom[0] = 252");
    EEPROM.write(0, 252); // write at first run
    // clear eeprom addr 1->512 to 0
    for (int i = 1; i < 512; i++) {
      EEPROM.write(i, 0);
    }
    EEPROM.commit();
  }
}

void initGPIO() {
  pinMode(pin_led, OUTPUT);
  pinMode(pin_buzz, OUTPUT);
  pinMode(pin_relay, OUTPUT);
  pinMode(pin_reset, OUTPUT);

  digitalWrite(pin_led, LOW);
  digitalWrite(pin_buzz, LOW);
  digitalWrite(pin_relay, LOW);
  digitalWrite(pin_reset, HIGH);
}

/* when reload alarm, need clean old alarm */
void loadAlarms() {
  int i = 0;
  for (i = 0; i < maxAlarm; i++ ) {
    Alarm.free(ids[i]);
  }

  int temp = 0;
  for (i = 0; i < maxAlarm * 4; i++) {
    temp = EEPROMreadInt(EEPROMstartAlarmInfoAddr + i);
    alarmInfo[i] = temp;

    if (!(i % 4)) {
      Serial.println();
    }
    Serial.print(temp, DEC); Serial.print(' ');
  }
  Serial.println();

  unsigned int point = 0;
  for (i = 0; i < maxAlarm; i++) {
    point = i * 4;
    if (alarmInfo[point] == 1) {
      switch (alarmInfo[point + 3]) {
        case 2: ids[i] = Alarm.alarmRepeat((int)alarmInfo[point + 1], (int)alarmInfo[point + 2], 0, buzz2); break;
        case 5: ids[i] = Alarm.alarmRepeat((int)alarmInfo[point + 1], (int)alarmInfo[point + 2], 0, buzz5); break;
        case 10: ids[i] = Alarm.alarmRepeat((int)alarmInfo[point + 1], (int)alarmInfo[point + 2], 0, buzz10); break;
        default: ids[i] = Alarm.alarmRepeat((int)alarmInfo[point + 1], (int)alarmInfo[point + 2], 0, buzz5); break;
      }
      Serial.print("Added alarm: "); Serial.println(i + 1);
    }
  }
  /* add alarm repeat check WiFi and reset esp day-by-day */
  Alarm.alarmRepeat(6, 0, 0, resetESP);
}

void buzz2() {
  Serial.println("start buzz");

  tit();
  digitalWrite(pin_relay, HIGH);
  delay(2000); /*2 giay*/
  digitalWrite(pin_relay, LOW);

  Serial.println("end buzz");
}

void buzz5() {
  Serial.println("start buzz");

  tit();
  digitalWrite(pin_relay, HIGH);
  delay(5000); /*5 giay*/
  digitalWrite(pin_relay, LOW);

  Serial.println("end buzz");
}
void buzz10() {
  Serial.println("start buzz");

  tit();
  digitalWrite(pin_relay, HIGH);
  delay(10000); /*10 giay*/
  digitalWrite(pin_relay, LOW);

  Serial.println("end buzz");
}

void tit() {
  tone(pin_buzz, 2637.02);
  delay(100);
  noTone(pin_buzz);
  delay(50);
}

void upDateTime() {
  // update GMT time
  while (!capnhat.sync("sangseu.github.io", "/gmt/index.html", 80)) {
    led();
  }
  setTime(capnhat.gio(), capnhat.phut(), capnhat.giay(), 1, 1, 11); // delault day-mont-year
  tit();
}

void resetESP() {
  Serial.print("Re-connecting to \""); Serial.print(WIFI_SSID); Serial.println("\"");
  int count = tryConnect;
  // Wait for WiFi connection
  while ( WiFi.status() != WL_CONNECTED &&  count > 0) {
    WiFi.begin(WIFI_SSID, WIFI_PSK);
    count--;
    Serial.print("Try: "); Serial.println(count);
    delay(5000);
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Connect OK");
    // have WiFi connect
    // update Time
    count = tryUpdate;// retry update Time
    while (!capnhat.sync("sangseu.github.io", "/gmt/index.html", 80) && count > 0) {
      count--;
      Serial.print("Try: "); Serial.println(count);
      led();
    }
    if (capnhat.sync("sangseu.github.io", "/gmt/index.html", 80)) {
      Serial.println("Update OK");
      // if can connect WiFi, can update Time -> RESET ESP
      digitalWrite(pin_reset, LOW);
      delay(50);
    }
    else Serial.println("Update FAIL, continue running");
  }
  else Serial.println("Re-connect FAIL, continue running");
}

int EEPROMreadInt(int addr) {
  return (int)EEPROM.read(addr);
}

void handleRoot() {
  String msg;
  msg = "<html>"
        "<head>"
        "<title>Xưởng lắp ráp</title>"
        "<meta charset=\"utf-8\" />"
        "<meta name=viewport content=\"width=device-width, initial-scale=1\">"
        "/<head>"

        "<body>"
        "<h1>Hẹn giờ báo chuông</h1>"

        "<h2>Danh sách giờ đặt:</h2>";

  msg += "<form class=\"form\" method=\"post\" action=\"/config\" >";

  for (int i = 0; i < maxAlarm * 4; i += 4) {
    msg += "<p>";

    msg += "Bật";
    msg += "<input type=\"radio\" name=";
    msg += 1 + i; // FIELD .4
    msg += " value=\"1\""; // field 4 - value 1
    if (alarmInfo[i] == 1 ) {
      msg += " checked>";
    }
    else msg += " >";

    msg += "Tắt";
    msg += "<input type=\"radio\" name=";
    msg += 1 + i; // FIELD .4
    msg += " value=\"0\""; // field 4 - value 0
    if (alarmInfo[i] != 1 ) {
      msg += " checked>";
    }
    else msg += " >";

    msg += "<input type=\"text\" style=\"width: 50px\" name=";
    msg += 2 + i; // FIELD .1
    msg += " id=\"gio\" required"
           " value=\"";
    msg += alarmInfo[i + 1]; // gio
    msg += "\""
           " pattern=\"\\b([0-9]{1,2})\\b\" />";
    msg += " : ";

    msg += "<input type=\"text\" style=\"width: 50px\" name=";
    msg += 3 + i; // FIELD .2
    msg += " id=\"phut\" required"  // field 2
           " value=\"";
    msg += alarmInfo[i + 2]; // phut
    msg += "\""
           " pattern=\"\\b([0-9]{1,2})\\b\" />";

    msg += ", Độ dài: ";
    msg += "<select name=";
    msg += 4 + i; // FIELD .3
    msg += " id=\"do_dai\" pattern=\"\\b([0-9]{1,2})\\b\" required>"; // field 3

    switch (alarmInfo[i + 3]) {
      case 2: {
          msg += "<option value=\"2\" selected>2</option>"
                 "<option value=\"5\">5</option>"
                 "<option value=\"10\">10</option>";
          break;
        }
      case 5: {
          msg += "<option value=\"2\">2</option>"
                 "<option value=\"5\" selected>5</option>"
                 "<option value=\"10\">10</option>";
          break;
        }
      case 10: {
          msg += "<option value=\"2\">2</option>"
                 "<option value=\"5\">5</option>"
                 "<option value=\"10\" selected>10</option>";
          break;
        }
      default : {
          msg += "<option value=\"2\">2</option>"
                 "<option value=\"5\" selected>5</option>"
                 "<option value=\"10\">10</option>";
          break;
        }
    }
    msg += "</select>";
    msg += " giây";

    msg += "</p>";
  }

  msg += "<input type=\"submit\" value=\"Cập nhật\">"
         "</form>";
  msg += "<form action=\"/delete\">"
         "<input type=\"submit\" value=\"Xóa tất cả hẹn giờ\">"
         "</form>"

         "</body>"

         "</html>";

  webserver.send ( 200, "text/html", msg );
}

void handleConfig() {
  //debug
  String message = "Config results\n\n";
  message += "URI: ";
  message += webserver.uri();
  message += "\nMethod: ";
  message += ( webserver.method() == HTTP_GET ) ? "GET" : "POST";
  message += "\nArguments: ";
  message += webserver.args();
  message += "\n";
  /* handle POST and add alarm */
  for ( uint8_t i = 0; i < webserver.args(); i++ ) {
    message += " " + webserver.argName ( i ) + ": " + webserver.arg ( i ) + "\n";
    EEPROM.write(EEPROMstartAlarmInfoAddr + i, webserver.arg ( i ).toInt());
  }
  EEPROM.commit();
  delay(100);

  /* free all alarm memory and reload alarm from EEPROM */
  loadAlarms();

  Serial.println(message);
  // debug

  webserver.send ( 200, "text/html", "OK Sky" );
}

void handleDelete() {
  //debug
  String message = "\nDelete results\n\n";
  message += "URI: ";
  message += webserver.uri();
  message += "\nMethod: ";
  message += ( webserver.method() == HTTP_GET ) ? "GET" : "POST";
  message += "\nArguments: ";
  message += webserver.args();
  message += "\n";
  for ( uint8_t i = 0; i < webserver.args(); i++ ) {
    message += " " + webserver.argName ( i ) + ": " + webserver.arg ( i ) + "\n";
  }
  // debug

  // delete all alarm in EEPROM
  Serial.println(message);
  for (int i = EEPROMstartAlarmInfoAddr; i < maxAlarm * 3; i++) {
    EEPROM.write(i, 0);
  }
  EEPROM.write(EEPROMnumberAlarmsAddr, 0);
  EEPROM.commit();

  webserver.send ( 200, "text/html", "Clear alarm OK" );
}

void handleNotFound() {
  handleRoot();
}
/*=============================================================================*/
void loop() {
  dnsServer.processNextRequest();
  webserver.handleClient();

  /* update Alarm */
  Alarm.delay(0);
  if (!(millis() % 1000)) {
    led();
    printNow();
  }
  /*
    if (!(millis() % 20000)) {
      resetESP();
    }
  */
}
/*=============================================================================*/

void printNow() {
  // digital clock display of the time
  Serial.print(hour());
  printDigits(minute());
  printDigits(second());
  Serial.println();
}

void printDigits(int digits) {
  Serial.print(":");
  if (digits < 10)
    Serial.print('0');
  Serial.print(digits);
}

// Attempt to connect to WiFi
void connectWiFi() {
  Serial.print("Connecting to \""); Serial.print(WIFI_SSID); Serial.println("\"");

  // Initiate connection with SSID and PSK
  WiFi.begin(WIFI_SSID, WIFI_PSK);

  // Wait for WiFi connection
  while ( WiFi.status() != WL_CONNECTED ) {
    WiFi.begin(WIFI_SSID, WIFI_PSK);
    delay(5000);
    Serial.print(".");
  }
  tit();
  Serial.println("\nConnected");
}

void startServer() {
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(ssid, password);
  delay(500);
  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());

  dnsServer.start(DNS_PORT, "pega.timer", apIP);

  webserver.on ( "/", handleRoot );
  webserver.on ( "/config", handleConfig );
  webserver.on ( "/delete", handleDelete );
  webserver.onNotFound( handleNotFound );
  webserver.begin();
}

void led() {
  digitalWrite(pin_led, !digitalRead(pin_led));
}

