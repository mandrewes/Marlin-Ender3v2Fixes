# Laser PWM route assessment (recommendation only)

**No firmware changes, builds, wiring changes, SD staging, or hardware commands in this document.**  
**Not laser-ready.** Target module context: Snapmaker Original **200 mW 405 nm** — separately powered **24 V**, control reported **active-low ~4 V** (module input still needs verification).

Preserved constraints: working **3D Touch PB0/PB1**; Display/SD remain occupied; heater low-side **shared-GND bypass** remains an unsolved power-path problem (ENA gate ≠ guaranteed load cutoff).

## What we already know we cannot do

| Idea | Verdict |
| --- | --- |
| Direct laser control from **Z-STOP header** | **No.** RC network (R13 10k + R6 pull-up + C13); measured ~3.3 / 1.6 V and ~844 mVpp curved 1 kHz. Not clean TTL. |
| Direct PA7 (or any 3.3 V GPIO) → module with **~4 V pull-up** without a designed interface | **No.** Risk to MCU / wrong levels; PA7 not 5 V-tolerant. |
| Treat published **fan opto (active-high ~5 V)** as Snapmaker-ready | **No.** Wrong polarity/level family; archive images are examples only. |
| Invent pad left/right or unproven free pin from silkscreen alone | **No.** |

**Existing Z cable:** the *wire* can be reused mechanically to a **new** interface later; the *socket circuit* is the problem. Bad cable vs bad socket is distinguished by MCU-side vs header-side probing (already done at header).

## Candidate comparison

### A — K-FAN1/2 (PA0) Soft-PWM / FanBench (present firmware)

| Aspect | Assessment |
| --- | --- |
| Ownership | `FAN0_PIN=PA0`; K-FAN1 and K-FAN2 **share** one MOSFET return |
| Rate today | `FAN_SOFT_PWM` @ TEMP ISR 1 kHz / 128 ≈ **7.8 Hz** — too slow for engraving PWM |
| Why Soft-PWM | `FAN_SOFT_PWM_REQUIRED` on Creality V4; PinMap first match **TIM2_CH1**, while **TEMP owns TIM2** on this HAL — hardware PWM on TIM2 would fight the temperature timer |
| Alternate timer on PA0 | PinMap also lists **TIM5_CH1** (commented) — **TIM5 is SERVO** for 3D Touch → not “free” if probe stays |
| Electrical | **24 V switched power**, not logic TTL |
| Snapmaker fit | Needs a **separately designed** interface (opto/level, active-low, guaranteed OFF at boot) from 24 V switched node → ~4 V control — **engineering**, not drop-in |
| Evidence vs hypothesis | Soft-PWM rate and TIM2 conflict are **code/PinMap evidence**. Module OFF/active-low details are **reported**, not yet instrumented on *this* module |

### B — MCU-side tap **before R13** on PA7 (TIM3_CH2) + buffer

| Aspect | Assessment |
| --- | --- |
| Timer | **TIM3_CH2** is the current laser PinMap choice; STEP=TIM4, TEMP=TIM2, SERVO=TIM5 — TIM3 is available when SPEAKER/tone unused |
| Socket | Still filtered — tap must be **MCU pad / R13 MCU side**, not the Z connector |
| Interface | Level-shift / buffer / opto for **active-low ~4 V**, series protection, defined idle (module OFF) |
| Invasiveness | **Limited board modification** (solder tap + external interface) — not unplug-and-play |
| Existing Z wire | Can carry the *buffered* signal afterward; does not fix the stock socket alone |
| Evidence vs hypothesis | Timer availability = firmware/HAL evidence. Exact Snapmaker Vin/Vih/Vil/polarity/idle = **missing module verification** |

### C — Same as B on **Y-STOP PA6** (TIM3_CH1)

Same RC class as Z (R12/R5/C12). Only interesting if PA7 tap is physically harder. Same buffer requirement. Homing/endstop ownership must be re-planned if Y socket is sacrificed — more motion risk than freeing Z (already freed in config).

### D — Filtered Z socket + threshold / comparator recovery

| Aspect | Assessment |
| --- | --- |
| Idea | Reconstruct a clean digital edge from the rounded header waveform, then drive a proper module interface |
| Bounds | Not categorically impossible; RC bandwidth already mangled ~1 kHz → engraving PWM headroom is poor; comparator adds delay/jitter |
| Verdict | Possible **engineered** path at **lower** frequency with validation — **not** preferred vs MCU-side clean tap |

### E — “Truly free” accessible clean GPIO

| Net | Notes |
| --- | --- |
| J1 / PA4 | No native timer PWM AF; input network; supply rail ambiguous (3.3 vs 5 V reports) |
| P3 / PC0 | Unpopulated pads; **no** native timer PWM; mislabelled “Reset” in some drawings |
| P2 SWD | Programming; no spare PWM |
| Display EXP | Occupied; reassigning breaks UI |
| PB0/PB1 | **Keep** for 3D Touch |
| Motor sockets | Coil drivers — not logic PWM |
| Thermistors | Analog only |

**No stock, accessible, clean, timer-capable spare connector** is evidenced for Snapmaker TTL without modification or breaking Display/probe/SD.

### F — Heater NOZ (PA1) as “power cut”

Low-side MOSFET confirmed. Useful only as **power** switching for a load that is designed for it. **Shared module GND can bypass** the FET — unsolved without a proper isolated/high-side or dual-pole power path. Not a substitute for the ~4 V control line.

## Recommendation (least invasive *toward a correct Snapmaker control path*)

1. **Best control-signal candidate:** **MCU-side PA7 (before R13) → purpose-built buffer/opto for active-low ~4 V**, with explicit idle = laser OFF, separate verified 24 V supply, and **no** reliance on the Z header RC.  
   - Keeps TIM3_CH2; avoids TIM2/TEMP and TIM5/SERVO conflicts that block sane HW PWM on PA0.  
   - Requires board tap + external interface design and **module input measurements** before build.

2. **Fan PA0:** Treat as **24 V power-channel research** (Creality-style) or Soft-PWM bench only — **not** the Snapmaker TTL solution. Making PA0 do ~1 kHz HW PWM is **not** a free lunch (TIM2 = TEMP; TIM5 = SERVO). Even then, electrical domain mismatch remains.

3. **Do not** pursue direct Z-socket laser hookup. Comparator-on-header is a fallback research topic only.

4. **Power path:** Plan ENA/heater/fan power switching separately from control; assume shared GND can defeat a single low-side cut until proven otherwise.

## Missing verification (blockers before any interface build)

- Snapmaker control pin: exact **Vih/Vil**, pull-up voltage, active level, idle-safe state, max frequency, input impedance  
- Whether module accepts open-collector / opto sink to its logic ground  
- Actual C13 (and siblings) on *Michael’s* board if relying on any header-side recovery  
- Physical accessibility of PA7 MCU-side pad vs R13 for a safe tap  
- Isolation / common-ground plan for 24 V laser supply vs board GND  

## Bottom line

**Nothing here is laser-ready.** Prefer a **clean MCU-side TIM3 PWM + designed active-low interface** over fan Soft-PWM or the filtered Z socket. Fan remains a **power-domain** option only with a proper interface — current ~7.8 Hz Soft-PWM is not an engraving PWM source, and TIM2 ownership blocks a naïve “just enable HW PWM on PA0” fix.
