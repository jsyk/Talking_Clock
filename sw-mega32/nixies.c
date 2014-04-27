#include "nixies.h"

/** display global variables */
volatile uint8_t disp_en_irq = 0;

static uint8_t disp_index = 0;
/*static*/ uint8_t disp_buf[6][3];

volatile uint8_t nixies_master_mute = 0;


#if 1
/* write the sample to a shift register, LSB first;
    PA7 = not connected or input
    PA6 = RCLK
    PA5 = SRCLK
    PA4 = SRDATA
    PA3-0 = not connected or inputs
*/

#define PORT_NIXIES     PORTA
#define DDR_NIXIES      DDRA
#define BIT_SRDATA      5
#define BIT_SRCLK       6
#define BIT_DISPCLK     7
#define BIT_AD          4
#else

#define PORT_NIXIES     PORTC
#define DDR_NIXIES      DDRC
#define BIT_SRDATA      0
#define BIT_SRCLK       1
#define BIT_DISPCLK     2

#endif

#define NIXBITS_MASK        ((1 << BIT_SRDATA) | (1 << BIT_SRCLK) | (1 << BIT_DISPCLK))

void init_nixie_ports()
{
    DDR_NIXIES |= NIXBITS_MASK;
    PORT_NIXIES &= ~NIXBITS_MASK;
}


static void write_shiftreg(uint8_t sample)
{
    /* zero out all ctrl signal */
    register uint8_t prt = (PORT_NIXIES & ~NIXBITS_MASK);
    PORT_NIXIES = prt;
    
    __asm__ volatile (
            // "eor __tmp_reg__, __tmp_reg__"  "\n\t"
            "mov __tmp_reg__, %4"     "\n\t"
        /* macro 'subprogram', 5c */
#define SHIFTOUT_BIT(bnum)              \
            "bst %0, " bnum             "\n\t"          /* 1c fetch data bit 'bnum' to T */ \
            "bld __tmp_reg__, %2"       "\n\t"          /* 1c transfer from T to BIT_SRDATA */ \
            "out %1, __tmp_reg__"       "\n\t"          /* 1c write data to port; falling edge on clk */ \
            "sbi %1, %3"                "\n\t"          /* 2c rising edge on clk */
        
        /* bits 0-7 */
        SHIFTOUT_BIT("0")
        SHIFTOUT_BIT("1")
        SHIFTOUT_BIT("2")
        SHIFTOUT_BIT("3")
        SHIFTOUT_BIT("4")
        SHIFTOUT_BIT("5")
        SHIFTOUT_BIT("6")
        SHIFTOUT_BIT("7")
        
        : "=r" (sample)
        : "I" (_SFR_IO_ADDR(PORT_NIXIES)),    // %1
          "I" (BIT_SRDATA),             // %2
          "I" (BIT_SRCLK),              // %3
          "r" (prt),                    // %4
          "0" (sample)          // %0
    );
#undef SHIFTOUT_BIT

    /* zero out all ctrl signal */
    PORT_NIXIES = prt;
    // PORT_NIXIES &= ~NIXBITS_MASK;
    // PORT_NIXIES = 0;
}

void isr_display_write()
{
    write_shiftreg(disp_buf[disp_index][0]);
    write_shiftreg(disp_buf[disp_index][1]);
    write_shiftreg(disp_buf[disp_index][2]);
    
    /* set DISPCLK */
    PORT_NIXIES |= (1 << BIT_DISPCLK);
    
    if (++disp_index > 5)
        disp_index = 0;
}


const uint8_t c_map[16][2] PROGMEM = {
    /*SYM       MSB                  LSB */
    /* (anode drivers are at [0]{b4,b1}, [1]{b6,b3,b0}; default off) */
    /*0*/ { 0b00010010, 0b01001101 },   // [0]
    /*1*/ { 0b10010010, 0b01001001 },
    /*2*/ { 0b01010010, 0b01001001 },
    /*3*/ { 0b00110010, 0b01001001 },
    /*4*/ { 0b00011010, 0b01001001 },
    /*5*/ { 0b00010110, 0b01001001 },
    /*6*/ { 0b00010011, 0b01001001 },
    /*7*/ { 0b00010010, 0b11001001 },
    /*8*/ { 0b00010010, 0b01101001 },
    /*9*/ { 0b00010010, 0b01011001 },   // [9]
    /*.*/ { 0b00010010, 0b01001011 },   // [10]
    /* */ { 0b00010010, 0b01001001 },   // [11]
    /* */ { 0b00010010, 0b01001001 },   // [12]
    /* */ { 0b00010010, 0b01001001 },
    /* */ { 0b00010010, 0b01001001 },
    /* */ { 0b00010010, 0b01001001 }    // [15]
};

const uint8_t a_map[6][3] PROGMEM = {
    /*NIX       MSB                             LSB */
    /*0*/ { 0b11101111, 0b11111111, 0b11111111 },
    /*1*/ { 0b11111101, 0b11111111, 0b11111111 },
    /*2*/ { 0b11111111, 0b10111111, 0b11111111 },
    /*3*/ { 0b11111111, 0b11110111, 0b11111111 },
    /*4*/ { 0b11111111, 0b11111110, 0b11111111 },
    /*5*/ { 0b11111111, 0b11111111, 0b01111111 }
};

void set_cathode_leds(uint8_t nix, uint8_t sym, char ndot, uint8_t leds)
{
    sym &= 0x0F;
    if (nixies_master_mute) {
        sym = 0x0F;         /* empty nixie (off) */
        ndot = 1;           /* no dot */
    }

    disp_en_irq = 0;
    //---
    disp_buf[nix][0] = (0x80 | leds) & pgm_read_byte_near(&a_map[nix][2]);      //LSB
    uint8_t dm = (ndot == 0) ? pgm_read_byte_near(&c_map[10][1]) : 0x00;                //dot cathode
    disp_buf[nix][1] = (pgm_read_byte_near(&c_map[sym][1]) | dm) & pgm_read_byte_near(&a_map[nix][1]);
    disp_buf[nix][2] = pgm_read_byte_near(&c_map[sym][0]) & pgm_read_byte_near(&a_map[nix][0]);         //MSB
    //---
    disp_en_irq = 0xFF;
}

void set_cathode(uint8_t nix, uint8_t sym, char ndot)
{
    set_cathode_leds(nix, sym, ndot, 0x00);
}


