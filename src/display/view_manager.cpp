#include "../../include/display/view_manager.h"
#include "../../include/control/control.h"
#include "../../include/safety/thermal_security.h"
#include "../../include/utils/strings.h"
#include "../../include/control/pid_manager.h"
#include <Adafruit_SSD1306.h>
#include <Fonts/FreeSans9pt7b.h>

ViewManager viewManager;

// Avoid sprintf (pulls large printf family on AVR)
static void fmtHHMM(char* buf, uint16_t h, uint16_t m) {
    buf[0] = (char)('0' + (h / 10) % 10);
    buf[1] = (char)('0' + h % 10);
    buf[2] = ':';
    buf[3] = (char)('0' + (m / 10) % 10);
    buf[4] = (char)('0' + m % 10);
    buf[5] = '\0';
}

void ViewManager::begin(Adafruit_SSD1306& disp) {
    _display = &disp;
}

void ViewManager::draw() {
    if (!_display) return;
    switch (_currentView) {
        case ViewID::SPLASH: drawSplash(); break;
        case ViewID::STANDBY: drawStandby(); break;
        case ViewID::INFO: drawInfo(); break;
        case ViewID::TEMP: drawTempConfig(); break;
        case ViewID::HUM: drawHumConfig(); break;
        case ViewID::MODE: drawModeConfig(); break;
        case ViewID::DRY_TIME: drawDryTimeConfig(); break;
        default: drawInfo(); break;
    }
}

void ViewManager::drawSplash() {
    _display->clearDisplay();

    _display->setTextSize(2);
    _display->setTextColor(SSD1306_WHITE);
    _display->setCursor(24, 15);
    printProgmemString(_display, str_voxel3d);

    _display->setTextSize(1);
    _display->setCursor(15, 44);
    printProgmemString(_display, str_code_by);

    _display->display();
}

void ViewManager::drawStandby() {
    _display->clearDisplay();
    _display->setTextSize(2);
    _display->setTextColor(SSD1306_WHITE);
    const char* msg;
    FanState fanState = getFanState();
    
    // Determine main status message (SYSTEM OFF, STANDBY, CYC, COOLING)
    if (stateManager.isSystemOff()) {
        msg = getProgmemString(str_system_off);
    } else if (fanState == FAN_COOLDOWN) {
        msg = getProgmemString(str_cooling);
    } else if (fanState == FAN_STANDBY_CYCLING_ON) {
        msg = getProgmemString(str_cycling);
    } else {
        msg = getProgmemString(str_standby);
    }
    
    // Show main status (SYSTEM OFF centered, others in normal position)
    _display->setCursor(stateManager.isSystemOff() ? 5 : 18, 4);
    _display->print(msg);
    
    _display->setTextSize(1);
    _display->setCursor(0, 22);
    printProgmemString(_display, str_temp_label);
    _display->print(sensorManager.getTemperature(), 1);
    printProgmemString(_display, str_space);
    printProgmemString(_display, getTemperatureUnit());
    _display->setCursor(0, 35);
    printProgmemString(_display, str_humidity_label);
    _display->print(sensorManager.getHumidity(), 1);
    printProgmemString(_display, str_percent);
    
    // Show control mode or cooling status at bottom
    if (stateManager.isSystemOff() && fanState == FAN_COOLDOWN) {
        // Show COOLING at bottom when in OFF state and cooldown
        _display->setTextSize(2);
        _display->setCursor(23, 48);
        printProgmemString(_display, str_cooling);
    } else if (!stateManager.isSystemOff()) {
        // Show control mode when not in OFF state
        _display->setTextSize(2);
        _display->setCursor(0, 48);
        printProgmemString(_display, stateManager.getControlMode() == CONTROL_AUTO ? str_auto : str_user);
    }
    _display->display();
}

void ViewManager::drawInfo() {
    extern StateManager stateManager;
    extern GyverPID heaterPID;
    _display->clearDisplay();
    static uint8_t animFrame = 0;
    static uint32_t lastAnim = 0;
    uint32_t currentTime = millis();
    if (currentTime - lastAnim > systemIntervals.animation) {
        animFrame = !animFrame;
        lastAnim = currentTime;
    }
    float displayTargetTemp = stateManager.getTargetTemp();
    _display->fillRect(0, 0, 128, 13, SSD1306_WHITE);
    _display->setTextColor(SSD1306_BLACK);
    _display->setTextSize(1);
    _display->setCursor(4, 3);
    if (getThermalSecurityState() == THERMAL_PROTECTION && animFrame) {
        printProgmemString(_display, str_warning);
    } else {
        if (stateManager.getMode() == DRY_MODE_BY_HUM) {
            printProgmemString(_display, str_printing);
        } else {
            printProgmemString(_display, str_drying);
            uint32_t remainingSeconds = stateManager.getRemainingDryTime();
            uint16_t hours = remainingSeconds / SECONDS_PER_HOUR;
            uint16_t minutes = (remainingSeconds % SECONDS_PER_HOUR) / 60;
            char timeBuf[6];
            fmtHHMM(timeBuf, hours, minutes);
            _display->setCursor(70, 3);
            printProgmemString(_display, str_eta_label);
            _display->print(timeBuf);
        }
    }
    _display->setTextColor(SSD1306_WHITE);
    _display->setFont(&FreeSans9pt7b);
    _display->setCursor(0, 30);
    _display->print(sensorManager.getTemperature(), 1);
    printProgmemString(_display, getTemperatureUnit());
    printProgmemString(_display, str_greater);
    _display->print(displayTargetTemp, 0);
    printProgmemString(_display, getTemperatureUnit());
    _display->setCursor(0, 48);
    _display->print(sensorManager.getHumidity(), 1);
    printProgmemString(_display, str_percent);
    if (stateManager.getMode() == DRY_MODE_BY_HUM) printProgmemString(_display, str_greater), _display->print(stateManager.getTargetHumidity()), printProgmemString(_display, str_percent);
    _display->setCursor(0, 62);
    _display->setFont();
    _display->setTextSize(1);
    bool heaterOn = stateManager.isHeaterOn();
    _display->setCursor(0, 55);
    if (heaterOn && animFrame) _display->fillRect(0, 52, 60, 12, SSD1306_WHITE);
    _display->setTextColor(heaterOn && animFrame ? SSD1306_BLACK : SSD1306_WHITE);
    if (heaterOn) {
        printProgmemString(_display, str_heat_label);
        _display->print((int)(heaterPID.output * 100.0f / 255.0f));
        printProgmemString(_display, str_percent);
    } else {
        printProgmemString(_display, str_heat_off);
    }
    ThermalSecurityState thermalState = getThermalSecurityState();
    FanState fanState = getFanState();
    bool fanActive = (thermalState == THERMAL_COOLDOWN) || (fanState == FAN_COOLDOWN) || isFanOn();
    _display->setCursor(70, 55);
    if (fanActive && animFrame) _display->fillRect(68, 52, 60, 12, SSD1306_WHITE);
    _display->setTextColor(fanActive && animFrame ? SSD1306_BLACK : SSD1306_WHITE);
    printProgmemString(_display, (thermalState == THERMAL_COOLDOWN) ? str_balance : 
                       (fanState == FAN_COOLDOWN) ? str_cooling :
                       isFanOn() ? str_cycling : str_fan_off);
    _display->setTextColor(SSD1306_WHITE);
    _display->display();
}

void ViewManager::drawTempConfig() {
    extern StateManager stateManager;
    _display->clearDisplay();
    _display->setTextSize(1);
    _display->setTextColor(SSD1306_WHITE);
    _display->setCursor(0, 0);
    printProgmemString(_display, str_set_temp);
    _display->setTextSize(2);
    if (stateManager.isEditing()) _display->fillRect(0, 15, 128, 30, SSD1306_WHITE);
    _display->setTextColor(stateManager.isEditing() ? SSD1306_BLACK : SSD1306_WHITE);
    _display->setCursor(40, 20);
    _display->print(stateManager.getTargetTemp());
    printProgmemString(_display, getTemperatureUnit());
    _display->setTextColor(SSD1306_WHITE);
    _display->setTextSize(1);
    _display->setCursor(0, 50);
    printProgmemString(_display, str_change_save);
    _display->display();
}

void ViewManager::drawHumConfig() {
    extern StateManager stateManager;
    _display->clearDisplay();
    _display->setTextSize(1);
    _display->setTextColor(SSD1306_WHITE);
    _display->setCursor(0, 0);
    printProgmemString(_display, str_set_humidity);
    _display->setTextSize(2);
    if (stateManager.isEditing()) _display->fillRect(0, 15, 128, 30, SSD1306_WHITE);
    _display->setTextColor(stateManager.isEditing() ? SSD1306_BLACK : SSD1306_WHITE);
    _display->setCursor(40, 20);
    _display->print(stateManager.getTargetHumidity());
    printProgmemString(_display, str_percent);
    _display->setTextColor(SSD1306_WHITE);
    _display->setTextSize(1);
    _display->setCursor(0, 50);
    printProgmemString(_display, str_change_save);
    _display->display();
}

void ViewManager::drawModeConfig() {
    extern StateManager stateManager;
    _display->clearDisplay();
    _display->setTextSize(1);
    _display->setTextColor(SSD1306_WHITE);
    _display->setCursor(0, 0);
    printProgmemString(_display, str_operation_mode);
    _display->setTextSize(2);
    if (stateManager.isEditing()) _display->fillRect(0, 15, 128, 30, SSD1306_WHITE);
    _display->setTextColor(stateManager.isEditing() ? SSD1306_BLACK : SSD1306_WHITE);
    _display->setCursor(0, 20);
    printProgmemString(_display, stateManager.getMode() == DRY_MODE_BY_HUM ? str_by_hum : str_dry);
    _display->setTextColor(SSD1306_WHITE);
    _display->setTextSize(1);
    _display->setCursor(0, 50);
    printProgmemString(_display, str_change_save);
    _display->display();
}

void ViewManager::drawDryTimeConfig() {
    extern StateManager stateManager;
    _display->clearDisplay();
    _display->setTextSize(1);
    _display->setTextColor(SSD1306_WHITE);
    _display->setCursor(0, 0);
    printProgmemString(_display, str_dry_mode_time);
    _display->setTextSize(2);
    if (stateManager.isEditing()) _display->fillRect(0, 15, 128, 30, SSD1306_WHITE);
    _display->setTextColor(stateManager.isEditing() ? SSD1306_BLACK : SSD1306_WHITE);
    _display->setCursor(30, 20);
    uint32_t duration = stateManager.getDryDuration();
    uint16_t hours = duration / 60;
    uint16_t minutes = duration % 60;
    
    char timeBuf[6];
    fmtHHMM(timeBuf, hours, minutes);
    _display->print(timeBuf);
    _display->setTextColor(SSD1306_WHITE);
    _display->setTextSize(1);
    _display->setCursor(0, 50);
    printProgmemString(_display, str_change_save);
    _display->display();
} 

 