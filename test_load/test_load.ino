//FastPWMadc
#include <TimerOne.h>
#include <FastPWMdac.h>
FastPWMdac fastPWMdac;
long last = 0;
int set_point = 0;
char cmd;

void setup() {
  fastPWMdac.init(10, 10);
  Serial.begin(9600);
  fastPWMdac.analogWrite10bit(set_point);
  pinMode(A0, INPUT);
  pinMode(A1, INPUT);
  digitalWrite(A0, LOW);
  digitalWrite(A1, LOW);
  last = millis();
}
void loop() {
  if (millis() - last > 100) {
    Serial.print(analogRead(A0));
    Serial.print("\t");
    Serial.print(analogRead(A1));
    Serial.print("\t");
    Serial.println(set_point);
    last = millis();
  }
  cmd = Serial.read();
  if(cmd=='u'){
    set_point +=2;
    fastPWMdac.analogWrite10bit(set_point);
  }
  if(cmd=='d'){
    set_point -=2;
    fastPWMdac.analogWrite10bit(set_point);
  }
  if(cmd=='0'){
    set_point = 0;
    fastPWMdac.analogWrite10bit(set_point);
  }
}

