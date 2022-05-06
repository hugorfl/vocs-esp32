#include "WiFi.h"
#include "HTTPClient.h"

// Change ssid and password according at your local connection
const char* ssid = "Wokwi-GUEST";
const char* password = "";
const char* serverName = "https://g14fcd8d7fc5e27-db70cmj.adb.ca-toronto-1.oraclecloudapps.com/ords/iot/vocs/insertVoc/";

String response = "";

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  wifiConnect();
}

void loop() {
  sendData();
  //testEndpointConnection();

  delay(2000); //Wait two seconds for next joke
}


// This method executes an HTTP POST action
void sendData() {

  Serial.println("Sending Data");

  //Initiate HTTP client
  HTTPClient http;

  //Start the request
  http.begin(serverName);

  http.addHeader("Content-Type", "application/json");

  int httpCode = http.POST("[{\"device_id\":90,\"device_name\":\"ESP32\",\"temperature\":\"90\",\"humidity\":\"90\",\"toluene\":\"111111\" }]");
  Serial.printf("[HTTP] POST... code: %d\n", httpCode);

  http.end();
}


void testEndpointConnection() {
  //Initiate HTTP client
  HTTPClient http;

  //Start the request
  http.begin(serverName);

  //Use HTTP GET request
  int httpCode = http.GET();
  Serial.printf("[HTTP] GET... code: %d\n", httpCode);

  //Response from server
  response = http.getString();

  Serial.print("RESPONSE: " + response);
  http.end();
}


void wifiConnect() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.print("Mac address: ");
  Serial.println(WiFi.macAddress());
}



/*

 bulk 5 minutos
JSON
[
    {
      "device_id":1,
      "device_name":"Uno",
      "temperature":"30",
      "humidity":"60",
      "toluene":"1000"
    },
    {
      "device_id":2,
      "device_name":"Dos",
      "temperature":"10",
      "humidity":"20",
      "toluene":"500"
    }
]
*/
