#include "setting.h"
#include "dcf.h"
#include "clock.h"

#include <avr/interrupt.h>

// #define PIN_DCF         ()

volatile uint16_t dcf_measure_width;
volatile uint8_t dcf_measure_polarity;
volatile uint8_t dcf_measure_done;


volatile uint8_t dcf_dec_insync = 0;
volatile uint8_t dcf_dec_index = 0;
volatile uint32_t dcf_dec_shift = 0;
volatile uint8_t dcf_dec_success = 0;


const uint8_t PROGMEM bcd_table[8] = { 1, 2, 4, 8, 10, 20, 40, 80 };

/* DCF77 Pulse Measuring */

char decode_dcf_measurement(uint8_t polarity, uint16_t width)
{
    char decoded = '?';
    if (polarity) {
        /* H */
        if (width >= MS2TICK(60) && width <= MS2TICK(140)) {
            /* Short pulse => binary 0 */
            decoded = '0';
        } else if (width >= MS2TICK(160) && width <= MS2TICK(240)) {
            /* Long pulse => binary 1 */
            decoded = '1';
        }
    } else {
        /* L */
        if (width >= MS2TICK(1600) && width <= MS2TICK(2030)) {
            /* Long no-pulse => Minute mark */
            decoded = 'M';
        } else if (width < MS2TICK(1600)) {
            decoded = ' ';
        }
    }
    return decoded;
}

/* compute parity of n least significant bits of x */
char parity(uint32_t x, char n)
{
    char r = 0;
    while (n--) {
        r ^= (x & 1);
        x >>= 1;
    }
    return r;
}


/* External Interrupt 1 - signal DCF77 */
ISR(INT1_vect)
{
    static uint16_t prev_tick = 0;
    uint16_t now = current_cpu_tick;

    uint16_t width = tick_diff(now, prev_tick);
#if 0
    uint8_t polarity = !((PIND >> 3) & 1);      /* invert to obtain previous polarity that has been just measured */
#else
    uint8_t polarity = ((PIND >> 3) & 1);
#endif
    dcf_measure_width = width;
    dcf_measure_polarity = polarity;
    ++dcf_measure_done;                         /* signal done */
    prev_tick = now;

    char decoded_bit = decode_dcf_measurement(polarity, width);

#define DECODE_FAIL         { dcf_dec_insync = 0; break; }
#define DECODE_CONTINUE     { goto here_decode_continue; }

    /* decoding state machine */
    switch (decoded_bit) {
        case 'M': {
            if (dcf_dec_insync && dcf_dec_index == 59) {
                /* completing a (correct) frame */
                g_clock_dcf_last_good = g_clock_dcf;
                gv_clock_main = g_clock_dcf;
                dcf_dec_success = 1;
            }

            /* starting new frame */
            dcf_dec_insync = 1;
            dcf_dec_index = 0;

            g_clock_dcf.msec128 = 0;
            g_clock_dcf.sec = 0;
            g_clock_dcf.min = 0;
            g_clock_dcf.hour = 0;
            g_clock_dcf.day = 0;
            g_clock_dcf.weekday = 0;
            g_clock_dcf.month = 0;
            g_clock_dcf.year = 0;
            break;
        }
        case '0':
        case '1': {
            if (!dcf_dec_insync)
                /* ignore if not in sync */
                break;

            if (dcf_dec_index >= 59) {
                DECODE_FAIL /* Bit 59 should be M */
            }

            if (dcf_dec_index == 0) {
                /* Frame start bit */
                if (decoded_bit != '0') {
                    DECODE_FAIL /* Bit 0 should be 0 */
                } else {
                    DECODE_CONTINUE
                }
            }

            if (dcf_dec_index >= 1 && dcf_dec_index <= 14) {
                /* Civil warning bits ignored */
                DECODE_CONTINUE
            }

            if (dcf_dec_index >= 15 && dcf_dec_index <= 19) {
                DECODE_CONTINUE
            }

            if (dcf_dec_index == 20) {
                /* Time info start bit */
                if (decoded_bit != '1') {
                    DECODE_FAIL /* Bit 20 should be 1 */
                } else {
                    DECODE_CONTINUE
                }                
            }

            if (dcf_dec_index >= 21 && dcf_dec_index <= 27) {
                /* BCD Minutes */
                if (decoded_bit == '1') {
                    g_clock_dcf.min += pgm_read_byte_near(&bcd_table[dcf_dec_index-21]);
                }
                DECODE_CONTINUE
            }

            if (dcf_dec_index == 28) {
                /* Parity for Minutes */
                char b = (decoded_bit == '1') ? 1 : 0;
                if (parity(dcf_dec_shift, 7) == b) {
                    DECODE_CONTINUE
                } else {
                    DECODE_FAIL
                }
            }

            if (dcf_dec_index >= 29 && dcf_dec_index <= 34) {
                /* BCD Hours */
                if (decoded_bit == '1') {
                    g_clock_dcf.hour += pgm_read_byte_near(&bcd_table[dcf_dec_index-29]);
                }
                DECODE_CONTINUE
            }

            if (dcf_dec_index == 35) {
                /* Even Parity for Hours */
                char b = (decoded_bit == '1') ? 1 : 0;
                if (parity(dcf_dec_shift, 6) == b) {
                    DECODE_CONTINUE
                } else {
                    DECODE_FAIL
                }
            }

            if (dcf_dec_index >= 36 && dcf_dec_index <= 41) {
                /* Days */
                if (decoded_bit == '1') {
                    g_clock_dcf.day += pgm_read_byte_near(&bcd_table[dcf_dec_index-36]);
                }
                DECODE_CONTINUE
            }

            if (dcf_dec_index >= 42 && dcf_dec_index <= 44) {
                /* Day of week */
                if (decoded_bit == '1') {
                    g_clock_dcf.weekday += pgm_read_byte_near(&bcd_table[dcf_dec_index-42]);
                }
                DECODE_CONTINUE
            }

            if (dcf_dec_index >= 45 && dcf_dec_index <= 49) {
                /* Month */
                if (decoded_bit == '1') {
                    g_clock_dcf.month += pgm_read_byte_near(&bcd_table[dcf_dec_index-45]);
                }
                DECODE_CONTINUE
            }

            if (dcf_dec_index >= 50 && dcf_dec_index <= 57) {
                /* Year */
                if (decoded_bit == '1') {
                    g_clock_dcf.year += pgm_read_byte_near(&bcd_table[dcf_dec_index-50]);
                }
                DECODE_CONTINUE
            }

            if (dcf_dec_index == 58) {
                /* Even Parity for Days...years */
                char b = (decoded_bit == '1') ? 1 : 0;
                if (parity(dcf_dec_shift, 22) == b) {
                    DECODE_CONTINUE
                } else {
                    DECODE_FAIL
                }
            }

            /* fail if not handled above */
            DECODE_FAIL

            /* jumps here when decoding was succesful so far and we should continue */
        here_decode_continue:
            ++dcf_dec_index;
            dcf_dec_shift = (dcf_dec_shift << 1) | ((decoded_bit == '1') ? 1 : 0);
            break;
        }
        case ' ': {
            /* ignore */
            break;
        }
        default: {
            /* fail */
            DECODE_FAIL
        }
    }
}
