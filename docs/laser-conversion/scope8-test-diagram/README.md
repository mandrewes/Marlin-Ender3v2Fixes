# SCOPE8 / SCOPE9 bench connection diagrams

**Research / bench documentation only. Not laser-ready. Does not flash or operate hardware.**

## Files

| File | Role |
| --- | --- |
| [`scope8-board-locations.png`](scope8-board-locations.png) | Where on a **community** V4.2.2 photo: A=K-FAN1/2, B=BED pair, C=DC-input pair |
| [`scope8-test-connections.png`](scope8-test-connections.png) | Functional circuits: load across +/−; scope tip on switched −; ground on DC− only |
| [`scope8-where-and-how.pdf`](scope8-where-and-how.pdf) | Printable 2-page PDF of the same content |
| [`scope8-where-and-how.svg`](scope8-where-and-how.svg) | Editable source vector |
| [`draw.py`](draw.py) | Reproduction script (paths originally under `/workspace/laser-hw-review/…`) |

**Not committed:** full-page `scope8-where-and-how.png` rendercheck intermediate (avoid duplicate of the two cropped pages).

## Critical reading rules

1. Board photo is a **V4.2.2 community reference** (ender3.club / Creality_4.2.2) — **not** a new photo of Michael’s unit.
2. Boxes A1/A2/B/C mark **connector pairs / sockets**, **not** which physical contact is + or −. Identify polarity from verified markings before connecting.
3. Scope ground **only** on confirmed board/PSU DC−. **Never** clip ground to FAN− or BED−.
4. One target at a time; no heater/laser load; leave 3D Touch PB0/PB1 wiring untouched.
