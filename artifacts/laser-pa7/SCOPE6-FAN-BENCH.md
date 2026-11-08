# SCOPE6 — FanBench (PA0) after abandoning Z-STOP for laser

**Version:** `2.1.16-MA-LASER-PA7-SCOPE6`  
**Probe:** unchanged (PB0/PB1). **No flash / no SD staging / no remote energize.**

Laser + heater loads stay **physically disconnected**. **No signal wire to the Snapmaker module** without a separately designed interface.

## Z-STOP findings (closed for laser hookup)

Michael SCOPE5 measurements vs verified board DC GND:

| Measurement | Result |
| --- | --- |
| Multimeter HIGH | ~3.3 V |
| Multimeter LOW | ~1.6 V |
| Scope | ~844 mVpp, half-cycle ~492 µs, curved PWM |

Consistent with Creality V4.2.2 Zmin path (community RE schematic): **PA7 → R13 10k → SIG**, **R6 10k pull-up**, **C13** to GND; other contact GND; no buffer.  
**Decision:** do **not** use the Z socket for laser control. Keep PA7 Scope menu only as a residual reference diag.

Schematic: https://github.com/Jyers/Marlin/files/8791332/Creality.4.2.2.-.Schematic.28-5-22.pdf  
Legacy Creality-laser-on-fan account: https://www.reddit.com/r/ender3v2/comments/kq2f3l/laser_install_and_printing_from_a_mac/ (K-FAN / M106–M107; both fan headers together). Newer official Creality laser with separate controller is out of scope.

## Why SCOPE6 (not “just use Temperature → Fan”)

| Item | SCOPE5 / stock path |
| --- | --- |
| `FAN0_PIN` | **PA0** (`pins_CREALITY_V4.h`) — owned by fan Soft-PWM / M106 |
| `FAN_SOFT_PWM` | Enabled; `SOFT_PWM_SCALE 0` |
| Soft-PWM rate | TEMP ISR **1000 Hz** / 128 steps ≈ **7.8 Hz** (not 1 kHz TTL) |
| Hardware | PA0 → HC245 → MOSFET **low-side 24 V switched** load supply (not logic TTL) |
| Host | **M106 S&lt;0–255&gt; / M107** already work |
| LCD Temperature → Fan | **Absent**: `HAS_TEMPERATURE` requires hotend/bed; `EXTRUDERS 0` removes that submenu |

So host M106 alone is enough for some benches; SCOPE6 adds **LCD FanBench** (persistent until Stop) because the Temperature fan UI is missing. **Laser PWM is not remapped to PA0** (fan already owns it).

Snapmaker 200 mW 405 nm: separate **24 V** plus reported **active-low ~4 V** control — **not** compatible with direct 24 V fan-header output.

## Exact controls (Laser Control menu)

| Menu | Action |
| --- | --- |
| `FanBench OFF` | `set_fan_speed(0, 0)`; PA1 forced OFF; PA7 Scope stopped |
| `FanBench 50%` | speed **128**; PA1 OFF |
| `FanBench 100%` | speed **255**; PA1 OFF |
| `FanBench Stop/OFF` | Explicit stop (fan 0, PA1 OFF, flag clear) |

Also OFF on: leave Laser Control, kill/fault/disable, reset.  
While FanBench active: normal laser Toggle/enable rejected; PA1 held OFF.

Equivalent host (no new binary required for host-only use):

```
M106 S0
M106 S128
M106 S255
M107
```

Duty is Soft-PWM fraction of ~7.8 Hz; frequency is **not** configurable via laser Frequency menu.

## Normal laser safety (unchanged)

`SPEED_POWER_STARTUP` **0%**, `LASER_SAFETY_TIMEOUT_MS` **1000**, idle `safety_timeout_cut`. FanBench is not a global timeout bypass.

## Safe Fluke PM3082 measurement (fan header)

**Loads disconnected from any laser.** Prefer a known **24 V fan already on that header** or another appropriately rated 24 V load — MOSFET OFF may leave the switched node **floating** when unloaded; do not invent jumpers/shorts.

1. Scope ground clip **only** on confirmed **board DC negative / PSU −** — **never** on the switched fan “−” terminal as if it were chassis ground.  
2. Distinguish:  
   - **Switched − to board GND** → MOSFET drain PWM / float behavior  
   - **Across fan +/−** → supply across the pair when the FET conducts  
3. Probe attenuation / rating suitable for **24 V** class signals.  
4. Physical left/right of silkscreen K-FAN1 is **not** asserted here — identify +/− on your board before probing.  
5. Auto-set / DC coupling while FanBench **50%** or **100%** is held (until Stop).

## Safest path (discussion only)

Fan header = **24 V switched power** for Creality-style laser *power* tricks with mechanical interlocks — **not** a Snapmaker TTL control substitute. Any laser interface needs a designed buffer/driver; no silent rewire in this workstream.
