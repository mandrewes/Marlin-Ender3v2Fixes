#include "../inc/MarlinConfig.h"

// Numeric pin_t evidence baked into .rodata as ASCII for `strings` on the .bin/.elf
#define Q(v) #v
#define QQ(v) Q(v)

#define EVIDENCE_LINE(label, expr) \
  static const char evidence_##label[] = "SCOPE_EVIDENCE " #label "=" QQ(expr);

EVIDENCE_LINE(SERVO0_PIN, SERVO0_PIN)
EVIDENCE_LINE(Z_MIN_PROBE_PIN, Z_MIN_PROBE_PIN)
EVIDENCE_LINE(Z_STOP_PIN, Z_STOP_PIN)
EVIDENCE_LINE(HEATER_0_PIN, HEATER_0_PIN)
EVIDENCE_LINE(HEATER_BED_PIN, HEATER_BED_PIN)
EVIDENCE_LINE(SPINDLE_LASER_PWM_PIN, SPINDLE_LASER_PWM_PIN)
EVIDENCE_LINE(SPINDLE_LASER_ENA_PIN, SPINDLE_LASER_ENA_PIN)
EVIDENCE_LINE(NUM_SERVOS, NUM_SERVOS)
EVIDENCE_LINE(EXTRUDERS, EXTRUDERS)
EVIDENCE_LINE(BLTOUCH_DELAY, BLTOUCH_DELAY)
EVIDENCE_LINE(SPINDLE_LASER_FREQUENCY, SPINDLE_LASER_FREQUENCY)
EVIDENCE_LINE(Z_PROBE_SERVO_NR, Z_PROBE_SERVO_NR)

#if ENABLED(BLTOUCH)
  static const char evidence_BLTOUCH[] = "SCOPE_EVIDENCE BLTOUCH=1";
#else
  static const char evidence_BLTOUCH[] = "SCOPE_EVIDENCE BLTOUCH=0";
#endif
#if ENABLED(USE_PROBE_FOR_Z_HOMING)
  static const char evidence_USE_PROBE_FOR_Z_HOMING[] = "SCOPE_EVIDENCE USE_PROBE_FOR_Z_HOMING=1";
#else
  static const char evidence_USE_PROBE_FOR_Z_HOMING[] = "SCOPE_EVIDENCE USE_PROBE_FOR_Z_HOMING=0";
#endif
#ifdef HAS_Z_SERVO_PROBE
  static const char evidence_HAS_Z_SERVO_PROBE[] = "SCOPE_EVIDENCE HAS_Z_SERVO_PROBE=1";
#endif
#ifdef HAS_SERVOS
  static const char evidence_HAS_SERVOS[] = "SCOPE_EVIDENCE HAS_SERVOS=1";
#endif
#if ENABLED(DEACTIVATE_SERVOS_AFTER_MOVE)
  static const char evidence_DEACTIVATE_SERVOS[] = "SCOPE_EVIDENCE DEACTIVATE_SERVOS_AFTER_MOVE=1";
#else
  static const char evidence_DEACTIVATE_SERVOS[] = "SCOPE_EVIDENCE DEACTIVATE_SERVOS_AFTER_MOVE=0";
#endif
#if ENABLED(SPEAKER)
  static const char evidence_SPEAKER[] = "SCOPE_EVIDENCE SPEAKER=1";
#else
  static const char evidence_SPEAKER[] = "SCOPE_EVIDENCE SPEAKER=0";
#endif
#if HAS_HOTEND
  static const char evidence_HAS_HOTEND[] = "SCOPE_EVIDENCE HAS_HOTEND=1";
#else
  static const char evidence_HAS_HOTEND[] = "SCOPE_EVIDENCE HAS_HOTEND=0";
#endif
#if HAS_HEATED_BED
  static const char evidence_HAS_HEATED_BED[] = "SCOPE_EVIDENCE HAS_HEATED_BED=1";
#else
  static const char evidence_HAS_HEATED_BED[] = "SCOPE_EVIDENCE HAS_HEATED_BED=0";
#endif
#if TEMP_SENSOR_0
  static const char evidence_TEMP_SENSOR_0_nonzero[] = "SCOPE_EVIDENCE TEMP_SENSOR_0_nonzero=1";
#else
  static const char evidence_TEMP_SENSOR_0_nonzero[] = "SCOPE_EVIDENCE TEMP_SENSOR_0_nonzero=0";
#endif
#if TEMP_SENSOR_BED
  static const char evidence_TEMP_SENSOR_BED_nonzero[] = "SCOPE_EVIDENCE TEMP_SENSOR_BED_nonzero=1";
#else
  static const char evidence_TEMP_SENSOR_BED_nonzero[] = "SCOPE_EVIDENCE TEMP_SENSOR_BED_nonzero=0";
#endif

// Force retention
const char* const scope_evidence_table[] = {
  evidence_SERVO0_PIN, evidence_Z_MIN_PROBE_PIN, evidence_Z_STOP_PIN,
  evidence_HEATER_0_PIN, evidence_HEATER_BED_PIN,
  evidence_SPINDLE_LASER_PWM_PIN, evidence_SPINDLE_LASER_ENA_PIN,
  evidence_NUM_SERVOS, evidence_EXTRUDERS, evidence_BLTOUCH_DELAY,
  evidence_SPINDLE_LASER_FREQUENCY, evidence_Z_PROBE_SERVO_NR,
  evidence_BLTOUCH, evidence_USE_PROBE_FOR_Z_HOMING,
  evidence_DEACTIVATE_SERVOS, evidence_SPEAKER,
  evidence_HAS_HOTEND, evidence_HAS_HEATED_BED,
  evidence_TEMP_SENSOR_0_nonzero, evidence_TEMP_SENSOR_BED_nonzero,
  nullptr
};

void scope_macro_dump_anchor() { (void)scope_evidence_table; }
