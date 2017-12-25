// Copyright 2017 OpenSWE1R Maintainers
// Licensed under GPLv2 or any later version
// Refer to the included LICENSE.txt file.

#ifndef __OPENSWE1R_EMULATION_H__
#define __OPENSWE1R_EMULATION_H__

#include <stdint.h>
#include <stdbool.h>

#define API(x) API__ ## x

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

Address CreateHlt();
void AddHltHandler(Address address, void(*callback)(void* uc, Address address, void* user_data), void* user_data);
Address CreateCallback(void* callback, void* user);

// Thread API

unsigned int CreateEmulatedThread(uint32_t eip);
void SleepThread(uint64_t duration);

// Debug API

void SetTracing(bool enabled);

#endif
