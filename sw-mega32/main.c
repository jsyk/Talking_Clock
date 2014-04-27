#include "setting.h"

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdlib.h>

#include "clock.h"
#include "serial.h"
#include "nixies.h"
#include "dcf.h"
#include "diskio.h"
#include "sound.h"
#include "loop.h"
#include "vfs.h"
#include "voice.h"
#include "debug.h"
#include "ir.h"
#include "btn.h"

#if 0
void cekej()
{
    int i = 1 << 15;
    while (--i)
        ;
}
#endif


#if F_CPU == 16000000L
#if SPIRATE == 250000
#define SPI_RATE_SPR    2
#define SPI_RATE_2X     0
#endif
#endif


#if F_CPU == 8000000L
#if SPIRATE == 250000
#define SPI_RATE_SPR    2
#define SPI_RATE_2X     1
#endif
#endif




/** global variables */
// extern uint8_t disp_buf[6][3];      /* FIXME: should be static in nixies.c */

volatile uint16_t current_cpu_tick = 0;


#define DS_KEEP             0
#define DS_ISR_MAIN_TIME    1

uint8_t display_state = DS_ISR_MAIN_TIME;


/* main clock, run by CPU xtal */
volatile clockinfo_t gv_clock_main = ZERO_CLOCK;       /* current running clock */
/* dcf clock */
clockinfo_t g_clock_dcf = ZERO_CLOCK;                 /* current being decoded, possibly incorrect */
clockinfo_t g_clock_dcf_last_good = ZERO_CLOCK;       /* last well-decoded dcf clock */


#if 0
uint8_t nixies_off_until_en = 0;
uint16_t nixies_off_until_tick = 0;
#endif


const uint8_t PROGMEM sin_22[22] = {
    127,   161,   193,   220,   240,   251,   254,   247,   231,   207,   178,   144,   110,    76,    
    47,    23,     7,     0,     3,    14,    34,    61 //,    93
};


/* ----------------------------------------------------- */


void show_clock_time(clockinfo_t* clk)
{
    uint8_t c_hour = clk->hour, c_min = clk->min, c_sec = clk->sec;
    uint8_t leds = 0;

    if (dcf_dec_insync) {
        if (nixies_master_mute) {
            int i;
            for (i = 0; i < dcf_dec_index / 10; ++i) {
                leds = (leds << 1) | 1;
            }
            leds <<= 1;
        }
        leds |= (dcf_dec_insync ? 1 : 0);
    }

    /* show hours */
    set_cathode_leds(0, c_hour / 10, 1, leds);
    set_cathode_leds(1, c_hour % 10, 0, leds);
    /* show minutes */
    set_cathode_leds(2, c_min / 10, 1, leds);
    set_cathode_leds(3, c_min % 10, 0, leds);
    /* show minutes */
    set_cathode_leds(4, c_sec / 10, 1, leds);
    set_cathode_leds(5, c_sec % 10, 0, leds);
}


/* ----------------------------------------------------- */
/*XXX Timer0: 1000 Hz */

#define TIM0_DIV_TO_1000        SOUND_SAMPLERATE_KHZ


/* Timer0: 22 kHz */
ISR(TIMER0_COMP_vect)
{
    static int8_t tim0_tick = TIM0_DIV_TO_1000;
#if 0
    ++current_cpu_tick;

    uint16_t d = tick_diff(current_cpu_tick, clock_last_10ms_tick);
    if (d >= 10) {
        clock_tick(&gv_clock_main, d/10);
        clock_last_10ms_tick = current_cpu_tick - (d % 10);
    }
#endif

    /* Sound */
    if (gv_sound_playing) {
        sound_isr_play();
    }

    /* Other */
    if (tim0_tick == 0) {
        /* here at 1000 Hz only */
    #if 0
        /* test Buttons */
        if (!PIN_BTN1) {
            /* Button 0 is pressed */
            nixies_off_until_en = 1;
            nixies_off_until_tick = current_cpu_tick + MS2TICK(5000*60);
            nixies_master_mute = 1;
            dcf_dec_success = 0;
        }

        if (!PIN_BTN2) {
            /* Button 1 is pressed */
            nixies_off_until_en = 0;
            nixies_master_mute = 0;
            dcf_dec_success = 0;
        }

        if (nixies_off_until_en && (nixies_off_until_tick == current_cpu_tick || dcf_dec_success)) {
            nixies_off_until_en = 0;
            nixies_master_mute = 0;
            dcf_dec_success = 0;
        }
    #endif

    #if 1
        if (display_state == DS_ISR_MAIN_TIME) {
            show_clock_time(&gv_clock_main);
        }

        if (disp_en_irq /*&& !nixies_off_until_en*/) {
            /* display write */
            isr_display_write();
        }
    #endif

        tim0_tick = TIM0_DIV_TO_1000;
    } else {
        --tim0_tick;
    }
}

/* ----------------------------------------------------- */

void print_clock_usart(clockinfo_t *clk)
{
    usart_sendhexb(clk->day);
    usart_transmit('.');
    usart_sendhexb(clk->month);
    usart_transmit('.');
    usart_sendhexb(clk->year);
    usart_transmit(' ');
    usart_sendhexb(clk->weekday);
    
    usart_transmit(' ');

    usart_sendhexb(clk->hour);
    usart_transmit(':');
    usart_sendhexb(clk->min);
    usart_transmit(':');
    usart_sendhexb(clk->sec);
    usart_transmit('.');
    usart_sendhexb(clk->msec128);
}


int main(void)
{
    // wdt_disable();              /* turn off the watchdog for this test; FIXME !! */
    wdt_enable(WDTO_500MS);

    usart_init();

    /* PORTC = Nixies */
    init_nixie_ports();

    btn_init();

    sound_init();


    /* PORTB = SD card using SPI */
    PORTB = 0b10110000;                 /* Enable drivers */
    DDRB  = 0b10110000;
    /* SPE = Enable; MSTR = Master select; SPR0 = SPI Clock rate */
    /* SPR0: 2 = f/64, 6 = f/32 */
    SPCR = (1<<SPE) | (1<<MSTR) | (SPI_RATE_SPR << SPR0);
    SPSR = (SPI_RATE_2X << SPI2X);


#if SOUND_SAMPLERATE_KHZ == 22
    /* Timer0: 22000 Hz */
#if F_CPU == 16000000L              /* 16MHz CPU clock */
    TCCR0 = (1 << WGM01) | (2 << CS00); /* CTC mode, prescaller 8 */
    OCR0 = 91;
#elif F_CPU == 8000000L             /* 8MHz CPU clock */
    TCCR0 = (1 << WGM01) | (2 << CS00); /* CTC mode, prescaller 8 */
    OCR0 = 46;
#else
    #error No constants for this F_CPU 
#endif
#endif

#if SOUND_SAMPLERATE_KHZ == 8
    /* Timer0: 8000 Hz */
#if F_CPU == 16000000L              /* 16MHz CPU clock */
    TCCR0 = (1 << WGM01) | (2 << CS00); /* CTC mode, prescaller 8 */
    OCR0 = 249;
#elif F_CPU == 8000000L             /* 8MHz CPU clock */
    TCCR0 = (1 << WGM01) | (2 << CS00); /* CTC mode, prescaller 8 */
    OCR0 = 124;
#else
    #error No constants for this F_CPU 
#endif
#endif

#if 0
    /* Timer0: 1000 Hz */
#if F_CPU == 16000000L              /* 16MHz CPU clock */
    TCCR0 = (1 << WGM01) | (3 << CS00); /* CTC mode, prescaller 64 */
    OCR0 = 250;
#elif F_CPU == 1000000L             /* 1MHz CPU clock */
    TCCR0 = (1 << WGM01) | (2 << CS00); /* CTC mode, prescaller 8 */
    OCR0 = 125;
#elif F_CPU == 8000000L             /* 8MHz CPU clock */
    TCCR0 = (1 << WGM01) | (3 << CS00); /* CTC mode, prescaller 64 */
    OCR0 = 125;
#else
    #error No constants for this F_CPU 
#endif
#endif


    /* Timer/Counter 2 in Asychronous mode, with external 32kHz Xtal */
#if 1
    ASSR = (1 << AS2);              /* enable async mode */
    // TCCR2 = (1 << WGM21) | (0b001 << CS20);        /* CTC mode, irq on COMP; clock source=clk_T2s */
    TCCR2 = (0b001 << CS20);        /* Normal mode, irq on OVF; Clock source=clk_T2s */
    TCNT2 = 0;                      /* inital counter value; not important */
    OCR2 = 255;                     /* not used on Normal mode */
    // OCR2 = 63;                     /* not used */
    /* wait until new values have been synced in hw */
    while (ASSR & 0x07)
        ;   /* wait */
#endif

    /* Timer/Counter Interrup mask register: */
    /* enable irq on OCR0 match; Enable irq in Timer2 overflow */
    // TIMSK = (1 << OCIE0) | (1 << OCIE1A);
    TIMSK = (1 << OCIE0) | (1 << TOIE2) /*| (1 << OCIE2)*/;

    /* MCUCR: MCU Control
     *  Any logical change on INT1 generates an interrupt request. [for DCF]
     *  Any logical change on INT0 generates an interrupt request. [for IR]
     */
    MCUCR = (1 << ISC10) | (1 << ISC00);

    usart_sendstr_P(PSTR("\nMCUCSR: ")); usart_sendhexb(MCUCSR);
    
    /* MCUCSR: MCU Control and status register
     * XXX ISC2=1 : External Interrupt 2 - activate on rising edge;
     * JTD=1 : Disable JTAG (! MUST WRITE TWICE !) */
    // MCUCSR = (1 << JTD) | (1 << ISC2); MCUCSR = (1 << JTD) | (1 << ISC2);
    MCUCSR = (1 << JTD); MCUCSR = (1 << JTD);
    

    /* GICR: General Interrupt Control Register
     * INT1=1 : Enable Ext.IRQ 1 (input signal DCF77) 
     * INT0=1 : Enable Ext.IRQ 1 (input signal IR receiver)
     */
    GICR = (1 << INT1) | (1 << INT0);
    
    /* rozsviti 0 na NIX0 */
    set_cathode(0, 0, 1);
    /* rozsviti 0 na NIX1 */
    set_cathode(1, 1, 1);
    /* rozsviti 0 na NIX2 */
    set_cathode(2, 2, 1);
    /* rozsviti 0 na NIX3 */
    set_cathode(3, 3, 1);
    /* rozsviti 0 na NIX4 */
    set_cathode(4, 4, 1);
    /* rozsviti 0 na NIX5 */
    set_cathode(5, 5, 1);
    
    disp_en_irq = 0xFF;

    gv_clock_main.msec128 = 0;
    gv_clock_main.sec = 0;
    gv_clock_main.min = 0;
    gv_clock_main.hour = 12;
    
    sei();
    
    loop_init();
    ir_init();
    voice_init();
    vfs_init();
    debug_init();

    loop_loop(0);
    
#if 0
    while (1) {
        usart_sendstr_P(PSTR("\nCmd? "));
        value = usart_receive();
        switch (value) {
            case 'n': {
                char val_str[10];
                usart_sendstr_P(PSTR(" / nixie num="));
                usart_getline(val_str, 10);
                uint8_t nix = hstr_to_ulong(val_str);
                usart_sendstr_P(PSTR("Symbol num [hex]="));
                usart_getline(val_str, 10);
                uint8_t symb = hstr_to_ulong(val_str);
                
                if (!(nix >= 0 && (nix <= 5 || nix == 0xA))) {
                    usart_sendstr_P(PSTR("Nixie out of range (0-5, A)\n"));
                    break;
                }
                if (symb < 0 || symb > 0x0B) {
                    usart_sendstr_P(PSTR("Symbol out of range (0-9, A, B)\n"));
                    break;
                }
                
                uint16_t cath = 0;
                if (symb < 0x0B) {
                    cath = (1 << symb);
                }
                
                disp_en_irq = 0;
                if (nix == 0xA) {
                    for (nix = 0; nix < 6; nix++) {
                        disp_buf[nix][1] = ((cath << 3) & 0xFF);
                        disp_buf[nix][2] = ((cath >> 4) & 0xFF);
                    }
                } else {
                    disp_buf[nix][1] = ((cath << 3) & 0xFF);
                    disp_buf[nix][2] = ((cath >> 4) & 0xFF);
                }
                disp_en_irq = 0xFF;
                
                usart_sendstr_P(PSTR("Cathode pattern "));
                usart_sendhexl(cath);
                usart_sendstr_P(PSTR("\n"));
                
                break;
            }
        #if 0
            case 'b': {
                nixie_counting();
                break;
            }
        #endif
            case 'h': {         //set clock
                char val_str[10];
                usart_sendstr_P(str_SetClock_EnterHour);
                usart_getline(val_str, 10);
                gv_clock_main.msec128 = 0;
                gv_clock_main.sec = 0;
                gv_clock_main.hour = hstr_to_ulong(val_str);
                usart_sendstr_P(str_EnterMinute);
                usart_getline(val_str, 10);
                gv_clock_main.min = hstr_to_ulong(val_str);
                usart_sendstr_P(str_ClockSet);
                //clock_atom = gv_clock_main;
                break;
            }
#if 0
            case 'k': {
                usart_sendstr_P(str_DisplayingClock);
                nixie_clock(&gv_clock_main);
                break;
            }
            case 'K': {
                usart_sendstr_P(str_DisplayingClock);
                nixie_clock(&clock_atom);
                break;
            }
#endif
            case 'd': {     /* DCF measure */
                uint16_t prev_done = dcf_measure_done;
                while (!usart_rx_ischar()) {
                    if (prev_done != dcf_measure_done) {
                        cli();
                        prev_done = dcf_measure_done;
                        uint16_t width = dcf_measure_width;
                        uint8_t polarity = dcf_measure_polarity;
                        sei();

#if 0
                        if (polarity) {
                            usart_sendstr_P(PSTR("H "));
                        } else {
                            usart_sendstr_P(PSTR("L "));
                        }
                        usart_sendhexl(width);
#endif
                        char decoded = decode_dcf_measurement(polarity, width);

                        if (decoded != ' ') {
                            if (decoded == '?') {
                                /* ERROR */
                                usart_transmit('(');
                                if (polarity) {
                                    usart_sendstr_P(PSTR("H "));
                                } else {
                                    usart_sendstr_P(PSTR("L "));
                                }
                                usart_sendhexl(width);
                                usart_transmit(')');
                            }

                            usart_transmit(decoded);
                            usart_transmit(',');
                            usart_sendhexb(dcf_dec_insync);
                            usart_transmit(',');
                            usart_sendhexb(dcf_dec_index);
                            usart_transmit('|');

                            if (decoded == 'M' || decoded == '?')
                                usart_send_nl();
                        }
                        
                    }

                    _delay_ms(20.0);
                }
                /* quit */
                usart_sendstr_P(PSTR("Quit!\n"));
                break;
            }
            case 'c': {
                /* print clocks */
                usart_sendstr_P(PSTR("\nMain clock:  "));      print_clock_usart(&gv_clock_main);
                usart_sendstr_P(PSTR("\nDCF good:    "));      print_clock_usart(&clock_dcf_last_good);
                usart_sendstr_P(PSTR("\nDCF current: "));      print_clock_usart(&clock_dcf);
                usart_sendstr_P(PSTR("\nMCUCSR: "));           usart_sendhexb(MCUCSR);
                // usart_sendstr_P(PSTR("\nCanaries: "));
                // usart_sendhexl(canary_1); usart_transmit(','); usart_sendhexl(canary_2); usart_transmit(','); usart_sendhexl(canary_3); usart_transmit(',');
                // usart_sendhexl(canary_4); usart_transmit(','); usart_sendhexl(canary_5); usart_transmit(','); usart_sendhexl(canary_6);
                usart_send_nl();
                break;
            }
            case 'i': {
                /* measure IR */
                uint16_t prev_done = ir_measure_done;
                uint64_t cmdcode = 0;
                uint8_t cmdlen = 0;
                while (!usart_rx_ischar()) {
                    if (prev_done != ir_measure_done) {
                        cli();
                        prev_done = ir_measure_done;
                        uint16_t width = ir_measure_width;
                        uint8_t polarity = ir_measure_polarity;
                        sei();

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

                        usart_transmit(decoded);

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
                                usart_transmit('=');
                                usart_sendhexl(cmdcode >> 32); usart_sendhexl(cmdcode);
                                usart_transmit('/');
                                usart_sendhexb(cmdlen);
                                // usart_send_nl();
                                // break;
                            }
                            default: {
                                cmdcode = 0;
                                cmdlen = 0;
                                break;
                            }
                        }
                    #endif

                        if (decoded == '?') {
                            if (polarity) {
                                usart_sendstr_P(PSTR("H "));
                            } else {
                                usart_sendstr_P(PSTR("L "));
                            }
                            usart_sendhexl(width);
                            usart_transmit('|');
                        }
                        if (decoded == ' ' || decoded == '?') {
                            usart_send_nl();
                        }

                    #if 0
                    #endif
                    }
                }
                break;
            }

#if 1
            case 'I': {
                sdcard_slow_mode();
                usart_sendstr_P(PSTR(" / disk init...\n"));
                unsigned char res = disk_initialize(0);
                usart_sendstr_P(PSTR("result of disk init = 0x"));
                usart_sendhexb(res);
                usart_send_nl();
                if (res == 0) {
                    /* fast mode is needed to sustain the demands of sound-play */
                    sdcard_fast_mode();
                    usart_sendstr_P(PSTR("SPI fast mode set.\n"));
                } else {
                    usart_sendstr_P(PSTR("WARNING: SPI fast mode NOT set.\n"));
                }
                break;
            }

            case 'S': {
                usart_sendstr_P(PSTR(" / disk status...\n"));
                unsigned char res = disk_status (0);
                usart_sendstr_P(PSTR("result of disk status = 0x"));
                usart_sendhexb(res);
                usart_send_nl();
                break;
            }

#if 0
            case 'G': {     // disk get sector
                char lba_str[30];
                int i;
                for (i = 0; i < SBUF_SIZE; i++) sbuf[i] = 0;
                
                usart_sendstr_P(PSTR(" / disk read: Enter LBA in hex = "));
                usart_getline(lba_str, 30);
                DWORD sector = hstr_to_ulong(lba_str);
                usart_sendstr_P(PSTR("\nLBA=0x"));
                usart_sendhexl(sector);
                
                DRESULT res = disk_read(0, (BYTE*)sbuf, sector, 1);
                usart_sendstr_P(PSTR("\nresult of disk read = 0x"));
                usart_sendhexl(res);
                usart_send_nl();
                
                if (res == RES_OK) {        // ok
                    for (i = 0; i < SBUF_SIZE; i++) {
                        usart_sendhexb(sbuf[i]);
                        if ((i & 0x0F) == 0x0F) {
                            usart_send_nl();
                        }
                    }
                }
                usart_send_nl();
                
                break;
            }
#endif
            case 'M': {     //mount
                usart_sendstr_P(PSTR(" / fat mount...\n"));
                FRESULT res = f_mount(0, &g_fatfs);
                usart_sendstr_P(PSTR("result fat mount = 0x"));
                usart_sendhexb((unsigned char)res);
                usart_send_nl();
                break;
            }

            case 'D': {     //opendir
                #define SBUF_SIZE   (2*SNDBUF_SIZE)
                char *sbuf = (char*)g_sound_buf;
                usart_sendstr_P(PSTR(" / opendir: Enter dir name = "));
                usart_getline(sbuf, SBUF_SIZE);
                usart_sendstr_P(PSTR("\nDir name: '"));
                usart_sendstr(sbuf);
                usart_sendstr_P(PSTR("'\n"));
                
                FRESULT res = f_opendir(&g_cwd, sbuf);
                usart_sendstr_P(PSTR("Result of opendir: 0x"));
                usart_sendhexb((unsigned char)res);
                usart_send_nl();
                break;
            }

            case 'L': {     // readdir (listing)
                usart_sendstr_P(PSTR(" / Directory listing:\n"));
                FILINFO finfo;
                while ((f_readdir(&g_cwd, &finfo) == FR_OK) && finfo.fname[0]) {
                    if (finfo.fattrib & AM_DIR) {
                        usart_sendstr_P(PSTR("  /"));
                    } else {
                        usart_sendstr_P(PSTR("   "));
                    }
                    usart_sendstr(finfo.fname);
                    usart_sendstr_P(PSTR("        S=0x"));
                    usart_sendhexl(finfo.fsize);
                    usart_sendstr_P(PSTR("  A=0x"));
                    usart_sendhexb(finfo.fattrib);
                    usart_sendstr_P(PSTR("  D=0x"));
                    usart_sendhexl(finfo.fdate);
                    usart_sendstr_P(PSTR("  T=0x"));
                    usart_sendhexl(finfo.ftime);
                    usart_send_nl();
                }
                usart_sendstr_P(PSTR("EOF\n"));
                g_cwd.index = 0;
                break;
            }

            case 'O': {     // file open
                char *sbuf = (char*)g_sound_buf;
                usart_sendstr_P(PSTR(" / file open: Enter abs. file name = "));
                usart_getline(sbuf, SBUF_SIZE);
                usart_sendstr_P(PSTR("\nFile name: '"));
                usart_sendstr(sbuf);
                usart_sendstr_P(PSTR("'\n"));
                
                FRESULT res = f_open(&g_file, sbuf, FA_READ|FA_OPEN_EXISTING);
                usart_sendstr_P(PSTR("Result of f_open: 0x"));
                usart_sendhexb((unsigned char)res);
                usart_send_nl();
                break;
            }

            case 'C': {     // file close
                usart_sendstr_P(PSTR(" / file close\n"));
                FRESULT res = f_close(&g_file);
                usart_sendstr_P(PSTR("Result of f_close: 0x"));
                usart_sendhexb((unsigned char)res);
                usart_send_nl();
                break;
            }

            case 'R': {     // file read
                int i;
                uint16_t cnt;
                char *sbuf = (char*)g_sound_buf;
                for (i = 0; i < SBUF_SIZE; i++) sbuf[i] = 0;
                
                usart_sendstr_P(PSTR(" / file read\n"));
                
                cnt = 0;
                FRESULT res = f_read(&g_file, sbuf, SBUF_SIZE, &cnt);
                usart_sendstr_P(PSTR("\nresult of file read = 0x"));
                usart_sendhexl(res);
                usart_sendstr_P(PSTR("\nRead bytes count = 0x"));
                usart_sendhexl(cnt);
                usart_send_nl();
                
                if (res == FR_OK) {        // ok
                    for (i = 0; i < cnt; i++) {
                        usart_sendhexb(sbuf[i]);
                        if ((i & 0x0F) == 0x0F)
                            usart_send_nl();
                    }
                }
                usart_send_nl();
                break;
            }

            case 'P': {     // file play
                usart_sendstr_P(PSTR(" / file play\n"));
                sound_play_file(&g_file);
                break;
            }
#endif
        }
    }
#endif

    return 0;
}
