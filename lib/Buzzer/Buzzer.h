//
// @author Jecelyin<jecelyin@gmail.com>
//

#pragma once

#define BUZZER_PIN 25

namespace Buzzer {
  void startup();
  void complete();
  void touch();
  void error();
}