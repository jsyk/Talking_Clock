#include "loop.h"
#include "clock.h"
#include <avr/io.h>

#define PIN_BTN1        (PIND & 0x10)
#define PIN_BTN2        (PIND & 0x20)
#define PIN_BTN3        (PIND & 0x40)
#define PIN_BTN4        (PIND & 0x80)
#define BTNBITS_MASK    0xF0


static uint8_t g_btn_cnt[4];
static uint8_t g_btn_states = 0;


void btn_init()
{
    /* buttons are inputs */
    DDRD &= ~BTNBITS_MASK;
    /* enable Pull-Up for button pins */
    PORTD |= BTNBITS_MASK;
}


static void btn_check(uint8_t pin, uint8_t idx)
{
    const uint8_t mask = 1 << idx;

    if (pin) {
        /* decrement */
        if (g_btn_cnt[idx] > 0) --g_btn_cnt[idx];
    } else {
        /* increment */
        if (g_btn_cnt[idx] < 15) ++g_btn_cnt[idx];
    }

    if ((g_btn_cnt[idx] == 0) && (g_btn_states & mask)) {
        g_btn_states &= ~mask;
        message_t *nmsg = loop_put_msg_begin();
        nmsg->cmd = Cmd_ButtonRelease;
        nmsg->arg1i = idx;
        loop_put_msg_end(nmsg);
    }

    if ((g_btn_cnt[idx] == 15) && !(g_btn_states & mask)) {
        g_btn_states |= mask;
        message_t *nmsg = loop_put_msg_begin();
        nmsg->cmd = Cmd_ButtonPress;
        nmsg->arg1i = idx;
        loop_put_msg_end(nmsg);
    }
}


int8_t btn_process(const message_t *msg)
{
    int8_t msg_recognized = 1;

    switch (msg->cmd) {
        case Cmd_ClockTick:
            btn_check(PIN_BTN1, 0);
            btn_check(PIN_BTN2, 1);
            btn_check(PIN_BTN3, 2);
            btn_check(PIN_BTN4, 3);
            break;

        default:
            msg_recognized = 0;
            break;
    }


    return msg_recognized;
}
