#define led_auto 7
#define led_start 8
#define buzz 6
#define pin_i_set 10
#define pin_i_load A1 // i_load is senIN
#define pin_i_s A2
#define pin_u_s A0
//#define pin_u_n A3
#define pin_stop 2
#define pin_up 4
#define pin_down 5
#define pin_start 3
#define pin_onNS 9
#define debug 0

#define Vref 4.97 //EXTERNAL TL431

//max_pwm_i_set value 2^10 - 1 = 1023
#define max_i_set_point 5000
#define min_i_set_point 0
#define max_pwm_i_set 1023
#define min_pwm_i_set 0

//FastPWMdac
#include <TimerOne.h>
#include <FastPWMdac.h>
FastPWMdac fastPWMdac;

//ADC threshold
// include the ResponsiveAnalogRead library
#include <ResponsiveAnalogRead.h>

// make a ResponsiveAnalogRead object, pass in the pin, and either true or false depending on if you want sleep enabled
// enabling sleep will cause values to take less time to stop changing and potentially stop changing more abruptly,
//   where as disabling sleep will cause values to ease into their correct position smoothly and more accurately
ResponsiveAnalogRead snap_i_load(pin_i_load, true);
ResponsiveAnalogRead snap_i_s(pin_i_s, true);
ResponsiveAnalogRead snap_u_s(pin_u_s, true);

const uint8_t latchPin = 13;
const uint8_t clockPin = 12;
const uint8_t dataPin = 11;

//led data==========================================================================
const int A[10] = {250, 130, 185, 171, 195, 107, 123, 162, 251, 235};// number without dot
const int B[10] = {254, 134, 189, 175, 199, 111, 127, 166, 255, 239};// number with dot
const int C[8] = {223, 191, 127, 239, 253, 251, 247, 254};// select LED
const float E[10] = {0, 1.111, 2.222, 3.333, 4.444, 5.555, 6.666, 7.777, 8.888, 9.999};
//led data==========================================================================

int cur_value_dac = 0;

unsigned long t_update_display, t_update_i_set, t_buzz, t_downI, t_upI;
int value_i_load, value_u_s, value_i_s;
int led_value_u_s, led_value_i_s, led_value_i_load;
float delta_i_set = 0;
int raw_i_load, raw_u_s, raw_i_s;
int i_set_point = 0;
int pwm_i_set = 0;

unsigned int button = 0;

unsigned long button_first_stop, button_first_start, button_first_up, button_first_down; // how long since the button was first pressed
long millis_held_stop, millis_held_start, millis_held_up, millis_held_down;    // How long the button was held (milliseconds)
long secs_held_stop, secs_held_start, secs_held_up, secs_held_down;      // How long the button was held (seconds)
long prev_secs_held_stop, prev_secs_held_start, prev_secs_held_up, prev_secs_held_down; // How long the button was held in the previous check

int cur_pin_stop, cur_pin_start, cur_pin_up, cur_pin_down;
int prev_pin_stop, prev_pin_start, prev_pin_up, prev_pin_down;
int stt_stop, stt_start, stt_up, stt_down;
//stt = 0 : no even
//stt = 1 : click
//stt = 2 : push_hold
bool stt_run = false;
bool err1, err2;
bool stt_set_i = true;
// err1: cann't control I_load, (I_load-I_set_point) > 0.5A
void setup() {
  Serial.begin(115200);
  Serial.println("dummy load V2");

  initIO(); // init led, buzzer
  //tit(); // play buzzer
  initled(); // init led 7-seg
  checkled(); // show 0.000 -> 9.999 to test

  initADC();

  initPWMDAC();
  off_i(); // set adc output = 0

  t_update_display = t_upI = t_downI = millis();

}
//loop===================================================================================
void loop() {
  // update the ResponsiveAnalogRead object every loop
  snap_i_load.update();
  snap_i_s.update();
  snap_u_s.update();

  checkbutton();

  if (stt_stop) { // stop button
    switch (stt_stop) {
      case 1: {
          stt_run = false;
          // turn off relay
          off_i();
          stt_stop = 0;
          //clear i_set_point
          i_set_point = 0;
          tit();
          break;
        }
      case 2: {
          stt_run = false;
          off_i();
          stt_stop = 0;
          //clear i_set_point
          i_set_point = 0;
          break;
        }
    }
  }
  else if (stt_start) { // start button
    stt_run = true;
    // turn on relay
    on_NS();

    stt_start = 0;
    tit();
  }
  else if (stt_up) { // up button
    switch (stt_up) {
      case 1: {
          i_set_point = i_set_point + 100;
          if (i_set_point > max_i_set_point) i_set_point = max_i_set_point;
          break;
        }
      case 2: {
          i_set_point = i_set_point + 500;
          if (i_set_point > max_i_set_point) i_set_point = max_i_set_point;
          break;
        }
    }
    stt_up = 0;
    stt_set_i = false;
  }
  else if (stt_down) { // down button
    switch (stt_down) {
      case 1: {
          i_set_point = i_set_point - 100;
          if (i_set_point < min_i_set_point) i_set_point = min_i_set_point;
          break;
        }
      case 2: {
          i_set_point = i_set_point - 500;
          if (i_set_point < min_i_set_point) i_set_point = min_i_set_point;
          break;
        }
    }
    stt_down = 0;
    stt_set_i = false;
  }

  sensing(); // get value
  if (stt_run) { // control I
    set_i(i_set_point);
  }

  if (stt_set_i == false) {
    // update live value
    led_value_i_load = value_i_load;
    led_value_u_s = value_u_s;
    led_value_i_s = value_i_s;
  }
  else {
    // cyclic update
    // 500 for run, 100 for debug
    if (millis() - t_update_display > 500) {
      t_update_display = millis();
      // update led 7-seg
      led_value_i_load = value_i_load;
      led_value_u_s = value_u_s;
      led_value_i_s = value_i_s;
/*
      // print led value
      Serial.print(raw_u_s);
      Serial.print("\t");
      Serial.print(value_u_s, 4);
      Serial.print("\t");
      Serial.println(led_value_u_s, 4);
*/
      // test raw ADC
      /*
      Serial.print(analogRead(pin_u_s));
      Serial.print(", ");
      Serial.print(analogRead(pin_i_load));
      Serial.print(", ");
      Serial.println(analogRead(pin_i_s));
      */


      // test raw snap ADC
      /*
            //Serial.println();
            Serial.println(raw_u_s);
            Serial.print("\t");
            Serial.print(raw_i_load);
            Serial.print("\t");
            Serial.println(raw_i_s);
      */
      // test snap ADC
      
      //Serial.println();
      Serial.print(value_u_s);
      Serial.print("\t");
      Serial.print(value_i_load);
      Serial.print("\t");
      Serial.println(value_i_s);
      
    }
  }

  show(); // update led 7-seg every loop
}
//end loop================================================================================
void show() {
  //show i_load i_s=========================================
  int l0, l1, l2, l3, r0, r1, r2, r3, x;
  x = led_value_i_load;
  //x = value_i_load;
  l0 = x / 1000;
  l1 = x % 1000 / 100;
  l2 = x % 1000 % 100 / 10;
  l3 = x % 1000 % 100 % 10;

  x = led_value_i_s;
  //x = value_i_s;
  r0 = x / 1000;
  r1 = x % 1000 / 100;
  r2 = x % 1000 % 100 / 10;
  r3 = x % 1000 % 100 % 10;

  digitalWrite(latchPin, LOW);
  shiftOut(dataPin, clockPin, MSBFIRST, C[0]);
  shiftOut(dataPin, clockPin, MSBFIRST, B[r0]);
  shiftOut(dataPin, clockPin, MSBFIRST, C[0]);
  shiftOut(dataPin, clockPin, MSBFIRST, B[l0]);
  digitalWrite(latchPin, HIGH);
  digitalWrite(latchPin, LOW);
  shiftOut(dataPin, clockPin, MSBFIRST, C[1]);
  shiftOut(dataPin, clockPin, MSBFIRST, A[r1]);
  shiftOut(dataPin, clockPin, MSBFIRST, C[1]);
  shiftOut(dataPin, clockPin, MSBFIRST, A[l1]);
  digitalWrite(latchPin, HIGH);
  digitalWrite(latchPin, LOW);
  shiftOut(dataPin, clockPin, MSBFIRST, C[2]);
  shiftOut(dataPin, clockPin, MSBFIRST, A[r2]);
  shiftOut(dataPin, clockPin, MSBFIRST, C[2]);
  shiftOut(dataPin, clockPin, MSBFIRST, A[l2]);
  digitalWrite(latchPin, HIGH);
  digitalWrite(latchPin, LOW);
  shiftOut(dataPin, clockPin, MSBFIRST, C[3]);
  shiftOut(dataPin, clockPin, MSBFIRST, A[r3]);
  shiftOut(dataPin, clockPin, MSBFIRST, C[3]);
  shiftOut(dataPin, clockPin, MSBFIRST, A[l3]);
  digitalWrite(latchPin, HIGH);

  //show i_set u_s==========================================
  x = i_set_point;
  l0 = x / 1000;
  l1 = x % 1000 / 100;
  l2 = x % 1000 % 100 / 10;
  l3 = x % 1000 % 100 % 10;

  x = led_value_u_s;
  r0 = x / 1000;
  r1 = x % 1000 / 100;
  r2 = x % 1000 % 100 / 10;
  r3 = x % 1000 % 100 % 10;

  digitalWrite(latchPin, LOW);
  shiftOut(dataPin, clockPin, MSBFIRST, C[4]);
  shiftOut(dataPin, clockPin, MSBFIRST, A[r0]);
  shiftOut(dataPin, clockPin, MSBFIRST, C[4]);
  shiftOut(dataPin, clockPin, MSBFIRST, B[l0]);
  digitalWrite(latchPin, HIGH);
  digitalWrite(latchPin, LOW);
  shiftOut(dataPin, clockPin, MSBFIRST, C[5]);
  shiftOut(dataPin, clockPin, MSBFIRST, B[r1]);
  shiftOut(dataPin, clockPin, MSBFIRST, C[5]);
  shiftOut(dataPin, clockPin, MSBFIRST, A[l1]);
  digitalWrite(latchPin, HIGH);
  digitalWrite(latchPin, LOW);
  shiftOut(dataPin, clockPin, MSBFIRST, C[6]);
  shiftOut(dataPin, clockPin, MSBFIRST, A[r2]);
  shiftOut(dataPin, clockPin, MSBFIRST, C[6]);
  shiftOut(dataPin, clockPin, MSBFIRST, A[l2]);
  digitalWrite(latchPin, HIGH);
  digitalWrite(latchPin, LOW);
  shiftOut(dataPin, clockPin, MSBFIRST, C[7]);
  shiftOut(dataPin, clockPin, MSBFIRST, A[r3]);
  shiftOut(dataPin, clockPin, MSBFIRST, C[7]);
  shiftOut(dataPin, clockPin, MSBFIRST, A[l3]);
  digitalWrite(latchPin, HIGH);
}

void show(int tic) {
  int l0, l1, l2, l3, r0, r1, r2, r3;
  l0 = l1 = l2 = l3 = r0 = r1 = r2 = r3 = tic;
  // right
  digitalWrite(latchPin, LOW);
  shiftOut(dataPin, clockPin, MSBFIRST, C[0]);
  shiftOut(dataPin, clockPin, MSBFIRST, B[r0]);
  shiftOut(dataPin, clockPin, MSBFIRST, C[0]);
  shiftOut(dataPin, clockPin, MSBFIRST, B[l0]);
  digitalWrite(latchPin, HIGH);
  digitalWrite(latchPin, LOW);
  shiftOut(dataPin, clockPin, MSBFIRST, C[1]);
  shiftOut(dataPin, clockPin, MSBFIRST, A[r1]);
  shiftOut(dataPin, clockPin, MSBFIRST, C[1]);
  shiftOut(dataPin, clockPin, MSBFIRST, A[l1]);
  digitalWrite(latchPin, HIGH);
  digitalWrite(latchPin, LOW);
  shiftOut(dataPin, clockPin, MSBFIRST, C[2]);
  shiftOut(dataPin, clockPin, MSBFIRST, A[r2]);
  shiftOut(dataPin, clockPin, MSBFIRST, C[2]);
  shiftOut(dataPin, clockPin, MSBFIRST, A[l2]);
  digitalWrite(latchPin, HIGH);
  digitalWrite(latchPin, LOW);
  shiftOut(dataPin, clockPin, MSBFIRST, C[3]);
  shiftOut(dataPin, clockPin, MSBFIRST, A[r3]);
  shiftOut(dataPin, clockPin, MSBFIRST, C[3]);
  shiftOut(dataPin, clockPin, MSBFIRST, A[l3]);
  digitalWrite(latchPin, HIGH);

  // left
  digitalWrite(latchPin, LOW);
  shiftOut(dataPin, clockPin, MSBFIRST, C[4]);
  shiftOut(dataPin, clockPin, MSBFIRST, B[r0]);
  shiftOut(dataPin, clockPin, MSBFIRST, C[4]);
  shiftOut(dataPin, clockPin, MSBFIRST, B[l0]);
  digitalWrite(latchPin, HIGH);
  digitalWrite(latchPin, LOW);
  shiftOut(dataPin, clockPin, MSBFIRST, C[5]);
  shiftOut(dataPin, clockPin, MSBFIRST, A[r1]);
  shiftOut(dataPin, clockPin, MSBFIRST, C[5]);
  shiftOut(dataPin, clockPin, MSBFIRST, A[l1]);
  digitalWrite(latchPin, HIGH);
  digitalWrite(latchPin, LOW);
  shiftOut(dataPin, clockPin, MSBFIRST, C[6]);
  shiftOut(dataPin, clockPin, MSBFIRST, A[r2]);
  shiftOut(dataPin, clockPin, MSBFIRST, C[6]);
  shiftOut(dataPin, clockPin, MSBFIRST, A[l2]);
  digitalWrite(latchPin, HIGH);
  digitalWrite(latchPin, LOW);
  shiftOut(dataPin, clockPin, MSBFIRST, C[7]);
  shiftOut(dataPin, clockPin, MSBFIRST, A[r3]);
  shiftOut(dataPin, clockPin, MSBFIRST, C[7]);
  shiftOut(dataPin, clockPin, MSBFIRST, A[l3]);
  digitalWrite(latchPin, HIGH);
}

void led_blink() {

}

int checkbutton() {
  //stop button==========================================================
  cur_pin_stop = digitalRead(pin_stop);
  // if the button state changes to pressed, remember the start time
  if (cur_pin_stop == LOW && prev_pin_stop == HIGH && (millis() - button_first_stop) > 200) {
    button_first_stop = millis();
  }

  millis_held_stop = (millis() - button_first_stop);
  secs_held_stop = millis_held_stop / 1000;

  if (millis_held_stop > 80) {

    if (cur_pin_stop == LOW && secs_held_stop > prev_secs_held_stop) {
      stt_stop = 2;
    }
    // check if the button was released since we last checked
    if (cur_pin_stop == HIGH && prev_pin_stop == LOW) {
      if (secs_held_stop <= 0) {
        stt_stop = 1;
        //Serial.println("click stop");
      }
    }
  }
  prev_pin_stop = cur_pin_stop;
  prev_secs_held_stop = secs_held_stop;
  //stop button==========================================================

  //start button=========================================================
  cur_pin_start = digitalRead(pin_start);
  // if the button state changes to pressed, remember the start time
  if (cur_pin_start == LOW && prev_pin_start == HIGH && (millis() - button_first_start) > 200) {
    button_first_start = millis();
  }

  millis_held_start = (millis() - button_first_start);
  secs_held_start = millis_held_start / 1000;

  if (millis_held_start > 80) {

    if (cur_pin_start == LOW && secs_held_start > prev_secs_held_start) {
      stt_start = 2;
      //Serial.println("hold start");
    }
    // check if the button was released since we last checked
    if (cur_pin_start == HIGH && prev_pin_start == LOW) {
      if (secs_held_start <= 0) {
        stt_start = 1;
        //Serial.println("click start");
      }
    }
  }
  prev_pin_start = cur_pin_start;
  prev_secs_held_start = secs_held_start;
  //start button=========================================================

  //up button============================================================
  cur_pin_up = digitalRead(pin_up);
  // if the button state changes to pressed, remember the start time
  if (cur_pin_up == LOW && prev_pin_up == HIGH && (millis() - button_first_up) > 200) {
    button_first_up = millis();
  }

  millis_held_up = (millis() - button_first_up);
  secs_held_up = millis_held_up / 1000;

  if (millis_held_up > 80) {

    if (cur_pin_up == LOW && secs_held_up > prev_secs_held_up) {
      stt_up = 2;
      //Serial.println("hold up");
    }
    // check if the button was released since we last checked
    if (cur_pin_up == HIGH && prev_pin_up == LOW) {
      if (secs_held_up <= 0) {
        stt_up = 1;
        //Serial.println("click up");
      }
    }
  }
  prev_pin_up = cur_pin_up;
  prev_secs_held_up = secs_held_up;
  //up button============================================================

  //down button==========================================================
  cur_pin_down = digitalRead(pin_down);
  // if the button state changes to pressed, remember the start time
  if (cur_pin_down == LOW && prev_pin_down == HIGH && (millis() - button_first_down) > 200) {
    button_first_down = millis();
  }

  millis_held_down = (millis() - button_first_down);
  secs_held_down = millis_held_down / 1000;

  if (millis_held_down > 80) {

    if (cur_pin_down == LOW && secs_held_down > prev_secs_held_down) {
      stt_down = 2;
      //Serial.println("hold down");
    }
    // check if the button was released since we last checked
    if (cur_pin_down == HIGH && prev_pin_down == LOW) {
      if (secs_held_down <= 0) {
        stt_down = 1;
        //Serial.println("click down");
      }
    }
  }
  prev_pin_down = cur_pin_down;
  prev_secs_held_down = secs_held_down;
  //down button==========================================================
}

void on_NS() {
  i_set_point = 0; //reset i_set_point to 0 then turn on N,S
  Serial.println("onNS");

  delay(500);
  digitalWrite(pin_onNS, HIGH);
  digitalWrite(led_start, HIGH);
}

void off_NS() {
  digitalWrite(led_start, LOW);
  digitalWrite(pin_onNS, LOW);
  delay(10);
  digitalWrite(pin_onNS, LOW);
}

void initled() {
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
}

void checkled() {
  int i = 10;
  unsigned long t_checkled = millis();
  while (i--) {
    while (millis() - t_checkled < 250) {
      show(i);
    }
    t_checkled = millis();
  }
}

void tit() {
  /*
  digitalWrite(buzz, HIGH);
  delay(50);
  digitalWrite(buzz, LOW);
  */
}

void initIO() {
  pinMode(led_start, OUTPUT);
  pinMode(led_auto, OUTPUT);
  pinMode(buzz, OUTPUT);
  pinMode(pin_onNS, OUTPUT);

  digitalWrite(pin_onNS, LOW);
  digitalWrite(led_auto, LOW);
  digitalWrite(led_start, LOW);

  pinMode(pin_start, INPUT_PULLUP);
  pinMode(pin_stop, INPUT_PULLUP);
  pinMode(pin_up, INPUT_PULLUP);
  pinMode(pin_down, INPUT_PULLUP);
}

void initADC() {
  pinMode(A0, INPUT);
  pinMode(A1, INPUT);
  pinMode(A2, INPUT);
  pinMode(A3, INPUT);

  digitalWrite(A0, LOW);
  digitalWrite(A1, LOW);
  digitalWrite(A2, LOW);
  digitalWrite(A3, LOW);

  analogReference(INTERNAL);
  delay(1000);
  // ADC voltage reference from TL431
  analogReference(EXTERNAL);

  // snap i_load
  snap_i_load.setActivityThreshold(1.0);
  snap_i_load.setSnapMultiplier(0.1);

  // snap_i_s
  snap_i_s.setActivityThreshold(1.0);
  snap_i_s.setSnapMultiplier(0.025);

  // snap_u_s
  snap_u_s.setActivityThreshold(1.0);
  snap_u_s.setSnapMultiplier(0.0075);
}

void initPWMDAC() {
  fastPWMdac.init(pin_i_set, 10); // init for 10 bit PWM resolution
}

void off_i() {
  off_NS();
  fastPWMdac.analogWrite10bit(0);
}

void error1() {
  off_NS();
  buzz1();
  t_buzz = millis();
  stt_run = false;
}
void buzz1() {
  if (millis() - t_buzz > 1000) {
    t_buzz = millis();
    //tit();
  }
}


void set_i(int i_set_point) {
  if (i_set_point == 0) pwm_i_set = 0;
  delta_i_set = abs(i_set_point - value_i_load);

  if (delta_i_set > 5) {
    if (value_i_load < i_set_point) {// increase I
      if (millis() - t_upI > 50) {
        if (++pwm_i_set > max_pwm_i_set) pwm_i_set = 1023;
        // 10 bit resolution, period = 7.8Khz
        fastPWMdac.analogWrite10bit(pwm_i_set);
        t_upI = millis();
        if (debug) {
          Serial.print(delta_i_set);
          Serial.println("\t+");
        }
      }
    }

    if (value_i_load > i_set_point) {// decrease I
      if (millis() - t_downI > 50) {
        if (--pwm_i_set < min_pwm_i_set) pwm_i_set = 0;
        fastPWMdac.analogWrite10bit(pwm_i_set);
        if (debug) {
          Serial.print(delta_i_set);
          Serial.println("\t-");
        }
      }
    }
    //stt_set_i = false;
  }
  else stt_set_i = true;
}

void sensing() {
  analogRead(pin_u_s);
  raw_u_s = snap_u_s.getValue();
  raw_i_s = snap_i_s.getValue();
  raw_i_load = snap_i_load.getValue();

  //voltage: raw/1023.0*Vref*19
  if (raw_u_s) value_u_s = raw_u_s * 9.275 + 69.75;
  else value_u_s = 0;

  // i_s
  if (raw_i_s) value_i_s = raw_i_s * 5.3 - 5.2;
  else value_i_s = 0;

  // i_load
  if (raw_i_load) value_i_load = raw_i_load * 5.3 + 10.0;
  else value_i_load = 0;
}



