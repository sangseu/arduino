#include "./eRCaGuy_analogReadXXbit.h"

#include <SPI.h>
#include <EEPROM.h>
// adc eliminating noise
// include the ResponsiveAnalogRead library
#include "./ResponsiveAnalogRead.h"

#define debug 0
#define a_1 25 // delta millis check
#define max_dac_u_set 4095
#define min_dac_u_set 0
#define encoder_s_step 100
#define encoder_l_step 1000
#define latchPin 8
#define clockPin 7
#define dataPin 6
#define encoder0PinA 5
#define encoder0PinB 4
#define encoderPush 9

const unsigned long max_u_set = 90000;
const unsigned long min_u_set = 0;
const uint8_t sen_i = A2;
const uint8_t sen_u = A1;
const int dacChipSelectPin = 10;      // set pin 10 as the chip select for the DAC:
const float MAX_READING_10_bit = 1023.0;
const float MAX_READING_11_bit = 2046.0;
const float MAX_READING_12_bit = 4092.0;
float analog_reading; //the ADC reading
unsigned int num_samples = 1;
const float Vref = 1.096;
//const float Vref = 5;

int u_xx = 0;

// make a ResponsiveAnalogRead object, pass in the pin, and either true or false depending on if you want sleep enabled
// enabling sleep will cause values to take less time to stop changing and potentially stop changing more abruptly,
//   where as disabling sleep will cause values to ease into their correct position smoothly and more accurately
ResponsiveAnalogRead analog_u(u_xx, true);
ResponsiveAnalogRead analog_i(sen_i, true);

// the next optional argument is snapMultiplier, which is set to 0.01 by default
// you can pass it a value from 0 to 1 that controls the amount of easing
// increase this to lessen the amount of easing (such as 0.1) and make the responsive values more responsive
// but doing so may cause more noise to seep through if sleep is not enabled
//=====================

eRCaGuy_analogReadXXbit adc;

//push=================
int cur, prev, stt_push;
long millis_held, secs_held, prev_secs_held;
unsigned long button_first;
//push=================

int val;

int addr_encoder0Pos = 0;
unsigned long encoder0Pos = 30000;
int encoder0PinALast = LOW;
int n = LOW; int A[10] = {250, 130, 185, 171, 195, 107, 123, 162, 251, 235};// number without dot
int B[10] = {254, 134, 189, 175, 199, 111, 127, 166, 255, 239};// number with dot
int C[8] = {223, 191, 127, 239, 253, 251, 247, 254};// select LED
int D[2] = {0, 255};
int m = 0;
int l = 0;
int u_raw, i_raw;
unsigned long u, i, u_led, i_led, u_set = 40000, u_set_point, dac_u_set = 2048;
int start_count = 0;

bool have_change = 0;
bool stt_set_u = false;

unsigned long late, t_update_display, t_upU, t_downU;

unsigned long test = 0;

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
  analogReference(INTERNAL);//INTERNAL Vref = 1.096
  //adc value (0,550]
  analog_u.setActivityThreshold(1.0);
  analog_u.setSnapMultiplier(0.001);
  analog_u.setAnalogResolution(4092);
  //adc sen_i
  analog_i.setActivityThreshold(1.0);
  analog_i.setSnapMultiplier(0.01);
  //=========================

  // set the ChipSelectPins high initially:
  digitalWrite(dacChipSelectPin, HIGH);
  // initialise SPI-DAC
  SPI.begin();
  SPI.setBitOrder(MSBFIRST);         // Not strictly needed but just to be sure.
  SPI.setDataMode(SPI_MODE0);        // Not strictly needed but just to be sure.
  encoder0Pos = 1000 * EEPROM.read(addr_encoder0Pos);
  // Set Num van
  pinMode(encoder0PinA, INPUT);
  pinMode(encoder0PinB, INPUT);
  pinMode(encoderPush, INPUT);
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);

  Serial.begin (9600);
  Serial.print("\nBench power\n");
  Serial.print("===========\n");
  Serial.print("U:   2-80 V\n");
  Serial.print("P:   100  W\n");

  late = millis();
  t_update_display = t_upU = t_downU = millis();

  while (start_count ++ < 1000) {
    //set default output voltage
    setDac(encoder0Pos);
    wellcome();
  }

}

void checkencoder() {
  n = digitalRead(encoder0PinA);
  if ((encoder0PinALast == LOW) && (n == HIGH)) {
    if (digitalRead(encoder0PinB) == LOW) {
      encoder0Pos -= encoder_s_step;
      if (m++ >= 5) {
        if (millis() - late < a_1)
        { encoder0Pos -= encoder_l_step;
          m = 0;
        }
      }
      late = millis();
    } else {
      encoder0Pos += encoder_s_step;
      if (m++ >= 5) {
        if (millis() - late < a_1)
        { encoder0Pos += encoder_l_step;
          m = 0;
        }
      }
      late = millis();
    }
    if (encoder0Pos < min_u_set) encoder0Pos = min_u_set;
    else if (encoder0Pos > max_u_set) encoder0Pos = max_u_set;
    //setDac(encoder0Pos);
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
        EEPROM.write(addr_encoder0Pos, encoder0Pos / 1000);
      }
    }
  }
  prev = cur;
  prev_secs_held = secs_held;
  //check Push===================================================

}

// loop==========================================================
void loop() {

  // update the ResponsiveAnalogRead object every loop
  analog_u.update();
  analog_i.update();
  /*
    u_xx = int(adc.analogReadXXbit(sen_u, 12, 2));
    analog_u.update(u_xx);

    u_raw = analog_u.getValue();
    i_raw = analog_i.getValue();
  */

  //u = u_raw/MAX_READING_10_bit*Vref*100;
  // 24.56 -> 24560
  // 1.234 -> 1234
  if (u_raw) u = (unsigned long)(u_raw * 25.959 + 819.265);
  else u = 0;
  i = i_raw / MAX_READING_10_bit * Vref * 10;

  // update value every encoder haveChnge or 1/4 seconds
  // 250 for run, 100 for debug
  if ( have_change || (millis() - t_update_display > 100))
  {
    u_xx = int(adc.analogReadXXbit(sen_u, 12, 1));
    analog_u.update(u_xx);

    u_raw = analog_u.getValue();
    i_raw = analog_i.getValue();

    //u = u_raw/MAX_READING_10_bit*Vref*100;
    // 24.56 -> 24560
    // 1.234 -> 1234
    if (u_raw) u = (unsigned long)(u_raw * 25.959 + 819.265);
    else u = 0;
    i = i_raw / MAX_READING_10_bit * Vref * 10;



    //analogRead(sen_u);
    //u_xx = int(adc.analogReadXXbit(sen_u, 12, 3));

    //u = (unsigned long)(u_xx * 25.959 + 819.265);

    Serial.print(u_raw); Serial.print("\t");
    Serial.println(analogRead(sen_u));
    //Serial.print(encoder0Pos); Serial.print("\t");
    //Serial.print(u_xx); Serial.print("\t");
    //Serial.print(u_raw); Serial.print("\t"); Serial.println(u);
    //Serial.print(i_raw); Serial.print("\t"); Serial.print(i);
    //Serial.print("\t"); Serial.println(encoder0Pos);

    /*
        Serial.print(u_xx); Serial.print("\t");
        Serial.print(u_raw); Serial.print("\t");
        Serial.println("xx");
    */
    i_led = i;
    u_led = u;
    //clear have_change encoder
    have_change = 0;
    t_update_display = millis();
  }// end if

  //Serial.print(test);Serial.print("\t"); Serial.println(u);
  // test set u
  //set_u(encoder0Pos);

  setDac(2000);

  hienthi(i_led, 1);
  hienthi(u_led, 2);

}
// end loop======================================================

void set_u(unsigned long u_set_point) {
  if (u_set_point == 0) dac_u_set = 0;
  long delta_u_set = u_set_point - u;
  delta_u_set = abs(delta_u_set);

  if (delta_u_set > 200) { // fast change
    if (u < u_set_point) { // increase U ~ decrease dac_u_set
      if (--dac_u_set < min_dac_u_set) dac_u_set = min_dac_u_set;
      setDac(dac_u_set);
      Serial.println("[++]");
    }

    if (u > u_set_point) { // decrease U ~ increase dac_u_set
      if (++dac_u_set > max_dac_u_set) dac_u_set = max_dac_u_set;
      setDac(dac_u_set);
      Serial.println("[--]");
    }
    Serial.println(delta_u_set);
  }
  else if (80 < delta_u_set && delta_u_set < 200) { // slow change to u_set_point
    if (millis() - t_upU > 500) {
      if (u < u_set_point) { // increase U ~ decrease dac_u_set
        if (--dac_u_set < min_dac_u_set) dac_u_set = min_dac_u_set;
        setDac(dac_u_set);
        t_upU = millis();
        Serial.println("[+]");
      }
    }
    if (millis() - t_downU > 500) {
      if (u > u_set_point) { // decrease U ~ increase dac_u_set
        if (++dac_u_set > max_dac_u_set) dac_u_set = max_dac_u_set;
        setDac(dac_u_set);
        t_downU = millis();
        Serial.println("[-]");
      }
    }
  }
  else {
    stt_set_u = true;
  }
}

void setDac(int value) {
  int channel = 0; // default chanel
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

void hienthi(unsigned long giatri, int m)
{ // hien thi  7-seg
  unsigned long a1, a2, a3, a4, x;
  switch (m) {
    case 1:// show I
      {
        x = giatri * 1000;
        a1 = x / 1000;
        a2 = x % 1000 / 100;
        a3 = x % 1000 % 100 / 10;
        a4 = x % 1000 % 100 % 10;

        digitalWrite(latchPin, LOW);
        shiftOut(dataPin, clockPin, MSBFIRST, C[0]);
        shiftOut(dataPin, clockPin, MSBFIRST, B[a1]);
        digitalWrite(latchPin, HIGH);
        digitalWrite(latchPin, LOW);
        shiftOut(dataPin, clockPin, MSBFIRST, C[1]);
        shiftOut(dataPin, clockPin, MSBFIRST, A[a2]);
        digitalWrite(latchPin, HIGH);
        digitalWrite(latchPin, LOW);
        shiftOut(dataPin, clockPin, MSBFIRST, C[2]);
        shiftOut(dataPin, clockPin, MSBFIRST, A[a3]);
        digitalWrite(latchPin, HIGH);
        digitalWrite(latchPin, LOW);
        shiftOut(dataPin, clockPin, MSBFIRST, C[3]);
        shiftOut(dataPin, clockPin, MSBFIRST, A[a4]);
        digitalWrite(latchPin, HIGH);

        break;
      }
    // hien thi  7-seg----2
    case 2:// show U
      {
        // 24.56 -> 24560
        // 10.234 -> 10234
        // 1.234 -> 1234

        if (giatri >= 10000) {// 10.00-99.99
          a1 = giatri / 10000;
          a2 = giatri % 10000 / 1000;
          a3 = giatri % 10000 % 1000 / 100;
          a4 = giatri % 10000 % 1000 % 100 / 10;

          digitalWrite(latchPin, LOW);
          shiftOut(dataPin, clockPin, MSBFIRST, C[4]);
          shiftOut(dataPin, clockPin, MSBFIRST, A[a1]);
          digitalWrite(latchPin, HIGH);
          digitalWrite(latchPin, LOW);
          shiftOut(dataPin, clockPin, MSBFIRST, C[5]);
          shiftOut(dataPin, clockPin, MSBFIRST, B[a2]);
          digitalWrite(latchPin, HIGH);
          digitalWrite(latchPin, LOW);
          shiftOut(dataPin, clockPin, MSBFIRST, C[6]);
          shiftOut(dataPin, clockPin, MSBFIRST, A[a3]);
          digitalWrite(latchPin, HIGH);
          digitalWrite(latchPin, LOW);
          shiftOut(dataPin, clockPin, MSBFIRST, C[7]);
          shiftOut(dataPin, clockPin, MSBFIRST, A[a4]);
          digitalWrite(latchPin, HIGH);
        }
        else {
          // 24.56 -> 24560
          // 10.234 -> 10234
          // 1.234 -> 1234
          x = giatri;//0.000-9.999
          a1 = x / 1000;
          a2 = x % 1000 / 100;
          a3 = x % 1000 % 100 / 10;
          a4 = x % 1000 % 100 % 10;
          digitalWrite(latchPin, LOW);
          shiftOut(dataPin, clockPin, MSBFIRST, C[4]);
          shiftOut(dataPin, clockPin, MSBFIRST, B[a1]);
          digitalWrite(latchPin, HIGH);
          digitalWrite(latchPin, LOW);
          shiftOut(dataPin, clockPin, MSBFIRST, C[5]);
          shiftOut(dataPin, clockPin, MSBFIRST, A[a2]);
          digitalWrite(latchPin, HIGH);
          digitalWrite(latchPin, LOW);
          shiftOut(dataPin, clockPin, MSBFIRST, C[6]);
          shiftOut(dataPin, clockPin, MSBFIRST, A[a3]);
          digitalWrite(latchPin, HIGH);
          digitalWrite(latchPin, LOW);
          shiftOut(dataPin, clockPin, MSBFIRST, C[7]);
          shiftOut(dataPin, clockPin, MSBFIRST, A[a4]);
          digitalWrite(latchPin, HIGH);
        }
        break;
      }
  }
}

void wellcome() {
  int a1 = 2, a2 = 0, a3 = 1, a4 = 7;
  digitalWrite(latchPin, LOW);
  shiftOut(dataPin, clockPin, MSBFIRST, C[4]);
  shiftOut(dataPin, clockPin, MSBFIRST, A[a1]);
  digitalWrite(latchPin, HIGH);
  digitalWrite(latchPin, LOW);
  shiftOut(dataPin, clockPin, MSBFIRST, C[5]);
  shiftOut(dataPin, clockPin, MSBFIRST, A[a2]);
  digitalWrite(latchPin, HIGH);
  digitalWrite(latchPin, LOW);
  shiftOut(dataPin, clockPin, MSBFIRST, C[6]);
  shiftOut(dataPin, clockPin, MSBFIRST, A[a3]);
  digitalWrite(latchPin, HIGH);
  digitalWrite(latchPin, LOW);
  shiftOut(dataPin, clockPin, MSBFIRST, C[7]);
  shiftOut(dataPin, clockPin, MSBFIRST, A[a4]);
  digitalWrite(latchPin, HIGH);
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
