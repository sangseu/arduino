#include <SPI.h>
#include <EEPROM.h>
//adc eliminating noise
// include the ResponsiveAnalogRead library
#include <ResponsiveAnalogRead.h>
#include <eRCaGuy_analogReadXXbit.h>

#define a_1 50
#define max_encoder 4095
#define min_encoder 0
#define step_encoder 100
#define latchPin 8
#define clockPin 7
#define dataPin 6
#define encoder0PinA 5
#define encoder0PinB 4
#define encoderPush 9


const uint8_t sen_i = A0;
const uint8_t sen_u = A1;
const int dacChipSelectPin = 10;      // set pin 9 as the chip select for the DAC:
const float MAX_READING_10_bit = 10.23;
const float i_MAX_READING_10_bit = 1023.0;
const float MAX_READING_11_bit = 2046.0;
const float MAX_READING_12_bit = 4092.0;
int analog_reading; //the ADC reading
unsigned int num_samples = 1;
const float Vref = 1.096;
//const float Vref = 5;

// make a ResponsiveAnalogRead object, pass in the pin, and either true or false depending on if you want sleep enabled
// enabling sleep will cause values to take less time to stop changing and potentially stop changing more abruptly,
//   where as disabling sleep will cause values to ease into their correct position smoothly and more accurately
ResponsiveAnalogRead analog_u(sen_u, true, 0.025);
ResponsiveAnalogRead analog_i(sen_i, true);

// the next optional argument is snapMultiplier, which is set to 0.01 by default
// you can pass it a value from 0 to 1 that controls the amount of easing
// increase this to lessen the amount of easing (such as 0.1) and make the responsive values more responsive
// but doing so may cause more noise to seep through if sleep is not enabled
//=====================
//instantiate an object of this library class; call it "adc"
eRCaGuy_analogReadXXbit adc;

//push=================
int cur, prev, stt_push;
long millis_held, secs_held, prev_secs_held;
unsigned long button_first;
//push=================

int val;

int addr_encoder0Pos = 0;
int encoder0Pos = 3000;
int encoder0PinALast = LOW;
int n = LOW; int A[10] = {250, 130, 185, 171, 195, 107, 123, 162, 251, 235};// number without dot
int B[10] = {254, 134, 189, 175, 199, 111, 127, 166, 255, 239};// number with dot
int C[8] = {223, 191, 127, 239, 253, 251, 247, 254};// select LED
int D[2] = {0, 255};
int m = 0;
int l = 0;
float u, u_temp, i, i_temp;
int u_count = 0;
int i_count = 0;

bool have_change = 0;

unsigned long late, t_update_display;
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
        //shiftOut(dataPin, clockPin, MSBFIRST, D[1]);
        //shiftOut(dataPin, clockPin, MSBFIRST, D[0]);
        shiftOut(dataPin, clockPin, MSBFIRST, C[0]);
        shiftOut(dataPin, clockPin, MSBFIRST, B[a1]);
        digitalWrite(latchPin, HIGH);
        digitalWrite(latchPin, LOW);
        //shiftOut(dataPin, clockPin, MSBFIRST, D[1]);
        //shiftOut(dataPin, clockPin, MSBFIRST, D[0]);
        shiftOut(dataPin, clockPin, MSBFIRST, C[1]);
        shiftOut(dataPin, clockPin, MSBFIRST, A[a2]);
        digitalWrite(latchPin, HIGH);
        digitalWrite(latchPin, LOW);
        //shiftOut(dataPin, clockPin, MSBFIRST, D[1]);
        //shiftOut(dataPin, clockPin, MSBFIRST, D[0]);
        shiftOut(dataPin, clockPin, MSBFIRST, C[2]);
        shiftOut(dataPin, clockPin, MSBFIRST, A[a3]);
        digitalWrite(latchPin, HIGH);
        digitalWrite(latchPin, LOW);
        //shiftOut(dataPin, clockPin, MSBFIRST, D[1]);
        //shiftOut(dataPin, clockPin, MSBFIRST, D[0]);
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
          //shiftOut(dataPin, clockPin, MSBFIRST, D[1]);
          //shiftOut(dataPin, clockPin, MSBFIRST, D[0]);
          shiftOut(dataPin, clockPin, MSBFIRST, C[4]);
          shiftOut(dataPin, clockPin, MSBFIRST, A[a1]);
          digitalWrite(latchPin, HIGH);
          digitalWrite(latchPin, LOW);
          //shiftOut(dataPin, clockPin, MSBFIRST, D[1]);
          //shiftOut(dataPin, clockPin, MSBFIRST, D[0]);
          shiftOut(dataPin, clockPin, MSBFIRST, C[5]);
          shiftOut(dataPin, clockPin, MSBFIRST, B[a2]);
          digitalWrite(latchPin, HIGH);
          digitalWrite(latchPin, LOW);
          //shiftOut(dataPin, clockPin, MSBFIRST, D[1]);
          //shiftOut(dataPin, clockPin, MSBFIRST, D[0]);
          shiftOut(dataPin, clockPin, MSBFIRST, C[6]);
          shiftOut(dataPin, clockPin, MSBFIRST, A[a3]);
          digitalWrite(latchPin, HIGH);
          digitalWrite(latchPin, LOW);
          //shiftOut(dataPin, clockPin, MSBFIRST, D[1]);
          //shiftOut(dataPin, clockPin, MSBFIRST, D[0]);
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
          //shiftOut(dataPin, clockPin, MSBFIRST, D[1]);
          //shiftOut(dataPin, clockPin, MSBFIRST, D[0]);
          shiftOut(dataPin, clockPin, MSBFIRST, C[4]);
          shiftOut(dataPin, clockPin, MSBFIRST, B[a1]);
          digitalWrite(latchPin, HIGH);
          digitalWrite(latchPin, LOW);
          //shiftOut(dataPin, clockPin, MSBFIRST, D[1]);
          //shiftOut(dataPin, clockPin, MSBFIRST, D[0]);
          shiftOut(dataPin, clockPin, MSBFIRST, C[5]);
          shiftOut(dataPin, clockPin, MSBFIRST, A[a2]);
          digitalWrite(latchPin, HIGH);
          digitalWrite(latchPin, LOW);
          //shiftOut(dataPin, clockPin, MSBFIRST, D[1]);
          //shiftOut(dataPin, clockPin, MSBFIRST, D[0]);
          shiftOut(dataPin, clockPin, MSBFIRST, C[6]);
          shiftOut(dataPin, clockPin, MSBFIRST, A[a3]);
          digitalWrite(latchPin, HIGH);
          digitalWrite(latchPin, LOW);
          //shiftOut(dataPin, clockPin, MSBFIRST, D[1]);
          //shiftOut(dataPin, clockPin, MSBFIRST, D[0]);
          shiftOut(dataPin, clockPin, MSBFIRST, C[7]);
          shiftOut(dataPin, clockPin, MSBFIRST, A[a4]);
          digitalWrite(latchPin, HIGH);
        }
        break;
      }
  }
}
void setup() {

  // Timer/Counter 1 initialization
  // Clock source: System Clock
  // Clock value: 2000.000 kHz
  // Mode: Normal top=0xFFFF
  // OC1A output: Disconnected
  // OC1B output: Disconnected
  // Noise Canceler: Off
  // Input Capture on Falling Edge
  // Timer Period: 0.5 ms
  // Timer1 Overflow Interrupt: On
  // Input Capture Interrupt: Off
  // Compare A Match Interrupt: Off
  // Compare B Match Interrupt: Off
  TCCR1A = (0 << COM1A1) | (0 << COM1A0) | (0 << COM1B1) | (0 << COM1B0) | (0 << WGM11) | (0 << WGM10);
  TCCR1B = (0 << ICNC1) | (0 << ICES1) | (0 << WGM13) | (0 << WGM12) | (0 << CS12) | (0 << CS11) | (1 << CS10);
  TCNT1H = 0xE0;
  TCNT1L = 0xC0;
  ICR1H = 0x00;
  ICR1L = 0x00;
  OCR1AH = 0x00;
  OCR1AL = 0x00;
  OCR1BH = 0x00;
  OCR1BL = 0x00;

  // Timer/Counter 1 Interrupt(s) initialization
  TIMSK1 = (0 << ICIE1) | (0 << OCIE1B) | (0 << OCIE1A) | (1 << TOIE1);

  //adc======================
  pinMode(sen_u, INPUT);
  pinMode(sen_i, INPUT);
  digitalWrite(sen_u, LOW);
  digitalWrite(sen_i, LOW);
  analogReference(INTERNAL);
  //adc value (0,550]
  analog_u.setActivityThreshold(2.0);
  analog_u.setSnapMultiplier(0.025);
  //adc sen_i
  analog_i.setActivityThreshold(5.0);
  analog_i.setSnapMultiplier(0.05);
  //=========================

  // set the ChipSelectPins high initially:
  digitalWrite(dacChipSelectPin, HIGH);
  // initialise SPI-DAC
  SPI.begin();
  SPI.setBitOrder(MSBFIRST);         // Not strictly needed but just to be sure.
  SPI.setDataMode(SPI_MODE0);        // Not strictly needed but just to be sure.
  encoder0Pos = 16 * EEPROM.read(addr_encoder0Pos);
  // Set Num van
  pinMode(encoder0PinA, INPUT);
  pinMode(encoder0PinB, INPUT);
  pinMode(encoderPush, INPUT);
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);

  Serial.begin (9600);

  late = millis();
  t_update_display = millis();
}

void checkencoder() {
  n = digitalRead(encoder0PinA);
  if ((encoder0PinALast == LOW) && (n == HIGH)) {
    if (digitalRead(encoder0PinB) == LOW) {
      encoder0Pos++;
      if (m++ >= 5) {
        if (millis() - late < a_1)
        { encoder0Pos += step_encoder;
          m = 0;
        }
      }
      late = millis();
    } else {
      encoder0Pos--;
      if (m++ >= 5) {
        if (millis() - late < a_1)
        { encoder0Pos -= step_encoder;
          m = 0;
        }
      }
      late = millis();
    }
    if (encoder0Pos < min_encoder) encoder0Pos = min_encoder;
    else if (encoder0Pos > max_encoder) encoder0Pos = max_encoder;
    setDac(encoder0Pos, 0);
    have_change = true;
  }
  encoder0PinALast = n;

  //check Push===================================================
  cur = digitalRead(encoderPush);
  // if the button state changes to pressed, remember the start time
  if (cur == LOW && prev == HIGH && (millis() - button_first) > 200) {
    button_first = millis();
  }

  millis_held = (millis() - button_first);
  secs_held = millis_held / 1000;

  if (millis_held > 80) {

    if (cur == LOW && secs_held > prev_secs_held) {
      stt_push = 2;
    }
    // check if the button was released since we last checked
    if (cur == HIGH && prev == LOW) {
      if (secs_held <= 0) {
        stt_push = 1;
        //Serial.println("click push");
        //save cur encoder0Pos to eeprom
        EEPROM.write(addr_encoder0Pos, encoder0Pos / 16);
      }
    }
  }
  prev = cur;
  prev_secs_held = secs_held;
  //check Push===================================================

}

void loop() {
  //set default output voltage
  setDac(encoder0Pos, 0);

  // update the ResponsiveAnalogRead object every loop
  analog_u.update();
  analog_i.update();

  //update value every encoder haveChnge or 1/4 seconds
  if ( have_change || (millis() - t_update_display > 250))
  {
    /*
    //=====================================================
    //debug calib sen_u===================================
    //
    int temp1, temp2, temp3;
    temp1 = analogRead(sen_u);

    //if hasChange, update Threshold
    if (analog.hasChanged()) {
      //check range adc to change threshold
      if (analog.getValue() < 550) threshold_1();
      else {
        threshold_2();
        //update the ResponsiveAnalogRead object
        analog.update();
      }
    }

    temp2 = analog.getValue();
    temp3 = map_u(temp2);

    Serial.print(temp1);
    Serial.print("\t");
    Serial.print(temp2);
    Serial.print("\t");
    Serial.println(temp3);

    u = temp3 / MAX_READING_10_bit * Vref;
    //===================================================*/


    //if hasChange on analog snap, update Threshold
    if (analog_u.hasChanged()) {
      //check range adc to change threshold
      if (analog_u.getValue() < 550) {
        //adc value (0;550)
        analog_u.setActivityThreshold(2.0);
        analog_u.setSnapMultiplier(0.025);
      }
      else {
        //adc value (550;1023)
        analog_u.setActivityThreshold(3.0);
        analog_u.setSnapMultiplier(0.01);
      }
      //update the ResponsiveAnalogRead object
      analog_u.update();
    }
    u = map_u(analog_u.getValue()) / MAX_READING_10_bit * Vref;

    int read_i = analog_i.getValue();
    i = read_i / i_MAX_READING_10_bit * Vref;
    Serial.println(read_i);


    //clear have_change encoder
    have_change = 0;
    t_update_display = millis();
  }

  hienthi(i, 1);
  hienthi(u, 2);

}

void setDac(int value, int channel) {
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

int map_u(int value) {
  if (value < 245) { //0-245
    if (value < 79) { //0-79
      if (value < 22) return map(value, 0, 22, 0, 28);
      else if (value < 41) return map(value, 22, 41, 28, 47);
      else return map(value, 41, 79, 47, 85);
    }
    else { //79-245
      if (value < 109) return map(value, 79, 109, 85, 112);
      else if (value < 223) return map(value, 109, 223, 112, 224);
      else return map(value, 223, 245, 224, 245);
    }
  }
  else { //245-1023
    if (value < 474) { //245-474
      if (value < 377) return map(value, 245, 377, 245, 374);
      else return map(value, 377, 474, 374, 468);
    }
    else if (value < 757) { //474-757
      if (value < 568) return map(value, 474, 568, 468, 560);
      else if (value < 665) return map(value, 568, 665, 560, 654);
      else return map(value, 665, 757, 654, 745);
    }
    else { //757-1023 [!] just get 757->745
      return map(value, 757, 1023, 745, 1023);
    }
  }
}

// Timer1 overflow interrupt service routine
ISR(TIMER1_OVF_vect)
{
  // Reinitialize Timer1 value
  TCNT1H = 0xE0C0 >> 8;
  TCNT1L = 0xE0C0 & 0xff;

  // Place your code here
  checkencoder();
}
