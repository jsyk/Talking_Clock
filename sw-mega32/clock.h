#ifndef CLOCK_H
#define CLOCK_H

#include <inttypes.h>
#include "loop.h"


/** clock global variables */
typedef struct {
    uint8_t msec128;     /* sec / 8 */
    uint8_t sec;
    uint8_t min;
    uint8_t hour;
    //
    uint8_t day;
    uint8_t weekday;
    uint8_t month;
    uint8_t year;
} clockinfo_t;

#define ZERO_CLOCK      { 0, 0, 0, 0 }


extern volatile uint16_t current_cpu_tick;

/* main clock, run by CPU xtal */
extern volatile clockinfo_t gv_clock_main;       /* current running clock */
/* dcf clock */
extern clockinfo_t g_clock_dcf;                 /* current clock being decoded, possibly incorrect */
extern clockinfo_t g_clock_dcf_last_good;       /* last well-decoded dcf clock */


/* 128Hz ticks */
#define MS2TICK(ms)                 ((int)( (long)(ms) / (1000/128)))


uint16_t tick_diff(uint16_t t2, uint16_t t1);

int8_t clock_process(const message_t *msg);

#endif
