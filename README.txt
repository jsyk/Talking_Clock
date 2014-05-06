Talking Clock
===============

See
    http://www.jsykora.info/?page_id=240
and
    http://hackaday.io/project/1082-Talking-Nixie-Clock
for more details.

Firmware for ATMEGA32 CPU is in the sw-mega32 directory. Use AVR gcc (a package in Fedora YUM is available) to compile by Makefile, and use uisp to download compiled hex file into AVR CPU via ISP cable.
Schematic and PCB drawings in KiCAD format for the nixie board and the CPU board are in the nixieboard and rcmainboard directories, respectively.
