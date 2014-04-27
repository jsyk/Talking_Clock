#include "clock.h"
#include "diskio.h"
#include <avr/interrupt.h>


uint16_t tick_diff(uint16_t t2, uint16_t t1)
{
    if (t2 >= t1)
        return t2 - t1;
    else
        return 65535 - (t1 - t2);
}

void clock_tick(clockinfo_t* clk, uint8_t dms128)
{
    clk->msec128 += dms128;
    if (clk->msec128 >= 128) {
        clk->msec128 -= 128;
        if (++clk->sec >= 60) {
            clk->sec -= 60;
            if (++clk->min >= 60) {
                clk->min -= 60;
                if (++clk->hour >= 24) {
                    clk->hour -= 24;
                }
            }
        }
    }
}

/* Timer2 : 128 Hz */
ISR(TIMER2_OVF_vect)            /* on overflow */
{
#if 1
    ++current_cpu_tick;

    //note: discard volatile, because we're in the interrupt context
    //and nobody else can change the variable.
    clock_tick((clockinfo_t*)&gv_clock_main, 1);

    /* MMC irq routine: Should be called every 10ms (100Hz), hopefully it won't matter
     * we call it 28% more often. */
    disk_timerproc();

#endif
#if 0
    if (display_state == DS_ISR_MAIN_TIME) {
        show_clock_time(&gv_clock_main);
    }

    if (disp_en_irq) {
        /* display write */
        isr_display_write();
    }
#endif
}


int8_t clock_process(const message_t *msg)
{
    int8_t msg_recognized = 1;
    static uint8_t last_tick = 0;

    switch (msg->cmd) {
        case Cmd_ClockTick:
            msg_recognized = 1;
            break;

        default:
            msg_recognized = 0;
            break;
    }

    if (last_tick != (current_cpu_tick & 0xFF)) {
        last_tick = current_cpu_tick;
        message_t *nmsg = loop_put_msg_begin();
        nmsg->cmd = Cmd_ClockTick;
        nmsg->arg1i = 1;
        loop_put_msg_end(nmsg);
    }



    return msg_recognized;
}
