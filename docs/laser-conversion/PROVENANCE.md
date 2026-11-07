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

## Authored on this branch (laser SCOPE-TEST)

- Move laser PWM from guessed **PA1** → **PA7**
- `#define Z_STOP_PIN -1` to free Z-stop for PWM while keeping BLTouch Z-homing
- Laser-only comments / frequency 1000 Hz / `SPEED_POWER_STARTUP 0`
- Version string `2.1.16-MA-LASER-PA7-SCOPE`
- `docs/laser-conversion/**`
- Build artifacts under `artifacts/laser-pa7/`

## Preserved from baseline but not “laser design”

- `CUSTOM_VERSION_FILE`, bed sensor off, preheat-before-probe off
- Makefile flash-ender targets and stamp script (macOS SD workflow leftovers)

## PDF

`ender-snapmaker-pinout` LC-02 Rev A is workshop reference only: connector numbering,
reported Snapmaker pin functions, and a dashed planning diagram. It explicitly marks
wiring as provisional and the laser as disconnected.
