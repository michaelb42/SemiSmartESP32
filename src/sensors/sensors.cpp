#include "../../include/sensors/sensors.h"
#include "../../include/core/system_config.h"
#include "../../include/sensors/sensor_manager.h"
#include <Arduino.h>
#include <Adafruit_SHT4x.h>

extern Adafruit_SHT4x sht4;

static int16_t lastValidTemperature = 0;
static bool firstReading = true;
static uint32_t lastValidReadTime = 0;
static uint32_t lastErrorTime = 0;
static bool sensorInErrorState = false;
bool sht4Available = false;

void initSensors() {
    if (!sht4Available) {
        sht4Available = sht4.begin();
    }
}

void readSensors() {
    if (!sht4Available) {
        return;
    }
    sensors_event_t humidity_event, temp_event;
    if (sht4.getEvent(&humidity_event, &temp_event)) {
        extern SensorManager sensorManager;
        int16_t tempRaw = (int16_t)(temp_event.temperature * 10.0f);
        int16_t humRaw = (int16_t)(humidity_event.relative_humidity * 10.0f);
        sensorManager.updateTemperature(temp_event.temperature, tempRaw);
        sensorManager.updateHumidity(humidity_event.relative_humidity, humRaw);
        sensorManager.setValid(tempRaw > 0 && humRaw > 0);
    }
}

bool validateTemperatureReading(int16_t temp) {
    return temp != (int16_t)(SECURITY_SENSOR_ERROR_VALUE * 10.0f) && 
           !isnan(temp) && 
           temp >= SENSOR_TEMP_MIN && 
           temp <= SENSOR_TEMP_MAX && 
           (lastValidTemperature <= 0 || (temp - lastValidTemperature) <= (int16_t)(SECURITY_SENSOR_MAX_TEMP_CHANGE * 10.0f));
}

int16_t readThermistorTemperature() {
    int analogValue = analogRead(NTC_PIN);
    uint32_t currentTime = millis();
    if (analogValue < SECURITY_MIN_ADC_VALUE || analogValue > SECURITY_MAX_ADC_VALUE) {
        if (!sensorInErrorState) lastErrorTime = currentTime, sensorInErrorState = true;
        return (int16_t)(SECURITY_SENSOR_ERROR_VALUE * 10.0f);
    }
    // R_ntc = R_series * (ADC_max/ADC - 1); temperature via Beta equation (same as master / R4)
    float resistance = THERMISTOR_SERIES_RESISTOR *
                       (ADC_MAX / (float)analogValue - 1.0f);
    if (resistance <= 0.0f) {
        if (!sensorInErrorState) lastErrorTime = currentTime, sensorInErrorState = true;
        return (int16_t)(SECURITY_SENSOR_ERROR_VALUE * 10.0f);
    }
    float tKelvin = resistance / NTC_DEFAULT_RESISTANCE_25C;
    tKelvin = log(tKelvin);
    tKelvin /= NTC_DEFAULT_B_VALUE;
    tKelvin += 1.0f / (NTC_DEFAULT_TEMP_25C + 273.15f);
    tKelvin = 1.0f / tKelvin;
    float tempC = tKelvin - 273.15f;
    int16_t temp_c = (int16_t)(tempC * 10.0f);
    if (firstReading) {
        lastValidTemperature = temp_c;
        firstReading = false;
        lastValidReadTime = currentTime;
        sensorInErrorState = false;
        return temp_c;
    }
    if (sensorInErrorState) {
        if (currentTime - lastErrorTime > SECURITY_SENSOR_RESET_TIME) {
            if (validateTemperatureReading(temp_c)) {
                sensorInErrorState = false;
                lastValidTemperature = temp_c;
                lastValidReadTime = currentTime;
                return temp_c;
            } else {
                lastErrorTime = currentTime;
                return (int16_t)(SECURITY_SENSOR_ERROR_VALUE * 10.0f);
            }
        }
        return (int16_t)(SECURITY_SENSOR_ERROR_VALUE * 10.0f);
    }
    if (!validateTemperatureReading(temp_c)) {
        sensorInErrorState = true;
        lastErrorTime = currentTime;
        return (int16_t)(SECURITY_SENSOR_ERROR_VALUE * 10.0f);
    }
    lastValidTemperature = temp_c;
    lastValidReadTime = currentTime;
    return temp_c;
}

