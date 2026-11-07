# Ender 3 V2 → Snapmaker Original laser (Creality V4.2.2)

**Status: SCOPE2 — PWM + ENA signal/MOSFET bench only. Laser and heater loads DISCONNECTED. Not laser-ready.**

Firmware prepared for Michael’s Ender 3 V2 (Creality V4.2.2 / `BOARD_CREALITY_V4` /
`STM32F103RE_creality`). This agent session does **not** flash hardware and does **not**
connect or fire the laser.

## 3D Touch status: UNRESOLVED

Do **not** treat SCOPE2 as a probe fix. Full history search
(`docs/laser-conversion/REGRESSION-3DTOUCH-AND-SCOPE2.md`): no custom
`SERVO0_PIN` / `Z_MIN_PROBE_PIN` in printer-era commits or the attached baseline
tar — software always falls through to Creality V4 defaults **PB0/PB1**, and
SCOPE1 already used that map when deploy failed. Firmware `#define BLTOUCH` is
the **protocol feature**, not proof of stock BLTouch header wiring. User reports
a multi-wire 3D Touch harness likely unlike stock BLTouch pins; that map is
**not in this clone**. SCOPE2 artifact kept (no SCOPE3).

## Scope of SCOPE2 (supersedes SCOPE1 + abandoned fan idea)

| Item | Decision |
| --- | --- |
| PWM | Keep **PA7** (freed Z-STOP). Remapped to **TIM3_CH2** after SCOPE1 had no observable TIM8_CH1N waveform |
| Fan route | **Forgotten** — do not use FAN0/PA0 as laser PWM |
| Enable | **PA1** = former `HEATER_0` MOSFET → `SPINDLE_LASER_ENA_PIN` (on/off only; **not** a thermally owned heater) |
| Thermal | `EXTRUDERS 0`, all `TEMP_SENSOR_* 0`, `HEATER_0_PIN`/`HEATER_BED_PIN` = `-1`, PID/thermal-protection off — removes Hotend/Nozzle/Preheat menus and thermal kill paths that owned PA1 |
| 3D Touch | Restored to working history **a9921b2065**: Marlin `BLTOUCH` + `USE_PROBE_FOR_Z_HOMING`; stock **SERVO0=PB0**, **Z_MIN_PROBE=PB1** (no Configuration pin remap) |
| Flash / hardware | **None from this agent** |

Flash identity (M115 / boot string): **`2.1.16-MA-LASER-PA7-SCOPE2`**.

## Provenance

| Source | Role |
| --- | --- |
| Attached tar `ender-laser-current-config` | **Baseline** of CURRENT LOCAL settings. Treat as printer config baseline — **not** the stale committed tree alone |
| Attached PDF `ender-snapmaker-pinout` (LC-02 Rev A) | Provisional connector notes. **Not** a verified connection |
| Remote `bugfix-2.1.x` | Git base for this branch/PR |
| Commit `a9921b2065` | Last known working 3D Touch / BLTouch config reference |

### Preserved unrelated baseline edits (not laser design)

- `CUSTOM_VERSION_FILE Version.h`
- Local `Makefile` SD-card helpers (`flash-ender*`) and `stamp_display_version.py`
- Other non-laser dirty-tree preferences carried from the attached baseline

Laser-authored deltas from that baseline:
`patches/laser-pa7-from-local-baseline.patch`.

Fuller tree patch from clean `bugfix-2.1.x`:
`patches/laser-pa7-from-bugfix-2.1.x.patch`.

## Pin map (this board / this config)

| Function | Pin | Source of truth |
| --- | --- | --- |
| Laser PWM | **PA7** | `SPINDLE_LASER_PWM_PIN`; stock `Z_STOP_PIN` overridden to `-1` |
| Laser enable | **PA1** | `SPINDLE_LASER_ENA_PIN`; stock `HEATER_0_PIN` in `pins_CREALITY_V4.h` is PA1; config forces `HEATER_0_PIN -1` so temperature.cpp cannot own it |
| Bed heater MOSFET | PA2 | Forced `HEATER_BED_PIN -1` (unused) |
| Fan soft-PWM | PA0 | Left as FAN0; **not** laser path |
| 3D Touch servo | PB0 | Stock `SERVO0_PIN` (`pins_CREALITY_V4.h`) |
| 3D Touch sense | PB1 | Stock `Z_MIN_PROBE_PIN` |

## Firmware behavior (MCU side)

| Item | Setting |
| --- | --- |
| Mode | `LASER_FEATURE` only (`SPINDLE_FEATURE` parked) |
| ENA active | `SPINDLE_LASER_ACTIVE_STATE HIGH` (matches non-inverting HEATER_0 gate drive: MOSFET ON when PA1 HIGH) |
| ENA boot / M5 / safety / disable | Writes `!ACTIVE_STATE` (LOW) — firmware OFF intent |
| PWM | `SPINDLE_LASER_PWM_INVERT false`, `SPINDLE_LASER_FREQUENCY 1000`, `CUTTER_POWER_UNIT PWM255`, `SPEED_POWER_STARTUP 0` |
| Timer | **TIM3_CH2** first match in `MARLIN_F103Rx` `PinMap_PWM` (TIM8_CH1N demoted after SCOPE1 miss). TIM3 is also `TIMER_TONE` — tone use during laser PWM may conflict; acceptable for SCOPE |
| Safety timeout | `LASER_SAFETY_TIMEOUT_MS 1000` |
| UI | `DWIN_MARLINUI_PORTRAIT` + native Laser Control (`menu_spindle_laser.cpp`) |
| Power-loss recovery | Disabled (EXTRUDERS 0 breaks PLR `flow_percentage` path) |

### ENA / PWM sequencing (Marlin `SpindleLaser`)

- `init()`: ENA → OFF, then PWM frequency + OFF duty
- `set_ocr(ocr)`: ENA → ON, then set PWM duty
- `ocr_off()`: PWM → 0, then ENA → OFF
- Menu Toggle / Fire Pulse / M3 / M5 / safety timeout all route through these helpers as far as stock Marlin allows

**Firmware does not guarantee pin state during NRST, bootloader, or watchdog before `SpindleLaser::init`.** Measure PA1/PA7 during reset if fail-safe OFF matters.

## Electrical HOLD POINT — former heater as “power enable”

**Honest constraint (do not skip):**

Creality V4 heater output is a **low-side switch**:

- Heater **+** terminal is typically **constant +24 V**
- Heater **−** is the MOSFET **sink** (floats when OFF, pulls toward GND when ON)

That is **not** an isolated high-side power supply or a guaranteed physical power cutoff.

### Why “wire laser module power across the heater header” is unsafe as enable

1. If the Snapmaker (or any module) ties its power negative / signal return to **board GND** through the PWM interface or a shared ground, current can **bypass** the low-side MOSFET (backfeed / return path), so “ENA OFF” may still leave the module partially powered or improperly referenced.
2. Driving the heater **−** as “enable” while also sharing GND with the control interface is **not** an independent power enable — it is a conductive ground-side switch that can be shorted around by common grounds.
3. User observation of ~24 V when heater control was on confirms the FET can switch the heater circuit; it does **not** prove the Snapmaker module’s power architecture is compatible with that header.

### Proposal (hold point — not implemented here)

| Approach | Verdict |
| --- | --- |
| Use PA1 low-side heater MOSFET alone as laser **power** enable with shared GND | **Reject** as guaranteed cutoff |
| PA1 as **signal** / opto / gate drive into a **dedicated high-side switch or relay** that interrupts module **+** supply, with module GND common only where intentional | **Preferred** next hardware step |
| SCOPE2 use of PA1 | Treat as **scope/logic/MOSFET gate observation only** with **heater load and laser disconnected** |

This firmware targets the *desired* behavior (laser ON also asserts former-heater output) **as a GPIO/MOSFET control signal**. It does **not** claim physical power cutoff if grounds are shared.

## Timer / channel history

| Build | PA7 map | Result |
| --- | --- | --- |
| SCOPE1 | Stock first match **TIM8_CH1N** | User reported **no PA7 waveform**; heater menus still present (`EXTRUDERS 1` + dummy thermistor) |
| SCOPE2 | **TIM3_CH2** preferred; TIM8_CH1N commented | Compile OK; waveform TBD on user’s scope |

Creality env: `-DSS_TIMER=4 -DTIMER_SERVO=TIM5`. HAL F103 defaults: STEP=TIM4, TEMP=TIM2, TONE=TIM3.

## Compile vs hardware validation

| Layer | SCOPE2 |
| --- | --- |
| Compile / link / size / version | Done — `artifacts/laser-pa7/` |
| Strings: Laser Control; no Hotend/Nozzle/Preheat | Done |
| EXTRUDER_COUNT:0 in M115 | Done |
| Scope waveform PA7 / PA1 polarity / boot bias | **User / not done here** |
| High-side power interface | **HOLD POINT — not designed/built here** |
| Laser or heater connected | **Forbidden for this SCOPE** |

## Conditional SCOPE procedure (loads disconnected)

1. Power off. **Laser disconnected. Heater cartridge disconnected from heater terminals.**
2. Identify board DC GND; use only confirmed board DC GND for scope ground.
3. Flash SCOPE2 `.bin` only when **you** choose (not this agent).
4. Boot: M115 / About should show `…LASER-PA7-SCOPE2`. UI: Laser Control present; **no** Hotend/Nozzle/Preheat.
5. Confirm 3D Touch deploy/stow (PB0/PB1 path) without thermal errors.
6. Scope **PA7**: idle OFF duty; Toggle / mid power / Fire Pulse → ~1 kHz duty tracking.
7. Scope **PA1** (heater MOSFET gate / header behavior with **no load**): LOW at boot/M5; HIGH when laser enabled. Do **not** treat header voltage as proof of safe module powering.
8. Do not connect module power across the heater header until a high-side / relay interface is designed.

## Artifacts

Under `artifacts/laser-pa7/` (delivery copies under `/opt/cursor/artifacts/`):

- `firmware-ender3v2-v422-laser-pa7-SCOPE2.bin` (+ dated twin)
- `CHECKSUMS-AND-SIZE.txt`
- `build-STM32F103RE_creality.log`
- Patches under `docs/laser-conversion/patches/`

## Build

```bash
pio run -e STM32F103RE_creality
```

Creality bootloader: rename/copy per board convention before SD flash (user-side only).
