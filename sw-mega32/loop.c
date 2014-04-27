#include "loop.h"
#include "voice.h"
#include "sound.h"
#include "vfs.h"
#include "serial.h"
#include "debug.h"
#include "ir.h"
#include "btn.h"
#include "clock.h"

#include <avr/wdt.h>

/*extern*/ message_buffer_t g_message_buffer;


static inline int8_t iwrap(int8_t x)
{
    if (x >= MAX_MSG_BUF_LEN) {
        x -= MAX_MSG_BUF_LEN;
    }
    return x;
}


void loop_init(void)
{
    g_message_buffer.wrpos = 0;
    g_message_buffer.rdpos = 0;
}

message_t *loop_put_msg_begin(void)
{
    int8_t next_wp = iwrap(g_message_buffer.wrpos+1);
    if (next_wp == g_message_buffer.rdpos) {
        /* Buffer full! */
        usart_sendstr_P(PSTR("[loop_put_msg_begin FULL]\n"));
        return NULL;
    }
    return &g_message_buffer.msg_buf[g_message_buffer.wrpos];
}

void loop_put_msg_end(message_t *msg)
{
    int8_t next_wp = iwrap(g_message_buffer.wrpos+1);
    g_message_buffer.wrpos = next_wp;
}

/**
 * Main message (infinite) loop.
 */
void loop_loop(int8_t once)
{
    do {
        message_t lmsg;
        if (g_message_buffer.rdpos == g_message_buffer.wrpos) {
            /* no message in the buffer: Run NOP message */
            lmsg.cmd = Cmd_Nop;
            lmsg.arg1i = 0;
        } else {
            /* a message in the buffer: retrieve and process */
            lmsg = g_message_buffer.msg_buf[g_message_buffer.rdpos];
            g_message_buffer.rdpos = iwrap(g_message_buffer.rdpos + 1);

            if (lmsg.cmd != Cmd_ClockTick) {
                usart_sendstr_P(PSTR("[Msg:0x"));
                usart_sendhexb(lmsg.cmd);
                usart_sendstr_P(PSTR("]"));
            }
        }

        int8_t used = 0;
        used |= voice_process(&lmsg);
        used |= vfs_process(&lmsg);
        used |= sound_process(&lmsg);
        used |= ir_process(&lmsg);
        used |= debug_process(&lmsg);
        used |= btn_process(&lmsg);
        used |= clock_process(&lmsg);

        if (!used && (lmsg.cmd != Cmd_Nop)) {
            usart_sendstr_P(PSTR("[Msg not recognized!]\n"));
        }

        /* reset watchdog timer */
        wdt_reset();
    } while (!once);
}
