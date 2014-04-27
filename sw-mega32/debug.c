#include "debug.h"
#include "serial.h"
#include "sound.h"
#include "clock.h"


/* rom string resources for main() */
const char str_Banner[] PROGMEM = "\n\n== DCF-NIX-VOICE ==\n\n";
const char str_SetClock_EnterHour[] PROGMEM = " / set clock: Enter hour in hex=";
const char str_EnterMinute[] PROGMEM = "Enter minute in hex=";
const char str_ClockSet[] PROGMEM = "Clock set.\n";
const char str_DisplayingClock[] PROGMEM = " / displaying the clock on nixies.";


void debug_init(void)
{
    usart_sendstr_P(str_Banner);
}


int8_t debug_process(const message_t *msg)
{
    int8_t msg_recognized = 1;

    switch (msg->cmd) {
        default:
            msg_recognized = 0;
            break;
    }

    if (usart_rx_ischar()) {
        unsigned char ch = usart_receive();
        switch (ch) {
            case 'v': {
                message_t *nmsg = loop_put_msg_begin();
                nmsg->cmd = Cmd_SayTime;
                nmsg->arg1p = &gv_clock_main;      // WARN: discards volatile
                loop_put_msg_end(nmsg);
                break;
            }

#define SBUF_SIZE   (2*SNDBUF_SIZE)
            case 'O': {
                char *sbuf = (char*)g_sound_buf;
                usart_sendstr_P(PSTR(" / file open: Enter abs. file name = "));
                usart_getline(sbuf, SBUF_SIZE);
                usart_sendstr_P(PSTR("\nFile name: '"));
                usart_sendstr(sbuf);
                usart_sendstr_P(PSTR("'\n"));
                
                message_t *nmsg = loop_put_msg_begin();
                nmsg->cmd = Cmd_PlayFileName;
                nmsg->arg1p = sbuf;
                loop_put_msg_end(nmsg);
                break;
            }

            default: {
                usart_sendstr_P(PSTR("\nCmd? "));
                break;
            }
        }

    }


    return msg_recognized;
}
