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

/**
 * feature/spindle_laser.cpp
 */

#include "../inc/MarlinConfig.h"

#if HAS_CUTTER

#include "spindle_laser.h"

#if ENABLED(SPINDLE_SERVO)
  #include "../module/servo.h"
#endif

#if ENABLED(I2C_AMMETER)
  #include "ammeter.h"
#endif

#if (ENABLED(LASER_SCOPE_PWM_DIAG) || ENABLED(FAN_BENCH_DIAG)) && HAS_MARLINUI_MENU
  #include "../lcd/marlinui.h"
#endif

#if ENABLED(FAN_BENCH_DIAG)
  #include "../module/temperature.h"
#endif

SpindleLaser cutter;
bool SpindleLaser::enable_state;                                      // Virtual enable state, controls enable pin if present and or apply power if > 0
uint8_t SpindleLaser::power,                                          // Actual power output 0-255 ocr or "0 = off" > 0 = "on"
        SpindleLaser::last_power_applied; // = 0                      // Basic power state tracking
#if ENABLED(LASER_SCOPE_PWM_DIAG)
  SpindleLaser::ScopeDiagMode SpindleLaser::scope_diag_mode; // = SCOPE_DIAG_OFF
#endif
#if ENABLED(FAN_BENCH_DIAG)
  SpindleLaser::PowerBenchCh SpindleLaser::power_bench_ch; // = PWR_BENCH_NONE
  bool SpindleLaser::fan_bench_active; // = false
#endif

#if HAS_SPINDLE_ACCELERATION
  uint32_t SpindleLaser::acceleration_spindle_deg_per_s2;             // (°/s/s) Spindle acceleration. Initialized by settings.load
#endif

#if ENABLED(LASER_FEATURE)
  cutter_test_pulse_t SpindleLaser::testPulse = 50;                   // (ms) Test fire pulse default duration
  uint8_t SpindleLaser::last_block_power; // = 0                      // Track power changes for dynamic inline power
  feedRate_t SpindleLaser::feedrate_mm_m = 1500,
             SpindleLaser::last_feedrate_mm_m; // = 0                 // (mm/min) Track feedrate changes for dynamic power
#endif

bool SpindleLaser::isReadyForUI = false;                              // Ready to apply power setting from the UI to OCR
CutterMode SpindleLaser::cutter_mode = CUTTER_MODE_STANDARD;          // Default is standard mode

constexpr cutter_cpower_t SpindleLaser::power_floor;
cutter_power_t SpindleLaser::menuPower = 0,                           // Power value via LCD menu in PWM, PERCENT, or RPM based on configured format set by CUTTER_POWER_UNIT.
               SpindleLaser::unitPower = 0;                           // Unit power is in PWM, PERCENT, or RPM based on CUTTER_POWER_UNIT.

cutter_frequency_t SpindleLaser::frequency;                           // PWM frequency setting; range: 2K - 50K

#define SPINDLE_LASER_PWM_OFF TERN(SPINDLE_LASER_PWM_INVERT, 255, 0)

/**
 * Init the cutter to a safe OFF state
 */
void SpindleLaser::init() {
  #if ENABLED(SPINDLE_SERVO)
    servo[SPINDLE_SERVO_NR].move(SPINDLE_SERVO_MIN);
  #elif PIN_EXISTS(SPINDLE_LASER_ENA)
    OUT_WRITE(SPINDLE_LASER_ENA_PIN, !SPINDLE_LASER_ACTIVE_STATE);    // Init spindle to off
  #endif
  #if ENABLED(SPINDLE_CHANGE_DIR)
    OUT_WRITE(SPINDLE_DIR_PIN, SPINDLE_INVERT_DIR);                   // Init rotation to clockwise (M3)
  #endif
  #if ENABLED(HAL_CAN_SET_PWM_FREQ) && SPINDLE_LASER_FREQUENCY
    frequency = SPINDLE_LASER_FREQUENCY;
    hal.set_pwm_frequency(pin_t(SPINDLE_LASER_PWM_PIN), SPINDLE_LASER_FREQUENCY);
  #endif
  #if ENABLED(SPINDLE_LASER_USE_PWM)
    SET_PWM(SPINDLE_LASER_PWM_PIN);
    hal.set_pwm_duty(pin_t(SPINDLE_LASER_PWM_PIN), SPINDLE_LASER_PWM_OFF); // Set to lowest speed
  #endif
  #if ENABLED(AIR_EVACUATION)
    OUT_WRITE(AIR_EVACUATION_PIN, !AIR_EVACUATION_ACTIVE);            // Init Vacuum/Blower OFF
  #endif
  #if ENABLED(AIR_ASSIST)
    OUT_WRITE(AIR_ASSIST_PIN, !AIR_ASSIST_ACTIVE);                    // Init Air Assist OFF
  #endif
  TERN_(I2C_AMMETER, ammeter.init());                                 // Init I2C Ammeter
}

#if ENABLED(SPINDLE_LASER_USE_PWM)
  /**
   * Set the cutter PWM directly to the given ocr value
   *
   * @param ocr Power value
   */
  void SpindleLaser::_set_ocr(const uint8_t ocr) {
    #if ENABLED(HAL_CAN_SET_PWM_FREQ) && SPINDLE_LASER_FREQUENCY
      hal.set_pwm_frequency(pin_t(SPINDLE_LASER_PWM_PIN), frequency);
    #endif
    #if HAS_SPINDLE_ACCELERATION
      const int16_t diff = ocr - last_power_applied;
      const uint8_t abs_diff = ABS(diff);
      uint8_t current_ocr = last_power_applied;
      // Duration between ocr increments. SPEED_POWER_MAX is in RPM.
      const millis_t duration = (float(SPEED_POWER_MAX) * (60000.f / 2550.f) / float(acceleration_spindle_deg_per_s2)) * abs_diff;
      millis_t next_ocr_change = millis() + duration;
      while (current_ocr != ocr) {
        while (PENDING(millis(), next_ocr_change)) marlin.idle();
        current_ocr += diff > 0 ? 1 : -1;
        hal.set_pwm_duty(pin_t(SPINDLE_LASER_PWM_PIN), current_ocr ^ SPINDLE_LASER_PWM_OFF);
        next_ocr_change += duration;
      }
    #else
      hal.set_pwm_duty(pin_t(SPINDLE_LASER_PWM_PIN), ocr ^ SPINDLE_LASER_PWM_OFF);
    #endif
  }

  void SpindleLaser::set_ocr(const uint8_t ocr) {
    #if PIN_EXISTS(SPINDLE_LASER_ENA)
      WRITE(SPINDLE_LASER_ENA_PIN,  SPINDLE_LASER_ACTIVE_STATE); // Cutter ON
    #endif
    _set_ocr(ocr);
  }

  void SpindleLaser::ocr_off() {
    _set_ocr(0);
    #if PIN_EXISTS(SPINDLE_LASER_ENA)
      WRITE(SPINDLE_LASER_ENA_PIN, !SPINDLE_LASER_ACTIVE_STATE); // Cutter OFF
    #endif
  }
#endif // SPINDLE_LASER_USE_PWM

/**
 * Apply power for Laser or Spindle
 *
 * Apply cutter power value for PWM, Servo, and on/off pin.
 *
 * @param opwr Power value. Range 0 to MAX.
 */
/**
 * Stock idle path called apply_power(0) which, while enable_state is true, uses
 * set_ocr(0) — and set_ocr always writes SPINDLE_LASER_ENA ACTIVE. So OCR went
 * to 0 but PA1 stayed HIGH and the Laser Toggle UI stayed ON. That matches
 * "menu didn't switch off / power remained on" after the 1 s idle timeout.
 *
 * This cut clears UI enable, forces ENA OFF via ocr_off(), and zeroes power.
 * It does not claim a physical power-path cutoff beyond the MOSFET gate pin.
 * M3 / menu Toggle / planner refresh re-enable only when explicitly requested.
 */
void SpindleLaser::safety_timeout_cut() {
  #if ENABLED(LASER_SCOPE_PWM_DIAG)
    if (scope_diag_active()) return; // Diag owns PA7; its own bound + ENA OFF
  #endif
  #if ENABLED(FAN_BENCH_DIAG)
    if (fan_bench_active) return; // FanBench owns FAN0; PA1 held OFF separately
  #endif
  if (!enable_state && !last_power_applied) return;

  power = 0;
  unitPower = 0;
  enable_state = false;
  isReadyForUI = false;
  #if ENABLED(SPINDLE_LASER_USE_PWM)
    ocr_off(); // duty 0 + ENA !ACTIVE
  #elif PIN_EXISTS(SPINDLE_LASER_ENA)
    WRITE(SPINDLE_LASER_ENA_PIN, !SPINDLE_LASER_ACTIVE_STATE);
  #endif
  last_power_applied = 0;
  TERN_(LASER_FEATURE, set_inline_enabled(false));
}

#if ENABLED(LASER_SCOPE_PWM_DIAG)

  void SpindleLaser::scope_diag_hold_ena_off() {
    #if PIN_EXISTS(SPINDLE_LASER_ENA)
      WRITE(SPINDLE_LASER_ENA_PIN, !SPINDLE_LASER_ACTIVE_STATE);
    #endif
  }

  void SpindleLaser::scope_diag_stop() {
    if (scope_diag_mode == SCOPE_DIAG_OFF) {
      scope_diag_hold_ena_off();
      return;
    }
    scope_diag_mode = SCOPE_DIAG_OFF;
    enable_state = false;
    isReadyForUI = false;
    last_power_applied = 0;
    #if ENABLED(SPINDLE_LASER_USE_PWM)
      // Return PA7 to PWM peripheral at OFF duty (not floating input).
      SET_PWM(SPINDLE_LASER_PWM_PIN);
      _set_ocr(0);
    #else
      OUT_WRITE(SPINDLE_LASER_PWM_PIN, LOW);
    #endif
    scope_diag_hold_ena_off();
  }

  void SpindleLaser::scope_diag_start(const ScopeDiagMode mode) {
    if (mode == SCOPE_DIAG_OFF) { scope_diag_stop(); return; }

    TERN_(FAN_BENCH_DIAG, fan_bench_stop()); // Mutual exclusion: PA7 Scope vs FanBench

    // Never energize heater/laser enable during header diagnostics.
    enable_state = false;
    isReadyForUI = false;
    power = 0;
    unitPower = 0;
    scope_diag_hold_ena_off();

    scope_diag_mode = mode; // Stays ON until Stop / leave menu / kill / fault (no timed auto-OFF)

    #if ENABLED(SPINDLE_LASER_USE_PWM)
      switch (mode) {
        case SCOPE_DIAG_LOW:
          // Steady LOW through R13 — header SIG ~ divider with R6 pullup (not 0 V).
          pinMode(SPINDLE_LASER_PWM_PIN, OUTPUT);
          digitalWrite(SPINDLE_LASER_PWM_PIN, LOW);
          last_power_applied = 0;
          break;
        case SCOPE_DIAG_HIGH:
          pinMode(SPINDLE_LASER_PWM_PIN, OUTPUT);
          digitalWrite(SPINDLE_LASER_PWM_PIN, HIGH);
          last_power_applied = 0;
          break;
        case SCOPE_DIAG_PWM:
          frequency = LASER_SCOPE_PWM_DIAG_HZ;
          #if ENABLED(HAL_CAN_SET_PWM_FREQ)
            hal.set_pwm_frequency(pin_t(SPINDLE_LASER_PWM_PIN), LASER_SCOPE_PWM_DIAG_HZ);
          #endif
          SET_PWM(SPINDLE_LASER_PWM_PIN);
          hal.set_pwm_duty(pin_t(SPINDLE_LASER_PWM_PIN), LASER_SCOPE_PWM_DIAG_OCR);
          last_power_applied = 0; // Do not look like cutter power to idle timeout
          break;
        default:
          scope_diag_stop();
          return;
      }
    #endif
    scope_diag_hold_ena_off();
  }

  void SpindleLaser::scope_diag_task() {
    if (!scope_diag_active()) return;
    scope_diag_hold_ena_off(); // Reinforce PA1 OFF every idle

    #if HAS_MARLINUI_MENU
      // Leaving Laser Control cancels the diagnostic (safe cancel; no silent continue).
      extern void menu_spindle_laser();
      if (ui.currentScreen != menu_spindle_laser) {
        scope_diag_stop();
        return;
      }
    #endif
  }

#endif // LASER_SCOPE_PWM_DIAG

#if ENABLED(FAN_BENCH_DIAG)

  void SpindleLaser::fan_bench_hold_ena_off() {
    #if PIN_EXISTS(SPINDLE_LASER_ENA)
      WRITE(SPINDLE_LASER_ENA_PIN, !SPINDLE_LASER_ACTIVE_STATE);
    #endif
  }

  static void _power_bench_park_pin(const pin_t pin) {
    #if ENABLED(FAN_BENCH_HW_PWM)
      if (PWM_PIN(pin)) hal.set_pwm_duty(pin, 0);
    #endif
    OUT_WRITE(pin, LOW);
  }

  void SpindleLaser::fan_bench_stop() {
    // Always park both channels + PA1 OFF (AllOFF / safe cancel)
    #if HAS_FAN0
      _power_bench_park_pin(FAN0_PIN);
      thermalManager.set_fan_speed(0, 0);
    #endif
    #ifdef BED_BENCH_PWM_PIN
      _power_bench_park_pin(BED_BENCH_PWM_PIN);
    #endif
    power_bench_ch = PWR_BENCH_NONE;
    fan_bench_active = false;
    enable_state = false;
    isReadyForUI = false;
    power = 0;
    unitPower = 0;
    last_power_applied = 0;
    fan_bench_hold_ena_off();
  }

  void SpindleLaser::power_bench_set(const PowerBenchCh ch, const uint8_t duty) {
    TERN_(LASER_SCOPE_PWM_DIAG, scope_diag_stop()); // PA7 inactive

    enable_state = false;
    isReadyForUI = false;
    power = 0;
    unitPower = 0;
    last_power_applied = 0;
    fan_bench_hold_ena_off();

    // One channel at a time: park the other first
    #if HAS_FAN0
      if (ch != PWR_BENCH_FAN0) {
        _power_bench_park_pin(FAN0_PIN);
        thermalManager.set_fan_speed(0, 0);
      }
      else
        thermalManager.set_fan_speed(0, 0); // Soft-PWM amount 0 while HW owns pin
    #endif
    #ifdef BED_BENCH_PWM_PIN
      if (ch != PWR_BENCH_BED) _power_bench_park_pin(BED_BENCH_PWM_PIN);
    #endif

    if (ch == PWR_BENCH_NONE) {
      fan_bench_stop();
      return;
    }

    power_bench_ch = ch;
    fan_bench_active = true;

    #if ENABLED(FAN_BENCH_HW_PWM)
      const pin_t pin = (ch == PWR_BENCH_FAN0) ? pin_t(FAN0_PIN) : pin_t(BED_BENCH_PWM_PIN);
      // Shared TIM2 @ FAN_BENCH_PWM_HZ: PA0=CH1, PA2=CH3 (AFIO_NONE)
      #if ENABLED(HAL_CAN_SET_PWM_FREQ)
        hal.set_pwm_frequency(pin, FAN_BENCH_PWM_HZ);
      #endif
      SET_PWM(pin);
      hal.set_pwm_duty(pin, duty); // duty = load-ON fraction (gate high → MOSFET on)
    #endif
    fan_bench_hold_ena_off();
  }

  void SpindleLaser::fan_bench_task() {
    if (!fan_bench_active) return;
    fan_bench_hold_ena_off();

    #if HAS_MARLINUI_MENU
      extern void menu_spindle_laser();
      extern void menu_fan_pa0_pwm();
      extern void menu_bed_pa2_pwm();
      // Allow Laser Control + FanPA0 / BedPA2 submenus; cancel on any other screen
      if (ui.currentScreen != menu_spindle_laser
          && ui.currentScreen != menu_fan_pa0_pwm
          && ui.currentScreen != menu_bed_pa2_pwm) {
        fan_bench_stop();
        return;
      }
    #endif
  }

#endif // FAN_BENCH_DIAG

void SpindleLaser::apply_power(const uint8_t opwr) {
  #if ENABLED(LASER_SCOPE_PWM_DIAG)
    if (scope_diag_active()) {
      scope_diag_hold_ena_off();
      return; // Diag owns PA7; never assert ENA from normal cutter path
    }
  #endif
  #if ENABLED(FAN_BENCH_DIAG)
    if (fan_bench_active) {
      fan_bench_hold_ena_off();
      return; // FanBench: never assert laser ENA / PA7 cutter path
    }
  #endif
  if (enabled() || opwr == 0) {                                   // 0 check allows us to disable where no ENA pin exists
    // Test the last power used to improve performance
    if (opwr == last_power_applied) return;
    // Handle PWM driven or just simple on/off
    #if ENABLED(SPINDLE_LASER_USE_PWM)
      if (CUTTER_UNIT_IS(RPM) && unitPower == 0)
        ocr_off();
      else if (ENABLED(CUTTER_POWER_RELATIVE) || enabled() || opwr == 0) {
        set_ocr(opwr);
        isReadyForUI = true;
      }
      else
        ocr_off();
    #elif ENABLED(SPINDLE_SERVO)
      servo[SPINDLE_SERVO_NR].move(opwr);
    #else
      WRITE(SPINDLE_LASER_ENA_PIN, enabled() ? SPINDLE_LASER_ACTIVE_STATE : !SPINDLE_LASER_ACTIVE_STATE);
      isReadyForUI = true;
    #endif
    last_power_applied = opwr;
  }
  else {
    #if PIN_EXISTS(SPINDLE_LASER_ENA)
      WRITE(SPINDLE_LASER_ENA_PIN, !SPINDLE_LASER_ACTIVE_STATE);
    #endif
    isReadyForUI = false; // Only used for UI display updates.
    TERN_(SPINDLE_LASER_USE_PWM, ocr_off());
  }
}

#if ENABLED(SPINDLE_CHANGE_DIR)
  /**
   * Set the spindle direction and apply immediately
   * Stop on direction change if SPINDLE_STOP_ON_DIR_CHANGE is enabled
   */
  void SpindleLaser::set_reverse(const bool reverse) {
    const bool dir_state = (reverse == SPINDLE_INVERT_DIR); // Forward (M3) HIGH when not inverted
    if (TERN0(SPINDLE_STOP_ON_DIR_CHANGE, enabled()) && READ(SPINDLE_DIR_PIN) != dir_state) disable();
    WRITE(SPINDLE_DIR_PIN, dir_state);
  }
#endif

#if ENABLED(AIR_EVACUATION)
  // Enable / disable Cutter Vacuum or Laser Blower motor
  void SpindleLaser::air_evac_enable()  { WRITE(AIR_EVACUATION_PIN,  AIR_EVACUATION_ACTIVE); } // Turn ON
  void SpindleLaser::air_evac_disable() { WRITE(AIR_EVACUATION_PIN, !AIR_EVACUATION_ACTIVE); } // Turn OFF
  void SpindleLaser::air_evac_toggle()  { TOGGLE(AIR_EVACUATION_PIN); } // Toggle state
#endif

#if ENABLED(AIR_ASSIST)
  // Enable / disable air assist
  void SpindleLaser::air_assist_enable()  { WRITE(AIR_ASSIST_PIN,  AIR_ASSIST_ACTIVE); } // Turn ON
  void SpindleLaser::air_assist_disable() { WRITE(AIR_ASSIST_PIN, !AIR_ASSIST_ACTIVE); } // Turn OFF
  void SpindleLaser::air_assist_toggle()  { TOGGLE(AIR_ASSIST_PIN); } // Toggle state
#endif

#endif // HAS_CUTTER
