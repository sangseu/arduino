#include <WiFiClient.h>
#include "gmt.h"
bool gmt::sync(char host[], char url[], uint16_t port)
{
  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  String gmt_string;
  String headerString;
  bool _return;

  Serial.print("connecting to ");
  Serial.println(host);

  // Attempt to make a connection to the remote server
  if ( !client.connect(host, port) ) {
    Serial.println("connection failed");
    _return = false;
  }

  // Make an HTTP GET request
  Serial.print("Requesting URL: ");
  Serial.println(url);
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Connection: close\r\n\r\n");

  delay(10);

  client.setTimeout(HTTP_TIMEOUT);
  int i = 0;
  while (client.available()) {
    headerString += char(client.read());
    if (i++ > 400) break;
  }
  int ds = headerString.indexOf("Date: ");
  gmt_string = headerString.substring(ds + 23, ds + 32);
  // date and time: Sun, 29 May 2016 10:00:14
  // gmt_string: 10:00:14
  Serial.print("Header time: ");
  Serial.println(gmt_string);

  headerString = "";// empty String
  _gio = gmt_string.substring(0, 2);
  _phut = gmt_string.substring(3, 5);
  _giay = gmt_string.substring(6, 8);

  if (_gio.toInt() || _phut.toInt() || _giay.toInt()) _return = true;
  else _return = false;
  return _return;
}

int gmt::gio()
{
  return _gio.toInt() + 7 > 24 ? _gio.toInt() + 7 - 24 : _gio.toInt() + 7;
}

int gmt::phut()
{
  return _phut.toInt();
}

int gmt::giay()
{
  return _giay.toInt();
}

