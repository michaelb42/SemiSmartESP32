#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include <Arduino.h>
#include <stdint.h>

class SensorManager {
public:
    void begin();
    void update();
    float getTemperature() const { return _temperature; }
    float getHumidity() const { return _humidity; }
    bool isValid() const { return _valid; }
    int16_t getTemperatureRaw() const { return _temperatureRaw; }
    int16_t getHumidityRaw() const { return _humidityRaw; }
    bool isSHT4Available() const { return _sht4Available; }
    void updateTemperature(float temp, int16_t tempRaw);
    void updateHumidity(float hum, int16_t humRaw);
    void setValid(bool valid);
private:
    float _temperature = 25.0f;
    float _humidity = 50.0f;
    int16_t _temperatureRaw = 250;  // 25.0°C * 10
    int16_t _humidityRaw = 500;     // 50.0% * 10
    bool _valid = true;
    bool _sht4Available = false;
};

extern SensorManager sensorManager;

#endif // SENSOR_MANAGER_H 