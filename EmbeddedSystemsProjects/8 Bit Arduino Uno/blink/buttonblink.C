#include <avr/io.h>
#include <util/delay.h>
#define MYDELAY 100

int main(void)
{
    DDRD = (1<<DDD5)|(1<<DDD6);
    PORTD = 1<<0|1<<1|1<<3;

    
    while(1){
        if(((PIND & (1<<PIND0))==0)&&((PIND & (1<<PIND1))!=0)&&((PIND & (1<<PIND3))!=0)){
            PORTD |= (1<<PORTD5);
            _delay_ms(MYDELAY);
            PORTD &= ~(1<<PORTD5);
            _delay_ms(MYDELAY);
        }
        
        if(((PIND & (1<<PIND1))==0)&&((PIND & (1<<PIND0))!=0)&&((PIND & (1<<PIND3))!=0)){
            PORTD |= (1<<PORTD6);
            _delay_ms(MYDELAY);
            PORTD &= ~(1<<PORTD6);
            _delay_ms(MYDELAY);
        }
        
        if(((PIND & (1<<PIND3))==0)&&((PIND & (1<<PIND0))!=0)&&((PIND & (1<<PIND1))!=0)){
            PORTD |= (1<<PORTD6);
            PORTD |= (1<<PORTD5);
            _delay_ms(MYDELAY);
            PORTD &= ~(1<<PORTD6);
            PORTD &= ~(1<<PORTD5);
            _delay_ms(MYDELAY);

        }
        

    }
    
    return 0;

}