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
const char *host = "esp32";
/*
 * Login page
 */
const char *loginIndex = "<form name='loginForm'>"
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

const char *serverIndex = "<script src='https://ajax.googleapis.com/ajax/libs/jquery/3.2.1/jquery.min.js'></script>"
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

void setupOTA()
{

  /*use mdns for host name resolution*/
  if (!MDNS.begin(host))
  { // http://esp32.local
    Serial.println("Error setting up MDNS responder!");
    while (1)
    {
      delay(1000);
    }
  }
  Serial.println("mDNS responder started");
  /*return index page which is stored in serverIndex */
  server.on("/", HTTP_GET, []()
            {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", loginIndex); });
  server.on("/serverIndex", HTTP_GET, []()
            {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", serverIndex); });
  /*handling uploading firmware file */
  server.on("/update", HTTP_POST, []()
            {
    server.sendHeader("Connection", "close");
    server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
    ESP.restart(); }, []()
            {
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
    } });
  server.begin();
}

// Wash durations in seconds
#define LONG_WASH_DURATION 80 * 60
#define SHORT_WASH_DURATION 60 * 60
// 脱水时间
#define SPINNING_TIME 2 * 60
// 排水时间
#define DRAINING_TIME 4 * 60

State currentState = INITIALIZING;
State prevState = INITIALIZING;
unsigned int washDuration = 0;
// 每个运行状态的工作时长（不包含暂停时间）
unsigned int workingTime = 0;
// 拆分成2次来洗
int washTimes = 2;
// 每次洗的时长
unsigned int washTime;
// 总累计时间
unsigned int totalTime = 0;
unsigned long prevTime;
unsigned long selectingStartTime, fillingStartTime;
// unsigned int remainingTime;
uint16_t waterLevel = 0;
volatile bool touched = false;
bool otaMode = false;
const unsigned long interval = 1000; // 间隔时间为1000毫秒 (1秒)
unsigned long previousMillis = 0;    // 记录上次执行时间计算任务的时间

void onStatusChanged()
{
  int remainingTime = washDuration - totalTime;
  OLED::displayStatus(currentState, remainingTime, waterLevel);
  int minutes = remainingTime / 60;
  int seconds = remainingTime % 60;
  Serial.print("Current state: ");
  Serial.print(currentState);
  Serial.print(" Remaining time: ");
  Serial.print(minutes);
  Serial.print(":");
  Serial.println(seconds);
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
  selectingStartTime = millis();
}

void startFillingWater()
{
  Buzzer::startup();
  currentState = FILLING_WATER;
  workingTime = 0;
  fillingStartTime = millis();
  prevTime = millis();
  
  onStatusChanged();
  digitalWrite(MIN_EN, HIGH);
}

void fillingError()
{
  digitalWrite(MIN_EN, LOW);
  currentState = ERROR;
  onStatusChanged();
  Buzzer::error();
}

void startWashing()
{
  digitalWrite(MIN_EN, LOW);
  currentState = WASHING;
  workingTime = 0;
  washTime = washDuration / washTimes - (DRAINING_TIME + SPINNING_TIME);
  onStatusChanged();
}

void startDraining()
{
  currentState = DRAINING;
  onStatusChanged();
  main_motor.brake(BRAKE);
  digitalWrite(MOUT_EN, HIGH);
  workingTime = 0;
}

void washing()
{
  if (workingTime >= washTime)
  { // 进入排水状态
    startDraining();
  }
  else if (workingTime / 10 % 2 == 0)
  { // 每10秒反转一次
    main_motor.setSpeed(255, CLOCKWISE);
    delay(500);
  }
  else
  {
    main_motor.setSpeed(255, COUNTERCLOCKWISE);
    delay(500);
  }
}

void startSpinning()
{
  currentState = SPINNING;
  workingTime = 0;
  onStatusChanged();
  main_motor.setSpeed(255, CLOCKWISE);
}

void spinningEnd()
{
  main_motor.brake(BRAKE);
  // 脱水完毕，关闭排水电磁阀
  digitalWrite(MOUT_EN, LOW);
  // 时间还有1分钟以上，进入下一次洗涤，避免时间计算误差
  if (totalTime < washDuration - 60)
  {
    startFillingWater();
  }
  else
  {
    currentState = DONE;
  }
}

void done()
{
  // 洗衣结束，停止电机
  main_motor.brake(BRAKE);
  digitalWrite(MOUT_EN, LOW);
  digitalWrite(MIN_EN, LOW);
  onStatusChanged();
}

bool isRunning()
{
  return currentState == FILLING_WATER || currentState == WASHING || currentState == DRAINING || currentState == SPINNING;
}

void onTouch()
{
  // 切换洗衣时长
  if (currentState == SELECTING_TIME || currentState == INITIALIZING)
  {
    currentState = SELECTING_TIME;
    washDuration = (washDuration == SHORT_WASH_DURATION) ? LONG_WASH_DURATION : SHORT_WASH_DURATION;
    selectingStartTime = millis();
    onStatusChanged();
    return;
  }
  if (isRunning())
  {
    prevState = currentState;
    Serial.print("paused, prev state: ");
    Serial.println(prevState);
    if (currentState == FILLING_WATER)
    {
      digitalWrite(MIN_EN, LOW);
    }
    else if (currentState == WASHING)
    {
      main_motor.brake(BRAKE);
    }
    else if (currentState == DRAINING)
    {
      digitalWrite(MOUT_EN, LOW);
    }
    else if (currentState == SPINNING)
    {
      main_motor.brake(BRAKE);
      digitalWrite(MOUT_EN, LOW);
    }
    currentState = PAUSED;
    onStatusChanged();
    return;
  }
  if (currentState == PAUSED)
  {
    currentState = prevState;
    prevTime = millis();
    if (currentState == FILLING_WATER)
    {
      digitalWrite(MIN_EN, HIGH);
    }
    else if (currentState == WASHING)
    {
      main_motor.setSpeed(255, CLOCKWISE);
    }
    else if (currentState == DRAINING)
    {
      digitalWrite(MOUT_EN, HIGH);
    }
    else if (currentState == SPINNING)
    {
      main_motor.setSpeed(255, CLOCKWISE);
      digitalWrite(MOUT_EN, HIGH);
    }
    onStatusChanged();
    return;
  }
}

void updateTotalTime()
{
  unsigned long now = millis();
  totalTime = totalTime + (now - prevTime) / 1000;
  workingTime = workingTime + (now - prevTime) / 1000;
  prevTime = now;
}

void loop()
{
  if (otaMode)
  {
    server.handleClient();
    delay(1);
    return;
  }
  waterLevel = digitalRead(WSENSE_PIN);
  // if (currentState == SELECTING_TIME && digitalRead(TOUCH_PIN) == HIGH) {
  //   onStatusChanged();
  // }
  // 3秒后进入加水状态
  if (currentState == SELECTING_TIME && millis() - selectingStartTime >= 3000)
  {
    totalTime = 0;
    startFillingWater();
  }
  // 加满水后进入清洗状态
  if (currentState == FILLING_WATER)
  {
    if (waterLevel == LOW)
    {
      startWashing();
    }
    else if (millis() - fillingStartTime >= 5 * 60 * 1000)
    {
      // 5分钟未加满水，进入错误状态
      fillingError();
    }
  }

  if (currentState == WASHING)
  {
    washing();
  }

  // 排水时间
  if (currentState == DRAINING && workingTime >= DRAINING_TIME)
  {
    startSpinning();
  }

  // 脱水(甩干)
  if (currentState == SPINNING && workingTime >= SPINNING_TIME)
  {
    spinningEnd();
  }

  if (currentState == DONE)
  {
    done();
  }

  if (touched)
  {
    touched = false;

    onTouch();
  }

  unsigned long currentMillis = millis();

  // 检查是否到达1秒的间隔
  if (isRunning() && currentMillis - previousMillis >= interval)
  {
    previousMillis = currentMillis; // 更新上次执行任务的时间

    updateTotalTime();

    onStatusChanged();
  }
}