#ifndef USER_CONFIG_H
#define USER_CONFIG_H

// ===================== USER CONFIGURABLE SETTINGS =====================

// ===================== GPIO Interface (Prusa) Integration =====================
// Disable for printer without GPIO interface eg. MK3 series
#define GPIO_INTERFACE_ENABLED 1

// ===================== Unit configuration =====================
// Temperature unit: Celsius only

// ===================== Fan PWM speeds (Configurable) =====================
// Recommended values for minimum noise and maximum efficiency (0 to 255 max)
#define FAN_SPEED_STANDBY_CYCLING_PWM 102  // 40% - Standby cycling
#define FAN_SPEED_NORMAL_CYCLING_PWM 153   // 60% - Normal operation cycling (BY TIME/BY HUM)
#define FAN_MIN_PWM_WHEN_HEATING 90        // 35% - Minimum fan speed when heater is active

// ===================== Fan control timing =====================
#define FAN_COOLDOWN_TIME 45000     // Cooldown time after heater off
#define FAN_CYCLE_ON_TIME 15000      // Standby cycle on time
#define FAN_CYCLE_OFF_TIME 300000    // Standby cycle off time
#define FAN_TRANSITION_TIME 2000     // Smooth transition time (2 seconds)

#endif // USER_CONFIG_H
