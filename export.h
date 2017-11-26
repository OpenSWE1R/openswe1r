#include <stdio.h>

#define EXPORT_STDCALL(library, returnType, symbol, ...) \
__attribute__((constructor)) void export_ ## library ## _ ## symbol() { \
  printf("Exporting stdcall '%s' from '%s'\n", #symbol, #library); \
} \
returnType library ## _ ## symbol()
