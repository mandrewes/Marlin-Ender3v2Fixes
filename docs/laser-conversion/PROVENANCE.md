# Provenance notes

## Attached baseline tar (authoritative local config)

Files taken from `ender-laser-current-config.tar` and used as the starting
printer configuration for this work:

- `Marlin/Configuration.h`
- `Marlin/Configuration_adv.h`
- `Marlin/Version.h`
- `Makefile` (includes local `flash-ender*` helpers — preserved, not executed here)
- `buildroot/share/scripts/stamp_display_version.py` (local helper — preserved)
- `platformio.ini` (identical to remote in the tar)

## Authored on this branch

### SCOPE1 (superseded for runtime, retained as history)

- Move laser PWM from guessed **PA1** → **PA7**
- `Z_STOP_PIN -1`; BLTouch Z-homing kept
- Version `2.1.16-MA-LASER-PA7-SCOPE`
- Still had `EXTRUDERS 1` + dummy thermistor → Hotend menus / thermal path

### SCOPE2 (current)

- PA7 PWM remapped **TIM8_CH1N → TIM3_CH2** in `MARLIN_F103Rx/PeripheralPins.c`
- **PA1** = `SPINDLE_LASER_ENA_PIN` (`ACTIVE_STATE HIGH`); `HEATER_0_PIN -1`
- `EXTRUDERS 0`; all temp sensors 0; bed heater pin unused; thermal protection / PID off
- Trim XYZ-only arrays (`AXIS_RELATIVE_MODES`, `DEFAULT_AXIS_STEPS_PER_UNIT`, `MANUAL_FEEDRATE`, …)
- Disable `POWER_LOSS_RECOVERY` (EXTRUDERS 0 / `flow_percentage`)
- Version `2.1.16-MA-LASER-PA7-SCOPE2`
- Docs: low-side MOSFET electrical HOLD POINT; fan route abandoned
- 3D Touch: align with working history `a9921b2065` (stock PB0/PB1, no pin remap)

## Preserved from baseline but not “laser design”

- `CUSTOM_VERSION_FILE`, local Makefile flash-ender targets, stamp script

## PDF

`ender-snapmaker-pinout` LC-02 Rev A is workshop reference only: connector numbering,
reported Snapmaker pin functions, and a dashed planning diagram. It explicitly marks
wiring as provisional and the laser as disconnected.

## Fan idea

User briefly asked about FAN PWM as an alternative; **superseded**: forget fans; keep PA7 PWM.

## 3D Touch regression note

See `REGRESSION-3DTOUCH-AND-SCOPE2.md` and `PROBE-HISTORY-LOCAL-EXPORT.md`.
Remote: `a9921b2065` → `6d031d083d` → … → SCOPE2.
Local-only export checked: `b05485af4c` / `9f1ae6c9f3` / `ender3v2-patch-pre-rebase`
→ same MCU map **PB0 control, PB1 trigger, PA7 Z-stop unused for probe**.
No SCOPE3 (nothing different to restore). Physical harness diagram still absent.
