#pragma once

#define TOUCH_PIN 26
#define WSENSE_PIN 12
#define AIN1_PIN 4
#define AIN2_PIN 16
#define MIN_EN 14
#define MOUT_EN 27

enum State {
  INITIALIZING,
  SELECTING_TIME,
  FILLING_WATER,
  WASHING,
  DRAINING,
  SPINNING,
  PAUSED,
  DONE,
  ERROR
};