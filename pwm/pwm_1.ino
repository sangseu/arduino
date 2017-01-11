#include <avr/io.h>
#include <util/delay.h>


int main(void)
{
  pinMode(9, OUTPUT); // output pin for OCR2B
  pinMode(5, OUTPUT); // output pin for OCR0B

  pinMode(13, OUTPUT);

  unsigned int duty = 5; // %

  /*
  // Set up the 250KHz output
  TCCR2A = _BV(COM2A1) | _BV(COM2B1) | _BV(WGM21) | _BV(WGM20);
  TCCR2B = _BV(WGM22) | _BV(CS20);
  OCR2A = 63;
  OCR2B = 0;
  */

  TCCR1A = (1 << COM1A1) | (0 << COM1A0) | (0 << COM1B1) | (0 << COM1B0) | (1 << WGM11) | (0 << WGM10);
  TCCR1B = (0 << ICNC1) | (0 << ICES1) | (1 << WGM13) | (1 << WGM12) | (0 << CS12) | (0 << CS11) | (1 << CS10);
  TCNT1H = 0x00;
  TCNT1L = 0x00;
  ICR1H = 0x3E;
  ICR1L = 0x7F;
  OCR1AH = 0x03;
  OCR1AL = 0x20;
  OCR1BH = 0x00;
  OCR1BL = 0x00;


  /*
    // Set up the 8MHz output
    TCCR0A = _BV(COM0A1) | _BV(COM0B1) | _BV(WGM01) | _BV(WGM00);
    TCCR0B = _BV(WGM02) | _BV(CS00);
    OCR0A = 1;
    OCR0B = 0;
  */
  while (1) {
  }
}
