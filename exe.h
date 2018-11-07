// Copyright 2017 OpenSWE1R Maintainers
// Licensed under GPLv2 or any later version
// Refer to the included LICENSE.txt file.

#include <stdint.h>

// PE / exe structures

typedef struct {
  char name[8];
  uint32_t virtualSize;
  uint32_t virtualAddress;
  uint32_t rawSize;
  uint32_t rawAddress;
  uint8_t reserved[12];
  uint32_t flags;
} PeSection;

typedef struct {
  uint16_t machine;
  uint16_t numberOfSections;
  uint32_t timeDateStamp;
  uint32_t pointerToSymbolTable;
  uint32_t numberOfSymbols;
  uint16_t sizeOfOptionalHeader;
  uint16_t characteristics;
} CoffHeader;

typedef struct { 
  uint32_t virtualAddress;
  uint32_t size;
} PeDataDirectory;

typedef struct {
  uint16_t signature; //decimal number 267 for 32 bit, 523 for 64 bit, and 263 for a ROM image. 
  uint8_t majorLinkerVersion; 
  uint8_t minorLinkerVersion;
  uint32_t sizeOfCode;
  uint32_t sizeOfInitializedData;
  uint32_t sizeOfUninitializedData;
  uint32_t addressOfEntryPoint;  //The RVA of the code entry point
  uint32_t baseOfCode;
  uint32_t baseOfData;
  /*The next 21 fields are an extension to the COFF optional header format*/
  uint32_t imageBase;
  uint32_t sectionAlignment;
  uint32_t fileAlignment;
  uint16_t majorOSVersion;
  uint16_t minorOSVersion;
  uint16_t majorImageVersion;
  uint16_t minorImageVersion;
  uint16_t majorSubsystemVersion;
  uint16_t minorSubsystemVersion;
  uint32_t win32VersionValue;
  uint32_t sizeOfImage;
  uint32_t sizeOfHeaders;
  uint32_t checksum;
  uint16_t subsystem;
  uint16_t dllCharacteristics;
  uint32_t sizeOfStackReserve;
  uint32_t sizeOfStackCommit;
  uint32_t sizeOfHeapReserve;
  uint32_t sizeOfHeapCommit;
  uint32_t loaderFlags;
  uint32_t numberOfRvaAndSizes;
} PeHeader;

typedef struct {
  FILE* f;
  CoffHeader coffHeader;
  PeHeader peHeader;
  // Can have any number of elements, matching the number in NumberOfRvaAndSizes.
  // However, it is always 16 in PE files.
  PeDataDirectory* dataDirectories; // NumberOfRvaAndSizes
  PeSection* sections;
  uint8_t** mappedSections;
} Exe;

// PE sections

typedef struct {
  uint32_t originalFirstThunk;
  uint32_t timeDateStamp;
  uint32_t forwarderChain;
  uint32_t name;
  uint32_t firstThunk;
} API(IMAGE_IMPORT_DESCRIPTOR);

typedef struct  {
  uint16_t hint;
  char name[];
} API(IMAGE_IMPORT_BY_NAME);

typedef struct {
  uint32_t virtualAddress;
  uint32_t sizeOfBlock;
} API(IMAGE_BASE_RELOCATION);
