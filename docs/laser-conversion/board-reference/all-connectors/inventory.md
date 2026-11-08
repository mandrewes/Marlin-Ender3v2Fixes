# Creality V4.2.2 — complete connector inventory

Reference: STM32F103RET6 Ender-3 V2. Callouts identify connectors, NOT physical pin numbers. Research-only; no changes made.

Hardware timer capability, stock socket function and current test firmware are different things. Inputs are not automatically outputs. All capacitor values and manufacturing variants require appropriate caution.

## 01 — 24 V IN
- **Contacts:** 2 screw terminals
- **Function:** Main DC supply input
- **Electrical:** 24 V DC, positive and board ground
- **Mcu:** None
- **Pwm:** NO: supply input
- **Notes:** Not mains. Physical screw polarity must follow the actual board markings.
- **Sources:** S1, S2

## 02 — FAN + / - (fixed-power pair)
- **Contacts:** 2 screw terminals
- **Function:** Always-powered hotend/heatsink fan supply
- **Electrical:** 24 V DC; permanent ground return
- **Mcu:** None
- **Pwm:** NO: fixed supply
- **Notes:** Not K-FAN1/2. Fan fuse/rail routing differs between published drawings and manufacturing reports; do not assume which fuse protects it.
- **Sources:** S1, S2

## 03 — BED + / -
- **Contacts:** 2 screw terminals
- **Function:** Heated-bed power output
- **Electrical:** 24 V positive; MOSFET-switched negative
- **Mcu:** PA2 -> buffer -> bed MOSFET
- **Pwm:** POWER PWM / heater switching; not TTL
- **Notes:** PA2 supports native timer channels at the MCU, but those are not exposed here. Negative terminal is not permanent ground.
- **Sources:** S1, S3, S4

## 04 — NOZ + / - (hotend heater)
- **Contacts:** 2 screw terminals
- **Function:** Original nozzle-heater output; current test laser-enable output
- **Electrical:** 24 V positive; MOSFET-switched negative
- **Mcu:** PA1 -> buffer -> nozzle MOSFET
- **Pwm:** POWER PWM capable; current laser test uses ON/OFF enable
- **Notes:** PA1 is not raw logic at the screws. Common ground tied to its switched negative can bypass the power switch. No heater or laser connection recommended.
- **Sources:** S1, S3, S4; current test context

## 05 — K-FAN1
- **Contacts:** 2-pin socket
- **Function:** Controlled fan output
- **Electrical:** 24 V positive; shared low-side MOSFET return
- **Mcu:** PA0 -> buffer -> fan MOSFET
- **Pwm:** POWER PWM, not TTL
- **Notes:** Shares one control/net with K-FAN2: not independent channels. Marlin declares FAN_SOFT_PWM_REQUIRED. Current SCOPE6 context reports about 7.8 Hz software PWM, not laser-frequency PWM.
- **Sources:** S1, S3; current test context

## 06 — K-FAN2
- **Contacts:** 2-pin socket
- **Function:** Second controlled fan output (often board-cooling fan)
- **Electrical:** 24 V positive; same switched return as K-FAN1
- **Mcu:** PA0, shared with K-FAN1
- **Pwm:** POWER PWM, not TTL
- **Notes:** Same command/duty as K-FAN1; not a separate spare output. Do not confuse it with the fixed FAN screw pair.
- **Sources:** S1, S3

## 07 — X MOTOR
- **Contacts:** 4-pin socket
- **Function:** X stepper-motor coil connections
- **Electrical:** Two current-regulated H-bridge coil pairs; approximately 24 V switching, not a DC supply
- **Mcu:** PC2 STEP, PB9 DIR; PC3 shared ENABLE (upstream of driver)
- **Pwm:** NO usable logic PWM at socket; internal driver chopper switching is NOT a laser signal
- **Notes:** MCU STEP/DIR pins are not the motor socket contacts. Driver type varies by V4.2.2 manufacturing run. All four enables share PC3.
- **Sources:** S1, S3

## 08 — Y MOTOR
- **Contacts:** 4-pin socket
- **Function:** Y stepper-motor coil connections
- **Electrical:** Two current-regulated H-bridge coil pairs; approximately 24 V switching, not a DC supply
- **Mcu:** PB8 STEP, PB7 DIR; PC3 shared ENABLE (upstream of driver)
- **Pwm:** NO usable logic PWM at socket; internal driver chopper switching is NOT a laser signal
- **Notes:** MCU STEP/DIR pins are not the motor socket contacts. Driver type varies by V4.2.2 manufacturing run. All four enables share PC3.
- **Sources:** S1, S3

## 09 — Z MOTOR
- **Contacts:** 4-pin socket
- **Function:** Z stepper-motor coil connections
- **Electrical:** Two current-regulated H-bridge coil pairs; approximately 24 V switching, not a DC supply
- **Mcu:** PB6 STEP, PB5 DIR; PC3 shared ENABLE (upstream of driver)
- **Pwm:** NO usable logic PWM at socket; internal driver chopper switching is NOT a laser signal
- **Notes:** MCU STEP/DIR pins are not the motor socket contacts. Driver type varies by V4.2.2 manufacturing run. All four enables share PC3.
- **Sources:** S1, S3

## 10 — E MOTOR
- **Contacts:** 4-pin socket
- **Function:** E stepper-motor coil connections
- **Electrical:** Two current-regulated H-bridge coil pairs; approximately 24 V switching, not a DC supply
- **Mcu:** PB4 STEP, PB3 DIR; PC3 shared ENABLE (upstream of driver)
- **Pwm:** NO usable logic PWM at socket; internal driver chopper switching is NOT a laser signal
- **Notes:** MCU STEP/DIR pins are not the motor socket contacts. Driver type varies by V4.2.2 manufacturing run. All four enables share PC3.
- **Sources:** S1, S3

## 11 — X- ENDSTOP
- **Contacts:** 2-pin socket
- **Function:** X minimum limit-switch input
- **Electrical:** Signal plus ground; 3.3 V input network, NO supply contact
- **Mcu:** PA5
- **Pwm:** INPUT as wired; not a clean logic-PWM output
- **Notes:** R11=10 kohm series, R4=10 kohm pull-up to 3.3 V, C11 to ground (100 nF ASSUMED by schematic author). No native timer PWM on PA5. Divider can hold a driven-low socket near 1.65 V. 
- **Sources:** S1, S3, S4

## 12 — Y- ENDSTOP
- **Contacts:** 2-pin socket
- **Function:** Y minimum limit-switch input
- **Electrical:** Signal plus ground; 3.3 V input network, NO supply contact
- **Mcu:** PA6
- **Pwm:** INPUT as wired; not a clean logic-PWM output
- **Notes:** R12=10 kohm series, R5=10 kohm pull-up to 3.3 V, C12 to ground (100 nF ASSUMED by schematic author). PA6 can carry TIM3_CH1 at the MCU. Divider can hold a driven-low socket near 1.65 V. 
- **Sources:** S1, S3, S4

## 13 — Z- ENDSTOP
- **Contacts:** 2-pin socket
- **Function:** Z minimum limit-switch input
- **Electrical:** Signal plus ground; 3.3 V input network, NO supply contact
- **Mcu:** PA7
- **Pwm:** INPUT as wired; not a clean logic-PWM output
- **Notes:** R13=10 kohm series, R6=10 kohm pull-up to 3.3 V, C13 to ground (100 nF ASSUMED by schematic author). PA7 can carry TIM3_CH2 / TIM8_CH1N at the MCU. Divider can hold a driven-low socket near 1.65 V. User measured about 3.3 V HIGH / 1.6 V LOW and rounded ~844 mVpp at ~1 kHz; this is user-reported, not a new measurement.
- **Sources:** S1, S3, S4; Z measurements supplied by user

## 14 — J1 / filament runout
- **Contacts:** 3-pin socket
- **Function:** Optional filament detector
- **Electrical:** PA4 signal, ground, sensor-supply contact
- **Mcu:** PA4
- **Pwm:** INPUT as wired; PA4 has NO native timer-PWM alternate function
- **Notes:** R10=10 kohm series; R3=10 kohm pull-up; no signal capacitor shown. V4.2.2 schematic/wiki specify 3.3 V supply, but a combined TH3D page illustrating V4.2.7 says 5 V. Actual rail unmeasured. No outer-contact physical order asserted.
- **Sources:** S1, S3, S4, S5, S6

## 15 — BL_T / probe (G V IN G OUT)
- **Contacts:** 5-pin socket
- **Function:** 3D Touch / BLTouch-compatible probe power, deploy and trigger
- **Electrical:** Two grounds, 5 V supply, 3.3 V MCU control/trigger nets
- **Mcu:** PB0 deploy/control; PB1 trigger
- **Pwm:** SERVO control pulses on PB0; PB1 is trigger INPUT
- **Notes:** Occupied by working 3D Touch: KEEP. PB0/PB1 have timer alternate functions, but are not spare laser pins. No series/endstop RC network shown in these probe signal paths; not permission to attach a laser.
- **Sources:** S1, S3, S4, S5; current wiring context

## 16 — TB / bed thermistor
- **Contacts:** 2-pin socket
- **Function:** Bed-temperature sensor input
- **Electrical:** Analog input plus ground, 3.3 V pull-up/filter network
- **Mcu:** PC4
- **Pwm:** NO native timer PWM; analog sensing input
- **Notes:** Reference: R58=4.7 kohm pull-up, C41 labelled 10 uF. Not a power output. Sensor connector positions can be mislabelled in marketing images; PCB silk and trace take precedence.
- **Sources:** S1, S3, S4

## 17 — TH / hotend thermistor
- **Contacts:** 2-pin socket
- **Function:** Nozzle-temperature sensor input
- **Electrical:** Analog input plus ground, 3.3 V pull-up/filter network
- **Mcu:** PC5
- **Pwm:** NO native timer PWM; analog sensing input
- **Notes:** Reference: R57=4.7 kohm pull-up, C40 labelled 10 uF. Not a power output. Capacitance values are reference-drawing values, not measurements of this board.
- **Sources:** S1, S3, S4

## 18 — Display / EXP (IDC ribbon)
- **Contacts:** 10-pin shrouded socket
- **Function:** Ender-3 V2 display serial link, encoder/button/beeper signals
- **Electrical:** 3.3 V MCU signals plus 5 V display supply and ground
- **Mcu:** RET6 map: PC6, PB2, PB10, PB11, PB14, PB13, PB12, PB15
- **Pwm:** NOT a stock spare PWM connector; some nets have timer alternate functions
- **Notes:** PB10/PB11 are USART3; display uses other nets for controls/beeper. Schematic includes 47-ohm series networks. Occupied display signals cannot simply be reassigned. No numbered ribbon-pin orientation given here.
- **Sources:** S1, S3, S5

## 19 — USB1 / micro-USB
- **Contacts:** 5 USB contacts plus shell
- **Function:** Computer serial connection through CH340G bridge
- **Electrical:** USB differential D+/D-, VBUS, ground; not a raw MCU USB/PWM header
- **Mcu:** CH340G -> MCU PA9 TX / PA10 RX (USART1)
- **Pwm:** NO: USB/serial interface
- **Notes:** USB can partially power logic; do not assume board is dead merely because 24 V is off. USB D+/D- do not expose PA11/PA12; those MCU pins serve the onboard EEPROM here.
- **Sources:** S1, S3

## 20 — microSD socket
- **Contacts:** 8 card contacts plus card-detect switch
- **Function:** Firmware/print-file storage
- **Electrical:** 3.3 V SDIO bus
- **Mcu:** PC8..PC11 data; PC12 clock; PD2 command; PC7 detect
- **Pwm:** NO usable stock PWM interface
- **Notes:** Storage bus contacts are not spare PWM outputs. SDIO uses these nets; PA4 is runout, not this socket chip-select.
- **Sources:** S1, S3

## 21 — P2 / SWD programming pads
- **Contacts:** 4 pads; header not fitted in reference photo
- **Function:** Debug/programming interface
- **Electrical:** 3.3 V reference, SWDIO, SWCLK, ground
- **Mcu:** PA13 SWDIO; PA14 SWCLK
- **Pwm:** NO native timer PWM on PA13/PA14
- **Notes:** Schematic calls this ICSP; on STM32 it is SWD, not AVR ICSP. Pads, not a fitted four-pin socket. Do not assume the voltage pad is 5 V.
- **Sources:** S1, S3, S4; photo P2 silk

## 22 — P3 auxiliary pads ("Reset" in drawing)
- **Contacts:** 2 pads; header not fitted
- **Function:** Auxiliary pulled-up input/test-pad pair
- **Electrical:** PC0 signal plus ground; 3.3 V pull-up/filter
- **Mcu:** PC0 (MCU pin 8) in traced circuit, NOT NRST pin 7
- **Pwm:** NO native timer PWM on PC0
- **Notes:** Drawing labels it P3 Reset, but its traced net and author correction identify PC0. R60=10 kohm pull-up, R61=0 ohm link, C39 to ground (value assumed). Treat reset naming cautiously; no shorting instructions.
- **Sources:** S1, S2, S4; photo P3 silk

## Evidence / limitations
High confidence: connector identity/counts, common GPIO assignments and power-versus-logic distinction. Medium/reference-level confidence: component values and precise rails on an individual manufacturing variant. J1 rail and physical end-contact orientation remain unresolved. P3 Reset is a misleading drawing label; not verified as an NRST connection.
Fuses F1/F2/F3, LEDs, ICs, crystals, resistors and onboard EEPROM are not external connectors. No additional three-pin endstop sockets or undocumented spare power headers were invented.
The reference photo exposes 20 connector/terminal interfaces plus 2 unpopulated pad groups = 22 callouts. No separate fitted reset switch or external I2C socket was identified.

## Sources
- S1: Community traced V4.2.2 schematic, 28-5-22; NOT an official Creality schematic. https://github.com/Jyers/Marlin/files/8791332/Creality.4.2.2.-.Schematic.28-5-22.pdf
- S2: Schematic author discussion/corrections, including PC0 at P3 and grounded MOSFET sources. https://github.com/Jyers/Marlin/discussions/814
- S3: Marlin upstream V4 pin map; RET6 configuration, not V4.2.7 overrides. https://github.com/MarlinFirmware/Marlin/blob/bugfix-2.1.x/Marlin/src/pins/stm32f1/pins_CREALITY_V4.h
- S4: ST STM32F103xC/D/E manufacturer datasheet, alternate-function Table 5. https://www.st.com/resource/en/datasheet/stm32f103re.pdf
- S5: Independent V4.2.2 community photo and J1/probe/display documentation. https://ender3.club/Creality_4.2.2
- S6: Combined TH3D 4.2.2/4.2.7 article: illustrates 4.2.7; conflicting J1 supply claim. https://tickets.th3dstudio.com/products/article/creality-v4-2-7-v4-2-2-wiring-and-pinout-diagram
- Photo: Original photograph visibly labelled V4.2.2; reference board, not a newly captured photo of Michael's unit. https://ender3.club/images/3/34/Pinount_creality_4.2.2.png