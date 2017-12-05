#include <stdio.h>

#include "main.h"

#define EXPORT_STDCALL(library, returnType, symbol, ...) \
INITIALIZER(export_ ## library ## _ ## symbol) { \
  printf("Exporting stdcall '%s' from '%s'\n", #symbol, #library); \
} \
returnType library ## _ ## symbol()
