#include <avr/io.h>
#include "i2c.h"
#include "SSD1306.h"
#include <util/delay.h>
#include "uart.h"

#define TRIG PB1
#define ECHO PB0
#define RPC 1.098

void timer0_init(void);

int main(void){
    unsigned int rec, fec, ewc;
    float target_range;
    DDRB = 1<<TRIG;
    PORTB &= ~(1<<TRIG);
    timer0_init();


    OLED_Init();
    while (1){
        TCNT0 = 0;
        PORTB |= 1<<TRIG;
        _delay_us(10);
        PORTB &= ~(1<<TRIG);

        while ((PINB &(1<<ECHO))==0);
        rec = TCNT0;
        while (!(PINB &(1<<ECHO))==0);
        fec = TCNT0;

        if (fec > rec){
            ewc = fec - rec;
            target_range = ewc * RPC;
            
        }
        OLED_GoToLine(3);
        OLED_DisplayString("Distance in cm: ");
        OLED_DisplayNumber(10,target_range,3);

        OLED_GoToLine(4);
        OLED_DisplayString("Distance in in: ");
        OLED_DisplayNumber(10,target_range/2.54,2);

        _delay_ms(50);
    }
    return 0;
}

void timer0_init(){
    TCCR0A = 0;
    TCCR0B = 5;
    TCNT0 = 0;
}