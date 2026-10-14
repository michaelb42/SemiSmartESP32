#ifndef STATE_MANAGER_H
#define STATE_MANAGER_H

#include <stdint.h>
#include "system_config.h"

// System states
enum SystemState : uint8_t {
    SYSTEM_OFF,      // Completely off, no cycling
    SYSTEM_STANDBY,  // With air cycling
    SYSTEM_ON        // Normal operation
};

enum class ViewID : uint8_t {
    SPLASH = 0,     // New splash screen with logo
    STANDBY = 1,
    INFO = 2,
    TEMP = 3,
    HUM = 4,
    MODE = 5,
    DRY_TIME = 6
};

class StateManager {
public:
    // Initialization
    void begin();
    
    // System state
    bool isSystemOn() const { return _systemState == SYSTEM_ON; }
    void setSystemOn(bool on) { 
        if (on) {
            _systemState = SYSTEM_ON;
            _systemOn = true;
        } else {
            _systemState = SYSTEM_OFF;
            _systemOn = false;
        }
    }
    
    // System state management
    SystemState getSystemState() const { return _systemState; }
    void setSystemState(SystemState state) { 
        _systemState = state;
        _systemOn = (state == SYSTEM_ON);
    }
    bool isSystemOff() const { return _systemState == SYSTEM_OFF; }
    bool isSystemStandby() const { return _systemState == SYSTEM_STANDBY; }
    
    // Component state
    bool isHeaterOn() const { return _heaterOn; }
    void setHeaterOn(bool on) { _heaterOn = on; }
    
    bool isFanOn() const { return _fanOn; }
    void setFanOn(bool on) { _fanOn = on; }
    
    // Operation mode
    OperationMode getMode() const { return _mode; }
    void setMode(OperationMode mode) { _mode = mode; }
    
    // Control mode
    ControlMode getControlMode() const { return _controlMode; }
    void setControlMode(ControlMode mode) { 
        #if GPIO_INTERFACE_ENABLED
        _controlMode = mode; 
        if (mode == CONTROL_AUTO) _mode = DRY_MODE_BY_HUM;
        #else
        // When GPIO is disabled, only USER mode is allowed
        _controlMode = CONTROL_USER;
        #endif
    }
    
    // Setpoints
    uint8_t getTargetTemp() const { return _targetTemp; }
    void setTargetTemp(uint8_t temp) { _targetTemp = temp; }
    
    uint8_t getTargetHumidity() const { return _targetHumidity; }
    void setTargetHumidity(uint8_t hum) { _targetHumidity = hum; }
    
    // Drying time
    uint32_t getDryDuration() const { return _dryDuration; }
    void setDryDuration(uint32_t duration) { _dryDuration = duration; }
    
    uint32_t getDryStartTime() const { return _dryStartTime; }
    void setDryStartTime(uint32_t time) { _dryStartTime = time; }
    
    // Current view
    ViewID getCurrentView() const { return _currentView; }
    void setCurrentView(ViewID view) { _currentView = view; }
    
    // Edit mode
    bool isEditing() const { return _editing; }
    void setEditing(bool editing) { _editing = editing; }
    

    
    // Print state
    bool isPrinting() const { return _printing; }
    void setPrintingState(bool printing) { _printing = printing; }
    
    // Remaining time in DRY mode (seconds)
    uint32_t getRemainingDryTime() const;

private:
    bool _systemOn = false;
    bool _heaterOn = false;
    bool _fanOn = false;
    bool _printing = false;
    OperationMode _mode = DRY_MODE_BY_HUM;
    #if GPIO_INTERFACE_ENABLED
    ControlMode _controlMode = CONTROL_AUTO;
    #else
    ControlMode _controlMode = CONTROL_USER;
    #endif
    uint8_t _targetTemp = DEFAULT_TEMP;
    uint8_t _targetHumidity = DEFAULT_HUM;
    uint32_t _dryDuration = 60;  // Default 60 minutes
    uint32_t _dryStartTime = 0;
    ViewID _currentView = ViewID::STANDBY;
    bool _editing = false;
    SystemState _systemState = SYSTEM_OFF; // Added for system state management
};

// Global instance
extern StateManager stateManager;

#endif // STATE_MANAGER_H 