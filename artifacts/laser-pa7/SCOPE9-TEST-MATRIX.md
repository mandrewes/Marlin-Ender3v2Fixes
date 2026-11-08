# SCOPE9 test matrix (source / build only — no flash)

| ID | Check | Expected | Result |
| --- | --- | --- | --- |
| T1 | `pio run -e STM32F103RE_creality` | SUCCESS | PASS |
| T2 | Version string in bin | `2.1.16-MA-LASER-PA7-SCOPE9` | PASS |
| T3 | Menus FanPA0 GPIO OFF/ON + PWM 25/50/75/100/Stop | strings present | PASS |
| T4 | Menus BedPA2 GPIO OFF/ON + PWM… | strings present | PASS |
| T5 | Menus ZPA7 GPIO LOW/HIGH + PWM… | strings present | PASS |
| T6 | PWM Bench AllOFF | string present | PASS |
| T7 | PinMap PA0 / PA2 / PA7 | TIM2_CH1 / TIM2_CH3 / TIM3_CH2 AFIO_NONE | PASS (PeripheralPins.c) |
| T8 | `TEMP_TIMER` | 7 | PASS (Configuration.h + TIM7_IRQHandler in ELF) |
| T9 | `HEATER_BED_PIN` / `TEMP_SENSOR_BED` | -1 / 0 | PASS |
| T10 | Soft-PWM skip uses `power_diag_owns_fan0()` | temperature.cpp | PASS (code) |
| T11 | AllOFF parks PA0+PA2+PA7 | `fan_bench_stop` | PASS (code) |
| T12 | One-at-a-time mutual exclusion | `power_diag_set` parks others | PASS (code) |
| T13 | PA1 held OFF during diag | `fan_bench_hold_ena_off` | PASS (code) |
| T14 | Physical 24 V full-swing on FET drain | User scope | NOT RUN (no flash; endpoints unconfirmed) |
