# SCOPE7 — K-FAN PA0 hardware PWM @ 1 kHz (measurable power PWM)

**Version:** `2.1.16-MA-LASER-PA7-SCOPE7`  
**Scope-bench only. Not laser-ready. No flash / no SD staging by this agent.**

Laser + heater loads **disconnected**. Working **3D Touch PB0/PB1** untouched.

## Problem solved

Stock Creality path forces `FAN_SOFT_PWM` → ~**7.8 Hz** (TEMP ISR 1 kHz / 128).  
PA0 PinMap first match is **TIM2_CH1**, but default **TEMP_TIMER = TIM2** — HW PWM on PA0 would fight the temperature tick.

**SCOPE7 allocation:**

| Timer | Owner | Notes |
| --- | --- | --- |
| **TIM7** | **TEMP** ISR (1 kHz tick) | Moved from TIM2; basic timer, no GPIO |
| **TIM2** | **FAN0 PA0 CH1** HW PWM during FanBench | Freed by TEMP move |
| **TIM4** | STEP | Unchanged |
| **TIM5** | SERVO (3D Touch) | Unchanged |
| **TIM3** | Laser PA7 Scope (inactive during FanBench) | Unchanged |

Compile/config proof: `#define TEMP_TIMER 7` in `Configuration.h`. Soft-PWM ISR **skips FAN0** while `fan_bench_active` so it cannot bit-bang over TIM2.

## Controls (Laser Control → FanPWM)

| Menu | Duty (OCR/255) | Freq |
| --- | --- | --- |
| `FanPWM OFF 0%` | 0 | 1 kHz carrier (idle low) |
| `FanPWM 25%` | 64 | **1000 Hz** |
| `FanPWM 50%` | 128 | **1000 Hz** |
| `FanPWM 75%` | 191 | **1000 Hz** |
| `FanPWM 100%` | 255 | **1000 Hz** |
| `FanPWM Stop/OFF` | stop HW PWM; Soft-PWM ownership restored at 0 | — |

Persists until **Stop**, leave Laser Control, kill/fault/disable, or reset.  
**PA1** forced OFF entire time. **PA7** Scope diag stopped when FanPWM starts.  
Normal laser `SPEED_POWER_STARTUP` **0%** and `LASER_SAFETY_TIMEOUT_MS` **1000** unchanged (not a global bypass).

## What you measure (not TTL)

K-FAN1/2 = **24 V low-side MOSFET switched power** (PA0 → buffer → FET). Expect a **power PWM** waveform on the switched node — **not** clean 3.3 V logic.

### Fluke PM3082

1. Scope **ground clip only** on confirmed **board DC negative / PSU −**.  
2. Tip on **fan switched minus** (FET drain side), with a **known 24 V fan/load** plugged in (unloaded OFF may float).  
3. Use attenuation / range suitable for **24 V**.  
4. DC coupling; auto-set while a mid duty (50%) is held.  
5. Do **not** treat switched − as chassis ground.

## Safeguards retained

- Startup: laser ENA OFF, FanBench inactive, Soft-PWM fans at 0  
- Laser idle timeout path unchanged when FanBench inactive  
- Probe GPIOs not used  

## Firmware tests (this agent)

| Check | Result |
| --- | --- |
| `pio run -e STM32F103RE_creality` | SUCCESS |
| Binary version string | `2.1.16-MA-LASER-PA7-SCOPE7` |
| Menu strings present | FanPWM OFF/25/50/75/100/Stop |
| TEMP_TIMER | **7** (config) |
| Oscilloscope confirmation of 1 kHz | **User** (hardware); agent does not flash |
