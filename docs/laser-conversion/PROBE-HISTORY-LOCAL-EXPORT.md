# Local-only probe history export (2026-11-07)

Parent Mac narrow-read found branch `ender3v2-patch-pre-rebase` and orphan
SHAs **`b05485af4c`**, **`9f1ae6c9f3`** that are **not** reachable from the
rebased GitHub remote. Attached `probe-history-export` nested tars were
audited in full for 3D Touch / servo / Z-stop pin assignments.

## Mapping to remote SHAs (same dates/messages, different hashes after rebase)

| Local (pre-rebase) | Date | Message | Remote cousin |
| --- | --- | --- | --- |
| `b05485af4c…` | 2023-09-16 | `configed-for-ender3v2-bltouch-newhotend` | `a9921b2065` |
| `9f1ae6c9f3…` | 2023-10-26 | `none` | `6d031d083d` |
| branch tip `ender3v2-patch-pre-rebase` | = | **identical** to `9f1ae6c9f3` tree for Config+pins | — |

`history.txt` lists only those **two** Michael commits atop upstream Marlin;
no further local commits between them change pins.

## Exact historical MCU pin assignments (evidence-backed)

Resolved from `Marlin/src/pins/stm32f1/pins_CREALITY_V4.h` in **all three**
extracts (identical SHA256 `0c3ff948…`) plus Configuration feature flags:

| Function | MCU pin | Basis |
| --- | --- | --- |
| **Control (deploy/stow servo PWM)** | **PB0** | `SERVO0_PIN` default when `HAS_PIN_27_BOARD` / `USE_PIN_27_BOARD` **not** set. No `#define SERVO0_PIN` in Configuration.h |
| **Trigger (probe sense)** | **PB1** | `Z_MIN_PROBE_PIN` default. No Configuration override. `Z_MIN_PROBE_USES_Z_MIN_ENDSTOP_PIN` is **commented off** → sense is **not** Z-stop |
| **Z-stop endstop** | **PA7** | `Z_STOP_PIN` default in pins file. **Not** used as probe trigger in this config. SCOPE1+ frees it (`Z_STOP_PIN -1`) for laser PWM |
| Alternate SERVO0 if Pin-27 board | PC6 | Only if `HAS_PIN_27_BOARD` / `USE_PIN_27_BOARD` enabled — **never enabled** in b054 / 9f1 / pre-rebase / remote a9921 |

### Servo / BLTOUCH options (working printer era)

| Option | b054 / 9f1 / pre-rebase |
| --- | --- |
| `MOTHERBOARD` | `BOARD_CREALITY_V4` |
| `#define BLTOUCH` | enabled (protocol feature for 3D Touch clone) |
| `USE_PROBE_FOR_Z_HOMING` | enabled |
| `Z_MIN_PROBE_USES_Z_MIN_ENDSTOP_PIN` | **disabled** |
| `NOZZLE_TO_PROBE_OFFSET` | `{ -45, -9, 0 }` |
| `PREHEAT_BEFORE_PROBING` | enabled (printer era; incompatible with SCOPE laser no-hotend) |
| `NUM_SERVOS` | unset → auto **1** via probe |
| `SERVO_DELAY` | `{ 300 }` |
| `DEACTIVATE_SERVOS_AFTER_MOVE` | off |
| `BLTOUCH_HS_MODE` | true |
| `BLTOUCH_DELAY` | default 500 |

### What changed b054 → 9f1 (not pins)

- PID hotend/bed values cleaned into the `#if` blocks (MA comments)
- `FT_MOTION` + `FT_MOTION_MENU` enabled; babystep GFX overlay disabled  
- **`pins_CREALITY_V4.h` unchanged** (same hash)

## vs current SCOPE2

| Item | Working local history | SCOPE2 |
| --- | --- | --- |
| Control PB0 / Trigger PB1 | yes | **same** (no override) |
| Z-stop PA7 | endstop | freed for laser PWM (`-1`) |
| Heater PA1 | HEATER_0 | `SPINDLE_LASER_ENA_PIN` |
| `EXTRUDERS` / temps | 1 / sensor 11 | 0 / all 0 |

**No evidence-backed pin remap to restore.** Firmware historical control+trigger
remain PB0+PB1. Physical multi-wire harness may still differ from silk-screen
“BLTouch” port expectations, but **no alternate MCU pins appear in this export**.

## SCOPE3

**Not built.** SCOPE2 preserved. No firmware pin delta justified by this export.

## Intermediate commits

`history.txt` shows no additional local pin-config commits beyond b054 and 9f1.
If an **older** tree exists (before 2023-09-16 first Ender config), a narrow
export of that commit’s Configuration + pins would be needed — ask parent only
for that case.

## Artifact checksums

See `artifacts/laser-pa7/probe-history/EXPORT-CHECKSUMS.json`.
