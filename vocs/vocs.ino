#include <DHT.h>
#include <MQUnifiedsensor.h>
#include <WiFiManager.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include "Button.h"
#include "DeviceInfo.h"
#include "Display.h"
#include "SpanishDisplayStrings.h"

// ##### Sensors
#define DHT_PIN 13
#define DHT_TYPE DHT22

#define MQ_PIN 36
#define RatioMQ135CleanAir 3.6
#define MQ_TYPE "MQ-135"

#define METRICS_DELAY 3000

// ##### Input devices
#define BTN_ACK_PIN 12

#define DEBUG_PIN 14

// ##### Ouput devices
#define LCD_I2C_ADDR 0x27
#define SCREEN_REFRESH_MS 1000

#define LED_GREEN_PIN 15
#define LED_YELLOW_PIN 0
#define LED_RED_PIN 4

#define BUZZER_PIN 16

// ##### WiFi
#define WIFI_AP_DEFAULT_PWD "abcd1234"

#define OCI_ENDPOINT "https://g14fcd8d7fc5e27-db70cmj.adb.ca-toronto-1.oraclecloudapps.com/ords/iot/vocs/insertVoc/"
#define SAVE_METRICS_DELAY 3000
#define REST_DELAY 15000

// ##### Cores programming
#define TASK_STACK_SIZE 10240 // 10kb
#define TASK_CORE_ZERO 0
#define TASK_CORE_ONE 1


// ##### Globals
DeviceInfo info;
LangDisplayStrings* strings = new SpanishDisplayStrings();

DHT dht(DHT_PIN, DHT_TYPE);
MQUnifiedsensor mq ("ESP-32", 5, 12, MQ_PIN, MQ_TYPE);
Display lcd(LCD_I2C_ADDR);
Button btnAck(BTN_ACK_PIN);

TaskHandle_t lcdTask;
TaskHandle_t metricsTask;
TaskHandle_t ackButtonTask;
TaskHandle_t restTask;
SemaphoreHandle_t semaphore;

bool isInitialiazed = false;
bool isDebugModeEnabled = false;
bool isDataReady = false;
bool buzzerEnabled = false;

float temperature = 0.0f;
float humidity = 0.0f;
float toluene = 0.0f;

const int Object_Properties = 5;
const int Array_Elements = 50;

const size_t CAPACITY = JSON_ARRAY_SIZE(Array_Elements) + Array_Elements * JSON_OBJECT_SIZE(Object_Properties);

int measure_counter = 0;
String payloadPOST = "";

struct Measure {
    String deviceId;
    String deviceName;
    float temperature;
    float humidity;
    float toluene;
};

struct Measure localHistory[Array_Elements];

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
  xTaskCreatePinnedToCore(metricsTaskLoop, "metricsTask", TASK_STACK_SIZE, NULL, 2, &metricsTask, TASK_CORE_ONE);
  xTaskCreatePinnedToCore(ackButtonTaskLoop, "ackButtonTask", TASK_STACK_SIZE, NULL, 1, &ackButtonTask, TASK_CORE_ONE);
  xTaskCreatePinnedToCore(restTaskLoop, "restTask", TASK_STACK_SIZE, NULL, 1, &restTask, TASK_CORE_ONE);
}

void initDevices() {
  const char* text = strings->initializingText();
  
  lcd.init();
  lcd.push(text, sizeof(text));

  pinMode(BUZZER_PIN, OUTPUT);

  pinMode(LED_GREEN_PIN, OUTPUT);
  pinMode(LED_YELLOW_PIN, OUTPUT);
  pinMode(LED_RED_PIN, OUTPUT);

  dht.begin();
  initMqSensor();

  pinMode(DEBUG_PIN, INPUT);

  if (digitalRead(DEBUG_PIN) == HIGH) {
    isDebugModeEnabled = true;
    lcd.push("Modo Debug", sizeof("Modo Debug"));
  }
  
  btnAck.init(INPUT_PULLUP);
  btnAck.addInterrupt(isrBtnAck, FALLING);

  initWifiService();
}

void initMqSensor() {
  mq.setRegressionMethod(1);
  mq.setA(44.947);
  mq.setB(-3.445);

  float calcR0 = 0;
  for(int i = 1; i<=10; i ++)
  {
    mq.update(); // Update data, the arduino will read the voltage from the analog pin
    calcR0 += mq.calibrate(RatioMQ135CleanAir);
    Serial.print(".");
  }
  mq.setR0(calcR0/10);

  mq.init();
}

void initWifiService() {
  WiFiManager wm;
  char ssid[32];

  //TODO: reset settings - remove after testing
  // wm.resetSettings();
  // wm.setTimeout(120);
  wm.setAPCallback(accessPointModeCallback);

  info.getDeviceName(ssid);

  bool isConnected = wm.autoConnect(ssid, WIFI_AP_DEFAULT_PWD);
}

void accessPointModeCallback(WiFiManager* wm) {
  char ssid[32];
  info.getDeviceName(ssid);
  
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
    printMetrics();
    lcd.show();
    xSemaphoreGive(semaphore);
    
    delay(SCREEN_REFRESH_MS);
  }
}

void metricsTaskLoop(void* param) {
  while(true) {
    xSemaphoreTake(semaphore, portMAX_DELAY);
    mq.update();
    toluene = mq.readSensor();
    temperature = dht.readTemperature();
    humidity = dht.readHumidity();
    xSemaphoreGive(semaphore);

    Serial.printf("tol: %f, tmp: %f, hum: %f\n", toluene, temperature, humidity);

    if (isInitialiazed) {
      saveMeasure(temperature, humidity, toluene);
      printLocalHistory();
    }
    
    delay(SAVE_METRICS_DELAY);
  }
}

void saveMeasure(float temperature, float humidity, float toluene) {
  char deviceName[64];

  info.getDeviceName(deviceName, sizeof(deviceName));

  localHistory[measure_counter].deviceId = info.getChipModel();
  localHistory[measure_counter].deviceName = deviceName;
  localHistory[measure_counter].temperature = temperature;
  localHistory[measure_counter].humidity = humidity;
  localHistory[measure_counter].toluene = toluene;
  measure_counter++;
}

void restTaskLoop(void* param) {
  while (true) {
    xSemaphoreTake(semaphore, portMAX_DELAY);
    xSemaphoreGive(semaphore);
    
    if (isInitialiazed) {
      sendDataToServer();
      clearLocalHistory();
    }
    
    delay(REST_DELAY);
  }
}

void printMetrics() {
  static char tolueneText[32], temperatureText[32], humidityText[32];

  strings->joinTolueneText(toluene, LCD_COLS, tolueneText, sizeof(tolueneText));
  strings->joinHumidityText(humidity, LCD_COLS, humidityText, sizeof(humidityText));
  strings->joinTemperatureText(temperature, LCD_COLS, temperatureText, sizeof(temperatureText));

  lcd.put(0, temperatureText, sizeof(temperatureText));
  lcd.put(1, humidityText, sizeof(humidityText));
  lcd.show();
  delay(4000);
  lcd.put(0, tolueneText, sizeof(tolueneText));
  lcd.put(1, "", 0);
  lcd.show();
  delay(2000);
}

void ackButtonTaskLoop(void* param) {
  while(true) {
    if ((toluene >= 100 || buzzerEnabled) && !btnAck.getIsPressed()) {
      buzzerEnabled = true;
      digitalWrite(BUZZER_PIN, HIGH);
    }

    if (btnAck.getIsPressed()) {
      btnAck.releaseButton();
      buzzerEnabled = false;
      digitalWrite(BUZZER_PIN, LOW);
    }
    delay(1000);
    
  }
}

void sendDataToServer() {
  createJSONPayload();
  executePOST();
}

void clearLocalHistory() {
  measure_counter = 0;
  payloadPOST = "";
  memset(localHistory, 0, sizeof(localHistory));
}

void createJSONPayload() {
  StaticJsonDocument<CAPACITY> doc;

  for (int i = 0; i < measure_counter; i++)
  {
    JsonObject obj = doc.createNestedObject();
    obj["device_id"] = localHistory[i].deviceId;
    obj["device_name"] = localHistory[i].deviceName;
    obj["temperature"] = localHistory[i].temperature;
    obj["humidity"] = localHistory[i].humidity;
    obj["toluene"] = localHistory[i].toluene;
  }

  serializeJson(doc, payloadPOST);
}

void executePOST() {
  HTTPClient http;
  
  http.begin(OCI_ENDPOINT);
  http.addHeader("Content-Type", "application/json");
  
  Serial.println("############### Payload #############");
  Serial.println(payloadPOST);
  Serial.println("############### End Payload #############");
  
  int httpCode = http.POST(payloadPOST);
  Serial.printf("> [POST] response code: %d\n", httpCode);
  
  http.end();
}

void printLocalHistory() {
  for (int i = 0; i < measure_counter; i++) {
    Serial.print("----------------------------- ");
    Serial.println(i);
    Serial.print("Temperature: ");
    Serial.println(localHistory[i].temperature);
    Serial.print("Humidity: ");
    Serial.println(localHistory[i].humidity);
    Serial.print("Toluene: ");
    Serial.println(localHistory[i].toluene);
  }
}
