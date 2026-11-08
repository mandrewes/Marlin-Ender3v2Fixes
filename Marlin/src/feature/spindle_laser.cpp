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

#if ENABLED(FAN_BENCH_DIAG) || ENABLED(LASER_SCOPE_PWM_DIAG)
  #include "../module/temperature.h"
#endif

SpindleLaser cutter;
bool SpindleLaser::enable_state;                                      // Virtual enable state, controls enable pin if present and or apply power if > 0
uint8_t SpindleLaser::power,                                          // Actual power output 0-255 ocr or "0 = off" > 0 = "on"
        SpindleLaser::last_power_applied; // = 0                      // Basic power state tracking
#if ENABLED(FAN_BENCH_DIAG) || ENABLED(LASER_SCOPE_PWM_DIAG)
  SpindleLaser::PowerBenchCh SpindleLaser::power_bench_ch; // = PWR_BENCH_NONE
  SpindleLaser::PowerDiagDrive SpindleLaser::power_diag_drive; // = PWR_DRIVE_GPIO_LOW
  uint8_t SpindleLaser::power_diag_duty; // = 0
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
  #if ENABLED(FAN_BENCH_DIAG) || ENABLED(LASER_SCOPE_PWM_DIAG)
    if (TERN0(FAN_BENCH_DIAG, fan_bench_active) || TERN0(LASER_SCOPE_PWM_DIAG, scope_diag_active()))
      return; // Power-diag owns pins; PA1 held OFF separately
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

#if ENABLED(FAN_BENCH_DIAG) || ENABLED(LASER_SCOPE_PWM_DIAG)

  void SpindleLaser::fan_bench_hold_ena_off() {
    #if PIN_EXISTS(SPINDLE_LASER_ENA)
      WRITE(SPINDLE_LASER_ENA_PIN, !SPINDLE_LASER_ACTIVE_STATE);
    #endif
  }

  /**
   * Park a diag pin as push-pull GPIO LOW.
   * If the pin had HW PWM AF, zero CCR first then remux to GPIO so Soft-PWM / idle
   * cannot see a live AF channel, and static-OFF is timer-independent.
   */
  static void _power_diag_park_pin(const pin_t pin) {
    if (PWM_PIN(pin)) {
      // Zero duty while still on the timer (safe no-op if timer not yet created)
      hal.set_pwm_duty(pin, 0);
    }
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
  }

  static pin_t _power_diag_pin(const SpindleLaser::PowerBenchCh ch) {
    switch (ch) {
      case SpindleLaser::PWR_BENCH_FAN0: return pin_t(FAN0_PIN);
      case SpindleLaser::PWR_BENCH_BED:
        #ifdef BED_BENCH_PWM_PIN
          return pin_t(BED_BENCH_PWM_PIN);
        #else
          return pin_t(PA2);
        #endif
      case SpindleLaser::PWR_BENCH_Z_PA7: return pin_t(SPINDLE_LASER_PWM_PIN);
      default: return pin_t(FAN0_PIN);
    }
  }

  void SpindleLaser::fan_bench_stop() {
    // AllOFF: park PA0 + PA2 + PA7, Soft-PWM fans 0, PA1 OFF
    #if HAS_FAN0
      _power_diag_park_pin(FAN0_PIN);
      thermalManager.set_fan_speed(0, 0);
    #endif
    #ifdef BED_BENCH_PWM_PIN
      _power_diag_park_pin(BED_BENCH_PWM_PIN);
    #else
      _power_diag_park_pin(PA2);
    #endif
    #if PIN_EXISTS(SPINDLE_LASER_PWM)
      _power_diag_park_pin(SPINDLE_LASER_PWM_PIN);
    #endif
    power_bench_ch = PWR_BENCH_NONE;
    power_diag_drive = PWR_DRIVE_GPIO_LOW;
    power_diag_duty = 0;
    fan_bench_active = false;
    enable_state = false;
    isReadyForUI = false;
    power = 0;
    unitPower = 0;
    last_power_applied = 0;
    fan_bench_hold_ena_off();
  }

  void SpindleLaser::power_diag_set(const PowerBenchCh ch, const PowerDiagDrive drive, const uint8_t duty/*=0*/) {
    // Never energize laser ENA during diagnostics.
    enable_state = false;
    isReadyForUI = false;
    power = 0;
    unitPower = 0;
    last_power_applied = 0;
    fan_bench_hold_ena_off();

    if (ch == PWR_BENCH_NONE) {
      fan_bench_stop();
      return;
    }

    // One target at a time: park the other two first (explicit mutual exclusion).
    #if HAS_FAN0
      if (ch != PWR_BENCH_FAN0) {
        _power_diag_park_pin(FAN0_PIN);
        thermalManager.set_fan_speed(0, 0);
      }
      else
        thermalManager.set_fan_speed(0, 0); // Soft-PWM amount 0 while diag owns FAN0
    #endif
    #ifdef BED_BENCH_PWM_PIN
      if (ch != PWR_BENCH_BED) _power_diag_park_pin(BED_BENCH_PWM_PIN);
    #endif
    #if PIN_EXISTS(SPINDLE_LASER_PWM)
      if (ch != PWR_BENCH_Z_PA7) _power_diag_park_pin(SPINDLE_LASER_PWM_PIN);
    #endif

    power_bench_ch = ch;
    power_diag_drive = drive;
    power_diag_duty = (drive == PWR_DRIVE_PWM) ? duty : 0;
    fan_bench_active = true;

    const pin_t pin = _power_diag_pin(ch);

    switch (drive) {
      case PWR_DRIVE_GPIO_LOW:
        // Static OFF — pure GPIO, no timer AF. Distinguishes FET path from TIM faults.
        pinMode(pin, OUTPUT);
        digitalWrite(pin, LOW);
        break;
      case PWR_DRIVE_GPIO_HIGH:
        pinMode(pin, OUTPUT);
        digitalWrite(pin, HIGH);
        break;
      case PWR_DRIVE_PWM: {
        // HW PWM: PA0→TIM2_CH1, PA2→TIM2_CH3 (shared 1 kHz), PA7→TIM3_CH2.
        // Do NOT pinMode(OUTPUT) first — that would drop AF; set_pwm_duty pinmap_pinouts.
        const uint16_t hz = (ch == PWR_BENCH_Z_PA7)
          ? uint16_t(TERN(LASER_SCOPE_PWM_DIAG, LASER_SCOPE_PWM_DIAG_HZ, FAN_BENCH_PWM_HZ))
          : uint16_t(FAN_BENCH_PWM_HZ);
        #if ENABLED(HAL_CAN_SET_PWM_FREQ)
          hal.set_pwm_frequency(pin, hz);
        #endif
        // duty = load-ON fraction (invert=false → pin HIGH fraction → MOSFET conducting)
        hal.set_pwm_duty(pin, duty);
        if (ch == PWR_BENCH_Z_PA7) frequency = hz;
        break;
      }
    }
    fan_bench_hold_ena_off();
  }

  void SpindleLaser::fan_bench_task() {
    if (!fan_bench_active) return;
    fan_bench_hold_ena_off();

    #if HAS_MARLINUI_MENU
      extern void menu_spindle_laser();
      extern void menu_fan_pa0_pwm();
      extern void menu_bed_pa2_pwm();
      extern void menu_z_pa7_pwm();
      // Allow Laser Control + FanPA0 / BedPA2 / ZPA7 submenus; cancel elsewhere
      if (ui.currentScreen != menu_spindle_laser
          && ui.currentScreen != menu_fan_pa0_pwm
          && ui.currentScreen != menu_bed_pa2_pwm
          && ui.currentScreen != menu_z_pa7_pwm) {
        fan_bench_stop();
        return;
      }
    #endif
  }

#endif // FAN_BENCH_DIAG || LASER_SCOPE_PWM_DIAG

void SpindleLaser::apply_power(const uint8_t opwr) {
  #if ENABLED(FAN_BENCH_DIAG) || ENABLED(LASER_SCOPE_PWM_DIAG)
    if (TERN0(FAN_BENCH_DIAG, fan_bench_active) || TERN0(LASER_SCOPE_PWM_DIAG, scope_diag_active())) {
      fan_bench_hold_ena_off();
      return; // Power-diag: never assert laser ENA / reclaim pins
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
