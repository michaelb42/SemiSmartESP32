// Core Arduino libraries
#include <Arduino.h>
#include <Wire.h>
#include <EEPROM.h>

// Display libraries
#include <Adafruit_SSD1306.h>

// Sensor libraries
#include <Adafruit_SHT4x.h>

// Project configuration
#include "user_config.h"
#include "include/core/system_config.h"

// Core system modules
#include "include/core/state_manager.h"
#include "include/sensors/sensor_manager.h"
#include "include/control/control.h"

// Utility modules
#include "include/utils/storage.h"
#include "include/utils/strings.h"
#include "include/utils/tasks.h"

// Specialized modules
#include "include/control/pid_manager.h"
#include "include/safety/thermal_security.h"
#include "include/display/view_manager.h"
#include "include/interface/gpio_interface.h"
#include "include/sensors/sensors.h"

SystemIntervals systemIntervals;
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
Adafruit_SHT4x sht4 = Adafruit_SHT4x();

struct Timer {
  uint32_t interval;
  uint32_t last;
  Timer(uint32_t interval_) : interval(interval_), last(0) {}
  bool tick() {
    uint32_t now = millis();
    if (now - last >= interval) {
      last = now;
      return true;
    }
    return false;
  }
};

Timer sensorTimer(2000);
Timer pidTimer(100);
Timer displayTimer(300);
Timer thermalTimer(100);
Timer stateTimer(250);

bool buttonAction = false;
bool buttonPlus = false;
bool buttonMinus = false;
bool buttonActionPrev = false;
bool buttonPlusPrev = false;
bool buttonMinusPrev = false;
uint32_t lastButtonTime = 0;

void setup() {
  Wire.begin();

  display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  pinMode(PIN_HEATER, OUTPUT);
  pinMode(PIN_FAN, OUTPUT);
  pinMode(PIN_BUTTON_PLUS, INPUT);
  pinMode(PIN_BUTTON_MINUS, INPUT);
  pinMode(PIN_ACTION_BUTTON, INPUT);

  initStorage();
  sensorManager.begin();
  stateManager.begin();
  viewManager.begin(display);

  extern bool sht4Available;
  if (!sht4Available) {
    pinMode(LED_BUILTIN, OUTPUT);
    while(1) {
      digitalWrite(LED_BUILTIN, HIGH);
      delay(200);
      digitalWrite(LED_BUILTIN, LOW);
      delay(200);
    }
  }

  initControl();
  initThermalSecurity();
  setupPID();
  initGPIOInterface();
  
  viewManager.setView(ViewID::SPLASH);
  viewManager.draw();
}

void loop() {
  if (sensorTimer.tick()) sensorTask();
  if (pidTimer.tick()) {
    pidTask();
    fanTask();
  }
  if (displayTimer.tick()) displayTask();
  if (thermalTimer.tick()) thermalTask();
  
  // Handle splash screen timer - priority over other state changes
  static uint32_t splashStartTime = 0;
  if (viewManager.getCurrentView() == ViewID::SPLASH) {
    if (splashStartTime == 0) splashStartTime = millis();
    if (millis() - splashStartTime >= 2500) {
      viewManager.setView(ViewID::STANDBY);
      stateManager.setCurrentView(ViewID::STANDBY);
      splashStartTime = 0;
    }
  } else if (stateTimer.tick()) updateStateTask();
  
  handleButtonLogic();
}

void handleButtonLogic() {
  uint32_t now = millis();
  
  static uint32_t actionPressStartTime = 0;
  static bool longPressActionTaken = false;

  buttonAction = digitalRead(PIN_ACTION_BUTTON) == HIGH;
  buttonPlus = digitalRead(PIN_BUTTON_PLUS) == HIGH;
  buttonMinus = digitalRead(PIN_BUTTON_MINUS) == HIGH;

  if (buttonAction && !buttonActionPrev) {
    actionPressStartTime = now;
    longPressActionTaken = false;
  }

  if (buttonAction && (now - actionPressStartTime >= 2000) && !longPressActionTaken) {
    // Long press (2s) - toggle ON/OFF or go to ON from OFF/STANDBY
          if (stateManager.getControlMode() == CONTROL_USER) {
        bool turnOn = !stateManager.isSystemOn();
        stateManager.setSystemState(turnOn ? SYSTEM_ON : SYSTEM_OFF);
        setStandbyMode(false);
        if (turnOn && stateManager.getMode() == DRY_MODE_BY_TIME) {
          stateManager.setDryStartTime(millis());
        } else if (!turnOn) {
          handleSystemShutdown();
        }
      }
    longPressActionTaken = true;
  }

  if (!buttonAction && buttonActionPrev && !longPressActionTaken) {
    // Short press - toggle between OFF and STANDBY when system is not ON
    if (!stateManager.isSystemOn()) {
      SystemState newState = stateManager.isSystemOff() ? SYSTEM_STANDBY : SYSTEM_OFF;
      stateManager.setSystemState(newState);
      setStandbyMode(newState == SYSTEM_STANDBY);
      if (newState == SYSTEM_OFF) handleSystemShutdown();
    } else if (stateManager.getControlMode() == CONTROL_USER) {
      // System is ON - handle editing mode (only in USER mode)
      ViewID currentView = stateManager.getCurrentView();
      if (currentView == ViewID::TEMP || currentView == ViewID::HUM || currentView == ViewID::MODE || currentView == ViewID::DRY_TIME) {
        stateManager.setEditing(!stateManager.isEditing());
        if (!stateManager.isEditing()) {
          switch(currentView) {
            case ViewID::TEMP: saveTargetTempEEPROM(stateManager.getTargetTemp()); break;
            case ViewID::HUM: saveTargetHumEEPROM(stateManager.getTargetHumidity()); break;
            case ViewID::MODE: saveModeEEPROM(stateManager.getMode()); break;
            case ViewID::DRY_TIME: saveDryDurationEEPROM(stateManager.getDryDuration()); break;
          }
        }
      }
    }
  }

  if (buttonPlus && ((stateManager.isSystemOn() && stateManager.getControlMode() == CONTROL_USER) || (stateManager.isSystemStandby()))) {
    if (!stateManager.isEditing() && !buttonPlusPrev) {
      if (stateManager.isSystemStandby()) {
        // Only allow mode change in STANDBY state
        #if GPIO_INTERFACE_ENABLED
        stateManager.setControlMode(stateManager.getControlMode() == CONTROL_USER ? CONTROL_AUTO : CONTROL_USER);
        saveControlModeEEPROM(stateManager.getControlMode());
        #else
        // When GPIO is disabled, only USER mode is available
        stateManager.setControlMode(CONTROL_USER);
        saveControlModeEEPROM(CONTROL_USER);
        #endif
      } else if (stateManager.isSystemOn()) {
        ViewID currentView = stateManager.getCurrentView();
        ViewID nextView;
        switch(currentView) {
            case ViewID::INFO: nextView = ViewID::TEMP; break;
            case ViewID::TEMP: nextView = (stateManager.getMode() == DRY_MODE_BY_HUM) ? ViewID::HUM : ViewID::DRY_TIME; break;
            case ViewID::HUM: nextView = ViewID::INFO; break;
            case ViewID::DRY_TIME: nextView = ViewID::INFO; break;
            default: nextView = ViewID::INFO; break;
        }
        stateManager.setCurrentView(nextView);
        viewManager.setView(nextView);
      }
    } else if (stateManager.isEditing() && now - lastButtonTime >= 100) {
      switch(stateManager.getCurrentView()) {
        case ViewID::TEMP:
          {
            uint8_t currentTemp = stateManager.getTargetTemp();
            if (currentTemp < TEMP_MAX) stateManager.setTargetTemp(currentTemp + 1);
          }
          break;
        case ViewID::HUM:
          if (stateManager.getTargetHumidity() < HUM_MAX) stateManager.setTargetHumidity(stateManager.getTargetHumidity() + 1);
          break;
        case ViewID::MODE:
          stateManager.setMode(DRY_MODE_BY_HUM);
          break;
        case ViewID::DRY_TIME:
          if (stateManager.getDryDuration() < DRY_DURATION_MAX) {
            stateManager.setDryDuration(stateManager.getDryDuration() + DRY_DURATION_STEP);
            stateManager.setDryStartTime(millis());
          }
          break;
      }
      lastButtonTime = now;
    }
  }

  if (buttonMinus && stateManager.isSystemOn() && stateManager.getControlMode() == CONTROL_USER) {
    if (!stateManager.isEditing() && !buttonMinusPrev) {
      ViewID currentView = stateManager.getCurrentView();
      ViewID nextView = (currentView == ViewID::INFO) ? ViewID::MODE : ViewID::INFO;
      stateManager.setCurrentView(nextView);
      viewManager.setView(nextView);
    } else if (stateManager.isEditing() && now - lastButtonTime >= 100) {
      switch(stateManager.getCurrentView()) {
        case ViewID::TEMP:
          {
            uint8_t currentTemp = stateManager.getTargetTemp();
            if (currentTemp > TEMP_MIN) stateManager.setTargetTemp(currentTemp - 1);
          }
          break;
        case ViewID::HUM:
          if (stateManager.getTargetHumidity() > HUM_MIN) stateManager.setTargetHumidity(stateManager.getTargetHumidity() - 1);
          break;
        case ViewID::MODE:
          stateManager.setMode(DRY_MODE_BY_TIME);
          stateManager.setDryStartTime(millis());
          break;
        case ViewID::DRY_TIME:
          if (stateManager.getDryDuration() > DRY_DURATION_MIN) {
            stateManager.setDryDuration(stateManager.getDryDuration() - DRY_DURATION_STEP);
            stateManager.setDryStartTime(millis());
          }
          break;
      }
      lastButtonTime = now;
    }
  }

  buttonActionPrev = buttonAction;
  buttonPlusPrev = buttonPlus;
  buttonMinusPrev = buttonMinus;

  // Don't change view if in splash screen
  if (viewManager.getCurrentView() == ViewID::SPLASH) {
    return;
  }

  if (!stateManager.isSystemOn()) {
    if (viewManager.getCurrentView() != ViewID::STANDBY) {
      viewManager.setView(ViewID::STANDBY);
      stateManager.setCurrentView(ViewID::STANDBY);
    }
  } else {
    ViewID currentView = viewManager.getCurrentView();
    // Only force INFO view if coming from STANDBY or if view is incompatible with current mode
    if (currentView == ViewID::STANDBY) {
      viewManager.setView(ViewID::INFO);
      stateManager.setCurrentView(ViewID::INFO);
    } else if ((stateManager.getMode() == DRY_MODE_BY_HUM && currentView == ViewID::DRY_TIME) ||
               (stateManager.getMode() == DRY_MODE_BY_TIME && currentView == ViewID::HUM)) {
      viewManager.setView(ViewID::INFO);
      stateManager.setCurrentView(ViewID::INFO);
    }
  }
}

