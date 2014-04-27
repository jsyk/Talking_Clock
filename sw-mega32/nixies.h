#ifndef NIXIES_H
#define NIXIES_H

#include <avr/pgmspace.h>

extern volatile uint8_t disp_en_irq;
extern volatile uint8_t nixies_master_mute;

void set_cathode_leds(uint8_t nix, uint8_t sym, char ndot, uint8_t leds);

void set_cathode(uint8_t nix, uint8_t sym, char ndot);

void isr_display_write();

void init_nixie_ports();

#endif
