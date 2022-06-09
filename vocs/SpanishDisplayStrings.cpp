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

void SpanishDisplayStrings::joinTolueneText(float toluene, int screenWidth, char* resTolueneText, int buffSize) {
  const char* text = "Tolueno";

  strHelper.c_format(resTolueneText, buffSize, "%s:%*.2f% PPM", text, getTextFittedScreenWidth(screenWidth, strlen(text) + 4), toluene);
}

void SpanishDisplayStrings::joinTemperatureText(float temperature, int screenWidth, char* resTemperatureText, int buffSize) {
  const char* text = "Temp.";

  strHelper.c_format(resTemperatureText, buffSize, "%s:%*.1f%cC", text, getTextFittedScreenWidth(screenWidth, strlen(text) + 3), temperature, 0xDF);
}

void SpanishDisplayStrings::joinHumidityText(float humidity, int screenWidth, char* resHumidityText, int buffSize) {
  const char* text = "Humedad";

  strHelper.c_format(resHumidityText, buffSize, "%s:%*.1f%%", text, getTextFittedScreenWidth(screenWidth, strlen(text) + 2), humidity);
}

int SpanishDisplayStrings::getTextFittedScreenWidth(int screenWidth, int textWidth) {
  return screenWidth - textWidth;
}
