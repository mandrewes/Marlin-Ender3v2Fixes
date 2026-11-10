# Regression investigation: 3D Touch + SCOPE1→SCOPE2 (software evidence)

**Verdict on 3D Touch: UNRESOLVED (hardware/deploy).**  
Firmware historical pin map is now confirmed from **local-only** pre-rebase
export (`b05485af4c`, `9f1ae6c9f3`, branch `ender3v2-patch-pre-rebase`) as well
as the rebased remote: **control PB0, trigger PB1, Z-stop PA7 unused for probe**.
That still does **not** prove the physical multi-wire harness, and SCOPE1 already
used the same software pins when deploy failed — so this is **not** a firmware
pin fix.

**SCOPE2 artifact preserved** (no SCOPE3 — nothing different to restore for pins).
See also `PROBE-HISTORY-LOCAL-EXPORT.md`.

---

## 1. Distinguish protocol feature vs physical header

| Layer | What it is |
| --- | --- |
| Marlin `#define BLTOUCH` | **Protocol / servo-emulation feature** for BLTouch *and* many 3D Touch clones (deploy/stow pulse on a servo pin + open-collector sense). Enabling it does **not** mean the probe is a branded Antclabs BLTouch or that wires land on Creality’s “BLTouch” silk-screen port. |
| `pins_CREALITY_V4.h` defaults | MCU map if Configuration does **not** override: `SERVO0_PIN=PB0`, `Z_MIN_PROBE_PIN=PB1`, `Z_STOP_PIN=PA7` |
| Physical harness | User reports many wires; fairly sure **not** the same pins as a stock BLTouch. That harness is **not** recorded as pin `#define`s in available history (see §2). |

---

## 2. Full printer-era history search (this repo + baseline tar)

### 2.1 Commits that form the printer lineage on `mandrewes/Marlin-Ender3v2Fixes`

| Date | Commit | Notes |
| --- | --- | --- |
| 2023-09-16 | `a9921b2065` | `configed-for-ender3v2-bltouch-newhotend` — first fork printer config |
| 2023-10-26 | `6d031d083d` | `none` — dummy thermistor / plans; adds `plans/changes.md` |
| 2026-10-05 | `dbca38abe8` | `no-hotend: dummy thermistor for laser/CNC conversion` |
| 2026-10-05 | `552ddd128e` | DWIN_SET / `ENDSTOP_INTERRUPTS_FEATURE` |
| 2026-11-07 | `6e8a3cc345` | **SCOPE1** — free `Z_STOP_PIN=-1` (PA7→laser PWM) |
| 2026-11-07 | `8aaf5f58df` | **SCOPE2** — PA1 ENA, EXTRUDERS 0, TIM3_CH2 |

Also present on GitHub remote: only `bugfix-2.1.x` and `cursor/ender-v422-laser-pa7-ba36`.  
`plans/changes.md` cites older SHAs `b05485af4c`, `9f1ae6c9f3` — **not objects in this clone** (`git cat-file` fails). Cannot audit those trees here.

### 2.2 Active pin overrides in `Configuration.h` / `_adv.h`

Searched every printer-era commit for active:

`#define SERVO0_PIN`, `#define Z_MIN_PROBE_PIN`, `#define NUM_SERVOS`, `#define Z_STOP_PIN`

| Commit | `SERVO0_PIN` override | `Z_MIN_PROBE_PIN` override | `NUM_SERVOS` | `Z_STOP_PIN` |
| --- | --- | --- | --- | --- |
| a9921 | **none** | **none** | none (auto) | none → **PA7** from pins file |
| 6d031d | none | none | none | none → PA7 |
| dbca38 | none | none | none | none → PA7 |
| 552ddd | none | none | none | none → PA7 |
| SCOPE1 | none | none | none | **-1** (free PA7) |
| SCOPE2 | none | none | none | **-1** |
| baseline tar `/tmp/ender-laser-baseline` | none | none | none | none → PA7 |

`USE_PIN_27_BOARD` is **commented out** in all of the above → `SERVO0` stays **PB0**, not PC6.

**Result: committed + attached-baseline history contains NO custom probe pin map.**  
Old→new pin map for servo/sense: **unchanged defaults PB0/PB1** across the working printer era through SCOPE2.  
The only probe-*adjacent* MCU pin change in firmware is **SCOPE1+: `Z_STOP_PIN -1`** (releases **PA7** from endstop init for laser PWM). That does **not** by itself reassign servo/sense in software.

### 2.3 What *did* change for probing (feature flags, not pins)

From `a9921` / `plans/changes.md` (documents BLTouch **feature**, offset `{ -45, -9, 0 }`):

| Item | Working printer (a9921 / plans) | SCOPE1 | SCOPE2 |
| --- | --- | --- | --- |
| `#define BLTOUCH` | on | on | on |
| `USE_PROBE_FOR_Z_HOMING` | on | on | on |
| `Z_MIN_PROBE_USES_Z_MIN_ENDSTOP_PIN` | **off** | off | off |
| `PREHEAT_BEFORE_PROBING` | **on** (120/50) | **off** | **off** |
| `BLTOUCH_HS_MODE` | true | true | true |
| `BLTOUCH_DELAY` | default 500 (unset) | same | same |
| `DEACTIVATE_SERVOS_AFTER_MOVE` | unset/off | off | off |
| `EXTRUDERS` | 1 | 1 | **0** |
| `TEMP_SENSOR_0` | 11 (then 998 in later laser prep) | **998** | **0** |
| `PIDTEMP` + `THERMAL_PROTECTION_HOTENDS` | on | **still on** | **off** |
| `HEATER_0_PIN` | PA1 (pins file) | PA1 (temp owns FET) | **-1** (ENA owns PA1) |

`plans/changes.md` title literally says “Ender 3 V2 + **BLTouch** + New Hotend” — firmware protocol naming, not a wiring diagram.

### 2.4 Local-only export (NOW CHECKED — 2026-11-07)

Attached `probe-history-export` (`b05485af4c`, `9f1ae6c9f3`, `pre-rebase`, `history.txt`):

- Orphan SHAs **exist** on Mac; tip of `ender3v2-patch-pre-rebase` ≡ `9f1ae6c9f3`
- Evidence-backed MCU map: **control PB0, trigger PB1, Z-stop PA7** (probe does not use Z-stop)
- No Configuration `SERVO0`/`Z_MIN_PROBE` overrides; Pin-27 board path never enabled
- b054→9f1: PID / FT_MOTION only; **pins_CREALITY_V4.h hash identical**
- Details: `PROBE-HISTORY-LOCAL-EXPORT.md` + `artifacts/laser-pa7/probe-history/EXPORT-CHECKSUMS.json`

### 2.5 Still not found

- No alternate firmware MCU pin pair for the multi-wire harness  
- No photo/diagram of which board header each probe wire uses  
- Snapmaker PDF is laser pinout, not probe harness  
- No local commits older than 2023-09-16 in `history.txt`

Physical wiring may still be nonstandard vs silk-screen; firmware history does not encode different pins. Ask parent for a tree **older than b054** only if one exists.

---

## 3. Why “restore stock PB0/PB1” is not a fix

1. SCOPE1 **already** compiled with default PB0/PB1 and `#define BLTOUCH` — user reported **no deploy** on that flash.  
2. SCOPE2 did **not** change servo/sense pin macros vs SCOPE1.  
3. Therefore pin-default “restoration” cannot explain a SCOPE1→working recovery and must not be reported as fixed.

Possible **software** contributors that are *not* pin remaps (unproven on hardware):

| Hypothesis | Evidence |
| --- | --- |
| Printer killed / halted by thermal path (SCOPE1: EXTRUDERS 1 + sensor 998 + PID + thermal protection still owning PA1) | Explains heater menu + 24 V on heater; **may** block or confuse UI/probe if printer is killed — **not proven** as deploy root cause |
| `PREHEAT_BEFORE_PROBING` removed vs a9921 | Affects G28/G29 preconditions; **should not** block a direct BLTouch deploy menu command |
| `Z_STOP_PIN -1` / PA7 PWM | Affects Z-stop endstop, **not** `SERVO0`/`Z_MIN_PROBE` in this config; only matters if **physical** sense was on Z-stop (PA7) despite firmware using PB1 — **no config evidence** |
| Servo timer vs laser PWM | Servo = **TIM5**; laser SCOPE1 = TIM8_CH1N; SCOPE2 = **TIM3_CH2**. `SPEAKER` **disabled** → HAL does not register `TIMER_TONE` ISR (gated `#if ENABLED(SPEAKER)`). TIM3 reserved as tone timer symbol but unused for beeps. **No proven TIM5 conflict** |
| `EXTRUDERS 0` → `NUM_SERVOS` | Auto `NUM_SERVOS=1` via `HAS_Z_SERVO_PROBE` / `Z_PROBE_SERVO_NR=0` — **independent of extruder count** |

---

## 4. Preprocessor / compile evidence (SCOPE2)

From `#pragma message` dump while compiling `Marlin/src/laser_scope_dump/scope_macro_dump.cpp` (build log `/tmp/scope2-dump-build.log` and `artifacts/laser-pa7/build-STM32F103RE_creality.log`):

| Macro | Resolved |
| --- | --- |
| `BLTOUCH` | ENABLED |
| `USE_PROBE_FOR_Z_HOMING` | ENABLED |
| `HAS_Z_SERVO_PROBE` | 1 |
| `HAS_SERVOS` | 1 |
| `NUM_SERVOS` | **1** |
| `Z_PROBE_SERVO_NR` | 0 |
| `BLTOUCH_DELAY` | **500** (default) |
| `DEACTIVATE_SERVOS_AFTER_MOVE` | DISABLED |
| `SPEAKER` | DISABLED |
| `EXTRUDERS` | 0 |
| `HAS_HOTEND` / `HAS_HEATED_BED` | 0 / 0 |
| `Z_STOP_PIN` / `HEATER_0_PIN` / `HEATER_BED_PIN` | **-1** / **-1** / **-1** |
| `SPINDLE_LASER_PWM_PIN` | PA7 (`PIN_A7` analog-alias encoding) |
| `SPINDLE_LASER_ENA_PIN` | PA1 |
| `SPINDLE_LASER_ACTIVE_STATE` | HIGH (0x1) |
| `SPINDLE_LASER_FREQUENCY` | **1000** |
| `TIMER_SERVO` | **TIM5** (APB1 + 0xC00) |
| `TIMER_TONE` | **TIM3** (APB1 + 0x400) — symbol only; SPEAKER off |
| `SS_TIMER` | 4 (STEP → TIM4) |
| `MF_TIMER_TEMP` | 1 (TEMP → TIM2) |

`SERVO0_PIN` / `Z_MIN_PROBE_PIN` expand to STM32duino `PIN_A8` / `PIN_A9` aliases for **PB0** / **PB1** (variant comments: digital 16 / 17). Same pins file path as a9921.

---

## 5. Why heater menus appeared on SCOPE1 (not “forgotten old flags”)

SCOPE1 **intentionally** kept (see its own Configuration comments and commit `6e8a3cc345`):

- `EXTRUDERS 1`  
- `TEMP_SENSOR_0 998` (dummy 25 °C)  
- `PIDTEMP` + `THERMAL_PROTECTION_HOTENDS` still enabled  
- **No** `HEATER_0_PIN -1` → temperature.cpp still owns **PA1**

That is sufficient for DWIN Hotend/Nozzle/Preheat UI and for driving the heater MOSFET.  
It was **not** residual a9921 “print” thermistor 11; it was the laser-prep dummy-thermistor pattern from `dbca38abe8` / baseline, left in place because SCOPE1 avoided `EXTRUDERS 0` array fallout.

SCOPE2 removes that ownership: sensors 0, EXTRUDERS 0, PID/thermal protection off, `HEATER_*_PIN -1`. Binary strings: Laser Control present; Hotend/Nozzle/Preheat **absent**.

---

## 6. EEPROM / bed / old heater settings (no blind M502)

Removing bed/hotend from firmware does **not** erase EEPROM. Old M500 data may still hold PID, preheat, Z-offset, mesh, etc.

**Recommended (user-side, after version verified) — selective, not blind factory reset:**

1. Confirm M115 / About shows `…LASER-PA7-SCOPE2`.  
2. `M503` — capture current EEPROM report (save host log).  
3. Prefer **targeted** clears over `M502`+`M500`:  
   - Temperature/PID menus should be gone; if host still sends `M301`/`M304`, ignore or avoid saving.  
   - Keep `M851` / mesh / steps / accel unless intentionally changing.  
4. Only if EEPROM CRC/size mismatch or corrupt boot prompts appear: then `M502` → re-enter calibration from `re-initialise-ender.txt` / saved M503 — **expect loss of stored machine calibration**.  
5. Do **not** M502 “just because” heaters were removed.

---

## 7. PA7 PWM TIM3_CH2 + TONE

| Item | Status |
| --- | --- |
| PinMap first match | **TIM3_CH2** (TIM8_CH1N commented after SCOPE1 no-waveform) |
| `SPINDLE_LASER_FREQUENCY` | 1000 — `HAL_STM32` `set_pwm_frequency` in `SpindleLaser::init` / `_set_ocr` |
| TONE conflict | `SPEAKER` disabled → tone timer **not** started in `timers.cpp`. TIM3 free for laser PWM for this config. If SPEAKER is later enabled, **remap laser or tone**. |
| Hardware waveform | Still **user scope** — software only asserts config |

---

## 8. PA1 ENA coverage table (software vs hardware unknown)

**Distinguish:**

- **MCU GPIO PA1** — MOSFET **gate** drive inside the board.  
- **Heater screw header** — typically **+24 V constant** and **low-side switched −**. That header is **not** “raw PA1”; an external relay/high-side switch must be designed against the **switched load circuit / gate behavior**, not treated as a logic-level PA1 breakout.

| Path | ENA (PA1) software | PWM (PA7) software | Hardware measured? |
| --- | --- | --- | --- |
| `SpindleLaser::init` | `!ACTIVE` (LOW) | OFF duty | **No** (boot/NRST bias unknown) |
| Menu Toggle (`menu_set_enabled`) | via `set_enabled` → `apply_power` / `set_ocr`/`ocr_off` | yes | No |
| Fire Pulse (`test_fire_pulse`) | on then off | pulse | No |
| M3 | enable + `apply_power` | yes | No |
| M5 | `apply_power(0)` + `set_enabled(false)` | off | No |
| Safety timeout | `apply_power(0)` in `temperature.cpp` | off | No |
| `cutter.kill()` / disable | off | off | No |
| Thermal error (SCOPE2) | no hotend path | — | N/A |
| Watchdog / reset / bootloader | **not guaranteed** | **not guaranteed** | Must measure |

`set_ocr`: ENA HIGH then PWM. `ocr_off`: PWM 0 then ENA LOW.

---

## 9. Minimal safe user checks (loads disconnected)

After flashing **only if you choose** (agent does not flash):

1. Verify `LASER-PA7-SCOPE2` / `EXTRUDER_COUNT:0`; no Hotend menu.  
2. **Laser + heater cartridge disconnected.**  
3. Probe deploy from DWIN BLTouch/Deploy (or `M280 P0 S10` / stow `S90` per Marlin BLTouch angles) — watch **physical** pin motion.  
4. Scope/meter **SERVO0 connector** (expected PB0 if stock port) during deploy — pulse present?  
5. Scope **Z-stop PA7** only for laser PWM tests; do not assume it is probe sense.  
6. If deploy pulse is absent on PB0 but present elsewhere on the harness, **physical wiring ≠ firmware map** — capture which board header each wire uses; supply that map before any firmware pin change.  
7. If pulse is present on PB0 but probe dead: probe power/5V/GND/wiring fault — not solved in firmware here.

---

## 10. Bottom line

| Question | Answer |
| --- | --- |
| Is 3D Touch fixed in SCOPE2? | **No — unresolved** |
| Did history show custom pins? | **Not in this repo, baseline tar, or available branches** |
| SCOPE1 vs SCOPE2 probe pins? | **Identical software defaults (PB0/PB1)** |
| Concrete servo init difference a9921→SCOPE2? | Same BLTOUCH/servo stack; differences are thermal/extruder/Z_STOP/laser timers — **none proven as deploy fix** |
| Need from parent? | Only if older **local** configs/branches exist outside this clone documenting the multi-wire pin map |

No rewiring prescription. No firmware pin “fix” without that evidence. SCOPE2 binary kept.
