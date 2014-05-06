EESchema Schematic File Version 2
LIBS:power
LIBS:device
LIBS:transistors
LIBS:conn
LIBS:linear
LIBS:regul
LIBS:74xx
LIBS:cmos4000
LIBS:adc-dac
LIBS:memory
LIBS:xilinx
LIBS:special
LIBS:microcontrollers
LIBS:dsp
LIBS:microchip
LIBS:analog_switches
LIBS:motorola
LIBS:texas
LIBS:intel
LIBS:audio
LIBS:interface
LIBS:digital-audio
LIBS:philips
LIBS:display
LIBS:cypress
LIBS:siliconi
LIBS:opto
LIBS:atmel
LIBS:contrib
LIBS:valves
LIBS:le33cz
LIBS:sdcard
LIBS:cpol
LIBS:rb_pack4
LIBS:tda7233
LIBS:reguls
LIBS:transfo_hahn_230v_9v
LIBS:rcmainboard-cache
EELAYER 27 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 5
Title ""
Date "12 mar 2014"
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Sheet
S 6750 3700 900  1150
U 52DDB48B
F0 "Sound Module" 50
F1 "audioamp.sch" 50
F2 "XMUTE" I L 6750 4450 60 
F3 "ASDI" I L 6750 4000 60 
F4 "ASCK" I L 6750 4100 60 
F5 "XACS" I L 6750 4200 60 
$EndSheet
$Sheet
S 6800 1800 900  1250
U 52DD9F0F
F0 "SD Card Interface" 50
F1 "sdiface.sch" 50
F2 "DETSD" O R 7700 1950 60 
F3 "XSDS" I L 6800 1950 60 
F4 "MOSI" I L 6800 2050 60 
F5 "SCK" I L 6800 2150 60 
F6 "MISO" O R 7700 2050 60 
$EndSheet
$Sheet
S 4000 2950 1350 1900
U 52D3010B
F0 "CPU Module" 50
F1 "cpumod.sch" 50
F2 "XSDS" O R 5350 3050 60 
F3 "MOSI" O R 5350 3150 60 
F4 "MISO" I L 4000 3050 60 
F5 "SCK" O R 5350 3250 60 
F6 "DETSD" I L 4000 3150 60 
F7 "ASDI" O R 5350 4000 60 
F8 "ASCK" O R 5350 4100 60 
F9 "XACS" O R 5350 4200 60 
F10 "XMUTE" O R 5350 4450 60 
$EndSheet
Wire Wire Line
	5350 3050 6050 3050
Wire Wire Line
	6050 3050 6050 1950
Wire Wire Line
	6050 1950 6800 1950
Wire Wire Line
	5350 3150 6150 3150
Wire Wire Line
	6150 3150 6150 2050
Wire Wire Line
	6150 2050 6800 2050
Wire Wire Line
	5350 3250 6250 3250
Wire Wire Line
	6250 3250 6250 2150
Wire Wire Line
	6250 2150 6800 2150
Wire Wire Line
	7700 2050 8300 2050
Wire Wire Line
	8300 2050 8300 1250
Wire Wire Line
	8300 1250 3650 1250
Wire Wire Line
	3650 1250 3650 3050
Wire Wire Line
	3650 3050 4000 3050
Wire Wire Line
	7700 1950 8150 1950
Wire Wire Line
	8150 1950 8150 1400
Wire Wire Line
	8150 1400 3750 1400
Wire Wire Line
	3750 1400 3750 3150
Wire Wire Line
	3750 3150 4000 3150
Wire Wire Line
	5350 4000 6750 4000
Wire Wire Line
	5350 4100 6750 4100
Wire Wire Line
	5350 4200 6750 4200
Wire Wire Line
	5350 4450 6750 4450
$Sheet
S 2500 3350 850  850 
U 52F53F89
F0 "Power Source" 50
F1 "power.sch" 50
$EndSheet
$Comp
L CONN_1 VRT1
U 1 1 530E3E36
P 2050 5650
F 0 "VRT1" H 2130 5650 40  0000 L CNN
F 1 "CONN_1" H 2050 5705 30  0001 C CNN
F 2 "~" H 2050 5650 60  0000 C CNN
F 3 "~" H 2050 5650 60  0000 C CNN
	1    2050 5650
	1    0    0    -1  
$EndComp
$Comp
L CONN_1 VRT2
U 1 1 530E3EE7
P 2050 5800
F 0 "VRT2" H 2130 5800 40  0000 L CNN
F 1 "CONN_1" H 2050 5855 30  0001 C CNN
F 2 "~" H 2050 5800 60  0000 C CNN
F 3 "~" H 2050 5800 60  0000 C CNN
	1    2050 5800
	1    0    0    -1  
$EndComp
$Comp
L CONN_1 VRT3
U 1 1 530E3EF6
P 2050 5950
F 0 "VRT3" H 2130 5950 40  0000 L CNN
F 1 "CONN_1" H 2050 6005 30  0001 C CNN
F 2 "~" H 2050 5950 60  0000 C CNN
F 3 "~" H 2050 5950 60  0000 C CNN
	1    2050 5950
	1    0    0    -1  
$EndComp
$Comp
L CONN_1 VRT4
U 1 1 530E3F05
P 2050 6100
F 0 "VRT4" H 2130 6100 40  0000 L CNN
F 1 "CONN_1" H 2050 6155 30  0001 C CNN
F 2 "~" H 2050 6100 60  0000 C CNN
F 3 "~" H 2050 6100 60  0000 C CNN
	1    2050 6100
	1    0    0    -1  
$EndComp
$Comp
L CONN_1 VRT5
U 1 1 530E5003
P 2050 6250
F 0 "VRT5" H 2130 6250 40  0000 L CNN
F 1 "CONN_1" H 2050 6305 30  0001 C CNN
F 2 "~" H 2050 6250 60  0000 C CNN
F 3 "~" H 2050 6250 60  0000 C CNN
	1    2050 6250
	1    0    0    -1  
$EndComp
$EndSCHEMATC