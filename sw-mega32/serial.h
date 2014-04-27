#ifndef SERIAL_H
#define SERIAL_H

#include <avr/pgmspace.h>
#include "loop.h"

void usart_init();

void usart_transmit(unsigned char data);
unsigned char usart_receive();
char usart_rx_ischar();

void usart_sendstr(const char* s);
void usart_sendstr_P(const char* PROGMEM s);
void usart_send_nl();

void usart_sendhexb(unsigned char b);
void usart_sendhexl(unsigned long l);
int usart_getline(char *buf, int buflen);

unsigned char hchar_to_nibble(char ch);
unsigned long hstr_to_ulong(const char *s);

#endif
