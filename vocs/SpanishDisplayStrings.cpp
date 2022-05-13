#include "SpanishDisplayStrings.h"

SpanishDisplayStrings::SpanishDisplayStrings() { }

const char* SpanishDisplayStrings::initializingText() {
  return "Inicializando...";
}

void SpanishDisplayStrings::joinNetworkSSIDText(const char* ssid, char* resJoinNetworkText, int buffSize) {
  const char* text = "Red";

  strHelper.c_format(resJoinNetworkText, buffSize, "%s: %s", text, ssid);
}

void SpanishDisplayStrings::joinNetworkPwdText(const char* pwd, char* resJoinNetworkText, int buffSize) {
  const char* text = "Pass";

  strHelper.c_format(resJoinNetworkText, buffSize, "%s: %s", text, pwd);
}

void SpanishDisplayStrings::joinTemperatureText(float temperature, char* resTemperatureText, int buffSize) {
  const char* text = "Temp.";

  strHelper.c_format(resTemperatureText, buffSize, "%s:  %.3f", text, temperature);
}

void SpanishDisplayStrings::joinHumidityText(float humidity, char* resHumidityText, int buffSize) {
  const char* text = "Humedad";

  strHelper.c_format(resHumidityText, buffSize, "%s: %.3f", text, humidity);
}
