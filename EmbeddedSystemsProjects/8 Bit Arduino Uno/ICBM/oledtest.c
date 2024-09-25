/*****************************************************************
 * oledtest.c  Program to test the SSD1306 OLED display
 * This source code is based on the example found here:
 * https://audiodiwhy.blogspot.com/2021/06/embedded-c-avr-getting-oled-display.html
 * the SSD1306 libraries i2c.c, i2c.h, SSD1306.c, and SSD1306.h were downloaded from
 * https://github.com/Preston-Sundar/AVR-OLED-SSD1306-IIC-DRIVER
 * Tested using Makefile with bare 328p on breadboard and Sparkfun Pocket Programmer
 * 
 * Date         Author              Revision
 * 12/27/21     D. McLaughlin       initial writing of this code
 * 4/5/22       D. McLaughlin       released to ECE-304 class
*******************************************************************/

#include <avr/io.h>
#include "i2c.h"
#include "SSD1306.h"
#include <util/delay.h>

int main(void) {  
  OLED_Init();  //initialize the OLED
  while (1) {
    for (int i=0; i<=7; i++){
      OLED_GoToLine(i);
      OLED_DisplayString("Line #");
      OLED_DisplayNumber(10,i,2);
    }
    OLED_ScrollMessage(7,"Bottom row...");
    _delay_ms(100);
  }
  return 0; // never reached
}

/*** End of File***/