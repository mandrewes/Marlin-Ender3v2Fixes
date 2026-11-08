# SCOPE8 — FanPA0 + BedPA2 hardware PWM @ 1 kHz

**Version:** `2.1.16-MA-LASER-PA7-SCOPE8`  
**Scope-bench only. Not laser-ready. No flash / no SD staging by this agent.**

Laser + bed heater **disconnected**. Working **3D Touch PB0/PB1** untouched. Z-diag (PA7) reference menus retained but inactive during power bench.

## Why SCOPE8

User wants measurable **power PWM** on both former fan and bed MOSFET paths:

| Target | MCU pin | Header | Timer channel |
| --- | --- | --- | --- |
| **FanPA0** | **PA0** | K-FAN1 **and** K-FAN2 (same net) | **TIM2_CH1** |
| **BedPA2** | **PA2** | BED MOSFET gate path | **TIM2_CH3** |

Both channels share **TIM2** at the **same** frequency (**1000 Hz**). Separate CCR duties are fine on one timer; firmware still runs **one channel at a time** for safety (selecting Fan parks Bed and vice versa).

### K-FAN1 / K-FAN2 are not independent

On Creality V4.2.2 both fan headers are wired to **FAN0 = PA0**. There is no second fan GPIO. One **FanPA0** menu drives both headers together — independent Fan1 vs Fan2 menus are impossible in hardware.

### Bed output is not reclaimable by temp ISR

`HEATER_BED_PIN = -1`, `TEMP_SENSOR_BED = 0`, `EXTRUDERS = 0` → no heated-bed Soft-PWM / thermal errors / dummy heating. Bed menus never appear. **PA2** is owned only by the BedPA2 bench while active.

## Timer allocation (verified)

PinMap (`MARLIN_F103Rx` / `Generic_F103Rx` PeripheralPins.c, AFIO_NONE):

- `PA0 → TIM2_CH1`
- `PA2 → TIM2_CH3`

| Timer | Owner | Notes |
| --- | --- | --- |
| **TIM7** | **TEMP** ISR (1 kHz tick) | Moved from TIM2 (`#define TEMP_TIMER 7`) |
| **TIM2** | FanPA0 CH1 / BedPA2 CH3 HW PWM | Freed by TEMP move; shared 1 kHz ARR |
| **TIM4** | STEP | Unchanged |
| **TIM5** | SERVO (3D Touch) | Unchanged |
| **TIM3** | Laser PA7 Scope (parked during bench) | Unchanged |

Soft-PWM ISR **skips FAN0** while `power_bench_ch == PWR_BENCH_FAN0` so it cannot bit-bang over TIM2_CH1.

## LCD controls (Laser Control)

Separate submenus + global stop:

| Menu | Action |
| --- | --- |
| **FanPA0 PWM** → OFF / 25 / 50 / 75 / 100% / Stop | TIM2_CH1 on PA0; parks BedPA2 |
| **BedPA2 PWM** → OFF / 25 / 50 / 75 / 100% / Stop | TIM2_CH3 on PA2; parks FanPA0 |
| **PWM Bench AllOFF** | Parks both channels |

Duty OCR values (load **ON** fraction, `invert=false` → pin HIGH fraction = MOSFET conducting):

| Label | OCR/255 |
| --- | --- |
| OFF 0% | 0 |
| 25% ON | 64 |
| 50% ON | 128 |
| 75% ON | 191 |
| 100% ON | 255 |

Persists until Stop / AllOFF, leave Laser Control (and FanPA0/BedPA2 submenus), kill/fault/disable, or reset.  
**PA1** (laser ENA) forced OFF the entire time. **PA7** Scope diag stopped when any power bench starts.  
Normal laser `SPEED_POWER_STARTUP` **0%** and `LASER_SAFETY_TIMEOUT_MS` **1000** unchanged.

## What you measure (not TTL)

These sockets are **24 V low-side MOSFET switched power** (MCU pin → buffer → FET). Expect a **power PWM** waveform on the switched node — **not** clean 3.3 V logic.

### Fluke PM3082 (clipboard DC negative)

1. Scope **ground clip only** on confirmed **board DC negative / PSU −** (clipboard reference).  
2. Probe tip on **switched minus** of the load under test (FET drain side), with an **appropriate 24 V fan / test load** plugged in (unloaded OFF may float).  
3. Use attenuation / range suitable for **24 V**. DC coupling; auto-set while a mid duty (50%) is held.  
4. **Never** put the ground clip on **BED−** or **FAN−** (those are the switched nodes).  
5. **Do not** hang an arbitrary high-power heating load on BedPA2 — bench only, heater disconnected.  
6. Optional: DC multimeter across the load pair shows average voltage; scope on drain shows the **inverse of 24 V** relative to PSU+ (ON ≈ near 0 V at drain, OFF ≈ near 24 V).

Test **one channel at a time** (firmware enforces this).

## Cleanup paths

| Event | Behavior |
| --- | --- |
| Stop / AllOFF | Both pins duty 0 + GPIO LOW; Soft-PWM fans at 0; PA1 OFF |
| Leave Laser Control tree | `fan_bench_task` cancels |
| kill / cutter disable / Scope start | `fan_bench_stop` |
| Reset | Cold start; benches inactive |

## Firmware tests (this agent)

| Check | Result |
| --- | --- |
| `pio run -e STM32F103RE_creality` | SUCCESS |
| Binary version string | `2.1.16-MA-LASER-PA7-SCOPE8` |
| Menu strings | FanPA0 / BedPA2 / AllOFF / K-FAN1+2 share PA0 |
| PinMap | PA0=TIM2_CH1, PA2=TIM2_CH3 (AFIO_NONE) |
| TEMP_TIMER | **7** |
| Oscilloscope confirmation of 1 kHz | **User** (hardware); agent does not flash |
