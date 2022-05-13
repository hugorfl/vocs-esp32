#ifndef LangDisplayStrings_h
#define LangDisplayStrings_h

class LangDisplayStrings {
  public:
    LangDisplayStrings() { }
    virtual const char* initializingText() = 0;
    virtual void joinNetworkSSIDText(const char* ssid, char* resJoinNetworkText, int buffSize) = 0;
    virtual void joinNetworkPwdText(const char* pwd, char* resJoinNetworkText, int buffSize) = 0;
    virtual void joinTemperatureText(float temperature, char* resTemperatureText, int buffSize) = 0;
    virtual void joinHumidityText(float humidity, char* resHumidityText, int buffSize) = 0;
};

#endif
