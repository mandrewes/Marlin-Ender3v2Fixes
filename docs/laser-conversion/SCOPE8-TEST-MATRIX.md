# SCOPE8 test matrix (source / build only — no flash)

| ID | Check | Expected | Result |
| --- | --- | --- | --- |
| T1 | `pio run -e STM32F103RE_creality` | SUCCESS | PASS |
| T2 | `SHORT_BUILD_VERSION` in bin | `2.1.16-MA-LASER-PA7-SCOPE8` | PASS |
| T3 | Menu: FanPA0 OFF/25/50/75/100/Stop | strings present | PASS |
| T4 | Menu: BedPA2 OFF/25/50/75/100/Stop | strings present | PASS |
| T5 | Menu: PWM Bench AllOFF | string present | PASS |
| T6 | Note: K-FAN1+2 share PA0 | string present | PASS |
| T7 | PinMap PA0 | TIM2_CH1 AFIO_NONE | PASS (PeripheralPins.c) |
| T8 | PinMap PA2 | TIM2_CH3 AFIO_NONE | PASS (PeripheralPins.c) |
| T9 | TEMP_TIMER | 7 (TIM7) | PASS (Configuration.h) |
| T10 | HEATER_BED_PIN | -1 | PASS |
| T11 | TEMP_SENSOR_BED | 0 | PASS |
| T12 | Mutual exclusion | power_bench_set parks other CH | PASS (code review) |
| T13 | Soft-PWM skip FAN0 when FanPA0 live | temperature.cpp guards | PASS (code review) |
| T14 | PA1 hold OFF / PA7 stop on bench | fan_bench_hold_ena_off + scope_diag_stop | PASS (code review) |
| T15 | Cleanup on menu exit / kill | fan_bench_task + kill() | PASS (code review) |
| T16 | Scope 1 kHz on hardware | User measures | NOT RUN (no flash) |

Artifact: `firmware-SCOPE8-20261108-035741.bin`
