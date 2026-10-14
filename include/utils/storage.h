#ifndef STORAGE_H
#define STORAGE_H

#include <Arduino.h>
#include <EEPROM.h>
#include "../core/system_config.h"

#define EEPROM_ADDR_VERSION 0
#define EEPROM_ADDR_TEMP 1
#define EEPROM_ADDR_HUM 2
#define EEPROM_ADDR_MODE 3
#define EEPROM_ADDR_DRY_DURATION 4
#define EEPROM_ADDR_CONTROL_MODE 8

#define EEPROM_VERSION 1

inline void initStorage() {
    if (EEPROM.read(EEPROM_ADDR_VERSION) != EEPROM_VERSION) {
        EEPROM.write(EEPROM_ADDR_TEMP, DEFAULT_TEMP);
        EEPROM.write(EEPROM_ADDR_HUM, DEFAULT_HUM);
        EEPROM.write(EEPROM_ADDR_MODE, DRY_MODE_BY_HUM);
        EEPROM.put(EEPROM_ADDR_DRY_DURATION, (uint32_t)60);
        #if GPIO_INTERFACE_ENABLED
        EEPROM.write(EEPROM_ADDR_CONTROL_MODE, CONTROL_AUTO);
        #else
        EEPROM.write(EEPROM_ADDR_CONTROL_MODE, CONTROL_USER);
        #endif
        EEPROM.write(EEPROM_ADDR_VERSION, EEPROM_VERSION);
    }
}

inline uint8_t readTargetTempEEPROM() { return EEPROM.read(EEPROM_ADDR_TEMP); }
inline uint8_t readTargetHumEEPROM() { return EEPROM.read(EEPROM_ADDR_HUM); }
inline uint8_t readModeEEPROM() { return EEPROM.read(EEPROM_ADDR_MODE); }
inline uint32_t readDryDurationEEPROM() { uint32_t duration; EEPROM.get(EEPROM_ADDR_DRY_DURATION, duration); return duration; }
inline uint8_t readControlModeEEPROM() { return EEPROM.read(EEPROM_ADDR_CONTROL_MODE); }

inline void saveTargetTempEEPROM(uint8_t temp) { EEPROM.write(EEPROM_ADDR_TEMP, temp); }
inline void saveTargetHumEEPROM(uint8_t hum) { EEPROM.write(EEPROM_ADDR_HUM, hum); }
inline void saveModeEEPROM(uint8_t mode) { EEPROM.write(EEPROM_ADDR_MODE, mode); }
inline void saveDryDurationEEPROM(uint32_t duration) { EEPROM.put(EEPROM_ADDR_DRY_DURATION, duration); }
inline void saveControlModeEEPROM(uint8_t mode) { EEPROM.write(EEPROM_ADDR_CONTROL_MODE, mode); }

#endif // STORAGE_H 