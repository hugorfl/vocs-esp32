#include <DHT.h>
#include <WiFiManager.h>
#include <pitches.h>
#include "Button.h"
#include "DeviceInfo.h"
#include "Display.h"
#include "SpanishDisplayStrings.h"

// ##### Sensors
#define DHT_PIN 13
#define DHT_TYPE DHT22
#define TEMP_HUMIDITY_DELAY 2000

#define MQ_PIN 36

// ##### Input devices
#define BTN_ACK_PIN 12

#define DEBUG_PIN 14

// ##### Ouput devices
#define LCD_I2C_ADDR 0x27
#define SCREEN_REFRESH_MS 1000

#define LED_GREEN_PIN 15
#define LED_YELLOW_PIN 2
#define LED_RED_PIN 4

#define BUZZER_PIN 16

// ##### WiFi
#define WIFI_AP_DEFAULT_PWD "abcd1234"

// ##### Cores programming
#define TASK_STACK_SIZE 10240 // 10kb
#define TASK_CORE_ZERO 0
#define TASK_CORE_ONE 1


// ##### Globals
DeviceInfo info;
LangDisplayStrings* strings = new SpanishDisplayStrings();

DHT dht(DHT_PIN, DHT_TYPE);
Display lcd(LCD_I2C_ADDR);
Button btnAck(BTN_ACK_PIN);

TaskHandle_t lcdTask;
TaskHandle_t tempHumidityTask;
TaskHandle_t ledsSemaphoreTask;
SemaphoreHandle_t semaphore;

bool isInitialiazed = false;
bool isDebugModeEnabled = false;

int currLed = 0;

void setup() {
  WiFi.mode(WIFI_STA);
  Serial.begin(115200);

  initTasks();
  initDevices();

  isInitialiazed = true;
}

void initTasks() {
  semaphore = xSemaphoreCreateMutex();
  xTaskCreatePinnedToCore(lcdTaskLoop, "lcdTask", TASK_STACK_SIZE, NULL, 1, &lcdTask, TASK_CORE_ONE);
  xTaskCreatePinnedToCore(
    tempHumiditySensorTaskLoop,
    "tempHumiditySensorTask",
    TASK_STACK_SIZE,
    NULL,
    2,
    &tempHumidityTask,
    TASK_CORE_ONE);
  xTaskCreatePinnedToCore(ledsSemaphoreTaskLoop, "ledsSemaphoreTask", TASK_STACK_SIZE, NULL, 1, &ledsSemaphoreTask, TASK_CORE_ONE);
}

void initDevices() {
  const char* text = strings->initializingText();
  
  lcd.init();
  lcd.push(text, sizeof(text));

  pinMode(BUZZER_PIN, OUTPUT);
  soundBuzzer();

  pinMode(LED_GREEN_PIN, OUTPUT);
  pinMode(LED_YELLOW_PIN, OUTPUT);
  pinMode(LED_RED_PIN, OUTPUT);

  dht.begin();

  pinMode(DEBUG_PIN, INPUT);

  if (digitalRead(DEBUG_PIN) == HIGH) {
    isDebugModeEnabled = true;
    lcd.push("Modo Debug", sizeof("Modo Debug"));
  }
  
  btnAck.init(INPUT_PULLUP);
  btnAck.addInterrupt(isrBtnAck, FALLING);

  initWifiService();
}

void initWifiService() {
  WiFiManager wm;
  char ssid[32];

  //TODO: reset settings - remove after testing
  wm.resetSettings();
  // wm.setTimeout(120);
  wm.setAPCallback(accessPointModeCallback);

  info.getSSIDName(ssid);

  bool isConnected = wm.autoConnect(ssid, WIFI_AP_DEFAULT_PWD);
}

void accessPointModeCallback(WiFiManager* wm) {
  char ssid[32];
  info.getSSIDName(ssid);
  
  printfWifiCredentialsLCD(ssid, WIFI_AP_DEFAULT_PWD);
}

void printfWifiCredentialsLCD(const char* ssid, const char* pwd) {
  static char ssidText[32], pwdText[16];
  
  strings->joinNetworkSSIDText(ssid, ssidText, sizeof(ssidText));
  strings->joinNetworkPwdText(pwd, pwdText, sizeof(pwdText));
  
  lcd.put(0, ssidText, sizeof(ssidText));
  lcd.put(1, pwdText, sizeof(pwdText));
}

void loop() {
  vTaskDelete(NULL);
}

ICACHE_RAM_ATTR void isrBtnAck() {
  btnAck.pressButton();
}

void lcdTaskLoop(void* param) {
  while(true) {
    xSemaphoreTake(semaphore, portMAX_DELAY);
    lcd.show();
    xSemaphoreGive(semaphore);
    
    delay(SCREEN_REFRESH_MS);
  }
}

void tempHumiditySensorTaskLoop(void* param) {
  float temperature;
  float humidity;
  
  while(true) {
    xSemaphoreTake(semaphore, portMAX_DELAY);
    temperature = dht.readTemperature();
    humidity = dht.readHumidity();
    xSemaphoreGive(semaphore);

    if (isInitialiazed) {
      printTemperatureHumidityMetrics(temperature, humidity);
      // TODO: rest connectivity
    }
    
    delay(TEMP_HUMIDITY_DELAY);
  }
}

void printTemperatureHumidityMetrics(float temperature, float humidity) {
  static char temperatureText[32], humidityText[32];
  
  strings->joinHumidityText(humidity, humidityText, sizeof(humidityText));
  strings->joinTemperatureText(temperature, temperatureText, sizeof(temperatureText));

  lcd.put(0, temperatureText, sizeof(temperatureText));
  lcd.put(1, humidityText, sizeof(humidityText));
}

void ledsSemaphoreTaskLoop(void* param) {
  while(true) {
    if (btnAck.getIsPressed()) {
      switch(currLed) {
        case 0:
          digitalWrite(LED_GREEN_PIN, HIGH);
          digitalWrite(LED_YELLOW_PIN, LOW);
          digitalWrite(LED_RED_PIN, LOW);
          currLed = 1;
          break;
        case 1:
          digitalWrite(LED_GREEN_PIN, LOW);
          digitalWrite(LED_YELLOW_PIN, HIGH);
          digitalWrite(LED_RED_PIN, LOW);
          currLed = 2;
          break;
        case 2:
          digitalWrite(LED_GREEN_PIN, LOW);
          digitalWrite(LED_YELLOW_PIN, LOW);
          digitalWrite(LED_RED_PIN, HIGH);
          currLed = 3;
          break;
        default:
          digitalWrite(LED_GREEN_PIN, LOW);
          digitalWrite(LED_YELLOW_PIN, LOW);
          digitalWrite(LED_RED_PIN, LOW);
          currLed = 0;
          btnAck.releaseButton();
      }
    }

    delay(1000);
  }
}


int melody[] = {
  NOTE_E5, NOTE_E5, NOTE_E5,
  NOTE_E5, NOTE_E5, NOTE_E5,
  NOTE_E5, NOTE_G5, NOTE_C5, NOTE_D5,
  NOTE_E5,
  NOTE_F5, NOTE_F5, NOTE_F5, NOTE_F5,
  NOTE_F5, NOTE_E5, NOTE_E5, NOTE_E5, NOTE_E5,
  NOTE_E5, NOTE_D5, NOTE_D5, NOTE_E5,
  NOTE_D5, NOTE_G5
};

int noteDurations[] = {
  8, 8, 4,
  8, 8, 4,
  8, 8, 8, 8,
  2,
  8, 8, 8, 8,
  8, 8, 8, 16, 16,
  8, 8, 8, 8,
  4, 4
};

void soundBuzzer() {
  int size = sizeof(noteDurations) / sizeof(int);

  for (int note = 0; note < size; note++) {
    int noteDuration = 1000 / noteDurations[note];
    tone(BUZZER_PIN, melody[note], noteDuration);

    int pauseBetweenNotes = noteDuration * 1.30;
    delay(pauseBetweenNotes);
    noTone(BUZZER_PIN);
  }
}
