# Fan → 5 V TTL transistor adapter (Creality V4.2.2 / PA0)

**Design only — not laser-ready. Laser and heaters stay disconnected for bench tests. No M3 / laser fire without Michael watching.**

Michael chose **fan output + discrete transistors** (not 4N35 opto — prior optos felt slow/sketchy at fast PWM).

## Plain summary

The V4.2.2 **K-FAN1 / K-FAN2** sockets are **24 V power**, not TTL. `PA0` drives a **low-side N-MOSFET**: **FAN+** stays at ~24 V; **FAN−** is the switched drain. Both K-FAN sockets **share PA0**.

1. Put a **2.2 kΩ 1 W** resistor (**Rload**) from **FAN+ to FAN−**. That **pulls the drain up** so OFF≈24 V and ON≈0 V at FAN− vs board DC− — fixing/testing the “stuck near 24 V with tiny ripple” case when there is no solid resistive path (a brushless fan can look inductive/capacitive and hide edges).
2. Sense FAN− with a **47 k / 10 k** divider into a **2N3904** inverter (**Q1**).
3. Pull Q1’s collector up to **+5 V** through **1 kΩ**, and force a **10 kΩ pull-down** on the output so the laser TTL input is **LOW** when the board is off, booting, or the fan output is OFF.
4. Result: **fan ON → OUT ≈ 5 V**, **fan OFF → OUT ≈ 0 V** (non-inverting vs fan/laser “on”). A **3.3 V** rail instead of 5 V gives the same logic levels on 3.3 V if the module wants that.

```text
FAN+ (24 V) ──┬── Rload 2.2k 1W ──┬── FAN− (switched)
              │                   │
              │                   ├── R1 47k ──●── R2 10k ── GND
              │                   │            │
              │                   │            Rb 4.7k
              │                   │            │
              │                   │            B Q1 2N3904
              │                   │           C/E
              │                   │            │
         (PSU)                   GND ←──────── E
                                               C ──●── OUT ── Rs 220Ω ──► laser TTL
                                                   │
                                          Rc 1k ←──●──→ Rpd 10k → GND
                                                   │
                                                  +5 V (buck from 24 V / DC−)
```

## Why Rload (and SCOPE8)

SCOPE8 menus produced **duty-responsive ~1 kHz** activity but the scope often sat **near 24 V** with only **tens of mV** of modulation. That is consistent with:

- tip/ground on an **always-hot** node, and/or  
- **FAN− unloaded / fan electronics** holding the node up so it never makes a clean **24 V ↔ 0 V** swing.

**Rload from FAN+ → FAN− is a drain pull-UP / defined load**, not a pull-down to ground. With the MOSFET OFF, FAN− rises to ~24 V through Rload; with the MOSFET ON, FAN− is ~0 V. That both **tests** the board switch and **feeds** the transistor stage a full-swing waveform.

| Rload | Current @ 24 V ON | Dissipation | Note |
| --- | --- | --- | --- |
| **2.2 kΩ 1 W (preferred)** | ≈ **10.9 mA** | ≈ **0.26 W** | Comfortable on a 1 W body |
| 1.0 kΩ 2 W (optional) | ≈ 24 mA | ≈ 0.58 W | Stronger edges; runs warmer |

If a real fan remains in parallel, add **Dfly** (UF4007 / Schottky) **cathode to FAN+**, anode to FAN−, for inductive kick. For the first bench, use **Rload only** (no fan, no laser).

## 5 V supply (documented vs preferred)

| Source | In board-reference? | Use for this adapter? |
| --- | --- | --- |
| **External 5 V buck** from PSU/**FAN+** and **DC−** | Always valid engineering choice | **Preferred** — does not steal probe/display |
| **BL_T / probe** “5 V supply” | Yes — inventory §15 | **Occupied** by working 3D Touch; **do not** reassign; contact order **not** asserted |
| **Display / EXP** “5 V display supply” | Yes — inventory §18 | **Occupied** by display; ribbon pin order **not** asserted |

**Never invent physical pin order** for K-FAN, BL_T, or EXP. Identify + / − / 5 V from silk and a meter with power off where possible.

**3.3 V option:** feed **Vcc = 3.3 V** instead of 5 V (same resistor set). OUT becomes ~0 / ~3.3 V. Trivial if the laser TTL is 3.3 V-tolerant / expects 3.3 V logic.

## Polarity / fail-safe

| Condition | FAN− vs DC− | Q1 | OUT |
| --- | --- | --- | --- |
| Fan PWM / GPIO **ON** (MOSFET on) | ≈ 0 V | OFF | ≈ **5 V** (laser ON) |
| Fan **OFF** | ≈ 24 V (via Rload) | ON | ≈ **0 V** (laser OFF) |
| Board unpowered / no 5 V | — | — | **LOW** via **Rpd** |
| Common ground | 24 V DC−, buck GND, adapter GND, laser GND | | Required |

The board MOSFET already inverts MCU→drain. **One** CE inverter on FAN− restores **MCU-non-inverting** “command ON → laser ON”. A **second inverter** alone would **flip** that again (wrong). If you want a buffer, use an **emitter-follower** after OUT, or skip it — Rc can drive typical TTL/PWM inputs through Rs.

## Speed estimate

With Rload = 2.2 kΩ and ~1 nF of wiring/stray on FAN−, τ ≈ **2 µs** → edges fine for **1 kHz** (SCOPE8) and comfortable through **~20 kHz** (edge budget ≪ period). The 2N3904 stage with Rc = 1 kΩ is similarly fast unloaded; laser input capacitance dominates — **Rs 220 Ω** helps damping.

| Target | Expectation |
| --- | --- |
| **1 kHz** (SCOPE8 Fan PWM) | Primary bench — full swing at FAN− and clean 0/5 V at OUT |
| **5–20 kHz** | Design aim with Rload + discrete NPN (no opto) |
| Opto (4N35) | Intentionally **not** used here |

These are **estimates**, not measured on Michael’s wiring.

## Parts list (common through-hole)

See [`BOM.csv`](BOM.csv). Core:

- Rload **2.2 kΩ 1 W**; R1 **47 k**; R2 **10 k**; Rb **4.7 k**; Rc **1 k**; Rpd **10 k**; Rs **220 Ω**
- Q1 **2N3904** (or BC547)
- Optional: Dfly **UF4007**, Cbp **100 nF**, mini **24→5 V buck**
- Stripboard / perfboard, hook-up wire

Australian-friendly generics (Jaycar / Altronics / AliExpress-style) — **no invented SKUs or prices**.

## Files

| File | Role |
| --- | --- |
| [`schematic.svg`](schematic.svg) / [`.png`](schematic.png) / [`.pdf`](schematic.pdf) | Circuit overview |
| [`perfboard.svg`](perfboard.svg) / [`.png`](perfboard.png) | Suggested wiring blocks |
| [`BOM.csv`](BOM.csv) | Components |
| [`generate_diagrams.py`](generate_diagrams.py) | Regenerates diagrams + optional plots |
| [`sim_1kHz.png`](sim_1kHz.png) / [`sim_10kHz.png`](sim_10kHz.png) | Simple model waveforms (if generated) |

## Bench procedure (resistor load only — laser disconnected)

Use SCOPE8 (or SCOPE9 FanPA0) menus. **Heaters disconnected. Laser disconnected.**

1. Power **OFF**. Fit **Rload** across verified **FAN+ / FAN−** on **one** K-FAN socket. Build adapter GND to **board DC−**. Buck 5 V from 24 V / DC−. Leave laser TTL unconnected or on a scope only.
2. Scope **ground clip only on board/PSU DC−**. Tip on **FAN−**.
3. LCD **FanPA0** (SCOPE9) or Fan PWM (SCOPE8): **GPIO OFF / PWM 0%** → expect FAN− ≈ **24 V**. **GPIO ON / 100%** → FAN− ≈ **0 V**.
4. Tip on **OUT**: OFF → ≈ **0 V**; ON → ≈ **5 V** (or 3.3 V).
5. PWM **25 / 50 / 75 / 100%** @ 1 kHz: FAN− should show inverted power PWM (more ON ⇒ more time near 0 V); OUT should show upright 0/5 V PWM with matching duty.
6. **AllOFF** before changing wiring. No M3 with a laser attached unless Michael is watching.

Connection geography: [`../scope8-test-diagram/`](../scope8-test-diagram/).

## Firmware implications (do **not** change firmware in this task)

| Topic | Note |
| --- | --- |
| Today | SCOPE8/9 already run **HW 1 kHz PWM on PA0 (TIM2_CH1)** for FanPA0 diag (`TEMP_TIMER=7`). Enough to **bench this adapter**. |
| Later laser use of PA0 | Would mean moving `SPINDLE_LASER_PWM_PIN` from **PA7** to **PA0**, and stopping use of PA0 as part-cooling Soft-PWM — **conflicts** with normal fan control and with any remaining Soft-PWM ownership. TIM2 would stay the PWM timer for PA0; PA7/TIM3 could be freed or left unused. |
| ENA | Keep a defined OFF at boot; PA1 ENA path is separate and still a 24 V MOSFET domain. |
| This commit | **Documentation / design only** — no Marlin edits, no build. |

## Uncertainties

- Exact K-FAN connector left/right polarity on Michael’s silk — **verify**, do not guess from community photos.
- Laser module Vih/Vil, max frequency, and whether it wants **5 V or 3.3 V** active-high — confirm on the module before calling it done.
- Snapmaker active-low ~4 V path remains **parked**; this adapter targets a **conventional active-high TTL/PWM** input.
- Simulation plots are a **simple model**, not SPICE of the Creality gate driver.

## Safety

- No heater load; no laser connected for Rload/transistor bring-up.  
- Scope earth/ground only on DC− — never on FAN−.  
- No unattended M3 / laser tests.
