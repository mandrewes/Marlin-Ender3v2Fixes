# Board connector reference (Creality V4.2.2)

**Research / bench documentation only. Not laser-ready. No flash, SD staging, or wiring changes from this import.**

## Qualifications (read first)

| Claim | Status |
| --- | --- |
| Photographs / silkscreen in this tree | **Reference V4.2.2** used for the inventory build — **not** asserted to be Michael’s own PCB |
| Schematic | **Community reverse-engineering** (Jyers / related sources) — **not** an official Creality drawing; manufacturing variants exist |
| Capacitor values (e.g. C13 100 nF) | Often **assumed** in the RE drawing — confirm on the actual board before design |
| Contact left/right orientation | **Not** invented here; identify on the physical board |
| User Z-STOP measurements (SCOPE5) | HIGH ≈ **3.3 V**, LOW ≈ **1.6 V**, curved PWM ≈ **844 mVpp** @ ~1 kHz — consistent with PA7→R13/R6/C13; **Z socket abandoned for direct laser control** |
| Published fan opto images under `conversions/` | Example of **active-high ~5 V / fan-style** adapters — **NOT** an approved Snapmaker Original interface |

Working **3D Touch** on **PB0 / PB1** must remain untouched in any future plan.

## Folder map

| Path | Contents |
| --- | --- |
| [`all-connectors/`](all-connectors/) | Printable PDF, high-res PNG, editable SVG, inventory MD/CSV/JSON, verification notes, `build_reference.py`, schematic crops used in the poster |
| [`three-pin-identification/`](three-pin-identification/) | J1 / runout identification aids and contact-sheet context |
| [`schematic-crops/`](schematic-crops/) | Useful RE schematic crops: heaters, endstops, Z detail |
| [`conversions/`](conversions/) | Published opto adapter photos (**not** Snapmaker-approved) |
| [`pwm-route-assessment.md`](pwm-route-assessment.md) | **Recommendation only** — what can still be used after Z is ruled out |
| [`CHECKSUMS.txt`](CHECKSUMS.txt) | SHA256 of primary PDF/PNG/SVG finals |

## Primary finals (do not drop)

- `all-connectors/v422-all-connectors-reference.pdf` — printable reference
- `all-connectors/v422-all-connectors.png` — high-res raster
- `all-connectors/v422-all-connectors.svg` — editable vector
- `all-connectors/inventory.md` (+ `.csv` / `.json`)
- `all-connectors/VERIFICATION.md`
- `all-connectors/build_reference.py`

## Related laser-conversion docs

- [`../SCOPE6-FAN-BENCH.md`](../SCOPE6-FAN-BENCH.md) — FanBench / Soft-PWM context  
- [`../SCOPE5-PA7-PWM.md`](../SCOPE5-PA7-PWM.md) / [`../SCOPE4-PA7-PWM.md`](../SCOPE4-PA7-PWM.md) — Z RC / Scope diag history  
- [`../README.md`](../README.md) — conversion overview  

**Nothing in this folder makes the printer laser-ready.**
