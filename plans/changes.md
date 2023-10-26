# Custom Changes: Ender 3 V2 + BLTouch + New Hotend

This documents all meaningful configuration changes made to the Marlin bugfix-2.1.x fork for a **Creality Ender 3 V2** with a **BLTouch** probe and a **replacement hotend** (higher-temp capable).

Fork base: `11f98adcce` (2023-08-26)  
Custom commits: `b05485af4c`, `9f1ae6c9f3`

---

## Hardware Profile

- **Board**: Creality V4 (`BOARD_CREALITY_V4`)
- **Machine name**: `Ender-3 V2`
- **Stepper drivers**: TMC2208 (standalone mode) on X, Y, Z, E0
- **Probe**: BLTouch, mounted at offset `{ -45, -9, 0 }` from nozzle
- **Bed size**: 235 × 235 mm, Z height 250 mm
- **Hotend thermistor**: Type 11 (replacement/aftermarket)

---

## Configuration.h Changes

### Board & Communication
| Setting | Upstream default | This fork |
|---|---|---|
| `MOTHERBOARD` | `BOARD_RAMPS_14_EFB` | `BOARD_CREALITY_V4` |
| `SERIAL_PORT` | `0` | `1` |
| `BAUDRATE` | `250000` | `115200` |
| `CUSTOM_MACHINE_NAME` | _(commented out)_ | `"Ender-3 V2"` |

### Stepper Drivers
All axes changed from `A4988` to `TMC2208_STANDALONE` (X, Y, Z, E0).

### Thermal — Hotend
| Setting | Upstream | This fork | Reason |
|---|---|---|---|
| `TEMP_SENSOR_0` | `1` | `11` | Replacement hotend thermistor |
| `HEATER_0_MINTEMP` | `5` | `0` | Avoid false cold errors on startup |
| `HEATER_0_MAXTEMP` | `275` | `315` | Higher-temp capable hotend |
| `DEFAULT_Kp/Ki/Kd` | `22.20 / 1.08 / 114.00` | `20.48 / 1.52 / 68.8` | PID-tuned for this specific hotend |
| `EXTRUDE_MINTEMP` | `170` | `180` | Safer cold-extrude threshold |
| `EXTRUDE_MAXLENGTH` | `200` | `1000` | Needed for Bowden-style long loads/unloads |

### Thermal — Bed
| Setting | Upstream | This fork | Reason |
|---|---|---|---|
| `BED_MINTEMP` | `5` | `0` | Avoid false errors at room temp |
| `BED_MAXTEMP` | `150` | `120` | Ender 3 V2 bed limit |
| `PIDTEMPBED` | disabled | **enabled** | PID instead of bang-bang bed control |
| `DEFAULT_bedKp/Ki/Kd` | `10.00 / 0.023 / 305.4` | `260.55 / 48.61 / 931.03` | PID-tuned for Ender 3 V2 bed |
| `THERMAL_PROTECTION_BED_PERIOD` | `20s` | `180s` | Avoid false thermal-runaway on slow-heating bed |
| `WATCH_BED_TEMP_PERIOD` | `60s` | `180s` | As above |
| `THERMAL_PROTECTION_CHAMBER` | enabled | **disabled** | No chamber on this printer |
| `THERMAL_PROTECTION_COOLER` | enabled | **disabled** | No laser cooler |

### BLTouch & Probing
| Setting | Upstream | This fork |
|---|---|---|
| `Z_MIN_PROBE_USES_Z_MIN_ENDSTOP_PIN` | enabled | **disabled** |
| `USE_PROBE_FOR_Z_HOMING` | disabled | **enabled** |
| `BLTOUCH` | disabled | **enabled** |
| `NOZZLE_TO_PROBE_OFFSET` | `{ 10, 10, 0 }` | `{ -45, -9, 0 }` |
| `Z_PROBE_OFFSET_RANGE_MIN/MAX` | `-20 / 20` | `-10 / 10` |
| `PREHEAT_BEFORE_PROBING` | disabled | **enabled** (nozzle 120°C, bed 50°C) |
| `BLTOUCH_HS_MODE` | disabled | **enabled** |
| `Z_SAFE_HOMING` | disabled | **enabled** |
| `AUTO_BED_LEVELING_3POINT` | disabled | **enabled** |
| `RESTORE_LEVELING_AFTER_G28` | disabled | **enabled** |
| `LCD_BED_LEVELING` | disabled | **enabled** |
| `LCD_BED_TRAMMING` | disabled | **enabled** |
| `MAX_SOFTWARE_ENDSTOP_Z` | enabled | **disabled** | Allows nozzle to go below 0 for z-offset tuning |

### Probing Margins (`Configuration_adv.h`)
| Setting | Upstream | This fork |
|---|---|---|
| `PROBING_MARGIN_LEFT` | `PROBING_MARGIN` | `60` (BLTouch can't reach far-left) |
| `PROBING_MARGIN_FRONT` | `PROBING_MARGIN` | `30` |
| `PROBING_MARGIN_BACK` | `PROBING_MARGIN` | `30` |

### Motion
| Setting | Upstream | This fork | Reason |
|---|---|---|---|
| `INVERT_Y_DIR` | `true` | `false` | Ender 3 V2 Y axis |
| `INVERT_Z_DIR` | `false` | `true` | Ender 3 V2 Z axis |
| `X_BED_SIZE / Y_BED_SIZE` | `200 / 200` | `235 / 235` | Ender 3 V2 print area |
| `Z_MAX_POS` | `200` | `250` | Ender 3 V2 Z height |
| `DEFAULT_MAX_ACCELERATION` | `{ 3000, 3000, 100, 10000 }` | `{ 500, 500, 100, 1000 }` | Conservative for print quality |
| `DEFAULT_ACCELERATION` | `3000` | `500` | Print acceleration |
| `DEFAULT_RETRACT_ACCELERATION` | `3000` | `500` | Retract acceleration |
| `DEFAULT_TRAVEL_ACCELERATION` | `3000` | `1000` | Travel acceleration |
| `CLASSIC_JERK` | disabled | **enabled** (10/10/0.3 mm/s) | Predictable jerk control |
| `QUICK_HOME` | disabled | **enabled** | Faster diagonal XY homing |

### EEPROM
| Setting | Upstream | This fork |
|---|---|---|
| `EEPROM_SETTINGS` | disabled | **enabled** |
| `EEPROM_CHITCHAT` | enabled | **disabled** (save flash) |
| `EEPROM_AUTO_INIT` | disabled | **enabled** |

### Preheat Presets
| Setting | Upstream | This fork |
|---|---|---|
| `PREHEAT_1_TEMP_HOTEND` (PLA) | `180` | `185` |
| `PREHEAT_1_FAN_SPEED` | `0` | `255` |
| `PREHEAT_2_TEMP_BED` (ABS) | `110` | `70` |
| `PREHEAT_2_FAN_SPEED` | `0` | `255` |

### Display & UI
| Setting | Upstream | This fork |
|---|---|---|
| `SDSUPPORT` | disabled | **enabled** |
| `SLIM_LCD_MENUS` | disabled | **enabled** |
| `ENCODER_PULSES_PER_STEP` | commented out | `4` |
| `ENCODER_STEPS_PER_MENU_ITEM` | commented out | `1` |
| `DWIN_MARLINUI_PORTRAIT` | disabled | **enabled** (Ender 3 V2 DWIN display) |
| `FAN_SOFT_PWM` | disabled | **enabled** |
| `NOZZLE_PARK_FEATURE` | disabled | **enabled** |

### Advanced Features (`Configuration_adv.h`)
| Setting | Upstream | This fork |
|---|---|---|
| `AUTOTEMP_MAX` | `250` | `300` |
| `ADAPTIVE_STEP_SMOOTHING` | disabled | **enabled** |
| `FT_MOTION` (Fixed-time motion) | disabled | **enabled** |
| `FT_MOTION_MENU` | disabled | **enabled** |
| `POWER_LOSS_RECOVERY` | disabled | **enabled** |
| `BABYSTEPPING` | disabled | **enabled** |
| `BABYSTEP_WITHOUT_HOMING` | disabled | **enabled** |
| `BABYSTEP_ALWAYS_AVAILABLE` | disabled | **enabled** |
| `BABYSTEP_DISPLAY_TOTAL` | disabled | **enabled** |
| `BABYSTEP_ZPROBE_OFFSET` | disabled | **enabled** |
| `EMERGENCY_PARSER` | disabled | **enabled** |
| `HOST_ACTION_COMMANDS` | disabled | **enabled** |
| `HOST_PROMPT_SUPPORT` | disabled | **enabled** |

---

## Source Code Change

### `Marlin/src/lcd/e3v2/common/dwin_api.cpp`
The `#if` guard around `dwinDrawRectangle()` in `dwinDrawString()` was commented out, so the background rectangle is always drawn regardless of LCD UI variant. This was likely to fix a display rendering issue with `DWIN_MARLINUI_PORTRAIT`.

---

## Extra Files Added (not in upstream)

- **`LCD Files/`** — DWIN display assets (icons, fonts, images) for the Ender 3 V2 colour LCD
- **`re-initialise-ender.txt`** — G-code runbook for: PID autotune (hotend + bed), e-step calibration, Z offset setup, soft endstop toggling, and FT motion commands
- **`command_reference.txt`** — Short G-code snippets for fixed-time motion and Z offset workflow
