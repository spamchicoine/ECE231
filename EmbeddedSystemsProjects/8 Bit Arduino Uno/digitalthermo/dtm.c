#include <avr/io.h>
#include <string.h>
#include <util/delay.h>
#include <stdlib.h>

void uart_init(void);
void uart_send(unsigned char letter);
void send_string(char *stringAddress);

// here are the function definitions
void send_string(char *stringAddress){

    for (unsigned char i = 0; i < strlen(stringAddress); i++)
        uart_send(stringAddress[i]);
}

void uart_init(void){
    UCSR0B = (1 << TXEN0); //enable the UART transmitter
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00); //set 8 bit character size
    UBRR0L = 103; //set baud rate to 9600 for 16 MHz crystal
}

void uart_send(unsigned char ch){
    while (!(UCSR0A & (1 << UDRE0))); //wait til tx data buffer empty
    UDR0 = ch; //write the character to the USART data register
}
int main(void){
    DDRD = 0xF0;
    DDRB = 0xFF;
    DDRC = 0x0F;
    PORTB = 0xFF;

    unsigned char digitsa[] = {0x0F, 0x06, 0x0B, 0x0F, 0x06, 0x0D, 0x0D, 0x07, 0x0F, 0x07, 0x09,0x01};
    unsigned char digitsb[] = {0x30, 0x00, 0x50, 0x40, 0x60, 0x60, 0x70, 0x00, 0x70, 0x60, 0x30 ,0x70};

    unsigned char digitsdp[] = {0xB0, 0x80, 0xD0, 0xC0, 0xE0, 0xE0, 0xF0, 0x80, 0xF0, 0xE0};

    ADCSRA = 0x87;
    ADMUX = 0xC5;
    unsigned int reading;

    int count;
    int tempc = 0;
    int tempf = 0;
    int temp= 0;

    int DIG1 = 0;
    int DIG2 = 0;
    int DIG3 = 0;
    int DIG4 = 0;

    char buffer[4];
    uart_init();
    while(1){

        ADCSRA |= 1<<ADSC;
        while (ADCSRA & (1<<ADIF) ==0);
        reading = ADCL | (ADCH << 8);
        tempc = (((reading*1.1)/10.24) - 50)*10;
        tempf = ((tempc/10 * 9.0/5.0) + 32)*10;

        if ((PINB & (1<<PINB0))==0){
            DIG4 = 11;
            _delay_ms(5);

            temp = tempf;
        }
        else{
            DIG4 = 10;
            temp = tempc;

            _delay_ms(5);
        }

        DIG1 = (temp/100)%10;
        DIG2 = (temp/10)%10;
        DIG3 = temp%10;

        int i;
        for (i=0; i<6; i++){

            PORTC = digitsa[DIG1];
            PORTD = digitsb[DIG1];
            PORTB = ~ (1<<1);

            _delay_ms(5);

            PORTC = digitsa[DIG2];
            PORTD = digitsdp[DIG2];
            PORTB = ~ (1<<2);

            _delay_ms(5);

            PORTC = digitsa[DIG3];
            PORTD = digitsb[DIG3];
            PORTB = ~ (1<<3);

            _delay_ms(5);

            PORTC = digitsa[DIG4];
            PORTD = digitsb[DIG4];
            PORTB = ~(1<<4);
            
            itoa(temp,buffer,10);

            send_string(buffer);
            uart_send(13);
            uart_send(10);

        }
        
    }
    return 0;
}