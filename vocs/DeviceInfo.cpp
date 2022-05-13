#include "DeviceInfo.h"

#include <stdint.h>

DeviceInfo::DeviceInfo() { }

const char* DeviceInfo::getChipModel() const {
  return ESP.getChipModel();
}

void DeviceInfo::getChipId(char* resChipId, int buffSize) const {
  if (!resChipId || buffSize < 1 || buffSize > 9) {
    return;
  }

  strHelper.c_format(resChipId, buffSize, "%08X", (uint32_t) ESP.getEfuseMac());
}

void DeviceInfo::getSSIDName(char* resDeviceSSID, int buffSize) const {
  char chipId[buffSize];
  
  if (!resDeviceSSID || buffSize < 1) {
    return;
  }
  
  getChipId(chipId);
  strHelper.c_format(resDeviceSSID, buffSize, "%s_%s", getChipModel(), chipId);
}
