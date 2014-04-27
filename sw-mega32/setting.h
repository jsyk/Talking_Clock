#ifndef SETTING_H
#define SETTING_H

/* CPU Clock frequency in Hz */
#define F_CPU           16000000L
// #define F_CPU           1000000L
// #define F_CPU           8000000L

/* Serial port Baud rate */
// #define BAUDRATE        2400
// #define BAUDRATE        4800
// #define BAUDRATE        9600
#define BAUDRATE        38400
// #define BAUDRATE        115200

/* SPI clock in Hz */
#define SPIRATE         250000


// #define SOUND_SAMPLERATE_KHZ    22
// #define SOUND_BITS_PER_SAMPLE    8

#define SOUND_SAMPLERATE_KHZ        8
#define SOUND_BITS_PER_SAMPLE       16

#endif
