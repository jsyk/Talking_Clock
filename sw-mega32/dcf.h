#ifndef DCF_H
#define DCF_H

#include <avr/pgmspace.h>


extern volatile uint16_t dcf_measure_width;
extern volatile uint8_t dcf_measure_polarity;
extern volatile uint8_t dcf_measure_done;


extern volatile uint8_t dcf_dec_insync;
extern volatile uint8_t dcf_dec_index;
extern volatile uint32_t dcf_dec_shift;
extern volatile uint8_t dcf_dec_success;


char decode_dcf_measurement(uint8_t polarity, uint16_t width);

char parity(uint32_t x, char n);


#endif
