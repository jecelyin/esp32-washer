//
// @author Jecelyin<jecelyin@gmail.com>
//

#include "OLED.h"
#include <U8g2lib.h>
#include <Wire.h>
#include "u8g2_font_pingfangsc_12.h"
#include "u8g2_font_pingfangsc_b_24.h"
// extern const uint8_t u8g2_font_pingfangsc_12[1639] U8G2_FONT_SECTION("u8g2_font_pingfangsc_12");
// extern const uint8_t u8g2_font_pingfangsc_b_24[1724] U8G2_FONT_SECTION("u8g2_font_pingfangsc_b_24");

namespace OLED
{
  U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* clock=*/SCL, /* data=*/SDA, /* reset=*/U8X8_PIN_NONE); // High speed I2C

  static const unsigned char image_wifi_bits[] U8X8_PROGMEM = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x03, 0x78, 0x1c, 0x0e, 0x20, 0xc2, 0x43, 0x70, 0x0c, 0x08, 0x10, 0xc0, 0x01, 0x60, 0x06, 0x00, 0x00, 0x80, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  static const unsigned char image_time_bits[] U8X8_PROGMEM = {0x00, 0x00, 0x00, 0x80, 0x0f, 0x00, 0xe0, 0x7b, 0x00, 0x30, 0xc0, 0x00, 0x18, 0x80, 0x01, 0x0c, 0x06, 0x03, 0x04, 0x06, 0x02, 0x04, 0x06, 0x06, 0x06, 0x06, 0x06, 0x02, 0x06, 0x06, 0x06, 0x04, 0x04, 0x06, 0x08, 0x06, 0x06, 0x10, 0x02, 0x04, 0x20, 0x02, 0x0c, 0x00, 0x03, 0x18, 0x80, 0x01, 0x30, 0xc0, 0x00, 0xe0, 0x7d, 0x00, 0x00, 0x1f, 0x00, 0x00, 0x00, 0x00};
  static const unsigned char image_water_level_bits[] U8X8_PROGMEM = {0x00, 0x00, 0x80, 0x01, 0x40, 0x02, 0x20, 0x04, 0x30, 0x0c, 0x10, 0x08, 0x08, 0x10, 0x08, 0x10, 0x08, 0x10, 0xf8, 0x19, 0x08, 0x1e, 0x08, 0x10, 0x18, 0x18, 0x30, 0x0c, 0xc0, 0x07, 0x00, 0x00};
  static const unsigned char image_play_bits[] U8X8_PROGMEM = {0x00, 0x00, 0x00, 0x00, 0x1c, 0x00, 0x80, 0xff, 0x01, 0xe0, 0x81, 0x03, 0x70, 0x00, 0x0e, 0x30, 0x00, 0x1c, 0x18, 0x00, 0x18, 0x0c, 0x00, 0x30, 0x0c, 0x06, 0x30, 0x04, 0x1e, 0x20, 0x04, 0x76, 0x60, 0x06, 0xc6, 0x61, 0x06, 0xe6, 0x60, 0x06, 0x76, 0x20, 0x04, 0x3e, 0x20, 0x0c, 0x0e, 0x30, 0x0c, 0x00, 0x30, 0x18, 0x00, 0x18, 0x38, 0x00, 0x0c, 0x70, 0x00, 0x0e, 0xc0, 0x81, 0x07, 0x80, 0xff, 0x01, 0x00, 0x38, 0x00, 0x00, 0x00, 0x00};
  static const unsigned char image_timer_bits[] U8X8_PROGMEM = {0xc0, 0x01, 0xc0, 0x03, 0x80, 0x01, 0xe0, 0x17, 0xf0, 0x1f, 0x38, 0x1c, 0x1c, 0x38, 0x8c, 0x31, 0xcc, 0x31, 0xee, 0x71, 0x0c, 0x30, 0x0c, 0x30, 0x1c, 0x38, 0xf8, 0x1f, 0xf0, 0x0f, 0xc0, 0x03};
  static const unsigned char image_pause_one_bits[] U8X8_PROGMEM = {0x00, 0x00, 0x00, 0x00, 0x1c, 0x00, 0x80, 0xff, 0x01, 0xe0, 0x81, 0x03, 0x70, 0x00, 0x0e, 0x30, 0x00, 0x1c, 0x18, 0x00, 0x18, 0x0c, 0x00, 0x30, 0x0c, 0x42, 0x30, 0x04, 0x42, 0x20, 0x04, 0x42, 0x60, 0x06, 0x42, 0x60, 0x06, 0x42, 0x60, 0x06, 0x42, 0x20, 0x04, 0x42, 0x20, 0x0c, 0x42, 0x30, 0x0c, 0x00, 0x30, 0x18, 0x00, 0x18, 0x38, 0x00, 0x0c, 0x70, 0x00, 0x0e, 0xc0, 0x81, 0x07, 0x80, 0xff, 0x01, 0x00, 0x38, 0x00, 0x00, 0x00, 0x00};
  static bool blink = false;

  void oled_setup(void)
  {
    u8g2.begin();
    u8g2.enableUTF8Print();
    u8g2.setFontPosTop();
    u8g2.setFontMode(1);
    u8g2.setBitmapMode(1);
  }

  void oled_wifi_cfg(String ssid, String password)
  {
    u8g2.clearBuffer();                    // clear the internal memory
    u8g2.setFont(u8g2_font_pingfangsc_12); // choose a suitable font

    u8g2.setCursor(0, 0);
    u8g2.drawXBMP(18, 5, 16, 16, image_wifi_bits);
    u8g2.drawUTF8(43, 5, "WiFi配置");
    u8g2.drawStr(12, 22, "SSID: ");     // write something to the internal memory
    u8g2.drawStr(44, 22, ssid.c_str()); // write something to the internal memory

    u8g2.drawUTF8(12, 36, "密码:");         // write something to the internal memory
    u8g2.drawStr(44, 36, password.c_str()); // write something to the internal memory
    u8g2.sendBuffer();                      // transfer internal memory to the display
    delay(1000);
  }

  void oled_ota()
  {
    u8g2.clearBuffer();                    // clear the internal memory
    u8g2.setFont(u8g2_font_pingfangsc_12); // choose a suitable font

    u8g2.setCursor(0, 0);
    u8g2.drawXBMP(18, 5, 16, 16, image_wifi_bits);
    u8g2.drawStr(43, 5, "OTA Mode");
    u8g2.drawStr(12, 22, "http://esp32.local/");

    u8g2.drawUTF8(12, 36, "admin/admin"); 
    u8g2.sendBuffer();                      // transfer internal memory to the display
    delay(1000);
  }

  void displayProgress(State status, int waterLevel)
  {
    u8g2.drawXBMP(21, 4, 16, 16, image_water_level_bits);
    u8g2.setCursor(38, 7);
    u8g2.print(waterLevel);
    if (status != PAUSED)
    {
      u8g2.drawXBMP(110, 32, 16, 16, image_timer_bits);
      u8g2.drawXBMP(9, 28, 24, 24, image_play_bits);
    }
    else
    {
      u8g2.drawXBMP(9, 28, 24, 24, image_pause_one_bits);
    }
    switch (status)
    {
    case FILLING_WATER:
      u8g2.drawUTF8(69, 6, "正在进水");
      break;
    case WASHING:
      u8g2.drawUTF8(69, 6, "清洗中");
      break;
    case DRAINING:
      u8g2.drawUTF8(69, 6, "正在排水");
      break;
    case SPINNING:
      u8g2.drawUTF8(69, 6, "正在脱水");
      break;
    case PAUSED:
      u8g2.drawUTF8(69, 6, "已暂停");
      break;
    default:
      break;
    }
    
  }

  void displayStatus(State status, int timeInSeconds, uint16_t waterLevel)
  {
    bool showTime = false;

    u8g2.clearBuffer();
    u8g2.drawXBMP(4, 4, 16, 16, image_wifi_bits);
    u8g2.setFont(u8g2_font_pingfangsc_12);

    if (status == SELECTING_TIME || status == INITIALIZING)
    {
      u8g2.drawUTF8(40, 6, "清洗时长");
      u8g2.drawXBMP(12, 31, 20, 20, image_time_bits);
      u8g2.setFont(u8g2_font_pingfangsc_b_24);
      showTime = true;
    }
    else if (status == ERROR)
    {
      u8g2.setFont(u8g2_font_pingfangsc_b_24);
      u8g2.drawUTF8(32, 21, "Error!");
    }
    else if (status != DONE)
    {
      displayProgress(status, waterLevel);
      showTime = true;
    }
    else
    {
      u8g2.setFont(u8g2_font_pingfangsc_b_24);
      u8g2.drawUTF8(32, 21, "清洗完毕");
    }

    if (showTime) {
      u8g2.setFont(u8g2_font_pingfangsc_b_24);
      int minutes = timeInSeconds / 60;
      int seconds = timeInSeconds % 60;
      char buffer[3];

      sprintf(buffer, "%02d", minutes);
      u8g2.drawStr(37, 30, buffer);
      if (status == PAUSED || blink) {
        u8g2.drawStr(67, 28, ":");
      }
      blink = !blink;

      sprintf(buffer, "%02d", seconds);
      u8g2.drawStr(76, 30, buffer);
    }
    u8g2.sendBuffer();
  }

} // namespace OLED