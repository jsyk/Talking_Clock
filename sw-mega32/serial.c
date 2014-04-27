#include "setting.h"
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/wdt.h>
#include "serial.h"

#if F_CPU == 16000000L
/*
 * seriovy port na ATMEGA32
 * fosc = 16MHz (xtal)
 * baud = 115.2k
 * Fuse Low Byte      = 0xff
 * Fuse High Byte     = 0xc9
 * Fuse Extended Byte = 0xff
 */

#if BAUDRATE == 9600
/* 9600 Baud @ 16MHz */
#define BAUD_RATE_UBRR      103
#define BAUD_RATE_U2X       0
#endif

#if BAUDRATE == 38400
/* 38.4 kBaud @ 16MHz */
#define BAUD_RATE_UBRR      25
#define BAUD_RATE_U2X       0
#endif

 #if BAUDRATE == 57600
/* 57.6 kBaud @ 16MHz, U2X */
#define BAUD_RATE_UBRR      34
#define BAUD_RATE_U2X       1
#endif

#if BAUDRATE == 115200
/* 115.2 kBaud @ 16MHz, U2X */
#define BAUD_RATE_UBRR          16
#define BAUD_RATE_U2X           1
#endif
#endif


#if F_CPU == 1000000L

#if BAUDRATE == 9600
/* 9600 Baud @ 1MHz */
#define BAUD_RATE_UBRR      12
#define BAUD_RATE_U2X       1
#endif

#if BAUDRATE == 4800
/* 4800 Baud @ 1MHz */
#define BAUD_RATE_UBRR      12
#define BAUD_RATE_U2X       0
#endif

#if BAUDRATE == 2400
/* 4800 Baud @ 1MHz */
#define BAUD_RATE_UBRR      25
#define BAUD_RATE_U2X       0
#endif

#endif


/* 8 MHz Clock */
#if F_CPU == 8000000L

#if BAUDRATE == 9600
/* 9600 Baud @ 8MHz */
#define BAUD_RATE_UBRR      51
#define BAUD_RATE_U2X       0
#endif

#if BAUDRATE == 38400
/* 38.4 kBaud @ 8MHz */
#define BAUD_RATE_UBRR      25
#define BAUD_RATE_U2X       1
#endif

#if BAUDRATE == 115200
/* 115.2 kBaud @ 8MHz, U2X */
#define BAUD_RATE_UBRR          8
#define BAUD_RATE_U2X           1
#endif

#endif


// const unsigned int BAUD_RATE_UBRR = 1;               /* 500 kBaud @ 16MHz */

void usart_init()
{
    /* USART init */
    UBRRH = (unsigned char)(BAUD_RATE_UBRR >> 8);
    UBRRL = (unsigned char)(BAUD_RATE_UBRR);
    UCSRA = (BAUD_RATE_U2X << U2X);
    /* zapnout prijimac a vysilac */
    UCSRB = (1 << RXEN) | (1 << TXEN);
    /* nastavit format ramce: 8b data, 1 stop bit */
    UCSRC = (1 << URSEL) | (3 << UCSZ0);
}

void usart_transmit(unsigned char data)
{
    /* wait for empty tx buffer */
    while (!(UCSRA & (1<<UDRE)))
        ;
    
    /* Put data into buffer, sends the data */
    UDR = data;
}

unsigned char usart_receive()
{
    /* Wait for data to be received */
    while ( !(UCSRA & (1<<RXC)) )
        ;
    /* Get and return received data from buffer */
    return UDR;
}

char usart_rx_ischar()
{
    return (UCSRA & (1<<RXC));
}

void usart_sendstr(const char* s)
{
    while (*s) {
        if (*s == '\n')
            usart_transmit('\r');       // firstly send CR
        usart_transmit(*s);
        ++s;
    }
}

void usart_sendstr_P(const char* PROGMEM s)
{
    char ch = pgm_read_byte_near(s);
    while (ch) {
        if (ch == '\n')
            usart_transmit('\r');       // firstly send CR
        usart_transmit(ch);
        ch = pgm_read_byte_near(++s);
    }
}

// static const hexchars[16] = {
//     '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
// };

static inline unsigned char to_hexchar(uint8_t b)
{
    if (b < 10)
        return b + '0';
    else
        return b + (-10 + 'A');
}

void usart_sendhexb(unsigned char b)
{
//     usart_transmit(hexchars[(b >> 4) & 0x0F]);
//     usart_transmit(hexchars[b & 0x0F]);
    usart_transmit(to_hexchar((b >> 4) & 0x0F));
    usart_transmit(to_hexchar(b & 0x0F));
}

void usart_sendhexl(unsigned long l)
{
    int i;
    for (i = 0; i < 4; i++) {
        usart_sendhexb(l >> 24);
        l <<= 8;
    }
}

int usart_getline(char *buf, int buflen)
{
    int cnt = 0;
    while (cnt < buflen) {
        char ch = usart_receive();
        if (ch == '\r') {
            buf[cnt] = '\0';
            return cnt;
        } else
            buf[cnt++] = ch;
    }
    /* we loose 1 char, but it's better than to risk buffer overflow */
    buf[buflen-1] = '\0';
    return cnt;
}

unsigned char hchar_to_nibble(char ch)
{
    if (ch >= '0' && ch <= '9')
        return ch - '0';
    else if (ch >= 'a' && ch <= 'f')
        return ch - 'a' + 10;
    else if (ch >= 'A' && ch <= 'F')
        return ch - 'A' + 10;
    else
        /* error */
        return 0;
}

unsigned long hstr_to_ulong(const char *s)
{
    unsigned long res = 0;
    while (*s) {
        res = (res << 4) | hchar_to_nibble(*s);
        s++;
    }
    return res;
}

void usart_send_nl()
{
    usart_transmit('\r');       //CR
    usart_transmit('\n');       //LF
}

