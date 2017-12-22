// Copyright 2017 OpenSWE1R Maintainers
// Licensed under GPLv2 or any later version
// Refer to the included LICENSE.txt file.

#include <stdio.h>

#include "main.h"

#define EXPORT_STDCALL(library, returnType, symbol, ...) \
INITIALIZER(export_ ## library ## _ ## symbol) { \
  printf("Exporting stdcall '%s' from '%s'\n", #symbol, #library); \
} \
returnType library ## _ ## symbol()
