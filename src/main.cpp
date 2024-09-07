#include <Arduino.h>
#include "ui/OLED.h"
#include "Buzzer.h"
#include "global.h"
#include <DRV8870.h>
#include <WiFiManager.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <ArduinoOTA.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <Update.h>

DRV8870 main_motor(AIN1_PIN, AIN2_PIN);
WiFiManager wifiManager;
WebServer server(80);
const char* host = "esp32";
/*
 * Login page
 */
const char* loginIndex = "<form name='loginForm'>"
    "<table width='20%' bgcolor='A09F9F' align='center'>"
        "<tr>"
            "<td colspan=2>"
                "<center><font size=4><b>ESP32 Login Page</b></font></center>"
                "<br>"
            "</td>"
            "<br>"
            "<br>"
        "</tr>"
        "<tr>"
             "<td>Username:</td>"
             "<td><input type='text' size=25 name='userid'><br></td>"
        "</tr>"
        "<br>"
        "<br>"
        "<tr>"
            "<td>Password:</td>"
            "<td><input type='Password' size=25 name='pwd'><br></td>"
            "<br>"
            "<br>"
        "</tr>"
        "<tr>"
            "<td><input type='submit' onclick='check(this.form)' value='Login'></td>"
        "</tr>"
    "</table>"
"</form>"
"<script>"
    "function check(form)"
    "{"
    "if(form.userid.value=='admin' && form.pwd.value=='admin')"
    "{"
    "window.open('/serverIndex')"
    "}"
    "else"
    "{"
    " alert('Error Password or Username')/*displays error message*/"
    "}"
    "}"
"</script>";

/*
 * Server Index Page
 */

const char* serverIndex = "<script src='https://ajax.googleapis.com/ajax/libs/jquery/3.2.1/jquery.min.js'></script>"
"<form method='POST' action='#' enctype='multipart/form-data' id='upload_form'>"
   "<input type='file' name='update'>"
        "<input type='submit' value='Update'>"
    "</form>"
 "<div id='prg'>progress: 0%</div>"
 "<script>"
  "$('form').submit(function(e){"
  "e.preventDefault();"
  "var form = $('#upload_form')[0];"
  "var data = new FormData(form);"
  " $.ajax({"
  "url: '/update',"
  "type: 'POST',"
  "data: data,"
  "contentType: false,"
  "processData:false,"
  "xhr: function() {"
  "var xhr = new window.XMLHttpRequest();"
  "xhr.upload.addEventListener('progress', function(evt) {"
  "if (evt.lengthComputable) {"
  "var per = evt.loaded / evt.total;"
  "$('#prg').html('progress: ' + Math.round(per*100) + '%');"
  "}"
  "}, false);"
  "return xhr;"
  "},"
  "success:function(d, s) {"
  "console.log('success!')"
 "},"
 "error: function (a, b, c) {"
 "}"
 "});"
 "});"
 "</script>";

void setup0()
{
  Serial.begin(9600);
  delay(1000);
  Serial.println("Starting application...");
  OLED::oled_setup();
  pinMode(TOUCH_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(WSENSE_PIN, INPUT_PULLUP);
  pinMode(MIN_EN, OUTPUT);
  pinMode(MOUT_EN, OUTPUT);
  main_motor.setMaxSpeed(255);
  Buzzer::startup();

  // wifiManager.resetSettings();
  OLED::oled_wifi_cfg("ESP-Washer", "20242024");

  wifiManager.autoConnect("ESP-Washer", "20242024");
  // WiFi连接成功后将通过串口监视器输出连接成功信息
  Serial.println("");
  Serial.print("ESP32 Connected to ");
  Serial.println(WiFi.SSID()); // WiFi名称
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP()); // IP
  Serial.print("PSK:\t");
  Serial.println(WiFi.psk()); // IP
  
}

void setupOTA() {

  /*use mdns for host name resolution*/
  if (!MDNS.begin(host)) { //http://esp32.local
    Serial.println("Error setting up MDNS responder!");
    while (1) {
      delay(1000);
    }
  }
  Serial.println("mDNS responder started");
  /*return index page which is stored in serverIndex */
  server.on("/", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", loginIndex);
  });
  server.on("/serverIndex", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", serverIndex);
  });
  /*handling uploading firmware file */
  server.on("/update", HTTP_POST, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
    ESP.restart();
  }, []() {
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
      Serial.printf("Update: %s\n", upload.filename.c_str());
      if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { //start with max available size
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      /* flashing firmware to ESP*/
      if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_END) {
      if (Update.end(true)) { //true to set the size to the current progress
        Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
      } else {
        Update.printError(Serial);
      }
    }
  });
  server.begin();
}

// Wash durations in seconds
#define WASH_DURATION_80MIN 80*60
#define WASH_DURATION_50MIN 3000
// 脱水时间
#define SPINNING_TIME  5 * 60 * 1000
// 排水时间
#define DRAINING_TIME  5 * 60 * 1000

State currentState = INITIALIZING;
unsigned int washDuration = WASH_DURATION_80MIN;
unsigned int startTime;
bool paused = false;
unsigned int remainingTime;
uint16_t waterLevel = 0;
volatile bool touched = false;
bool otaMode = false;

void onStatusChanged()
{
  OLED::displayStatus(currentState, remainingTime, waterLevel);
}

// ISR for the touch button
void IRAM_ATTR handleTouch()
{
  touched = true;
  Buzzer::touch();
  // onStatusChanged();
}

void setup()
{
  setup0();

  attachInterrupt(TOUCH_PIN, handleTouch, RISING);

  remainingTime = washDuration;
  onStatusChanged();
    // To use a specific port and path uncomment this line
    // Defaults are 8080 and "/webota"
    // webota.init(8888, "/update");

    // If you call useAuth() in your setup function WebOTA will use
    // HTTP digest authentication to request credentials from the user
    // before allowing uploads
    // webota.useAuth("username", "password");
  if (digitalRead(TOUCH_PIN) == HIGH)
  {
    otaMode = true;
    setupOTA();
    OLED::oled_ota();
  }
}

void loop()
{
  if (otaMode) {
    server.handleClient();
    delay(1);
    return;
  }
  waterLevel = digitalRead(WSENSE_PIN);
  // if (currentState == SELECTING_TIME && digitalRead(TOUCH_PIN) == HIGH) {
  //   onStatusChanged();
  // }
  // 3秒后进入加水状态
  if (currentState == SELECTING_TIME && millis() - startTime >= 3000)
  {
    Buzzer::startup();
    currentState = FILLING_WATER;
    startTime = millis();
    onStatusChanged();
    digitalWrite(MIN_EN, HIGH);
  }
  // 加满水后进入清洗状态
  if (currentState == FILLING_WATER)
  {
    if (waterLevel == LOW)
    {
      digitalWrite(MIN_EN, LOW);
      currentState = WASHING;
      startTime = millis();
      remainingTime = washDuration;
      onStatusChanged();
    }
    else if (millis() - startTime >= 15 * 60 * 1000)
    {
      // 15分钟未加满水，进入错误状态
      digitalWrite(MIN_EN, LOW);
      currentState = ERROR;
      onStatusChanged();
      Buzzer::error();
    }
  }

  if (currentState == WASHING && !paused)
  {
    remainingTime = washDuration - (millis() - startTime) / 1000;
    if (remainingTime <= 0)
    { // 进入排水状态
      currentState = DRAINING;
      onStatusChanged();
      main_motor.brake(BRAKE);
      digitalWrite(MOUT_EN, HIGH);
      startTime = millis();
    }
    else if ((millis() - startTime) / 10000 % 2 == 0)
    { // 每10秒反转一次
      main_motor.setSpeed(255, CLOCKWISE);
    }
    else
    {
      main_motor.setSpeed(255, COUNTERCLOCKWISE);
    }
  }
  
  // 排水时间
  if (currentState == DRAINING && millis() - startTime >= 20000)
  {
    currentState = SPINNING;
    startTime = millis();
    onStatusChanged();
    main_motor.setSpeed(255, CLOCKWISE);
  }
  
  // 脱水(甩干)
  if (currentState == SPINNING && millis() - startTime >= (SPINNING_TIME))
  {
    main_motor.brake(BRAKE);
    // 脱水完毕，关闭排水电磁阀
    digitalWrite(MOUT_EN, LOW);
    currentState = (washDuration == WASH_DURATION_50MIN) ? DONE : FILLING_WATER;
    if (currentState == FILLING_WATER)
    {
      washDuration = WASH_DURATION_80MIN; // Reset for the second cycle
      remainingTime = washDuration;
      startTime = millis();
      digitalWrite(MIN_EN, HIGH);
    }
    onStatusChanged();
  }

  if (currentState == DONE)
  {
    // 洗衣结束，停止电机
    main_motor.brake(BRAKE);
    digitalWrite(MOUT_EN, LOW);
    digitalWrite(MIN_EN, LOW);
  }

  if (touched)
  {
    touched = false;

    // 切换洗衣时长
    if (currentState == SELECTING_TIME || currentState == INITIALIZING)
    {
      startTime = millis();
      currentState = SELECTING_TIME;
      washDuration = (washDuration == WASH_DURATION_80MIN) ? WASH_DURATION_50MIN : WASH_DURATION_80MIN;
      remainingTime = washDuration;
    }
    else if (currentState == WASHING || currentState == PAUSED)
    {
      // 暂停/继续
      paused = !paused;
      currentState = paused ? PAUSED : WASHING;
      if (paused) {
        main_motor.brake(COAST);
      }
    }
  }
  // Update display every second
  if (millis() % 1000 == 0)
  {
    // Serial.println("##!");
    onStatusChanged();
  }
}