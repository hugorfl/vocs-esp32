#include "WiFi.h"
#include "HTTPClient.h"
#include "ArduinoJson.h"

const char *SSID = "Wokwi-GUEST";
const char *SSID_PASSWORD = "";
const char *serverName = "https://g14fcd8d7fc5e27-db70cmj.adb.ca-toronto-1.oraclecloudapps.com/ords/iot/vocs/insertVoc/";

const int Object_Properties = 10;
const int Array_Elements = 4;

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

struct Measure localHistory[4];

void setup() {
    Serial.begin(115200);
    wifiConnect();
}

void loop() {

    test();

    delay(2000); // Wait two seconds for next joke
}

/*

Methods to use:
- saveMeasure()
- sendDataToServer();
- clearLocalHistory();

*/
void test() {
    Serial.println("**************************************");

    // step 1: Save as many measures as required in a certain period of time
    saveMeasure("10a", "11", "12");
    saveMeasure("20b", "21", "22");
    saveMeasure("30c", "31", "32");
    saveMeasure("40d", "41", "42");
    
    // optional, for debugging
    printLocalHistory(); 

    // step 2: Send data to server
    sendDataToServer();

    // step 3: Clear history
    clearLocalHistory();
}

void saveMeasure(String temperature, String humidity, String toluene) {
    localHistory[measure_counter].deviceId = "01";
    localHistory[measure_counter].deviceName = "VOCs-01-A";
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
    http.begin(serverName);

    // Define content-type to send
    http.addHeader("Content-Type", "application/json");

    // Execute POST request
    int httpCode = http.POST(payloadPOST);


    Serial.println(" ");
    Serial.printf("> [POST] response code: %d\n", httpCode);

    // Terminate connection
    http.end();
}

void printLocalHistory() {
    for (int i = 0; i < measure_counter; i++) {
        Serial.println("-----------------------------" + i);
        Serial.println("Temperature: " + localHistory[i].temperature);
        Serial.println("Humidity: " + localHistory[i].humidity);
        Serial.println("Toluene: " + localHistory[i].toluene);
    }
}

void wifiConnect() {

    WiFi.mode(WIFI_STA);
    WiFi.begin(SSID, SSID_PASSWORD);

    Serial.print("Stablishing WiFi connection");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println(" ");
    Serial.println("WiFi connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.print("Mac address: ");
    Serial.println(WiFi.macAddress());
}