// Copyright 2019 OpenSWE1R Maintainers
// Licensed under GPLv2 or any later version
// Refer to the included LICENSE.txt file.

// This is a temporary placeholder for a memory allocator.

#include "alloc.h"

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "common.h"


typedef struct _Allocator {
  unsigned int block_count;
  unsigned int block_size;
  uint8_t block_usage[1];
} Allocator;


static void block_mark_used(Allocator* allocator, unsigned int block, bool used) {
  uint8_t mask = 1 << (block % 8);
  if (used) {
    allocator->block_usage[block / 8] |= mask;
  } else {
    allocator->block_usage[block / 8] &= ~mask;
  }
}

static bool block_is_used(Allocator* allocator, unsigned int block) {
  uint8_t mask = 1 << (block % 8);
  return allocator->block_usage[block / 8] & mask;
}

static unsigned int count_blocks(Allocator* allocator, unsigned int block, bool used) {
  unsigned int count = 0;
  while(block < allocator->block_count) {
    if (block_is_used(allocator, block) != used) {
      break;
    }
    block++;
    count++;
  }
  return count;
}

static unsigned int find_free_blocks(Allocator* allocator, unsigned int needed_blocks) {

  unsigned int best_count = 0;
  unsigned int best_block = 0;

  unsigned int block = 0;
  while(block < allocator->block_count) {

    bool block_used = block_is_used(allocator, block);
    unsigned int available_blocks = count_blocks(allocator, block, block_used);

    // Only look at unused blocks that are large enough
    if (!block_used && (available_blocks >= needed_blocks)) {

      // Check if we don't have a result yet, or if this is a better fit
      if ((best_count == 0) || (available_blocks < best_count)) {

        // Update result
        best_block = block;
        best_count = available_blocks;

        // If we have an exact fit, use that
        if (best_count == needed_blocks) {
          break;
        }

      }
    }

    block += available_blocks;
  }

  // Assert that we have a result
  assert(best_count != 0);

  // Returns best block address
  return best_block;
}

Allocator* alloc_create(unsigned int size, unsigned int block_size) {
  assert(size % block_size == 0);
  unsigned int block_count = size / block_size;

  size_t usage_byte_count = (block_count + 7) / 8;
  Allocator* allocator = malloc(sizeof(Allocator) + usage_byte_count - 1);
  allocator->block_count = block_count;
  allocator->block_size = block_size;
  memset(allocator->block_usage, 0x00, usage_byte_count);
  return allocator;
}

void alloc_destroy(Allocator* allocator) {
  free(allocator);
}

unsigned int alloc_allocate(Allocator* allocator, unsigned int size) {

  unsigned int aligned_size = AlignUp(size, allocator->block_size);
  unsigned int count = aligned_size / allocator->block_size;

  // Don't allow zero-size allocations (breaks algorithm)
  assert(count > 0);

  //FIXME: `+ 2` is a hack, so we can recognize block splits
  unsigned int block = find_free_blocks(allocator, count + 2) + 1;

  for(unsigned int i = 0; i < count; i++) {
    block_mark_used(allocator, block + i, true);
  }

  return block * allocator->block_size;
}

void alloc_free(Allocator* allocator, unsigned int offset) {

  assert(offset % allocator->block_size == 0);
  unsigned int block = offset / allocator->block_size;

  unsigned int used_blocks = count_blocks(allocator, block, true);

  // Avoid double-free
  assert(used_blocks > 0);

  for(unsigned int i = 0; i < used_blocks; i++) {
    block_mark_used(allocator, block + i, false);
  }

}
