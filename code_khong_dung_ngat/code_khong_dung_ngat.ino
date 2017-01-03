#include <SPI.h>
#define a_1 50
#define max_encoder 4095
#define step_encoder 200
#define sen_i A0
#define sen_u A1
const int dacChipSelectPin = 10;      // set pin 9 as the chip select for the DAC:
int val;
int latchPin = 8;
int clockPin = 7;
int dataPin = 6;
int encoder0PinA = 5;
int encoder0PinB = 4;
int encoder0Pos = 2000;
int encoder0PinALast = LOW;
int n = LOW; int A[10] = {250, 130, 185, 171, 195, 107, 123, 162, 251, 235};
int B[10] = {254, 134, 189, 175, 199, 111, 127, 166, 255, 239};
int C[8] = {223, 191, 127, 239, 253, 251, 247, 254};
int D[2] = {0, 255};
int m = 0;
int l = 0;
float u, i;
unsigned long late, update_display;
void hienthi(float giatri, int m)
{ // hien thi  7 seg 1
  int a1, a2, a3, a4, x;
  switch (m) {
    case 1:
      {
        x = giatri * 1000;
        a1 = x / 1000;
        a2 = x % 1000 / 100;
        a3 = x % 1000 % 100 / 10;
        a4 = x % 1000 % 100 % 10;

        digitalWrite(latchPin, LOW);
        shiftOut(dataPin, clockPin, MSBFIRST, D[1]);
        shiftOut(dataPin, clockPin, MSBFIRST, D[0]);
        shiftOut(dataPin, clockPin, MSBFIRST, C[0]);
        shiftOut(dataPin, clockPin, MSBFIRST, B[a1]);
        digitalWrite(latchPin, HIGH);
        digitalWrite(latchPin, LOW);
        shiftOut(dataPin, clockPin, MSBFIRST, D[1]);
        shiftOut(dataPin, clockPin, MSBFIRST, D[0]);
        shiftOut(dataPin, clockPin, MSBFIRST, C[1]);
        shiftOut(dataPin, clockPin, MSBFIRST, A[a2]);
        digitalWrite(latchPin, HIGH);
        digitalWrite(latchPin, LOW);
        shiftOut(dataPin, clockPin, MSBFIRST, D[1]);
        shiftOut(dataPin, clockPin, MSBFIRST, D[0]);
        shiftOut(dataPin, clockPin, MSBFIRST, C[2]);
        shiftOut(dataPin, clockPin, MSBFIRST, A[a3]);
        digitalWrite(latchPin, HIGH);
        digitalWrite(latchPin, LOW);
        shiftOut(dataPin, clockPin, MSBFIRST, D[1]);
        shiftOut(dataPin, clockPin, MSBFIRST, D[0]);
        shiftOut(dataPin, clockPin, MSBFIRST, C[3]);
        shiftOut(dataPin, clockPin, MSBFIRST, A[a4]);
        digitalWrite(latchPin, HIGH);

        break;
      }
    // hien thi  7 seg 2
    case 2:
      {
        if (giatri >= 10) {// 10.00-99.99
          a1 = (int)giatri / 10;
          a2 = (int)giatri % 10;
          x = (int)(giatri * 100) % 1000 % 100;
          a3 = x / 10;
          a4 = x % 10;

          digitalWrite(latchPin, LOW);
          shiftOut(dataPin, clockPin, MSBFIRST, D[1]);
          shiftOut(dataPin, clockPin, MSBFIRST, D[0]);
          shiftOut(dataPin, clockPin, MSBFIRST, C[4]);
          shiftOut(dataPin, clockPin, MSBFIRST, A[a1]);
          digitalWrite(latchPin, HIGH);
          digitalWrite(latchPin, LOW);
          shiftOut(dataPin, clockPin, MSBFIRST, D[1]);
          shiftOut(dataPin, clockPin, MSBFIRST, D[0]);
          shiftOut(dataPin, clockPin, MSBFIRST, C[5]);
          shiftOut(dataPin, clockPin, MSBFIRST, B[a2]);
          digitalWrite(latchPin, HIGH);
          digitalWrite(latchPin, LOW);
          shiftOut(dataPin, clockPin, MSBFIRST, D[1]);
          shiftOut(dataPin, clockPin, MSBFIRST, D[0]);
          shiftOut(dataPin, clockPin, MSBFIRST, C[6]);
          shiftOut(dataPin, clockPin, MSBFIRST, A[a3]);
          digitalWrite(latchPin, HIGH);
          digitalWrite(latchPin, LOW);
          shiftOut(dataPin, clockPin, MSBFIRST, D[1]);
          shiftOut(dataPin, clockPin, MSBFIRST, D[0]);
          shiftOut(dataPin, clockPin, MSBFIRST, C[7]);
          shiftOut(dataPin, clockPin, MSBFIRST, A[a4]);
          digitalWrite(latchPin, HIGH);
        }
        else {
          x = giatri * 1000;//0.000-9.999
          a1 = x / 1000;
          a2 = x % 1000 / 100;
          a3 = x % 1000 % 100 / 10;
          a4 = x % 1000 % 100 % 10;
          digitalWrite(latchPin, LOW);
          shiftOut(dataPin, clockPin, MSBFIRST, D[1]);
          shiftOut(dataPin, clockPin, MSBFIRST, D[0]);
          shiftOut(dataPin, clockPin, MSBFIRST, C[4]);
          shiftOut(dataPin, clockPin, MSBFIRST, B[a1]);
          digitalWrite(latchPin, HIGH);
          digitalWrite(latchPin, LOW);
          shiftOut(dataPin, clockPin, MSBFIRST, D[1]);
          shiftOut(dataPin, clockPin, MSBFIRST, D[0]);
          shiftOut(dataPin, clockPin, MSBFIRST, C[5]);
          shiftOut(dataPin, clockPin, MSBFIRST, A[a2]);
          digitalWrite(latchPin, HIGH);
          digitalWrite(latchPin, LOW);
          shiftOut(dataPin, clockPin, MSBFIRST, D[1]);
          shiftOut(dataPin, clockPin, MSBFIRST, D[0]);
          shiftOut(dataPin, clockPin, MSBFIRST, C[6]);
          shiftOut(dataPin, clockPin, MSBFIRST, A[a3]);
          digitalWrite(latchPin, HIGH);
          digitalWrite(latchPin, LOW);
          shiftOut(dataPin, clockPin, MSBFIRST, D[1]);
          shiftOut(dataPin, clockPin, MSBFIRST, D[0]);
          shiftOut(dataPin, clockPin, MSBFIRST, C[7]);
          shiftOut(dataPin, clockPin, MSBFIRST, A[a4]);
          digitalWrite(latchPin, HIGH);
        }
        break;
      }
  }
}
void setup() {
  // Set DAC
  pinMode (dacChipSelectPin, OUTPUT);
  // set the ChipSelectPins high initially:
  digitalWrite(dacChipSelectPin, HIGH);
  // initialise SPI:
  SPI.begin();
  SPI.setBitOrder(MSBFIRST);         // Not strictly needed but just to be sure.
  SPI.setDataMode(SPI_MODE0);        // Not strictly needed but just to be sure.
  // Set Num van
  pinMode (encoder0PinA, INPUT);
  pinMode (encoder0PinB, INPUT);
  Serial.begin (9600);
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
  late = millis();
  update_display = millis();

}

void loop() {
  setDac(encoder0Pos, 0);
  n = digitalRead(encoder0PinA);
  if ((encoder0PinALast == LOW) && (n == HIGH)) {
    if (digitalRead(encoder0PinB) == LOW) {
      encoder0Pos--;
      if (m++ >= 5) {
        if (millis() - late < a_1)
        { encoder0Pos = encoder0Pos - step_encoder;
          m = 0;
        }
      }
      late = millis();
    } else {
      encoder0Pos++;
      if (m++ >= 5) {
        if (millis() - late < a_1)
        { encoder0Pos = encoder0Pos + step_encoder;
          m = 0;
        }
      }
      late = millis();
    }
    setDac(encoder0Pos, 0);
  }


  if (millis() - update_display > 250)
  { u = 1.0 * analogRead(sen_u) / 255;
    i = 1.0 * analogRead(sen_i) / 255;
    update_display = millis();
  }
  
  hienthi(i, 1);
  hienthi(u, 2);

  encoder0PinALast = n;
}

void setDac(int value, int channel) {
  if (value > max_encoder) value = max_encoder;
  byte dacRegister = 0b00110000;                        // Sets default DAC registers B00110000, 1st bit choses DAC, A=0 B=1, 2nd Bit bypasses input Buffer, 3rd bit sets output gain to 1x, 4th bit controls active low shutdown. LSB are insignifigant here.
  int dacSecondaryByteMask = 0b0000000011111111;        // Isolates the last 8 bits of the 12 bit value, B0000000011111111.
  byte dacPrimaryByte = (value >> 8) | dacRegister;     //Value is a maximum 12 Bit value, it is shifted to the right by 8 bytes to get the first 4 MSB out of the value for entry into th Primary Byte, then ORed with the dacRegister
  byte dacSecondaryByte = value & dacSecondaryByteMask; // compares the 12 bit value to isolate the 8 LSB and reduce it to a single byte.
  // Sets the MSB in the primaryByte to determine the DAC to be set, DAC A=0, DAC B=1
  switch (channel) {
    case 0:
      dacPrimaryByte &= ~(1 << 7);
      break;
    case 1:
      dacPrimaryByte |= (1 << 7);
  }
  noInterrupts(); // disable interupts to prepare to send data to the DAC
  digitalWrite(dacChipSelectPin, LOW); // take the Chip Select pin low to select the DAC:
  SPI.transfer(dacPrimaryByte); //  send in the Primary Byte:
  SPI.transfer(dacSecondaryByte);// send in the Secondary Byte
  digitalWrite(dacChipSelectPin, HIGH); // take the Chip Select pin high to de-select the DAC:
  interrupts(); // Enable interupts
}
