/**
 * Marlin 3D Printer Firmware
 * Copyright (c) 2020 MarlinFirmware [https://github.com/MarlinFirmware/Marlin]
 *
 * Based on Sprinter and grbl.
 * Copyright (c) 2011 Camiel Gubbels / Erik van der Zalm
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

//
// Spindle / Laser Menu
//

#include "../../inc/MarlinConfig.h"

#if HAS_MARLINUI_MENU && HAS_CUTTER

  #include "menu_item.h"

  #include "../../feature/spindle_laser.h"

  #if ENABLED(FAN_BENCH_DIAG)

    // K-FAN1 and K-FAN2 share PA0 — one menu controls both headers.
    void menu_fan_pa0_pwm() {
      START_MENU();
      BACK_ITEM(MSG_CUTTER(MENU));
      STATIC_ITEM_F(F("K-FAN1+2 share PA0"), SS_INVERT);
      ACTION_ITEM_F(F("FanPA0 OFF 0%"), []{
        cutter.power_bench_set(SpindleLaser::PWR_BENCH_FAN0, 0);
        ui.set_status(F("FanPA0 1kHz 0%"));
      });
      ACTION_ITEM_F(F("FanPA0 25% ON"), []{
        cutter.power_bench_set(SpindleLaser::PWR_BENCH_FAN0, FAN_BENCH_DUTY_25);
        ui.set_status(F("FanPA0 1kHz 25%"));
      });
      ACTION_ITEM_F(F("FanPA0 50% ON"), []{
        cutter.power_bench_set(SpindleLaser::PWR_BENCH_FAN0, FAN_BENCH_DUTY_50);
        ui.set_status(F("FanPA0 1kHz 50%"));
      });
      ACTION_ITEM_F(F("FanPA0 75% ON"), []{
        cutter.power_bench_set(SpindleLaser::PWR_BENCH_FAN0, FAN_BENCH_DUTY_75);
        ui.set_status(F("FanPA0 1kHz 75%"));
      });
      ACTION_ITEM_F(F("FanPA0 100% ON"), []{
        cutter.power_bench_set(SpindleLaser::PWR_BENCH_FAN0, FAN_BENCH_DUTY_100);
        ui.set_status(F("FanPA0 1kHz 100%"));
      });
      ACTION_ITEM_F(F("FanPA0 Stop"), []{
        if (cutter.power_bench_ch == SpindleLaser::PWR_BENCH_FAN0) cutter.fan_bench_stop();
        ui.set_status(F("FanPA0 Stopped"));
      });
      END_MENU();
    }

    void menu_bed_pa2_pwm() {
      START_MENU();
      BACK_ITEM(MSG_CUTTER(MENU));
      STATIC_ITEM_F(F("BED MOSFET PA2"), SS_INVERT);
      ACTION_ITEM_F(F("BedPA2 OFF 0%"), []{
        cutter.power_bench_set(SpindleLaser::PWR_BENCH_BED, 0);
        ui.set_status(F("BedPA2 1kHz 0%"));
      });
      ACTION_ITEM_F(F("BedPA2 25% ON"), []{
        cutter.power_bench_set(SpindleLaser::PWR_BENCH_BED, FAN_BENCH_DUTY_25);
        ui.set_status(F("BedPA2 1kHz 25%"));
      });
      ACTION_ITEM_F(F("BedPA2 50% ON"), []{
        cutter.power_bench_set(SpindleLaser::PWR_BENCH_BED, FAN_BENCH_DUTY_50);
        ui.set_status(F("BedPA2 1kHz 50%"));
      });
      ACTION_ITEM_F(F("BedPA2 75% ON"), []{
        cutter.power_bench_set(SpindleLaser::PWR_BENCH_BED, FAN_BENCH_DUTY_75);
        ui.set_status(F("BedPA2 1kHz 75%"));
      });
      ACTION_ITEM_F(F("BedPA2 100% ON"), []{
        cutter.power_bench_set(SpindleLaser::PWR_BENCH_BED, FAN_BENCH_DUTY_100);
        ui.set_status(F("BedPA2 1kHz 100%"));
      });
      ACTION_ITEM_F(F("BedPA2 Stop"), []{
        if (cutter.power_bench_ch == SpindleLaser::PWR_BENCH_BED) cutter.fan_bench_stop();
        ui.set_status(F("BedPA2 Stopped"));
      });
      END_MENU();
    }

  #endif // FAN_BENCH_DIAG

  void menu_spindle_laser() {
    bool is_enabled = cutter.enabled();
    #if ENABLED(SPINDLE_CHANGE_DIR)
      bool is_rev = cutter.is_reverse();
    #endif

    START_MENU();
    BACK_ITEM(MSG_MAIN_MENU);

    #if ENABLED(SPINDLE_LASER_USE_PWM)
      EDIT_ITEM_FAST(CUTTER_MENU_POWER_TYPE, MSG_CUTTER(POWER), &cutter.menuPower,
        cutter.mpower_min(), cutter.mpower_max(), cutter.update_from_mpower);
    #endif

    editable.state = is_enabled;
    EDIT_ITEM(bool, MSG_CUTTER(TOGGLE), &is_enabled, []{
      #if ENABLED(SPINDLE_FEATURE)
        if (editable.state) cutter.disable(); else cutter.enable_same_dir();
      #else
        cutter.menu_set_enabled(!editable.state);
      #endif
    });

    #if ENABLED(AIR_EVACUATION)
      bool evac_state = cutter.air_evac_state();
      EDIT_ITEM(bool, MSG_CUTTER(EVAC_TOGGLE), &evac_state, cutter.air_evac_toggle);
    #endif

    #if ENABLED(AIR_ASSIST)
      bool air_assist_state = cutter.air_assist_state();
      EDIT_ITEM(bool, MSG_CUTTER(ASSIST_TOGGLE), &air_assist_state, cutter.air_assist_toggle);
    #endif

    #if ENABLED(SPINDLE_CHANGE_DIR)
      if (!is_enabled) {
        editable.state = is_rev;
        ACTION_ITEM_F(is_rev ? GET_TEXT_F(MSG_CUTTER(REVERSE)) : GET_TEXT_F(MSG_CUTTER(FORWARD)), []{ cutter.set_reverse(!editable.state); });
      }
    #endif

    #if ENABLED(LASER_FEATURE)
      EDIT_ITEM_FAST(CUTTER_MENU_PULSE_TYPE, MSG_LASER_PULSE_MS, &cutter.testPulse, LASER_TEST_PULSE_MIN, LASER_TEST_PULSE_MAX);
      ACTION_ITEM(MSG_LASER_FIRE_PULSE, cutter.test_fire_pulse);
      #if ENABLED(HAL_CAN_SET_PWM_FREQ) && SPINDLE_LASER_FREQUENCY
        EDIT_ITEM_FAST(CUTTER_MENU_FREQUENCY_TYPE, MSG_CUTTER_FREQUENCY, &cutter.frequency, 500, 65535, cutter.refresh_frequency);
      #endif
      #if ENABLED(LASER_SCOPE_PWM_DIAG)
        ACTION_ITEM_F(F("Scope LOW untilOFF"), []{
          cutter.scope_diag_start(SpindleLaser::SCOPE_DIAG_LOW);
          ui.set_status(F("PA7 LOW ENA OFF"));
        });
        ACTION_ITEM_F(F("Scope HIGH untilOFF"), []{
          cutter.scope_diag_start(SpindleLaser::SCOPE_DIAG_HIGH);
          ui.set_status(F("PA7 HIGH ENA OFF"));
        });
        ACTION_ITEM_F(F("ScopePWM 1k50% ON"), []{
          cutter.scope_diag_start(SpindleLaser::SCOPE_DIAG_PWM);
          ui.set_status(F("PA7 1kHz50% ENA OFF"));
        });
        ACTION_ITEM_F(F("ScopePWM Stop/OFF"), []{
          cutter.scope_diag_stop();
          ui.set_status(F("Scope diag OFF"));
        });
      #endif
      #if ENABLED(FAN_BENCH_DIAG)
        // Separate FanPA0 / BedPA2 benches; selecting one stops the other. PA1 OFF.
        SUBMENU_F(F("FanPA0 PWM"), menu_fan_pa0_pwm);
        SUBMENU_F(F("BedPA2 PWM"), menu_bed_pa2_pwm);
        ACTION_ITEM_F(F("PWM Bench AllOFF"), []{
          cutter.fan_bench_stop();
          ui.set_status(F("All power PWM OFF"));
        });
      #endif
    #endif
    END_MENU();
  }

#endif // HAS_MARLINUI_MENU && HAS_CUTTER
