# SCOPE4 — PA7 Z-header circuit limits + bounded Scope diag

> **Superseded for diag timing by SCOPE5** (`SCOPE5-PA7-PWM.md`): SCOPE5 keeps the same LOW/HIGH/PWM modes and PA1-OFF rule but **removes the 5 s auto-OFF** so the signal stays until Stop. Keep this file for schematic / timeout research.

**Version:** `2.1.16-MA-LASER-PA7-SCOPE4`  
**Probe:** unchanged (PB0/PB1). **No flash / no rewire by this agent.**

Laser + heater loads must stay **physically disconnected** for all Scope diags.

## Schematic evidence (board-variant caution)

Reverse-engineered Creality V4.2.2 schematic (Jyers discussion / community PDF, **not a factory guarantee of every PCB spin**):

- [Creality 4.2.2 Schematic PDF](https://github.com/Jyers/Marlin/files/8791332/Creality.4.2.2.-.Schematic.28-5-22.pdf)
- Discussion: https://github.com/Jyers/Marlin/discussions/814
- Corroborating forum notes: https://www.spainlabs.com/foros/showthread.php?tid=13411

### Z-STOP / Zmin path (as drawn)

| Node | Role |
| --- | --- |
| MCU **PA7** (package pin 23) | GPIO / TIM channel |
| **R13** 10 kΩ series | MCU → header SIG |
| Header **SIG** | Z- connector signal contact |
| **R6** 10 kΩ pull-up to 3.3 V | on SIG |
| **C13** to GND | filter cap (drawing; **100 nF often assumed**, confirm on your board) |
| Other Z contact | **GND** |
| Buffer | **None** |

Heater path on the same schematic family: **low-side** MOSFET (source to GND). Metering ~24 V across heater +/− only when ENA is on is consistent with that topology. **PA1 = gate**, not a raw MCU breakout on the screw header.

### Idealized header levels (assumed 100 nF; push-pull MCU)

| MCU PA7 | Approximate header SIG |
| --- | --- |
| Steady LOW | ~**1.65 V** (3.3 V ÷ R6/R13) — **not 0 V** |
| Steady HIGH | ~**3.3 V** |
| 1 kHz 50% PWM | Rounded / raised waveform; rough band often cited ~**2.1–2.9 V** with assumed C13 |

Firmware **cannot** remove R13/R6/C13. Do **not** promise a clean square wave at the Z header. Do **not** connect a laser control input directly to that header without a designed buffer/level interface.

**PA7 is not 5 V-tolerant** (STM32F103). Keep external interfaces at 3.3 V logic.

### What the RC network does *not* explain

- Identical waveform on a **verified** board GND contact  
- No response to command / diagnostic endpoints when probing the correct SIG vs GND with DC coupling  

Those remain measurement, connector, or drive-path issues — not “software fixed the filter.”

## Timer mapping (software)

| Timer | Use on PA7 | Notes |
| --- | --- | --- |
| **TIM3_CH2** | SCOPE PinMap first match | Needs **no AFIO remap** (`AFIO_NONE`) |
| TIM8_CH1N | Demoted (SCOPE1) | Complementary channel; needs TIM8 MOE / BDTR enable — easy to get no visible edge |

## Idle safety timeout — existing (SCOPE2) vs revised (SCOPE4)

### Three distinct states (do not conflate)

| State | Meaning |
| --- | --- |
| Menu **Toggle / enable_state** | UI + virtual “laser enabled” |
| **OCR / PA7 duty** | PWM compare value |
| **PA1 ENA pin** | MOSFET **gate** drive |

### Stock Marlin timeout path (SCOPE2 behavior)

`Temperature::isr` → `cutter.apply_power(0)` while still `enabled()`:

1. `apply_power(0)` takes the PWM branch → **`set_ocr(0)`**
2. **`set_ocr` always writes `SPINDLE_LASER_ENA` ACTIVE (HIGH)** before setting duty 0  
3. **`enable_state` is not cleared** → Laser Toggle UI stays ON  
4. Timeout only ran when `last_power_applied != 0` — Toggle ON with OCR already 0 never tripped  

So after ~1 s idle: **OCR could be 0 while PA1 stayed HIGH and the menu still showed ON**. That matches “didn’t switch off in the menu / power remained on,” without claiming a complete physical power cutoff or invalidating the measurement.

### SCOPE4 revised timeout

`cutter.safety_timeout_cut()`:

- Clears **`enable_state`** (Toggle reads OFF on next menu draw)  
- Calls **`ocr_off()`** → duty 0 **and** PA1 **!ACTIVE**  
- Clears `last_power_applied` / inline enable  
- Triggers when **`enabled() || last_power_applied`** past `LASER_SAFETY_TIMEOUT_MS` (still **1000 ms**)  

**Re-enable:** explicit menu Toggle, M3, or planner paths that call `set_enabled` / `apply_power` again — not silent auto-on.

**Limits:** Still only the MOSFET **gate**. Shared module GND can bypass the FET. Not a guaranteed load disconnect.

## Normal laser safety retained (vs rejected SCOPE3)

| Parameter | SCOPE2 | Rejected SCOPE3 | SCOPE4 |
| --- | --- | --- | --- |
| `SPEED_POWER_STARTUP` | 0% | 50% | **0%** |
| `LASER_SAFETY_TIMEOUT_MS` | 1000 | 0 (disabled) | **1000** |
| Idle cutoff ENA/UI | ENA often stayed ON; UI ON | n/a | **ENA OFF + UI OFF** |
| Frequency menu min | 2000 | 500 | **500** (UI only; not a safety bypass) |

## Bounded Scope diagnostic (PA1 forced OFF)

Laser Control menu (auto-stop **5 s**; also Stop / leave menu / disable / kill / fault):

| Menu | Action |
| --- | --- |
| `Scope LOW 5s ENA0` | GPIO LOW on PA7 |
| `Scope HIGH 5s ENA0` | GPIO HIGH on PA7 |
| `ScopePWM 1k50% 5s` | TIM3_CH2 1 kHz, OCR 128 |
| `ScopePWM Stop` | Immediate OFF |

Use these to separate **steady RC endpoint levels** from **PWM rounding** — still at the **header**, not MCU pad.

## Safest path forward (discussion only — no silent rewire)

1. **Bench with SCOPE4 diags** on Z SIG vs verified GND (DC coupling). Expect non-zero LOW and rounded PWM at the header.  
2. **Do not** feed a laser module from the Z-STOP header without buffering.  
3. **Future hardware options (require explicit user approval before any pin/config change):**  
   - Buffer + level shift from a **clean MCU-side tap before R13**, or  
   - Another **verified free GPIO** (not PA7 through the endstop RC; not 5 V-stressed), or  
   - Keep PA1 as low-side ENA only with a proper PWM driver elsewhere  
4. **Probe pins stay PB0/PB1** until the user asks otherwise.

## Artifacts

See `artifacts/laser-pa7/` — `firmware-ender3v2-v422-laser-pa7-SCOPE4.bin`, `CHECKSUMS-SCOPE4.txt`.  
**Wait for parent before SD staging / flash.**
