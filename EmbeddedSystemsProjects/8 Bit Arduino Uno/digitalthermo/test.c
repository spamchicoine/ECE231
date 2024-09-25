#include <avr/io.h>
#include <string.h>
#include <util/delay.h>
#include <stdlib.h>

int main(void){
    DDRD = 0xF0;
    DDRB = 0xFF;
    DDRC = 0x0F;
    PORTB = 0xFF;

    unsigned char digitsa[] = {0x0F, 0x06, 0x0B, 0x0F, 0x06, 0x0D, 0x0D, 0x07, 0x0F, 0x07};
    unsigned char digitsb[] = {0x30, 0x00, 0x50, 0x40, 0x60, 0x60, 0x70, 0x00, 0x70, 0x60};

    unsigned char digitsdp[] = {0xB0, 0x80, 0xD0, 0xC0, 0xE0, 0xE0, 0xF0, 0x80, 0xF0, 0xE0};

    unsigned int count = 0;

    while(1){
        count++;

        if (count>9){
            count = 0;
        }

        PORTC = digitsa[count];
        PORTD = digitsb[count];
        PORTB = ~(1<<2);
        _delay_ms(500);

        PORTD = digitsdp[count];
        PORTB = ~(1<<2);
        _delay_ms(500);

    }
    return 0;
}