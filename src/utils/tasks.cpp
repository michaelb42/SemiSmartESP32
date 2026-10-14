#include "../../include/utils/tasks.h"
#include "../../include/display/view_manager.h"
#include "../../include/sensors/sensor_manager.h"
#include "../../include/core/state_manager.h"
#include "../../include/control/control.h"
#include "../../include/safety/thermal_security.h"
#include "../../include/core/system_config.h"
#include "../../include/interface/gpio_interface.h"

extern ViewManager viewManager;
extern SensorManager sensorManager;
extern StateManager stateManager;
extern ControlSystem controlSystem;

void displayTask() {
  viewManager.draw();
}

void sensorTask() {
  sensorManager.update();
}

void fanTask() {
    controlSystem.updateFanState();
    controlSystem.updateFanTransition();
}

void thermalTask() {
    updateThermalSecurity();
}

void updateStateTask() {
    stateManager.setHeaterOn(controlSystem.isHeaterOn());
    stateManager.setFanOn(controlSystem.isFanOn());
    #if GPIO_INTERFACE_ENABLED
    pollGPIOInterface();
    #endif
} 