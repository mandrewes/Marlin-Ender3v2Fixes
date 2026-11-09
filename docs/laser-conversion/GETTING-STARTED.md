# Getting started again — Ender 3 V2 laser conversion

**Parked handoff — 2026-11-08 (Australia/Melbourne).**  
**Read this first** when you return. Full dated snapshot: [`STATE-2026-11-08.md`](STATE-2026-11-08.md).

## Where we are (one screen)

| Item | Status |
| --- | --- |
| Goal | Accessible **measurable** power/logic PWM toward a future laser; **Snapmaker module compatibility parked** |
| Board | Creality **V4.2.2** / STM32F103RE / Marlin branch `cursor/ender-v422-laser-pa7-ba36` / PR [#1](https://github.com/mandrewes/Marlin-Ender3v2Fixes/pull/1) |
| **On SD now** | **SCOPE8** prepared as `LASR0008.bin` — **153660** bytes, SHA256 `29225f06d93c73844685d0234b26ef8f1bc5c9f478ab472815eae9bb20d4a354` (checksum verified, card ejected). Old `LASR0005` backed up at `/Users/mandrewes/Ender-SD-backups/ENDER-20261108-150352`. Card: **4 GB FAT32 / 4096-byte clusters** preserved. |
| **SCOPE9** | Built & pushed in repo: **154532** bytes, SHA256 `e659bda89b0891dda06cb4f75bd5e974721042bdc3f8978b10c9f57a8de0a651` — artifact `artifacts/laser-pa7/firmware-ender3v2-v422-laser-pa7-SCOPE9.bin`. **Not** staged / flashed / physically verified. Proposed future SD name only: `LASR0009.bin` (re-identify the 4 GB card; **do not** touch the 960 GB Verbatim drive or format). |
| Probe | Working **3D Touch PB0/PB1** — do not change |
| Loads | **No heater / no laser** connected for these benches |
| PA7 Z header | Measured HIGH≈**3.3 V**, LOW≈**1.6 V**, rounded ~**844 mVpp** @ ~1 kHz → **R13 10k / R6 pull-up / C13** (not a bad cable). Abandoned for direct laser TTL. |
| PA0 / PA2 | User saw ~1 kHz duty-responsive **~50–58 mV** modulation, **near 24 V, no full swing**; 24 V fan spinning; tip/ground endpoints **not independently confirmed**. **No established root cause.** |
| Firmware bugs fixed in SCOPE9 | **D1–D5** ownership/cleanup/GPIO diag (see state doc). These do **not** claim to have “fixed” the no-swing measurement. |
| Interface design (new) | Discrete **fan→5 V TTL** adapter designed — [`fan-transistor-adapter/`](fan-transistor-adapter/). Not built/verified on hardware yet. Opto path declined. |

## Resume checklist (ordered — do this next session)

1. Open [`STATE-2026-11-08.md`](STATE-2026-11-08.md) and this file; confirm branch/PR still open (no merge required for parking).
2. Confirm which firmware is actually on the printer (M115 / LCD About). **Do not assume** reports after SCOPE8 staging were SCOPE8 without readback.
3. If advancing firmware: stage **SCOPE9** only as proposed `LASR0009.bin` on the **known 4 GB** card after re-identifying it — never the 960 GB Verbatim; no format; no Makefile flash from a dirty local tree.
4. **First electrical checks after SCOPE9 menus:** static **GPIO OFF then GPIO ON** on **FanPA0**, then **BedPA2** (AllOFF between). Tip on **FAN− / BED−**; ground **only** on board/PSU **DC−**. Expect with load: OFF≈24 V / ON≈0 V at tip (inverted drain). Across the load: OFF≈0 V / ON≈24 V.
5. Only after static endpoints make sense: PWM 25/50/75/100% @ 1 kHz. DC coupling, 24 V–rated probe/range. Do **not** burn another session on endless scope auto-set loops.
6. Leave **PB0/PB1** alone; keep **PA1 OFF** during diag (firmware forces this). No heater/laser load.
7. Hardware interface research notes: [`hw-review/requested-links/reddit-redsail-findings.md`](hw-review/requested-links/reddit-redsail-findings.md). Bench location diagrams: [`scope8-test-diagram/`](scope8-test-diagram/).
8. **Next hardware design (chosen path):** discrete **fan → 5 V TTL transistor adapter** — [`fan-transistor-adapter/`](fan-transistor-adapter/) (Rload + 2N3904; not opto). Start with the **overall wiring** picture: [`fan-transistor-adapter/wiring.png`](fan-transistor-adapter/wiring.png). Bench with Rload only first; laser disconnected.

## Key links (in-repo)

| Doc | Why |
| --- | --- |
| [`STATE-2026-11-08.md`](STATE-2026-11-08.md) | Full dated state, D1–D5, SD facts, wrong assumptions corrected |
| [`fan-transistor-adapter/`](fan-transistor-adapter/) | **Fan PA0 → 5 V logic PWM** discrete adapter (chosen path) |
| [`SCOPE9-POWER-DIAG.md`](SCOPE9-POWER-DIAG.md) | SCOPE9 menus, timer map, confirmed vs hypothesis |
| [`SCOPE8-POWER-PWM.md`](SCOPE8-POWER-PWM.md) | What was on the SD as LASR0008 |
| [`board-reference/`](board-reference/) | All **22** connectors, diagrams, schematic crops — **retain** |
| [`board-reference/all-connectors/inventory.md`](board-reference/all-connectors/inventory.md) | Interfaces 01–22 |
| [`board-reference/pwm-route-assessment.md`](board-reference/pwm-route-assessment.md) | Route recommendation (updated for TEMP→TIM7) |
| [`scope8-test-diagram/`](scope8-test-diagram/) | Where to clip / how to wire (community board photo) |
| [`hw-review/requested-links/reddit-redsail-findings.md`](hw-review/requested-links/reddit-redsail-findings.md) | Reddit + Fiorillo + RedSail research |
| [`README.md`](README.md) | Conversion overview (historical layers kept) |

## Hard stops

- No flash / SD staging / remote energize from the cloud agent in this parking commit.
- Do not invent pin polarity from the community photo boxes.
- Do not blame the user or claim firmware fixed unverified hardware.
- Snapmaker active-low ~4 V path remains **parked** until a designed interface exists.
