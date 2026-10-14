#include "../../include/interface/gpio_interface.h"
#include "../../include/core/system_config.h"
#include "../../include/core/state_manager.h"
#include "../../include/control/control.h"
#include "../../include/safety/thermal_security.h"
#include "../../include/sensors/print_status_sensor.h"

extern StateManager stateManager;
extern PrintStatusSensor printStatusSensor;

#if GPIO_INTERFACE_ENABLED

void initGPIOInterface() {
    printStatusSensor.begin();
}

void pollGPIOInterface() {
    static bool lastPrintState = false;
    bool currentPrintState = printStatusSensor.isPrinting();
    
    if (currentPrintState != lastPrintState) {
        stateManager.setPrintingState(currentPrintState);
        lastPrintState = currentPrintState;
    
    // Automatic system ON/OFF based on print status - only in AUTO control mode
    if (stateManager.getControlMode() == CONTROL_AUTO) {
            if (currentPrintState && !stateManager.isSystemOn()) {
            // Printer started printing - turn ON the system
            stateManager.setSystemState(SYSTEM_ON);
            setStandbyMode(false);
            stateManager.setMode(DRY_MODE_BY_HUM);
            } else if (!currentPrintState && stateManager.isSystemOn()) {
            // Printer stopped printing - turn OFF the system
            stateManager.setSystemState(SYSTEM_OFF);
            setStandbyMode(false);
            handleSystemShutdown();
        }
    }
    }
}

#endif // GPIO_INTERFACE_ENABLED 