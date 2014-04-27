#include "ir.h"
#include "clock.h"
#include "serial.h"
#include <avr/interrupt.h>
/* IR reciever */


const ir_code_t PROGMEM known_ir_codes[1] = {
    { 0x0000000000510401ULL, 0x19, { Cmd_SayTime, {.arg1p = &gv_clock_main} } },
};

uint16_t g_ir_last_good_time = 0;

#if 0
volatile uint16_t ir_measure_width;
volatile uint8_t ir_measure_polarity;
volatile uint8_t ir_measure_done;
#endif

volatile uint64_t gv_ir_d_code;
volatile uint8_t gv_ir_d_codelen;


ISR(INT0_vect)
{
    static uint16_t prev_tick = 0;
    static uint64_t cmdcode = 0;
    static uint8_t cmdlen = 0;

    uint16_t now = (current_cpu_tick << 8) | TCNT2;     /* range: 2s */
    uint16_t width = tick_diff(now, prev_tick);

#if 0
    uint8_t polarity = ((PIND >> 2) & 1);
    ir_measure_width = width;
    ir_measure_polarity = polarity;
    ++ir_measure_done;                         /* signal done */
#endif

    prev_tick = now;

    char decoded = '?';
    if (width < 29) {
        decoded = '0';
    } else if (width > 29 && width < 69) {
        decoded = '1';
    } else if (width > 70 && width < 90) {
        decoded = 'S';
    } else if (width > 300) {
        decoded = ' ';
    }

#if 1
    switch (decoded) {
        case 'S': {
            cmdcode = 0;
            cmdlen = 1;
            break;
        }
        case '0': {
            if (cmdlen) {
                cmdcode <<= 1;
                ++cmdlen;
            }
            break;
        }
        case '1': {
            if (cmdlen) {
                cmdcode = (cmdcode << 1) | (uint64_t)1;
                ++cmdlen;
            }
            break;
        }
        case ' ': {     /* stop */
            /* code complete */
            gv_ir_d_code = cmdcode;
            gv_ir_d_codelen = cmdlen;
            /* fall through */
        }
        default: {
            /* clear, restart */
            cmdcode = 0;
            cmdlen = 0;
            break;
        }
    }
#endif
}



void ir_init(void)
{

}

int8_t ir_process(const message_t *msg)
{
    int8_t msg_recognized = 1;

    switch (msg->cmd) {
        default:
            msg_recognized = 0;
            break;
    }

    /* sample */
    uint8_t cmdlen = gv_ir_d_codelen;
    uint32_t cmdcode = gv_ir_d_code;

    if (cmdlen != 0) {
        /* IR has received a good code */
        /* invalidate in global shared variables */
        gv_ir_d_codelen = 0;
        gv_ir_d_code = 0;

        usart_sendstr_P(PSTR("IR:0x"));
        /*usart_sendhexl(cmdcode >> 32);*/ usart_sendhexl(cmdcode);
        usart_sendstr_P(PSTR("/0x"));
        usart_sendhexb(cmdlen);
        usart_send_nl();

        if ((g_ir_last_good_time == 0) || tick_diff(current_cpu_tick, g_ir_last_good_time) > MS2TICK(1000)) {
            g_ir_last_good_time = 0;

            for (int8_t i = 0; i < sizeof(known_ir_codes); ++i) {
                if ((pgm_read_byte_near(&known_ir_codes[i].cmdlen) == cmdlen) 
                    && (pgm_read_dword_near(&known_ir_codes[i].cmdcode) == cmdcode)) {
                    /* found! */
                    message_t *nmsg = loop_put_msg_begin();
                    nmsg->cmd = pgm_read_byte_near(&known_ir_codes[i].genmsg.cmd);
                    nmsg->arg1i = pgm_read_word_near(&known_ir_codes[i].genmsg.arg1i);
                    loop_put_msg_end(nmsg);
                    g_ir_last_good_time = current_cpu_tick | 1;
                    break;
                }
            }
        }

    }

    return msg_recognized;
}
