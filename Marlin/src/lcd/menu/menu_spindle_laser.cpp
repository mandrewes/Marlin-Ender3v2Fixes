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
// Spindle / Laser Menu — SCOPE9 power-output diagnostics
//

#include "../../inc/MarlinConfig.h"

#if HAS_MARLINUI_MENU && HAS_CUTTER

  #include "menu_item.h"

  #include "../../feature/spindle_laser.h"

  #if ENABLED(FAN_BENCH_DIAG) || ENABLED(LASER_SCOPE_PWM_DIAG)

    // K-FAN1 and K-FAN2 share PA0 — one menu controls both headers.
    void menu_fan_pa0_pwm() {
      START_MENU();
      BACK_ITEM(MSG_CUTTER(MENU));
      STATIC_ITEM_F(F("K-FAN1+2 share PA0"), SS_INVERT);
      ACTION_ITEM_F(F("FanPA0 GPIO OFF"), []{
        cutter.power_diag_set(SpindleLaser::PWR_BENCH_FAN0, SpindleLaser::PWR_DRIVE_GPIO_LOW);
        ui.set_status(F("FanPA0 GPIO LOW"));
      });
      ACTION_ITEM_F(F("FanPA0 GPIO ON"), []{
        cutter.power_diag_set(SpindleLaser::PWR_BENCH_FAN0, SpindleLaser::PWR_DRIVE_GPIO_HIGH);
        ui.set_status(F("FanPA0 GPIO HIGH"));
      });
      ACTION_ITEM_F(F("FanPA0 PWM 25%"), []{
        cutter.power_diag_set(SpindleLaser::PWR_BENCH_FAN0, SpindleLaser::PWR_DRIVE_PWM, FAN_BENCH_DUTY_25);
        ui.set_status(F("FanPA0 1kHz 25%"));
      });
      ACTION_ITEM_F(F("FanPA0 PWM 50%"), []{
        cutter.power_diag_set(SpindleLaser::PWR_BENCH_FAN0, SpindleLaser::PWR_DRIVE_PWM, FAN_BENCH_DUTY_50);
        ui.set_status(F("FanPA0 1kHz 50%"));
      });
      ACTION_ITEM_F(F("FanPA0 PWM 75%"), []{
        cutter.power_diag_set(SpindleLaser::PWR_BENCH_FAN0, SpindleLaser::PWR_DRIVE_PWM, FAN_BENCH_DUTY_75);
        ui.set_status(F("FanPA0 1kHz 75%"));
      });
      ACTION_ITEM_F(F("FanPA0 PWM 100%"), []{
        cutter.power_diag_set(SpindleLaser::PWR_BENCH_FAN0, SpindleLaser::PWR_DRIVE_PWM, FAN_BENCH_DUTY_100);
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
      ACTION_ITEM_F(F("BedPA2 GPIO OFF"), []{
        cutter.power_diag_set(SpindleLaser::PWR_BENCH_BED, SpindleLaser::PWR_DRIVE_GPIO_LOW);
        ui.set_status(F("BedPA2 GPIO LOW"));
      });
      ACTION_ITEM_F(F("BedPA2 GPIO ON"), []{
        cutter.power_diag_set(SpindleLaser::PWR_BENCH_BED, SpindleLaser::PWR_DRIVE_GPIO_HIGH);
        ui.set_status(F("BedPA2 GPIO HIGH"));
      });
      ACTION_ITEM_F(F("BedPA2 PWM 25%"), []{
        cutter.power_diag_set(SpindleLaser::PWR_BENCH_BED, SpindleLaser::PWR_DRIVE_PWM, FAN_BENCH_DUTY_25);
        ui.set_status(F("BedPA2 1kHz 25%"));
      });
      ACTION_ITEM_F(F("BedPA2 PWM 50%"), []{
        cutter.power_diag_set(SpindleLaser::PWR_BENCH_BED, SpindleLaser::PWR_DRIVE_PWM, FAN_BENCH_DUTY_50);
        ui.set_status(F("BedPA2 1kHz 50%"));
      });
      ACTION_ITEM_F(F("BedPA2 PWM 75%"), []{
        cutter.power_diag_set(SpindleLaser::PWR_BENCH_BED, SpindleLaser::PWR_DRIVE_PWM, FAN_BENCH_DUTY_75);
        ui.set_status(F("BedPA2 1kHz 75%"));
      });
      ACTION_ITEM_F(F("BedPA2 PWM 100%"), []{
        cutter.power_diag_set(SpindleLaser::PWR_BENCH_BED, SpindleLaser::PWR_DRIVE_PWM, FAN_BENCH_DUTY_100);
        ui.set_status(F("BedPA2 1kHz 100%"));
      });
      ACTION_ITEM_F(F("BedPA2 Stop"), []{
        if (cutter.power_bench_ch == SpindleLaser::PWR_BENCH_BED) cutter.fan_bench_stop();
        ui.set_status(F("BedPA2 Stopped"));
      });
      END_MENU();
    }

    void menu_z_pa7_pwm() {
      START_MENU();
      BACK_ITEM(MSG_CUTTER(MENU));
      STATIC_ITEM_F(F("Z PA7 RC 10k/10k/C"), SS_INVERT);
      ACTION_ITEM_F(F("ZPA7 GPIO LOW"), []{
        cutter.power_diag_set(SpindleLaser::PWR_BENCH_Z_PA7, SpindleLaser::PWR_DRIVE_GPIO_LOW);
        ui.set_status(F("PA7 GPIO LOW"));
      });
      ACTION_ITEM_F(F("ZPA7 GPIO HIGH"), []{
        cutter.power_diag_set(SpindleLaser::PWR_BENCH_Z_PA7, SpindleLaser::PWR_DRIVE_GPIO_HIGH);
        ui.set_status(F("PA7 GPIO HIGH"));
      });
      ACTION_ITEM_F(F("ZPA7 PWM 25%"), []{
        cutter.power_diag_set(SpindleLaser::PWR_BENCH_Z_PA7, SpindleLaser::PWR_DRIVE_PWM, FAN_BENCH_DUTY_25);
        ui.set_status(F("PA7 1kHz 25%"));
      });
      ACTION_ITEM_F(F("ZPA7 PWM 50%"), []{
        cutter.power_diag_set(SpindleLaser::PWR_BENCH_Z_PA7, SpindleLaser::PWR_DRIVE_PWM, FAN_BENCH_DUTY_50);
        ui.set_status(F("PA7 1kHz 50%"));
      });
      ACTION_ITEM_F(F("ZPA7 PWM 75%"), []{
        cutter.power_diag_set(SpindleLaser::PWR_BENCH_Z_PA7, SpindleLaser::PWR_DRIVE_PWM, FAN_BENCH_DUTY_75);
        ui.set_status(F("PA7 1kHz 75%"));
      });
      ACTION_ITEM_F(F("ZPA7 PWM 100%"), []{
        cutter.power_diag_set(SpindleLaser::PWR_BENCH_Z_PA7, SpindleLaser::PWR_DRIVE_PWM, FAN_BENCH_DUTY_100);
        ui.set_status(F("PA7 1kHz 100%"));
      });
      ACTION_ITEM_F(F("ZPA7 Stop"), []{
        if (cutter.power_bench_ch == SpindleLaser::PWR_BENCH_Z_PA7) cutter.fan_bench_stop();
        ui.set_status(F("ZPA7 Stopped"));
      });
      END_MENU();
    }

  #endif // FAN_BENCH_DIAG || LASER_SCOPE_PWM_DIAG

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
      #if ENABLED(FAN_BENCH_DIAG) || ENABLED(LASER_SCOPE_PWM_DIAG)
        // One target at a time; AllOFF parks PA0+PA2+PA7; PA1 forced OFF in firmware.
        SUBMENU_F(F("FanPA0 Diag"), menu_fan_pa0_pwm);
        SUBMENU_F(F("BedPA2 Diag"), menu_bed_pa2_pwm);
        SUBMENU_F(F("ZPA7 Diag"), menu_z_pa7_pwm);
        ACTION_ITEM_F(F("PWM Bench AllOFF"), []{
          cutter.fan_bench_stop();
          ui.set_status(F("All PA0/PA2/PA7 OFF"));
        });
      #endif
    #endif
    END_MENU();
  }

#endif // HAS_MARLINUI_MENU && HAS_CUTTER
