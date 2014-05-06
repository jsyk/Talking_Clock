EESchema Schematic File Version 2  date So 19. září 2009, 17:07:52 CEST
LIBS:power,triple_led,z573m,cpol,sn75468,device,conn,linear,regul,74xx,cmos4000,adc-dac,memory,xilinx,special,microcontrollers,dsp,microchip,analog_switches,motorola,texas,intel,audio,interface,digital-audio,philips,display,cypress,siliconi,contrib,valves,ei30-12-5-2,nixieboard-cache
EELAYER 23  0
EELAYER END
$Descr A4 11700 8267
Sheet 1 4
Title "NukaClock Nixieboard"
Date "17 aug 2009"
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Sheet
S 2500 3000 1700 1250
U 4AAE6CF7
F0 "HV Anode Power Source" 60
F1 "hvpower.sch" 60
$EndSheet
Wire Wire Line
	6900 4350 7700 4350
Wire Wire Line
	6900 4150 7700 4150
Wire Wire Line
	6900 3950 7700 3950
Wire Wire Line
	6900 3750 7700 3750
Wire Wire Line
	6900 3550 7700 3550
Wire Wire Line
	6900 3350 7700 3350
Wire Wire Line
	6900 2950 7700 2950
Wire Wire Line
	6900 2750 7700 2750
Wire Wire Line
	6900 2550 7700 2550
Wire Wire Line
	7700 2650 6900 2650
Wire Wire Line
	7700 2850 6900 2850
Wire Wire Line
	7700 3050 6900 3050
Wire Wire Line
	7700 3450 6900 3450
Wire Wire Line
	7700 3650 6900 3650
Wire Wire Line
	7700 3850 6900 3850
Wire Wire Line
	7700 4050 6900 4050
Wire Wire Line
	7700 4250 6900 4250
$Sheet
S 7700 2400 1000 2450
U 4AAE6CF9
F0 "Nixies" 60
F1 "nixies.sch" 60
F2 "A5" I L 7700 3050 60 
F3 "A4" I L 7700 2950 60 
F4 "A3" I L 7700 2850 60 
F5 "A2" I L 7700 2750 60 
F6 "A1" I L 7700 2650 60 
F7 "A0" I L 7700 2550 60 
F8 "C10" I L 7700 4350 60 
F9 "C9" I L 7700 4250 60 
F10 "C8" I L 7700 4150 60 
F11 "C7" I L 7700 4050 60 
F12 "C6" I L 7700 3950 60 
F13 "C5" I L 7700 3850 60 
F14 "C4" I L 7700 3750 60 
F15 "C3" I L 7700 3650 60 
F16 "C2" I L 7700 3550 60 
F17 "C1" I L 7700 3450 60 
F18 "C0" I L 7700 3350 60 
$EndSheet
Text Notes 5800 5350 0    60   ~ 0
BT RGB LED, RB LOCKED LED
$Sheet
S 5150 2400 1750 2500
U 4AAE6CFB
F0 "Driver Logic" 60
F1 "driver.sch" 60
F2 "A5" I R 6900 3050 60 
F3 "A4" I R 6900 2950 60 
F4 "A3" I R 6900 2850 60 
F5 "A2" I R 6900 2750 60 
F6 "A1" I R 6900 2650 60 
F7 "A0" I R 6900 2550 60 
F8 "C10" I R 6900 4350 60 
F9 "C9" I R 6900 4250 60 
F10 "C8" I R 6900 4150 60 
F11 "C7" I R 6900 4050 60 
F12 "C6" I R 6900 3950 60 
F13 "C5" I R 6900 3850 60 
F14 "C4" I R 6900 3750 60 
F15 "C3" I R 6900 3650 60 
F16 "C2" I R 6900 3550 60 
F17 "C1" I R 6900 3450 60 
F18 "C0" I R 6900 3350 60 
$EndSheet
$EndSCHEMATC
