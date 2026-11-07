# SCOPE3 — PA7 PWM investigation + bench diagnostic

**Probe: STOP changing.** User fixed loose connector; leave PB0/PB1 alone.

**Version:** `2.1.16-MA-LASER-PA7-SCOPE3`

## Heater “power only when laser ON” vs low-side (no power-path change)

Creality heater header is still a **low-side** switch in the board design:

- Heater **+** is typically tied to constant **+24 V**
- Heater **−** is the MOSFET **drain** (open when OFF, sinks toward GND when ON)

Measuring **voltage between the two heater terminals**:

- FET **OFF** → − floats / little differential → ~0 V across the pair (no load path)
- FET **ON** → − pulled down → you read ~24 V across +/−

That pattern (**voltage appears only when laser/ENA is ON**) is **exactly what a low-side switch looks like** when you meter across the load terminals. It does **not** contradict low-side topology, and it does **not** by itself prove an isolated high-side supply. No firmware power-path change.

**PA1** remains the MOSFET **gate** (logic), not a raw MCU breakout on the screw header.

## PA7 / Z-STOP physical vs software

| Item | Evidence |
| --- | --- |
| MCU pin | **PA7** = `SPINDLE_LASER_PWM_PIN` |
| Arduino alias | `PIN_A7` → digitalPinHasPWM / `digitalPinToPinName` → `PA_7` |
| PinMap (SCOPE3) | First match **TIM3_CH2**, `AFIO_NONE`, AF_PP |
| Freed endstop | `Z_STOP_PIN -1` |
| `USE_Z_MIN` | Off when `USE_PROBE_FOR_Z_HOMING` and not `Z_MIN_PROBE_USES_Z_MIN_ENDSTOP_PIN` → endstop init / `ENDSTOP_INTERRUPTS` **do not** attach PA7 |
| Init order | `endstops.init()` then later `cutter.init()` → PWM AF applied after endstops |
| DWIN / RET6 | This config uses DWIN MarlinUI, not VET6 EXP3 path that also lists PA7 |

Community Creality V4.2.2 Z-STOP header is a **2-pin** connector: **signal = PA7**, other pin **GND** (silkscreen “Z”). Firmware cannot see which physical contact you probe; power-off continuity from candidate GND to board DC GND is required.

## Why SCOPE2 menu/PWM could look “dead” on the scope (software)

| Cause | Effect |
| --- | --- |
| `SPEED_POWER_STARTUP 0` + Toggle ON with Power 0% | ENA can go ON (heater responds) while **OCR stays 0** → flat line / no PWM edges |
| `LASER_SAFETY_TIMEOUT_MS 1000` | After **1 s without motion**, OCR forced to 0 while stationary bench scoping |
| Frequency menu min **2000** Hz | Editing Frequency could not select the configured **1000** Hz (jumped to ≥2 kHz) |
| Same ~2 V blob on **both** Z contacts / vs random grounds | Strongly suggests **measurement/pickup** (floating scope GND, AC coupling, probing neither solid signal nor solid GND) — **not** proof of PWM. Do not treat ~2 V as a duty average |

Hardware PWM path (TIM3_CH2 PinMap + `hal.set_pwm_duty`) is coherent in source; **not proven buggy**. Same-on-both-contacts is not explained by a pinmux that drives only PA7.

## SCOPE3 software changes

1. **Safety timeout = 0** (disabled) for continuous stationary PWM  
2. **SPEED_POWER_STARTUP = 50%** so Toggle-ON yields mid OCR when power was 0  
3. Frequency menu floor **500** Hz (was 2000)  
4. **`LASER_SCOPE_PWM_DIAG`**: Laser Control → **Scope PWM Diag**  
   - Forces **PA1 ENA OFF** always while active  
   - Drives **PA7 @ 1000 Hz, OCR 128 (~50%)** via HAL, bypassing Toggle/Power/safety  
   - Blocks normal `apply_power` / enable from asserting ENA while diag is on  

## Minimal measurements (parent / user)

Laser + heater **disconnected**. Prefer **Scope PWM Diag** ON for a known output.

1. Power **OFF**: continuity between each Z- header contact and board DC GND → identify **true GND**.  
2. Scope: **DC** coupling, probe tip on **signal** contact only, clip on **confirmed** board DC GND (not the other Z pin until proven GND).  
3. Photo of scope (timebase + coupling visible).  
4. Expect with Diag ON: ~1 kHz square (or filtered edges), ~0–3.3 V class levels at MCU/header signal — **not** a ~2 V identical blob on both pins.  
5. Diag OFF: idle near solid OFF level on signal vs GND.  
6. Optional: Toggle path with Power ≥ 50% (safety off in SCOPE3) should also show duty tracking; Frequency edit can now go to 1000 Hz.

If both contacts still show the same waveform **after** a verified GND reference and DC coupling, suspect wrong connector or open signal wire — not “PWM average.”
