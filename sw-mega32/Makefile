
FILE=dcfnix
CSRC=main.c serial.c nixies.c dcf.c mmc.c sound.c tff.c loop.c voice.c vfs.c debug.c clock.c ir.c btn.c

CC=avr-gcc
OBJCOPY=avr-objcopy
OBJDUMP=avr-objdump

CFLAGS=-g -mmcu=atmega32 -Os -Wall -std=c99
LDFLAGS=-g -mmcu=atmega32

OBJS=$(patsubst %.c,%.o,$(CSRC))

$(FILE).hex: $(FILE).elf
	$(OBJCOPY) -j .text -j .data -O ihex $(FILE).elf $(FILE).hex
	$(OBJDUMP) -h -D -S $(FILE).elf > $(FILE).listing

$(FILE).elf: $(OBJS)
	$(CC) $(LDFLAGS) -o $(FILE).elf -Wl,-Map,$(FILE).map  $(OBJS)

makefile.dep:
	$(CC) -MM $(CFLAGS) $(CSRC) >makefile.dep

prog: $(FILE).hex
	uisp -dprog=stk200 --erase --upload if=$(FILE).hex --verify

clean:
	rm -f *.o makefile.dep
	rm -f *.map
	rm -f *.elf
	rm -f *.hex
	rm -f *.listing
	rm -f *~

-include makefile.dep
