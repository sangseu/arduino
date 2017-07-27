#define led1 0
#define led2 1
#define led3 2
#define led4 3
#define led5 4
#define led6 5
#define led7 6
#define led8 7

int a = 0;
int b = 0;

void setup() {
  // put your setup code here, to run once:
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(led3, OUTPUT);
  pinMode(led4, OUTPUT);
  pinMode(led5, OUTPUT);
  pinMode(led6, OUTPUT);
  pinMode(led7, OUTPUT);
  //pinMode(led8, OUTPUT);

  offAll();

  a = 30;
}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(led1, LOW);
  delay(a);
  digitalWrite(led2, LOW);
  delay(a);
  digitalWrite(led3, LOW);
  delay(a);
  digitalWrite(led4, LOW);
  delay(a);
  digitalWrite(led5, LOW);
  delay(a);
  digitalWrite(led6, LOW);
  delay(a);
  digitalWrite(led7, LOW);
  //delay(a);
  //digitalWrite(led8, LOW);
  
  b = 500-a*6;
  delay(b);
  
  offAll();
  delay(500);
  
}

void offAll() {
  digitalWrite(led1, HIGH);
  digitalWrite(led2, HIGH);
  digitalWrite(led3, HIGH);
  digitalWrite(led4, HIGH);
  digitalWrite(led5, HIGH);
  digitalWrite(led6, HIGH);
  digitalWrite(led7, HIGH);
  //digitalWrite(led8, HIGH);
}

