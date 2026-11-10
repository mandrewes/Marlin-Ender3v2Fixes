| ID | Interface (contacts) | MCU / PWM classification |
|---|---|---|
| 01 | 24 V IN (2 screw terminals) | None; NO: supply input |
| 02 | FAN + / - (fixed-power pair) (2 screw terminals) | None; NO: fixed supply |
| 03 | BED + / - (2 screw terminals) | PA2 -> buffer -> bed MOSFET; POWER PWM / heater switching; not TTL |
| 04 | NOZ + / - (hotend heater) (2 screw terminals) | PA1 -> buffer -> nozzle MOSFET; POWER PWM capable; current laser test uses ON/OFF enable |
| 05 | K-FAN1 (2-pin socket) | PA0 -> buffer -> fan MOSFET; POWER PWM, not TTL |
| 06 | K-FAN2 (2-pin socket) | PA0, shared with K-FAN1; POWER PWM, not TTL |
| 07 | X MOTOR (4-pin socket) | PC2 STEP, PB9 DIR; PC3 shared ENABLE (upstream of driver); NO usable logic PWM at socket; internal driver chopper switching is NOT a laser signal |
| 08 | Y MOTOR (4-pin socket) | PB8 STEP, PB7 DIR; PC3 shared ENABLE (upstream of driver); NO usable logic PWM at socket; internal driver chopper switching is NOT a laser signal |
| 09 | Z MOTOR (4-pin socket) | PB6 STEP, PB5 DIR; PC3 shared ENABLE (upstream of driver); NO usable logic PWM at socket; internal driver chopper switching is NOT a laser signal |
| 10 | E MOTOR (4-pin socket) | PB4 STEP, PB3 DIR; PC3 shared ENABLE (upstream of driver); NO usable logic PWM at socket; internal driver chopper switching is NOT a laser signal |
| 11 | X- ENDSTOP (2-pin socket) | PA5; INPUT as wired; not a clean logic-PWM output |
| 12 | Y- ENDSTOP (2-pin socket) | PA6; INPUT as wired; not a clean logic-PWM output |
| 13 | Z- ENDSTOP (2-pin socket) | PA7; INPUT as wired; not a clean logic-PWM output |
| 14 | J1 / filament runout (3-pin socket) | PA4; INPUT as wired; PA4 has NO native timer-PWM alternate function |
| 15 | BL_T / probe (G V IN G OUT) (5-pin socket) | PB0 deploy/control; PB1 trigger; SERVO control pulses on PB0; PB1 is trigger INPUT |
| 16 | TB / bed thermistor (2-pin socket) | PC4; NO native timer PWM; analog sensing input |
| 17 | TH / hotend thermistor (2-pin socket) | PC5; NO native timer PWM; analog sensing input |
| 18 | Display / EXP (IDC ribbon) (10-pin shrouded socket) | RET6 map: PC6, PB2, PB10, PB11, PB14, PB13, PB12, PB15; NOT a stock spare PWM connector; some nets have timer alternate functions |
| 19 | USB1 / micro-USB (5 USB contacts plus shell) | CH340G -> MCU PA9 TX / PA10 RX (USART1); NO: USB/serial interface |
| 20 | microSD socket (8 card contacts plus card-detect switch) | PC8..PC11 data; PC12 clock; PD2 command; PC7 detect; NO usable stock PWM interface |
| 21 | P2 / SWD programming pads (4 pads; header not fitted in reference photo) | PA13 SWDIO; PA14 SWCLK; NO native timer PWM on PA13/PA14 |
| 22 | P3 auxiliary pads ("Reset" in drawing) (2 pads; header not fitted) | PC0 (MCU pin 8) in traced circuit, NOT NRST pin 7; NO native timer PWM on PC0 |