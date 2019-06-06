// Copyright 2019 OpenSWE1R Maintainers
// Licensed under GPLv2 or any later version
// Refer to the included LICENSE.txt file.

#ifndef __OPENSWE1R_ALLOC_H__
#define __OPENSWE1R_ALLOC_H__

#include <stdint.h>
#include <stdbool.h>

typedef struct _Allocator Allocator;

Allocator* alloc_create(unsigned int size, unsigned int block_size);
void alloc_destroy(Allocator* allocator);
unsigned int alloc_allocate(Allocator* allocator, unsigned int size);
void alloc_free(Allocator* allocator, unsigned int offset);

#endif
