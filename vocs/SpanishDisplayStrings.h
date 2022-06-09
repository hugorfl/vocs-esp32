#ifndef SpanishDisplayStrings_h
#define SpanishDisplayStrings_h

#include <string.h>
#include "LangDisplayStrings.h"
#include "StringsHelper.h"

class SpanishDisplayStrings : public LangDisplayStrings {
  private:
    StringsHelper strHelper;
  public:
    SpanishDisplayStrings();
    const char* initializingText();
    void joinNetworkSSIDText(const char* ssid, char* resJoinNetworkText, int buffSize);
    void joinNetworkPwdText(const char* pwd, char* resJoinNetworkText, int buffSize);
    void joinTolueneText(float toluene, int screenWidth, char* resTolueneText, int buffSize);
    void joinTemperatureText(float temperature, int screenWidth, char* resTemperatureText, int buffSize);
    void joinHumidityText(float humidity, int screenWidth, char* resHumidityText, int buffSize);
  private:
    int getTextFittedScreenWidth(int screenWidth, int textWidth);
};

#endif
