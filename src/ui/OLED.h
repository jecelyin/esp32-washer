//
// @author Jecelyin<jecelyin@gmail.com>
//

#pragma once

#include <Arduino.h>
#include "global.h"

namespace OLED {
  void oled_setup(void);
  void oled_loop(void);
  void oled_ota();
  void oled_wifi_cfg(String ssid, String password);
  void displayStatus(State status, int timeInSeconds, uint16_t waterLevel);
}