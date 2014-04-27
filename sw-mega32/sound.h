#ifndef SOUND_H
#define SOUND_H

#include <avr/io.h>
#include "tff.h"
#include "loop.h"

/* size of one sound frame buffer (there are two of them for double-buffering) */
// #define SNDBUF_SIZE         128          /* some underflows at 16MHz/22kHz */
#define SNDBUF_SIZE         256             /* no underflows on 22kHz observed */

typedef enum {
    SP_NoPlay,
    SP_Draining,
    SP_Running,
} sound_play_state_t;


extern volatile sound_play_state_t gv_sound_playing;
extern uint8_t g_sound_buf[2][SNDBUF_SIZE];


void sound_init(void);

/* This should be called with 22kHz frequency when the sound is playing (sound_playing) */
void sound_isr_play(void);

void sound_play_file(FIL *file);

int8_t sound_process(const message_t *msg);

#endif
