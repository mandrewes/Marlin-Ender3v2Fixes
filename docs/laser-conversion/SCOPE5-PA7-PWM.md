# SCOPE5 — Scope diag stays ON until Stop (PA1 forced OFF)

> **Follow-on:** SCOPE6 abandons Z for laser hookup after Michael’s RC-consistent measurements and adds **FanBench** on PA0 — see `SCOPE6-FAN-BENCH.md`.

**Version:** `2.1.16-MA-LASER-PA7-SCOPE5`  
**Probe:** unchanged (PB0/PB1). **No flash / no SD staging by this agent.**

Laser + heater loads must stay **physically disconnected**.

SCOPE5 is identifiable vs SCOPE4: M115 / About shows `…LASER-PA7-SCOPE5`. SCOPE4’s **5 s auto-OFF** is removed so you can set DC coupling / timebase without racing the timer.

## Normal laser safety (unchanged)

| Parameter | Value |
| --- | --- |
| `SPEED_POWER_STARTUP` | **0%** |
| `LASER_SAFETY_TIMEOUT_MS` | **1000** |
| Idle cutoff | `safety_timeout_cut()` → OCR 0 + PA1 OFF + UI OFF |

Scope diag does **not** disable the idle timeout globally. While diag is active, PA1 is forced OFF and normal cutter `apply_power` / `set_enabled` are rejected.

## Laser Control → Scope menu actions

| Menu label | Action | Off semantics |
| --- | --- | --- |
| `Scope LOW untilOFF` | GPIO LOW on PA7; PA1 forced OFF | Stays until OFF path |
| `Scope HIGH untilOFF` | GPIO HIGH on PA7; PA1 forced OFF | Stays until OFF path |
| `ScopePWM 1k50% ON` | TIM3_CH2 1 kHz, OCR 128 (~50%); PA1 forced OFF | Stays until OFF path |
| `ScopePWM Stop/OFF` | Explicit stop: PA7 OFF duty, PA1 OFF, diag cleared | Immediate |

**Also stops (safe cancel):** leave Laser Control menu, `disable()` / `kill()`, cutter fault / ERROR mode, MCU reset.

**While diag active:** Toggle / Fire Pulse / M3-style enable cannot assert PA1 or take over PA7 (`set_enabled` forced false; `apply_power` no-ops).

**No** automatic signal at boot. **No** laser-connected use.

## Z-header circuit (unchanged docs)

See `SCOPE4-PA7-PWM.md` for R13/R6/C13 schematic notes, idealized levels, and “do not wire laser to Z header.” Research diagrams/docs retained.

## Artifacts

`artifacts/laser-pa7/firmware-ender3v2-v422-laser-pa7-SCOPE5.bin` + `CHECKSUMS-SCOPE5.txt`.  
**Wait for parent before SD staging / flash.**
