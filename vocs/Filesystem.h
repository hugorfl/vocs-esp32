#include "SPIFFS.h"

class Filesystem {
  void begin();
  void fileExists(String filename);
  void openFile(String filename);
  std::unique_ptr<char[]> getBytes():
  void close();
}
