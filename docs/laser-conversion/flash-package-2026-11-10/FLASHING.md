# Flash package — 2026-11-10 (Michael does the flashing)

**Agent does not flash hardware, does not insert SD cards, and does not energize the printer.**  
This folder is a **self-flash kit** after a dead DWIN screen + USB drop (zip-tying 3D Touch). Check wiring first, then flash.

## What’s in this package

| Item | File | Notes |
| --- | --- | --- |
| Mainboard firmware | **`LASR0010.bin`** | SCOPE9 (`2.1.16-MA-LASER-PA7-SCOPE9`), Creality V4.2.2 / STM32F103RE. PA7 laser PWM + Fan/Bed/Z diag menus. BLTouch / 3D Touch on PB0/PB1; `USE_PROBE_FOR_Z_HOMING` + `Z_SAFE_HOMING`. Laser OFF at boot / safety timeout. |
| Long name (optional copy) | `firmware-ender3v2-v422-laser-pa7-SCOPE9-LASR0010.bin` | Same bytes as `LASR0010.bin` — **do not** put this long name alone on the Creality SD; use the 8.3 name. |
| Screen assets | **`DWIN_SET/`** | Official Marlin unified set for **stock DWIN** (T5UIC1). Matches firmware UI **`DWIN_MARLINUI_PORTRAIT`**. |
| Archive copy | `DWIN_SET.tar.gz` | Same DWIN_SET; provenance in `SOURCE-DWIN_SET.txt`. |
| Checksums | `CHECKSUMS.txt`, `LASR0010.bin.sha256` | Verify before flashing. |

**Zip of this whole folder:** `flash-package-2026-11-10.zip` (sibling or inside — see README).

### SD names already used / reserved

| Name | Status |
| --- | --- |
| `LASR0005.bin` | Prior SD (backed up) |
| `LASR0008.bin` | Last prepared SD = SCOPE8 |
| `LASR0009.bin` | Reserved in parking docs for SCOPE9; **not** used here |
| **`LASR0010.bin`** | **This package** (unique 8.3 for Creality bootloader) |

This printer is **very sensitive** to SD **filesystem size**, **FAT32 cluster size**, and **8.3 filenames**. Prefer the known **4 GB** card. Do **not** use the 960 GB Verbatim drive. Do **not** invent other names like `firmware.bin` unless recovering.

---

## Hardware checklist (dead screen + USB drop)

Do these **before** flashing. Power **OFF**, PSU unplugged if probing shorts.

1. **3D Touch 5 V / GND** — zip-tie work can pinch the probe harness. Check for **5 V shorted to GND** on the probe plug / board BLTouch header (PB0 servo / PB1 sense / 5 V / GND). A 5 V rail short kills the DWIN and often USB.
2. **Screen ribbon (EXP / display cable)** — reseat both ends (board EXP3 and display). Bent pins or a half-seated FFC/ribbon = blank screen with a live board.
3. **USB cable / port** — try another cable and port after power is healthy; USB drop alone does not prove MCU death.
4. **24 V supply** — confirm PSU LED / board 24 V IN. No 24 V → nothing boots.
5. If 5 V is shorted: **fix the short first**. Flashing will not revive a rail held at 0 V.

---

## Which order to flash

Recommended after wiring looks sane:

1. **Screen (`DWIN_SET`)** first — restores panel graphics/firmware independently of Marlin.  
2. **Mainboard (`LASR0010.bin`)** second — so Marlin and the panel speak the same session after both are updated.  
3. Power-cycle with **no** flash SD in the display slot and only the mainboard card (or none) as you prefer for normal use.

If the board is clearly dead (no USB, no fan twitch, no 5 V) fix power first; flashing the screen alone will not help.

---

## A. Flash the DWIN screen

**Card:** microSD **≤ 8–16 GB**, **FAT32**, **4096-byte (4 KB) clusters**. Empty root except what you copy.

1. Copy the entire folder **`DWIN_SET`** to the **root** of the card (path must be `DWIN_SET\…`, not nested oddly).  
2. Power the printer **OFF**.  
3. Open the screen back (stock Ender-3 V2 DWIN). Insert the card into the **display’s own microSD slot** (not the mainboard SD slot).  
4. Power **ON**. Wait for the colour change (typically **blue → orange**). That means the panel accepted the set.  
5. Power **OFF**, **remove** the card from the display, close the case.  
6. Power **ON** again. Expect a Creality/Marlin boot image, then MarlinUI once the mainboard firmware is present and talking.

**DACAI / SYNWIT / TJC clones:** this `DWIN_SET` is for **stock DWIN**. Clones need different assets — not included.

---

## B. Flash the mainboard

**Card:** Prefer the known **4 GB FAT32 / 4096-byte clusters** card. Root should contain **one** `.bin` only for a clean flash.

1. Copy **`LASR0010.bin`** to the **root** of the card (8.3 name exactly). Remove other `LASR*.bin` / old firmware bins from the root so the bootloader does not pick a stale file.  
2. Insert into the **mainboard** SD slot.  
3. Power **ON** and wait (Creality bootloader copies from SD; often a minute; display may look odd until done).  
4. Power **OFF**, remove or leave the card per your usual habit; delete or rename `LASR0010.bin` after a successful flash so it does not reflash every boot.  
5. Power **ON**. LCD About / `M115` should show **`2.1.16-MA-LASER-PA7-SCOPE9`**.

---

## Verify

| Check | Expect |
| --- | --- |
| Screen | Boot image, then MarlinUI portrait menus; encoder works |
| `M115` / About | `2.1.16-MA-LASER-PA7-SCOPE9` |
| USB | Host sees serial again (after 5 V/USB healthy) |
| Probe | BLTouch/3D Touch deploy/stow from LCD; Z home uses probe (`USE_PROBE_FOR_Z_HOMING`) |
| Laser / heaters | **Keep laser and heaters disconnected** until you choose otherwise. Diag menus still present; laser stays safe/off at boot |

---

## Recovery if the screen stays blank

1. Re-run the **hardware checklist** (especially **5 V short** and **EXP ribbon**).  
2. Re-flash **`DWIN_SET`** on a freshly formatted **≤16 GB FAT32 4 KB** card.  
3. Confirm mainboard has **`LASR0010.bin`** flashed and 24 V / 5 V present.  
4. Try the **other** known-good SD only if it is also small FAT32 4 KB — large cards often fail on this machine.  
5. If the panel never goes blue→orange during DWIN flash: wrong card format, card not in the **display** slot, or not a DWIN unit / dead panel.  
6. Stock Creality UI-only recovery sets are a last resort and **do not** match this MarlinUI build as well as Marlin’s unified `DWIN_SET` — prefer the set in this package.

---

## Uncertainties (read before blaming firmware)

- Dead screen + USB at probe zip-tie time is **most often a 5 V short or ribbon**, not a corrupt `DWIN_SET`.  
- Display manufacturer clones (DACAI etc.) are **not** verified for this kit.  
- Local repo `LCD Files/DWIN_SET` differs slightly on `4.ICO` / `7.ICO`; this package uses **official Marlin Configurations** `DWIN_SET.tar.gz` (see `SOURCE-DWIN_SET.txt`).  
- Binary SHA differs from the Nov-8 SCOPE9 artifact because of a **fresh rebuild** today; **source identity is still SCOPE9** (same `Version.h` / features). Size **154532** bytes.
