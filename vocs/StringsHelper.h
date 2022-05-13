#ifndef StringsHelper_h
#define StringsHelper_h

class StringsHelper {
  public:
    StringsHelper();
    void c_format(char* res, int size, const char* format, ...) const;
};

#endif
