#ifndef VIEW_MANAGER_H
#define VIEW_MANAGER_H

#include <stdint.h>
#include "../core/state_manager.h"
#include "../sensors/sensor_manager.h"

class Adafruit_SSD1306;

class ViewManager {
public:
    void begin(Adafruit_SSD1306& disp);
    void draw();
    void setView(ViewID view) { _currentView = view; }
    ViewID getCurrentView() const { return _currentView; }
private:
    void drawSplash();
    void drawStandby();
    void drawInfo();
    void drawTempConfig();
    void drawHumConfig();
    void drawModeConfig();
    void drawDryTimeConfig();
    Adafruit_SSD1306* _display = nullptr;
    ViewID _currentView = ViewID::STANDBY;
};

extern ViewManager viewManager;

#endif // VIEW_MANAGER_H 