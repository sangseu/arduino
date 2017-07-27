#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Max72xxPanel.h>
#include <SoftwareSerial.h>
#include <Fonts/unispaceB7pt7b.h>
SoftwareSerial BTserial(2, 3); // RX | TX

int pinCS = 10; // Attach CS to this pin, DIN to MOSI and CLK to SCK (cf http://arduino.cc/en/Reference/SPI )

//ARDUINO CONNECT MAX_MATRIX7219
//CS ----- PIN10
//DIN ----- PIN11
//CLK ----- PIN13

int numberOfHorizontalDisplays = 2;  //số trong Hiển thị theo chiều ngang
int numberOfVerticalDisplays = 8;   // Số trong hiển thị theo chiều dọc.

Max72xxPanel matrix = Max72xxPanel(pinCS, numberOfHorizontalDisplays, numberOfVerticalDisplays); // cấu hình matrix

String tape = "The quick brown fox jumps over the lazy dog 1234567890";   // ký tự muốn truyền lên matrix "Youtube: Nhomreview - Gmail: nhomreview@gmail.com >>>TRONGSANG.FEE<<<"
int wait = 50; // thời gian chạy chữ.

int spacer = 1; // khoảng cách cách chữ
int width = 8 + spacer; // độ rộng của font là 5 fixel

String BTreceive = " ";

void setup() {

  BTserial.begin(9600);
  Serial.begin(9600);

  matrix.setIntensity(15); // cài đặt giá trị độ tương phản từ 0 đến 15.

  // điều chỉnh hiển thị theo nhu câu của người dùng.
  //  matrix.setPosition(0, 1, 3);  // The first display is at <0, 0>
  //   matrix.setPosition(1, 1, 0);  // The second display is at <1, 0>
  //   matrix.setPosition(2, 2, 0);  // The third display is at <2, 0>
  //  matrix.setPosition(3, 3, 0);  // And the last display is at <3, 0>
  //  ...
  // matrix.setRotation(0, 2);    // Màn hình hiển thị đầu tiên là vị trí đảo ngược
  matrix.setRotation(1);    // The same hold for the last display
  matrix.setFont(&unispaceB7pt7b);

  delay(50);
  /*
    matrix.drawChar(0, 11, 'A', HIGH, LOW, 1);
    matrix.drawChar(9, 11, 'B', HIGH, LOW, 1);
    matrix.drawChar(18, 11, '@', HIGH, LOW, 1);
    matrix.drawChar(27, 11, '#', HIGH, LOW, 1);
    matrix.drawChar(36, 11, '$', HIGH, LOW, 1);
    matrix.drawChar(45, 11, '%', HIGH, LOW, 1);
    matrix.drawChar(54, 11, '&', HIGH, LOW, 1);
    matrix.write();
  */


}

void loop() {
  int i = 0;

  // just show
  /*
  for (i = 0; 5 * i < 32; i++) {
    matrix.drawChar(i * 9, 12, tape[i], HIGH, LOW, 1);
    //matrix.write();
    //delay(30);
  }
  matrix.write();
  while (1);
  */

  for (int j=0; i<numberOfVerticalDisplays*8; j++) {
    for (i = 0; 5 * i < 32; i++) {
      matrix.drawChar(i * 9, 12, tape[i], HIGH, LOW, 1);
    }
    matrix.fillRect(j, 0, numberOfVerticalDisplays*8-j, numberOfHorizontalDisplays*8, 0);
    matrix.write();
    delay(1);
  }

  /*

  matrix.fillScreen(LOW);

  matrix.drawChar(0, 11, 'A', HIGH, LOW, 1);
  matrix.drawChar(9, 11, 'B', HIGH, LOW, 1);
  matrix.drawChar(18, 11, '@', HIGH, LOW, 1);
  matrix.drawChar(27, 11, '#', HIGH, LOW, 1);
  matrix.drawChar(36, 11, '$', HIGH, LOW, 1);
  matrix.drawChar(45, 11, '%', HIGH, LOW, 1);
  matrix.drawChar(54, 11, '&', HIGH, LOW, 1);
  matrix.write();
  delay(3000);
  */
  // left to right
  /*
  for ( int i = 0 ; i < width * tape.length() + matrix.width() - 1 - spacer; i++ )
  {

    matrix.fillScreen(LOW);

    int letter = i / width;
    int x = (matrix.width() - 1) - i % width;
    //int y = (matrix.height() - 8) / 2; // center the text vertically
    int y = 12;

    while ( x + width - spacer >= 0 && letter >= 0 ) {
      if ( letter < tape.length() ) {
        matrix.drawChar(x, y, tape[letter], HIGH, LOW, 1);
      }

      letter--;
      x -= width;
      //delay(500);
    }

    matrix.write(); // Send bitmap to display

    delay(12);
  }
  */



  /*
  matrix.drawChar(0,0,'<', HIGH, LOW, 1);
  matrix.write();
  //matrix.fillScreen(LOW);
  delay(25);

  matrix.drawChar(6,0,'/', HIGH, LOW, 1);
  matrix.write();
  //matrix.fillScreen(LOW);
  delay(25);

  matrix.drawChar(11,0,'>', HIGH, LOW, 1);
  matrix.write();
  delay(500);

  matrix.fillScreen(LOW);
  matrix.write();
  delay(500);

  */

  /*
      matrix.writePixel(0, 3, true);
      matrix.writePixel(1, 3, true);
      matrix.writePixel(2, 3, true);
      matrix.writePixel(3, 3, true);
      matrix.write();
      delay(10);

      matrix.writePixel(4, 3, true);
      matrix.writePixel(5, 3, true);
      matrix.writePixel(6, 3, true);
      matrix.writePixel(7, 3, true);
      matrix.write();
      delay(10);

      matrix.writePixel(8, 3, true);
      matrix.writePixel(9, 3, true);
      matrix.writePixel(10, 3, true);
      matrix.write();
      delay(10);

      matrix.writePixel(11, 3, true);
      matrix.writePixel(12, 3, true);
      matrix.writePixel(13, 3, true);
      matrix.write();
      delay(10);

      matrix.writePixel(14, 3, true);
      matrix.writePixel(15, 3, true);
      matrix.write();
      delay(10);

      matrix.writePixel(16, 3, true);
      matrix.writePixel(17, 3, true);
      matrix.write();
      delay(10);

      for (i = 18; i < 32; i++) {
        matrix.writePixel(i, 3, true);
        matrix.write();
        delay(10);
      }
      delay(500 - 190);

      matrix.fillScreen(LOW);
      matrix.write();
      delay(500);

      //============

      matrix.fillScreen(LOW);
      matrix.write();

      for (i = 0; 5 * i < 32; i++) {
        matrix.drawChar(i * 5, 0, tape[i], HIGH, LOW, 1);
        //matrix.write();
        //delay(30);
      }
      matrix.write();

      delay(500);

      for (i = 0; i < 32; i++) {
        matrix.writePixel(i, 7, true);
      }
      matrix.write();

      delay(500);

      matrix.fillScreen(LOW);
      matrix.write();
      delay(500);
    */

  /*

  int i = 1;
  matrix.drawChar(i,0,128, HIGH, LOW, 1);
  matrix.drawChar(i+=6,0,129, HIGH, LOW, 1);
  matrix.drawChar(i+=6,0,130, HIGH, LOW, 1);
  matrix.drawChar(i+=6,0,131, HIGH, LOW, 1);
  matrix.drawChar(i+=6,0,132, HIGH, LOW, 1);
  matrix.write();
  delay(1000);

  matrix.fillScreen(LOW);
  matrix.write();
  delay(250);
  */

  /*
  char c = ' ';

  if (BTserial.available())
  {
    BTreceive = BTreceive + BTserial.readStringUntil('\n');
    i = 1;
  }

  if (i == 1) {
    matrix.fillScreen(LOW);
    matrix.write();

    for (i = 0; i < BTreceive.length() - 1; i++) {
      matrix.drawChar(i * 6, 0, BTreceive[i], HIGH, LOW, 1);
      //matrix.write();
      //delay(30);
    }
    Serial.println(BTreceive);
    matrix.write();
    BTreceive = "";
  }
  */
}

