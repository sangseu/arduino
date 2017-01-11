int SER_Pin = 8;   //pin 14 on the 75HC595
int RCLK_Pin = 9;  //pin 12 on the 75HC595
int SRCLK_Pin = 10; //pin 11 on the 75HC595

//How many of the shift registers - change this
#define number_of_74hc595s 4 

//do not touch
#define numOfRegisterPins number_of_74hc595s * 8

boolean registers[numOfRegisterPins];

void setup(){
  pinMode(SER_Pin, OUTPUT);
  pinMode(RCLK_Pin, OUTPUT);
  pinMode(SRCLK_Pin, OUTPUT);


  //reset all register pins
  clearRegisters();
  writeRegisters();
}               


//set all register pins to LOW
void clearRegisters(){
  for(int i = numOfRegisterPins - 1; i >=  0; i--){
    if(i>7 && i <16) registers[i] = HIGH;
    else if(i>23) registers[i] = HIGH;
    else registers[i] = LOW;
  }
} 


//Set and display registers
//Only call AFTER all values are set how you would like (slow otherwise)
void writeRegisters(){

  digitalWrite(RCLK_Pin, LOW);

  for(int i = numOfRegisterPins - 1; i >=  0; i--){
    digitalWrite(SRCLK_Pin, LOW);

    int val = registers[i];

    digitalWrite(SER_Pin, val);
    digitalWrite(SRCLK_Pin, HIGH);

  }
  digitalWrite(RCLK_Pin, HIGH);

}

//set an individual pin HIGH or LOW
void setRegisterPin(int index, int value){
  registers[index] = value;
}
void set7seg(int number, bool dot) {
  for (int i=0; i<8; i++) {
    registers[7-i] = (number>>i)&1;
  }
  if (dot) registers[7] = 1;
  
}

void set7seg2(int number, bool dot) {
  for (int i=16; i<24; i++) {
    registers[24-i] = (number>>i)&1;
  }
  if (dot) registers[24] = 1;
  
}

void turnOnLed(int index) {
  if (index < 8) registers[index+7] = 0;
  if (index > 7) registers[index+15] = 0;
}


void loop(){

  /* setRegisterPin(2, 1);
  setRegisterPin(3, 1);
  setRegisterPin(4, 0);
  setRegisterPin(5, 1);
  setRegisterPin(7, 1); */
  //setRegisterPin(1, 1);
  //setRegisterPin(2, 1);
  set7seg(0x60, true); //1
  
  set7seg2(0xDA, true); //2
  
  turnOnLed(9);
  //turnOnLed(2);
  //turnOnLed(9);
  
  /*
  setRegisterPin(1, 1);
  setRegisterPin(2, 1);
  setRegisterPin(3, 1);
  setRegisterPin(4, 1);
  setRegisterPin(5, 1);
  setRegisterPin(6, 1);
  setRegisterPin(7, 1);
  setRegisterPin(8, 1);

  setRegisterPin(9, 1);
  setRegisterPin(10, 1);
  setRegisterPin(11, 1);
  setRegisterPin(12, 1);
  setRegisterPin(13, 1);
  setRegisterPin(14, 1);
  setRegisterPin(15, 1);
  setRegisterPin(16, 1);
  */
  


  writeRegisters();  //MUST BE CALLED TO DISPLAY CHANGES
  clearRegisters();
  //Only call once after the values are set how you need.
}
