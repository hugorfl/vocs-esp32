#include <DHT.h>
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
#define TEMP_HUMIDITY_DELAY 2000

#define MQ_PIN 36
#define MQ_DELAY 30000

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

#define OCI_ENDPOINT "https://g14fcd8d7fc5e27-db70cmj.adb.ca-toronto-1.oraclecloudapps.com/ords/iot/vocs/insertVoc/"
#define REST_DELAY 300000

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
TaskHandle_t restTask;
SemaphoreHandle_t semaphore;

bool isInitialiazed = false;
bool isDebugModeEnabled = false;

int currLed = 0;





const int Object_Properties = 5;
const int Array_Elements = 50;

// TODO: 
const size_t CAPACITY = JSON_ARRAY_SIZE(Array_Elements) + Array_Elements * JSON_OBJECT_SIZE(Object_Properties);

int measure_counter = 0;
String payloadPOST = "";

struct Measure {
    String deviceId;
    String deviceName;
    String temperature;
    String humidity;
    String toluene;
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
  xTaskCreatePinnedToCore(
    tempHumiditySensorTaskLoop,
    "tempHumiditySensorTask",
    TASK_STACK_SIZE,
    NULL,
    2,
    &tempHumidityTask,
    TASK_CORE_ONE);
  xTaskCreatePinnedToCore(ledsSemaphoreTaskLoop, "ledsSemaphoreTask", TASK_STACK_SIZE, NULL, 1, &ledsSemaphoreTask, TASK_CORE_ONE);
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
      saveMeasure(String(temperature), String(humidity), "12test");

      printLocalHistory();
    }
    
    delay(MQ_DELAY);
    // delay(TEMP_HUMIDITY_DELAY);
  }
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





/*
void test() {
    Serial.println("**************************************");

    // step 1: Save as many measures as required in a certain period of time
    saveMeasure("10a", "11", "12");
    saveMeasure("20b", "21", "22");
    saveMeasure("30c", "31", "32");
    saveMeasure("40d", "41", "42");
    
    // optional, for debugging
    // printLocalHistory(); 

    // step 2: Send data to server
    sendDataToServer();

    // step 3: Clear history
    clearLocalHistory();
}
*/

void saveMeasure(String temperature, String humidity, String toluene) {
  char deviceName[64];

  info.getDeviceName(deviceName, sizeof(deviceName));
  
    localHistory[measure_counter].deviceId = info.getChipModel();
    localHistory[measure_counter].deviceName = deviceName;
    localHistory[measure_counter].temperature = temperature;
    localHistory[measure_counter].humidity = humidity;
    localHistory[measure_counter].toluene = toluene;
    measure_counter++;
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

    // Initiate HTTP client
    HTTPClient http;

    // Start request
    http.begin(OCI_ENDPOINT);

    // Define content-type to send
    http.addHeader("Content-Type", "application/json");


Serial.println("############### Payload #############");

Serial.println(payloadPOST);

Serial.println("############### End Payload #############");

    // Execute POST request
    int httpCode = http.POST(payloadPOST);


    Serial.println(" ");
    Serial.printf("> [POST] response code: %d\n", httpCode);

    // Terminate connection
    http.end();
}

void printLocalHistory() {
    for (int i = 0; i < measure_counter; i++) {
        Serial.print("----------------------------- ");
        Serial.println(i);
        Serial.println("Temperature: " + localHistory[i].temperature);
        Serial.println("Humidity: " + localHistory[i].humidity);
        Serial.println("Toluene: " + localHistory[i].toluene);
    }
}
