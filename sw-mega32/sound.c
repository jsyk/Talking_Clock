#include "sound.h"
#include "setting.h"
#include "serial.h"
#include "nixies.h"
#include "tff.h"
#include "clock.h"
#include <avr/pgmspace.h>
#include <stdlib.h>

/** voice global variables */

/* Is currently sound playing? */
volatile sound_play_state_t gv_sound_playing = SP_NoPlay;
/* Sample double-frame buffer */
uint8_t g_sound_buf[2][SNDBUF_SIZE];
/* Current playing frame number; range [0; 1] */
uint8_t g_sound_bframe = 0;
/* Current playing index within the frame; range [0; SNDBUF_SIZE) */
uint8_t g_sound_bindex = 0;
/* Which buffers contain valid (yet unplayed) sound samples */
volatile uint8_t gv_sound_fvalid[2] = { 0, 0 };

/* Number of buffer underflows experienced */
volatile int gv_sound_underflows = 0;

/* Current fetching frame number; range [0; 1] */
uint8_t g_sound_fframe = 0;           
/* Current playing file */
FIL *g_sound_file = NULL;

/* The time (cpu tick) when the play has stopped. Used for auto-mute. */
uint16_t g_sound_stopped_time = 0;

// uint8_t sound_volume = 0;       // volume control


// #define SBUF_SIZE   (2*SNDBUF_SIZE)
// char sbuf[SBUF_SIZE];
// char *sbuf = (char*)sound_buf;

const unsigned char sinetab[16] PROGMEM = { 
    128, 177, 218, 245, 255, 245, 218, 177, 128, 79, 38, 11,  1, 11, 38, 79 
};


#if 0
/* write sound sample to the DAC hardware */
static inline void write_dac(uint8_t sample)
{
    PORTA = sample;
}

void sound_init(void)
{
    /* PORTA = D/A for sound */
    PORTA = 0;
    DDRA = 0xFF;        /* all outputs */

    /* PORTD.7 = XMUTE output, default 0 (sound muted) */
    DDRD |= (1 << 7);
    PORTD &= ~(1 << 7);
}
#endif

#if 1

/* DAC interface */
#define PORT_DAC        PORTA
#define DDR_DAC         DDRA
#define DAC_MASK        0x07
#define BIT_SRDATA      0
#define BIT_SRCLK       1
#define BIT_XACS        2

/* MUTE interface */
#define PORT_XMUTE      PORTA
#define DDR_XMUTE       DDRA
#define BIT_XMUTE       3

#if 0
#define PORT_XMUTE      PORTD
#define DDR_XMUTE       DDRD
#define BIT_XMUTE       7
#endif

static void write_shiftreg_msb(uint8_t sample)  
{
    /* Zero out all ctrl signal. XACS goes low or stays low, 
     * starting a new transfer in the first case. */
    register uint8_t prt = (PORT_DAC & ~DAC_MASK);
    PORT_DAC = prt;

    __asm__ volatile (
            //"eor __tmp_reg__, __tmp_reg__"  "\n\t"
            "mov __tmp_reg__, %4"     "\n\t"

        /* macro 'subprogram', 5c */
#define SHIFTOUT_BIT(bnum)              \
            "bst %0, " bnum             "\n\t"          /* 1c fetch data bit 'bnum' to T */ \
            "bld __tmp_reg__, %2"       "\n\t"          /* 1c transfer from T to BIT_SRDATA */ \
            "out %1, __tmp_reg__"       "\n\t"          /* 1c write data to port; falling edge on clk */ \
            "sbi %1, %3"                "\n\t"          /* 2c rising edge on clk */
        
        /* bits 7:0, MSB first */
        SHIFTOUT_BIT("7")
        SHIFTOUT_BIT("6")
        SHIFTOUT_BIT("5")
        SHIFTOUT_BIT("4")
        SHIFTOUT_BIT("3")
        SHIFTOUT_BIT("2")
        SHIFTOUT_BIT("1")
        SHIFTOUT_BIT("0")
        
        : /* outputs*/
          "=r" (sample)
        : /* inputs */
          "I" (_SFR_IO_ADDR(PORT_DAC)),    // %1
          "I" (BIT_SRDATA),             // %2
          "I" (BIT_SRCLK),              // %3
          "r" (prt),                    // %4
          "0" (sample)          // %0
    );
#undef SHIFTOUT_BIT

    /* zero out all ctrl signal */
    PORT_DAC = prt;
}


/* write sound sample to the DAC hardware */
static inline void write_dac8(uint8_t sample)
{
#if 1
    write_shiftreg_msb(0b00110000 | ((sample >> 4) & 0x0F));
    write_shiftreg_msb((sample << 4));
#endif
#if 0
    write_shiftreg_msb(0b00110000);
    write_shiftreg_msb((sample));
#endif

    /* set XACS to HIGH (inactive), others zero. This properly terminates the transfer. */
    PORT_DAC = (PORT_DAC & ~DAC_MASK) | (1 << BIT_XACS);
}

/* write sound sample to the DAC hardware */
static inline void write_dac16(uint16_t sample)
{
    /* 16b -> 12b */
    sample >>= 4;
    write_shiftreg_msb(0b00110000 | ((sample >> 8) & 0x0F));
    write_shiftreg_msb((sample & 0xFF));

    /* set XACS to HIGH (inactive), others zero. This properly terminates the transfer. */
    PORT_DAC = (PORT_DAC & ~DAC_MASK) | (1 << BIT_XACS);
}


void sound_init(void)
{
    /* PORTA = DAC */
    PORT_DAC |= (1 << BIT_XACS);        /* set XACS to HIGH (inactive), others zero */
    DDR_DAC |= 0x07;        /* low 3 bits outputs */

    /* PORTD.7 = XMUTE output, default 0 (sound muted) */
    DDR_XMUTE |= (1 << BIT_XMUTE);
    PORT_XMUTE &= ~(1 << BIT_XMUTE);
}
#endif

#define BYTES_PER_SAMPLE        (SOUND_BITS_PER_SAMPLE/8)

/* This should be called with 22kHz frequency when the sound is playing (sound_playing) */
void sound_isr_play(void)
{
    /* is the current frame valid? */
    if (gv_sound_fvalid[g_sound_bframe]) {
        /* play a sample */
    #if SOUND_BITS_PER_SAMPLE == 8
        /* 8b */
        write_dac8(g_sound_buf[g_sound_bframe][g_sound_bindex]);
    #endif
    #if SOUND_BITS_PER_SAMPLE == 16
        /* 16b, little endian */
        uint16_t sample = g_sound_buf[g_sound_bframe][g_sound_bindex];
        write_dac16( sample | ((uint16_t)g_sound_buf[g_sound_bframe][g_sound_bindex+1] << 8));
    #endif

        /* already on the last sample? */
        if (g_sound_bindex == SNDBUF_SIZE-BYTES_PER_SAMPLE) {
            /* frame completed */
            gv_sound_fvalid[g_sound_bframe] = 0;         /* current frame no longer valid */
            g_sound_bframe = ~g_sound_bframe & 0x01;    /* next frame mod 2 */
            g_sound_bindex = 0;                       /* first sample */
        } else {
            /* move to next sample;  */
            g_sound_bindex += BYTES_PER_SAMPLE;
        }
    } else {
        /* current frame is not valid - buffer underflow! */
        g_sound_bframe = ~g_sound_bframe & 0x01;    /* next frame mod 2 */
        g_sound_bindex = 0;           /* first sample */
        ++gv_sound_underflows;
    }
}

/* Unmute speaker */
static void amplifier_pwr_up(void)
{
    /* Set PC5 XMUTE to HIGH */
    PORT_XMUTE |= (1 << BIT_XMUTE);
}

/* Mute the speaker */
static void amplifier_pwr_down()
{
    /* Set PC5 XMUTE to LOW */
    PORT_XMUTE &= ~(1 << BIT_XMUTE);
}


#if 0
void sound_play_file(FIL *file)
{
    uint8_t cframe = 0;
    long numframe = 0;
    
    /* firts of all, stop all sound */
    gv_sound_playing = SP_NoPlay;
    // cekej();
    
    /* power up the amplifier */
    amplifier_pwr_up();
    
    /* init */
    g_sound_bindex = 0;           /* reset index in the playing frame */
    g_sound_bframe = 0;           /* reset playing frame */
    gv_sound_fvalid[0] = 0;        /* both frames are empty */
    gv_sound_fvalid[1] = 0;
    
    
    /* read the file */
    while (1) {
        if (!gv_sound_fvalid[cframe]) {
            /* Current local frame not valid, load it from the file.
             * Note that sound_fvalid[] is updated asynchronously in ISR.
             */
            int i;
            uint16_t cnt = 0;
            FRESULT res = f_read(file, g_sound_buf[cframe], SNDBUF_SIZE, &cnt);
            if (res) {
                /* read error! */
                usart_sendstr_P(PSTR("Reading error!\n"));
                usart_sendstr_P(PSTR("result of file read = 0x"));
                usart_sendhexl(res);
                usart_send_nl();
                break;
            }
            if (cnt == 0) {
                /* eof */
                usart_sendstr_P(PSTR("EOF.\n"));
                break;
            }
            
            /* if read less than SNDBUF_SIZE, fillup by quiet */
            for (i = cnt; i < SNDBUF_SIZE; i++) {
                g_sound_buf[cframe][i] = 0x80;
            }
            
            gv_sound_fvalid[cframe] = 0xFF;
            gv_sound_playing = SP_Running;
            
            // print volume
            // usart_sendhexb(sound_volume);
            
        #if 0
            /* Compute average sound level in the buffer */
            cnt = 0;
            for (i = 0; i < SNDBUF_SIZE; i++) {
                cnt += abs(((int)sound_buf[cframe][i]) - 0x80);
            }
            cnt /= 2*SNDBUF_SIZE;       /* [0; 32] */

            /* Print the sound level as a bar to UART */
            usart_transmit('[');
            for (i = 0; i < 32; i++) {
                if (i < cnt)
                    usart_transmit('=');
                else
                    usart_transmit(' ');
            }   
            usart_sendstr("]\r");   // only carrige-return
            
            /* leds indicate the sound level */
            uint8_t leds = 0x00;
            for (i = 0; i < 6; i++) {
                if (cnt > 5*i) {
                    leds |= (1 << (6-i));
                }
                set_cathode_leds(i, i, 1, leds);
            }
        #endif
            
            /* go to the next frame */
            cframe = ~cframe & 0x01;
            
            /* On occasions, print playing statistics to UART */
            if ((++numframe & 0x1FF) == 0) {
                usart_sendstr_P(PSTR("Fetched frames=0x"));
                usart_sendhexl(numframe);
                usart_sendstr_P(PSTR(", cframe="));
                usart_sendhexb(cframe);
                usart_sendstr_P(PSTR(", underflows=0x"));
                usart_sendhexl(gv_sound_underflows);
                usart_send_nl();
            }
        }
    
        /* Check if a stop is requested */
        if (usart_rx_ischar()) {
            uint8_t eof = 0;
            
            switch (usart_receive()) {
                // case '+': 
                //     set_volume(sound_volume + 1);
                //     break;
                // case '-':
                //     set_volume(sound_volume - 1);
                //     break;
                default: /* quit */
                    usart_sendstr_P(PSTR("Paused!\n"));
                    eof = 1;
                    break;
            }
            
            if (eof)
                break;
        }
    }
    
    /* wait till both buffers are empty */
    while (gv_sound_fvalid[0] || gv_sound_fvalid[1])
        ;
    
    gv_sound_playing = SP_NoPlay;
    amplifier_pwr_down();
}
#endif

int8_t sound_process(const message_t *msg)
{
    int8_t msg_recognized = 1;

    switch (msg->cmd) {
        case Cmd_PlayFileHandle: {
            g_sound_file = msg->arg1p;
            /* firts of all, stop all sound */
            gv_sound_playing = SP_NoPlay;
            
            /* power up the amplifier */
            amplifier_pwr_up();
            
            /* init */
            g_sound_bindex = 0;           /* reset index in the playing frame */
            g_sound_bframe = 0;           /* reset playing frame */
            gv_sound_fvalid[0] = 0;        /* both frames are empty */
            gv_sound_fvalid[1] = 0;

            gv_sound_playing = SP_Running;
            break;
        }
        default:
            msg_recognized = 0;
            break;
    }

    if (gv_sound_playing == SP_Running) {
        if (!gv_sound_fvalid[g_sound_fframe]) {
            /* Current local frame not valid, load it from the file.
             * Note that gv_sound_fvalid[] is updated asynchronously in ISR.
             */
            int i;
            uint16_t cnt = 0;
            FRESULT res = f_read(g_sound_file, g_sound_buf[g_sound_fframe], SNDBUF_SIZE, &cnt);
            if (res) {
                /* read error! */
                usart_sendstr_P(PSTR("Reading error!\n"));
                usart_sendstr_P(PSTR("result of file read = 0x"));
                usart_sendhexl(res);
                usart_send_nl();
                goto fin_play;
            }
            if (cnt == 0) {
                /* eof */
                usart_sendstr_P(PSTR("EOF.\n"));
                gv_sound_playing = SP_Draining;
                goto out;
            }
            
            /* if read less than SNDBUF_SIZE, fillup by quiet */
            for (i = cnt; i < SNDBUF_SIZE; i++) {
                g_sound_buf[g_sound_fframe][i] = 0x80;
            }
            
            gv_sound_fvalid[g_sound_fframe] = 0xFF;
            
            /* go to the next frame */
            g_sound_fframe = ~g_sound_fframe & 0x01;
        }
    }

    if (gv_sound_playing == SP_Draining) {
        /* wait till both buffers are empty */
        if (!gv_sound_fvalid[0] && !gv_sound_fvalid[1]) {
            /* Stop playing */
            gv_sound_playing = SP_NoPlay;
            g_sound_stopped_time = current_cpu_tick;
            goto fin_play;
        }
    }

    if (gv_sound_playing == SP_NoPlay) {
        /* mute after two seconds of no play */
        if (tick_diff(current_cpu_tick, g_sound_stopped_time) > MS2TICK(2000)) {
            amplifier_pwr_down();
        }
    }

out:
    return msg_recognized;

fin_play: {
    message_t *nmsg = loop_put_msg_begin();
    nmsg->cmd = Cmd_FinishedPlay;
    nmsg->arg1p = g_sound_file;
    loop_put_msg_end(nmsg);
    goto out;
  }
}
