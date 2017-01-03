#include <SPI.h>
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

//instantiate an object of this library class; call it "adc"
eRCaGuy_analogReadXXbit adc;

const uint8_t sen_i = A0;
const uint8_t sen_u = A1;
const int dacChipSelectPin = 10;      // set pin 9 as the chip select for the DAC:
const float MAX_READING_10_bit = 10.23;
const float MAX_READING_11_bit = 2046.0;
const float MAX_READING_12_bit = 4092.0;
float analog_reading; //the ADC reading
uint8_t adc_12bits = 12; //bits of precision for the ADC (Analog to Digital Converter)
uint8_t adc_11bits = 11;
unsigned int num_samples = 1;
const float Vref = 1.096;
//const float Vref = 5;

//adc eliminating noise
// include the ResponsiveAnalogRead library
#include <ResponsiveAnalogRead.h>

// define the pin you want to use
const int ANALOG_PIN = A1;

// make a ResponsiveAnalogRead object, pass in the pin, and either true or false depending on if you want sleep enabled
// enabling sleep will cause values to take less time to stop changing and potentially stop changing more abruptly,
//   where as disabling sleep will cause values to ease into their correct position smoothly and more accurately
ResponsiveAnalogRead analog(ANALOG_PIN, true, 0.05);

// the next optional argument is snapMultiplier, which is set to 0.01 by default
// you can pass it a value from 0 to 1 that controls the amount of easing
// increase this to lessen the amount of easing (such as 0.1) and make the responsive values more responsive
// but doing so may cause more noise to seep through if sleep is not enabled

//=====================

int val;

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
  analogReference(INTERNAL);
  //=========================

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
}

void loop() {
  setDac(encoder0Pos, 0);
  // update the ResponsiveAnalogRead object every loop
  analog.update();

  /*
  u_temp += do_a_bunch_of_samples(num_samples, sen_u, 10);
  u_count++;
  i_temp += do_a_bunch_of_samples(num_samples, sen_i, 10);
  i_count++;
  */

  if ( have_change || (millis() - t_update_display > 250))
  {
    /*
    //u = 99 * do_a_bunch_of_samples(num_samples, sen_u, 10); // mach chia ap lay 1/10
    //i = 10 * do_a_bunch_of_samples(num_samples, sen_i, 10);
    //Serial.println(do_a_bunch_of_samples(num_samples, sen_u, 12),6);


    u = 99*(u_temp/u_count);// must 100 * U but error is 1% -> 99 * U
    u_count = 0;
    u_temp = 0;

    i = 10*(i_temp/i_count);
    i_count = 0;
    i_temp = 0;
    */

    int temp1,temp2;
    temp1 = analogRead(sen_u);
    temp2 = analog.getValue();

    u = temp2 / MAX_READING_10_bit * Vref;

    Serial.print(temp1);Serial.print("\t"); Serial.println(temp2);



    have_change = 0;
    t_update_display = millis();
  }

  hienthi(i, 1);
  hienthi(u, 2);
  
  
  
  /*
  Serial.print(analogRead(A1));
  

  //Serial.print(analog.getRawValue());

  Serial.print("\t");
  Serial.print(analog.getValue());

  // if the repsonsive value has change, print out 'changed'
  if(analog.hasChanged()) {
    Serial.print("\tchanged");
  }

  Serial.println("");

  delay(250);
  */
  
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

float do_a_bunch_of_samples(unsigned long num_samples, uint8_t pin, int bits)
{
  switch (bits) {
    case 12: {
        analog_reading = adc.analogReadXXbit(pin, adc_12bits, num_samples); //get the avg. of [num_samples] 12-bit readings
        return analog_reading / MAX_READING_12_bit * Vref; //voltage
        break;
      }
    case 11: {
        analog_reading = adc.analogReadXXbit(pin, adc_11bits, num_samples); //get the avg. of [num_samples] 12-bit readings
        return analog_reading / MAX_READING_11_bit * Vref; //voltage
        break;
      }
    case 10: {
        return analogRead(pin) / MAX_READING_10_bit * Vref;
        break;
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
