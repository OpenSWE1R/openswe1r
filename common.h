#include <stdbool.h>
#include <stdint.h>

static bool IsZero(const void* data, size_t size) {
  uint8_t* bytes = (uint8_t*)data;
  for(size_t i = 0; i < size; i++) {
    if (*bytes++ != 0x00) {
      return false;
    }
  }
  return true;
}

static uint32_t AlignDown(uint32_t address, uint32_t size) {
  return address - address % size;
}

static uint32_t AlignUp(uint32_t address, uint32_t size) {
  return AlignDown(address + size - 1, size);
}
