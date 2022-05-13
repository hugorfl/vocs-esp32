#ifndef DeviceInfo_h
#define DeviceInfo_h

#include <Arduino.h>
#include "StringsHelper.h"

class DeviceInfo {
  private:
    StringsHelper strHelper;
  public:
    DeviceInfo();
    const char* getChipModel() const ;
    void getChipId(char *resChipId, int buffSize = 9) const;
    void getDeviceName(char* resDeviceName, int buffSize = 32) const;
};

#endif
