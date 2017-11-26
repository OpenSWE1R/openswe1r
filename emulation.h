#ifndef __OPENSWE1R_EMULATION_H__
#define __OPENSWE1R_EMULATION_H__

#include <stdint.h>
#include <stdbool.h>

//FIXME: use these..
typedef uint32_t Address;
typedef uint32_t Size;

typedef struct {
  uint32_t eip;
} Context;

void InitializeEmulation();
void CleanupEmulation();

void RunEmulation();

// Memory API

void MapMemory(void* data, uint32_t address, uint32_t size, bool read, bool write, bool execute);
Address Allocate(Size size);
void Free(Address address);
void* Memory(uint32_t address);

// Hook API

// If size is 0 this is a code hook
//FIXME: Use proper type for callback!
void CreateBreakpoint(uint32_t address, void* callback, void* user);
Address CreateCallback(void* callback, void* user);

// Thread API

unsigned int CreateEmulatedThread(uint32_t eip);
void SleepThread(uint64_t duration);

// Debug API

void SetTracing(bool enabled);

#endif
