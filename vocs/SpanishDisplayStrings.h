#ifndef SpanishDisplayStrings_h
#define SpanishDisplayStrings_h

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
    void joinTemperatureText(float temperature, char* resTemperatureText, int buffSize);
    void joinHumidityText(float humidity, char* resHumidityText, int buffSize);
};

#endif
