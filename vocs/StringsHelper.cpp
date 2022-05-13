#include "StringsHelper.h"

#include <stdio.h>
#include <stdarg.h>

StringsHelper::StringsHelper() { }

void StringsHelper::c_format(char* res, int size, const char* format, ...) const {
  va_list argsList;

  va_start(argsList, format);
  vsnprintf(res, size, format, argsList);
  va_end(argsList);
  
  res[size - 1] = '\0';
}
