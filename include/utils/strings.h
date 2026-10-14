#ifndef STRINGS_H
#define STRINGS_H

#include <Arduino.h>
#include <avr/pgmspace.h>
#include <Adafruit_SSD1306.h>

class Adafruit_SSD1306;
extern Adafruit_SSD1306 display;

// Navigation strings
static const char PROGMEM str_cooling[] = "COOLING";
static const char PROGMEM str_cycling[] = "CYCLING";
static const char PROGMEM str_system_off[] = "SYS OFF";
static const char PROGMEM str_heat_off[] = "HEAT OFF";
static const char PROGMEM str_fan_off[] = "FAN OFF";
static const char PROGMEM str_standby[] = "Standby";

// Operation mode strings
static const char PROGMEM str_printing[] = "PRINTING";
static const char PROGMEM str_drying[] = "DRYING";
static const char PROGMEM str_warning[] = "WARN";
static const char PROGMEM str_balance[] = "BALANCE";

// Configuration strings
static const char PROGMEM str_set_temp[] = "Set Temp";
static const char PROGMEM str_change_save[] = "+/-:Chg O:Save";
static const char PROGMEM str_set_humidity[] = "Set Hum";
static const char PROGMEM str_operation_mode[] = "Op Mode";
static const char PROGMEM str_by_hum[] = "By Hum";
static const char PROGMEM str_dry[] = "Dry Timed";

// Control mode strings
static const char PROGMEM str_auto[] = "Mode: AUTO";
static const char PROGMEM str_user[] = "Mode: USER";

// Dry mode string
static const char PROGMEM str_dry_mode_time[] = "Set time";

// Units
static const char PROGMEM str_celsius[] = "C";
static const char PROGMEM str_percent[] = "%";

// Display labels
static const char PROGMEM str_temp_label[] = "Temperature ";
static const char PROGMEM str_humidity_label[] = "Humidity ";
static const char PROGMEM str_eta_label[] = "ETA ";
static const char PROGMEM str_heat_label[] = "Heat ";
static const char PROGMEM str_space[] = " ";
static const char PROGMEM str_greater[] = " > ";

// Helper functions for PROGMEM strings
inline void printProgmemString(Adafruit_SSD1306* disp, const char* str) {
  if (!disp) return;
  char buffer[20];
  strcpy_P(buffer, str);
  disp->print(buffer);
}

inline const char* getProgmemString(const char* progmemString) {
  static char buffer[20];
  strcpy_P(buffer, progmemString);
  return buffer;
}

// Logo strings
static const char PROGMEM str_voxel3d[] = "VOXEL3D";
static const char PROGMEM str_code_by[] = "Code by 3DMonkey";

#endif // STRINGS_H 