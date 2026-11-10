# SCOPE9 — PA0 / PA2 / PA7 power-output diagnostic

**Version:** `2.1.16-MA-LASER-PA7-SCOPE9`  
**Bench / source only. Not laser-ready. Agent does not flash or SD-stage.**

Laser + bed heater **disconnected**. **3D Touch PB0/PB1** untouched. Snapmaker/module compatibility **parked** (goal = measurable MCU/MOSFET-path PWM only).

---

## Confirmed firmware defects (source evidence) → fixed in SCOPE9

| ID | Defect | Evidence | Change / why |
| --- | --- | --- | --- |
| **D1** | **AllOFF did not park PA7** | SCOPE8 menu `PWM Bench AllOFF` called only `fan_bench_stop()`, which parked PA0+PA2. Separate `scope_diag_*` state could leave PA7 live. | Unified `fan_bench_stop()` / AllOFF now parks **PA0 + PA2 + PA7** and forces PA1 OFF. |
| **D2** | **No static GPIO modes on Fan/Bed** | SCOPE8 Fan/Bed menus were PWM-only (`power_bench_set` → TIM AF). Could not separate timer/AF faults from MOSFET buffer path without a timed wait. | Added persistent **GPIO OFF** and **GPIO ON** per FanPA0 / BedPA2 (and ZPA7). |
| **D3** | **PA7 diag cancelled on Fan/Bed submenu** | `scope_diag_task` allowed only `menu_spindle_laser`. Entering FanPA0/BedPA2 submenus stopped Scope; allowlists were inconsistent across the two systems. | One `fan_bench_task` allowlist: Laser Control + FanPA0 + BedPA2 + ZPA7. |
| **D4** | **Park path remuxed AF then GPIO awkwardly** | `_power_bench_park_pin` did `set_pwm_duty(0)` then `OUT_WRITE`. PWM start also did `SET_PWM` (= `pinMode(OUTPUT)` on STM32 HAL) **before** `set_pwm_duty`, briefly forcing GPIO before AF. | Park = zero duty (if PWM-capable) then explicit `pinMode(OUTPUT)+LOW`. PWM start **omits** pre-`SET_PWM` GPIO clobber; `set_pwm_duty` → `pinmap_pinout` alone. |
| **D5** | **Soft-PWM ownership check was channel-equality only** | Skip used `power_bench_ch == FAN0`. Fine for PWM, but static GPIO on FAN0 must also suppress Soft-PWM ISR `WRITE`s or they fight GPIO HIGH every TEMP tick. | `power_diag_owns_fan0()` gates Soft-PWM whenever FanPA0 diag is live (GPIO or PWM). |

**Not claimed:** these defects are **not** proven to be the sole cause of the ~50–58 mVpp @ ~1 kHz near-24 V scope observation. They are real ownership/cleanup/diagnostic gaps fixed because they are wrong regardless.

---

## Audit results that are **not** firmware defects (honest)

| Check | Result |
| --- | --- |
| PinMap PA0 | **TIM2_CH1**, `AFIO_NONE` (`MARLIN_F103Rx/PeripheralPins.c`) |
| PinMap PA2 | **TIM2_CH3**, `AFIO_NONE` |
| PinMap PA7 | **TIM3_CH2**, `AFIO_NONE` (first match) |
| TEMP vs TIM2 | `TEMP_TIMER 7` in `Configuration.h`; ELF has `TIM7_IRQHandler` + `TIM2_IRQHandler` (PWM). STEP TIM4, SERVO TIM5 unchanged. |
| TIM2 remap | No Creality/Marlin `AFIO_TIM2_*` remap applied in this tree for V4 |
| HEATER_BED reclaim | `HEATER_BED_PIN=-1`, `TEMP_SENSOR_BED=0` → no bed Soft-PWM on PA2 |
| Duty meaning | `hal.set_pwm_duty(..., invert=false)` → OCR = pin **HIGH** fraction = load-ON for non-inverting gate drive (same family as documented HEATER_0) |
| PA7 header | Physical **10k / 10k pull-up / C13** — HIGH≈3.3 V, LOW≈1.6 V, rounded PWM expected; firmware cannot remove |

**No source defect found** that would force MOSFET drains to stay near 24 V while only emitting tens of mV of PWM. That observation remains **unconfirmed probe/setup vs hardware**, not established as a TIM/CCR bug.

---

## Hypotheses (not established)

| H | Statement | Why not “confirmed” |
| --- | --- | --- |
| **H1** | Scope tip/ground on wrong nodes (e.g. FAN+/rail vs switched −) → always ~24 V + tiny supply ripple at PWM rate | Endpoints “not confirmed” per user; matches mVpp @ 20 mV/div and flatline @ 5 V/div **if** tip is on always-hot |
| **H2** | Meter “24 V on those wires” measured FAN+ to PSU− (always 24 V) | Exact endpoints unconfirmed |
| **H3** | Fan spinning + near-24 V on switched − would contradict full MOSFET OFF; spinning is consistent with partial/full conduction **or** wrong node | Need GPIO ON/OFF drain readings under known load |
| **H4** | External laser-kit pages implying “add a pull-down” | **Not** treated as evidence for this V4.2.2 firmware path; hardware research is separate |

Reddit / RedSail links provided by user: **not verified** as V4.2.2-accurate or as proof of a Marlin bug. RedSail text references generic “fan header (D9)” AVR-style wiring — not this STM32F103RE pin map. Do not flash or wire from those pages via this agent.

---

## LCD menus (Laser Control)

| Submenu | Items |
| --- | --- |
| **FanPA0 Diag** | GPIO OFF, GPIO ON, PWM 25/50/75/100%, Stop |
| **BedPA2 Diag** | GPIO OFF, GPIO ON, PWM 25/50/75/100%, Stop |
| **ZPA7 Diag** | GPIO LOW, GPIO HIGH, PWM 25/50/75/100%, Stop |
| **PWM Bench AllOFF** | Parks PA0+PA2+PA7; PA1 OFF |

One target at a time (selecting another parks the others). Leaves Laser Control tree / kill / fault / reset → AllOFF cleanup. K-FAN1 and K-FAN2 **share PA0** (not independent).

---

## Expected static readings (**under known 24 V fan/test load**; not agent-verified)

Probe **tip = switched minus (FET drain / FAN− or BED−)**; **ground clip = board DC− / PSU− only**. Never clip to FAN−/BED−.

| Mode | MCU pin | Expect at switched − vs PSU− | Load behavior |
| --- | --- | --- | --- |
| GPIO OFF | LOW | ~24 V (MOSFET off; drain floats via load) | Fan stopped / no heat |
| GPIO ON | HIGH | ~0 V (MOSFET on) | Fan full / load ON |
| PWM 50% | AF 1 kHz | ~24 V ↔ ~0 V swing (power PWM, not TTL) | Partial |

PA7 header (no laser): GPIO HIGH ~3.3 V, GPIO LOW ~1.6 V, PWM rounded — **by design of RC**.

If GPIO ON/OFF do not move the switched node as above, problem is **probe point, wiring, or power-path hardware** — not “need another duty menu.” If GPIO works but PWM does not, investigate TIM/AF (then we have a real timer symptom).

---

## Physical test limits

- No heater / laser connected; no arbitrary high-power bed load  
- Appropriate 24 V fan/test load only  
- Scope range for **24 V**; start DC, 5–10 V/div to see full swing, then refine  
- Agent: **no flash / no SD / no remote energize**

---

## Build / artifact

| Item | Value |
| --- | --- |
| `pio run -e STM32F103RE_creality` | SUCCESS |
| Artifact | `artifacts/laser-pa7/firmware-ender3v2-v422-laser-pa7-SCOPE9.bin` |
| See | `CHECKSUMS-SCOPE9.txt`, `SCOPE9-TEST-MATRIX.md` |
