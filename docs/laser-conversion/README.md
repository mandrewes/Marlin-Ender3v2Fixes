# Ender 3 V2 → Snapmaker Original laser (Creality V4.2.2)

**Status: SCOPE-TEST ONLY — not laser-ready.**

This directory documents the firmware prepared for Michael’s Ender 3 V2
(Creality V4.2.2 / `BOARD_CREALITY_V4` / `STM32F103RE_creality`) using the
freed two-pin Z-stop signal (**PA7**) as a PWM probe point while retaining
BLTouch Z-homing. Firmware is prepared and compiled only — **do not flash
from this agent session; do not connect or fire the laser.**

## Provenance

| Source | Role |
| --- | --- |
| Attached tar `ender-laser-current-config` | **Baseline** of CURRENT LOCAL UNCOMMITTED settings (`Configuration.h` / `_adv.h`, `Version.h`, `Makefile`, `stamp_display_version.py`). Treat as the printer config baseline — **not** the stale committed tree alone. |
| Attached PDF `ender-snapmaker-pinout` (LC-02 Rev A) | Provisional electrical / connector notes. **Not** a verified connection or full circuit. |
| Remote `bugfix-2.1.x` | Git base for this branch/PR. |

### Baseline vs remote (committed) before laser edits

The attached baseline already differed from clean `bugfix-2.1.x` in ways that
are **preserved** here and are **not** authored as laser work:

- `CUSTOM_VERSION_FILE Version.h` enabled
- `TEMP_SENSOR_BED 0`, `PREHEAT_BEFORE_PROBING` disabled
- `LASER_FEATURE` enabled (but PWM pin was the incorrect guess **PA1**)
- Local `Makefile` SD-card flash helpers (`flash-ender*`) and
  `buildroot/share/scripts/stamp_display_version.py`

Laser-authored deltas from that baseline are isolated in
`patches/laser-pa7-from-local-baseline.patch` (apply onto a tree that already
matches the attached baseline without clobbering other dirty files).

A fuller tree patch from clean `bugfix-2.1.x` is also provided:
`patches/laser-pa7-from-bugfix-2.1.x.patch`.

## What this firmware does

- **Board / env:** `BOARD_CREALITY_V4`, PlatformIO `STM32F103RE_creality`
  (unified `HAL/STM32`, variant `MARLIN_F103Rx`)
- **Laser mode only:** `LASER_FEATURE` on; `SPINDLE_FEATURE` parked
- **PWM pin:** `SPINDLE_LASER_PWM_PIN = PA7` (freed Z-stop). **Not** PA1 (heater), **not** PA0 (fan)
- **Z-stop freed:** `Z_STOP_PIN -1` so endstop init does not claim PA7; Z homing via
  `USE_PROBE_FOR_Z_HOMING` + `BLTOUCH` on **PB0** (SERVO0) / **PB1** (`Z_MIN_PROBE`)
- **UI:** `DWIN_MARLINUI_PORTRAIT`; native Laser Control menu from
  `Marlin/src/lcd/menu/menu_spindle_laser.cpp` (power, toggle, pulse ms, Fire Pulse).
  Confirmed linked in the build (`menu_spindle_laser`, `SpindleLaser::test_fire_pulse`)
  and present as UI strings in the `.bin`
- **Safety knobs:** `LASER_SAFETY_TIMEOUT_MS 1000`, `SPEED_POWER_STARTUP 0`,
  cutter init drives PWM to OFF duty before enabling output
- **Extruder/hotend:** Keep `EXTRUDERS 1` + `TEMP_SENSOR_0 998` (baseline-sane).
  `EXTRUDERS 0` was tried and rejected — it breaks `LOGICAL_AXES` / relative-mode
  array sizes across this config. Heater FET PA1 remains in the pins map but is
  **not** the laser output

## Timer / channel mapping (verified in this tree)

Creality env flags: `-DSS_TIMER=4 -DTIMER_SERVO=TIM5`. Unified STM32 HAL defaults
for F103: **STEP = TIM4**, **TEMP = TIM2**. Variant default **TIMER_TONE = TIM3**.

`buildroot/share/PlatformIO/variants/MARLIN_F103Rx/PeripheralPins.c` `PinMap_PWM` for **PA7**:

| Entry | Status in variant | Notes |
| --- | --- | --- |
| **TIM8_CH1N** | **First active match** | Complementary channel; `HardwareTimer` sets `isComplementaryChannel` from `STM_PIN_INVERTED` and enables CCxNE / MOE paths |
| TIM1_CH1N | Secondary | Also complementary |
| TIM3_CH2 | **Commented out** | Non-complementary would be cleaner, but TIM3 is `TIMER_TONE`; not blindly remapped |

**Chosen path:** stock first match **TIM8_CH1N** — TIM8 is free of STEP/TEMP/SERVO/TONE.
No PeripheralPins remap was applied (validate usable config rather than guess a remap).

### Intended Marlin PWM semantics (MCU side only)

| Item | Setting / expectation |
| --- | --- |
| Pin | PA7 |
| Timer/channel | TIM8_CH1N (complementary) |
| Frequency | `SPINDLE_LASER_FREQUENCY 1000` Hz (`HAL_CAN_SET_PWM_FREQ` honored on STM32) |
| Duty scale | `CUTTER_POWER_UNIT PWM255` (0–255 OCR); menu/power in percent maps through cutter helpers |
| Invert | `SPINDLE_LASER_PWM_INVERT false` → OFF duty = **0** |
| Reset / startup | `SpindleLaser::init` → `set_pwm_duty(..., SPINDLE_LASER_PWM_OFF)` with OFF = 0 when invert false |
| ENA pin | None (PWM-only) |

**Blockers to laser-ready (why SCOPE-TEST ONLY):**

1. Complementary **CH1N** polarity vs CCR duty is not scope-confirmed on this board.
2. Stock Z-stop path may include edge filtering / input conditioning — driving it as an
   MCU AF push-pull PWM is untested electrically.
3. Snapmaker Original (community): reported active-low PWM (~0–4 V @ ~625 Hz on the
   *original* controller). PA7 is 3.3 V MCU CMOS — **buffer/level, pull-up, and safe
   floating/startup OFF remain unresolved.** Do not wire PA7 directly to the module.
4. Z− connector A/B (signal vs GND) still unverified per the PDF.

## Compile vs hardware validation

| Layer | This PR |
| --- | --- |
| Compile / link / size / version string | Done — see `artifacts/laser-pa7/` |
| Timer selection from PinMap | Done (source verification) |
| Scope waveform / polarity / boot-reset | **Not done** |
| Electrical interface to laser | **Not done** |
| Laser connected / fired | **Forbidden here** |

Successful compile is **not** electrical verification.

## Conditional scope procedure (laser unplugged)

1. Power off. Laser **disconnected**. Identify board DC GND with continuity; use **only**
   confirmed board DC GND for scope ground. Never short powered laser pins to ground.
2. Identify Z− connector A/B (which contact is PA7 vs GND) with the board unpowered.
3. Flash the SCOPE `.bin` only when you choose to (not part of this agent delivery).
4. Boot; confirm UI shows Laser Control with Power / Toggle / Test Pulse ms / Fire Pulse.
5. With laser still unplugged, scope PA7:
   - Idle / after boot: expect OFF duty (Marlin-side duty 0). Note actual high/low idle —
     CH1N may invert relative to a normal channel.
   - Menu: set a mid power, Toggle on/off; Fire Pulse at short ms. Confirm ~1 kHz and duty
     tracking. If polarity is inverted vs expectation, note it; laser-ready may need
     `SPINDLE_LASER_PWM_INVERT true` **after** the external interface is designed.
6. Only after a designed buffer/level circuit and OFF-bias proof: consider laser-ready
   firmware. Until then keep the SCOPE label.

## Artifacts

Under `artifacts/laser-pa7/` (and mirrored delivery copies under `/opt/cursor/artifacts/`):

- `firmware-*-SCOPE.bin` / dated `firmware-YYYYMMDD-HHMMSS.bin`
- `CHECKSUMS-AND-SIZE.txt`
- `build-STM32F103RE_creality.log`
- Patches listed above

## Apply patch later without nuking a dirty tree

If your working tree already matches the attached baseline and you only want the laser
delta:

```bash
# Review first
patch -p1 --dry-run < docs/laser-conversion/patches/laser-pa7-from-local-baseline.patch
# Then apply to Configuration*.h / Version.h only
patch -p1 < docs/laser-conversion/patches/laser-pa7-from-local-baseline.patch
```

Paths inside the local-baseline patch are relative to `Marlin/` filenames as written by
`diff -u`; adjust `-p` if needed. Prefer applying onto copies of the baseline files rather
than a mixed dirty tree.

Build:

```bash
pio run -e STM32F103RE_creality
```
