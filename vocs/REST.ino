#include "WiFi.h"
#include "HTTPClient.h"
#include "ArduinoJson.h"

const char* SSID = "Wokwi-GUEST";
const char* SSID_PASSWORD = "";
const char* serverName = "https://g14fcd8d7fc5e27-db70cmj.adb.ca-toronto-1.oraclecloudapps.com/ords/iot/vocs/insertVoc/";

String serverResponse = "";
String payloadPOST = "";


const size_t CAPACITY = JSON_ARRAY_SIZE(3);



void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  //wifiConnect(); // enable for send/read DataFromServer methods
}

void loop() {

  //test_createJSON();
  //test_sendDataToServer();
  //test_readDataFromServer();

  delay(2000); //Wait two seconds for next joke
}



void testCreateJSON() {

  StaticJsonDocument<CAPACITY> doc;

  JsonObject obj = doc.createNestedObject();
  obj["device_id"] = "dd-1";
  obj["device_name"] = "Demo Device 1";
  obj["temperature"] = "23";
  obj["humidity"] = "33";
  obj["toluene"] = "9876";

  serializeJson(doc, payloadPOST);

  Serial.println("JSON= " + payloadPOST);

}



void testSendDataToServer() {

  //Initiate HTTP client
  HTTPClient http;

  //Start request
  http.begin(serverName);

  //Define content-type to send
  http.addHeader("Content-Type", "application/json");
  
  //Execute POST request
  int httpCode = http.POST("[{\"device_id\":88,\"device_name\":\"ESP32\",\"temperature\":\"90\",\"humidity\":\"90\",\"toluene\":\"111111\" }]");
  
  Serial.printf("[POST] response code: %d\n", httpCode);

  //Terminate connection
  http.end();

}


void testReadDataFromServer() {

  //Initiate HTTP client
  HTTPClient http;

  //Start request
  http.begin(serverName);

  //Execute GET request
  int httpCode = http.GET();

  Serial.printf("[GET] response code: %d\n", httpCode);
  
  // read response from server
  serverResponse = http.getString();

  Serial.print("RESPONSE: " + serverResponse);

  //Terminate connection
  http.end();

}

void wifiConnect() {

  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID, SSID_PASSWORD);

  Serial.print("Stablishing connection with WiFi");
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