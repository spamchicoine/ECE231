#include <avr/io.h>
#include <string.h>

void uart_init(void);
void uart_send(unsigned char letter);
void send_string(char *stringAddress);