// Copyright 2017 OpenSWE1R Maintainers
// Licensed under GPLv2 or any later version
// Refer to the included LICENSE.txt file.

#include "main.h"

#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <inttypes.h>
#include <assert.h>

#include "common.h"
#include "descriptor.h"
#include "emulation.h"
#include "exe.h"

//FIXME: Alternative for non-posix OS!
#include <time.h>


#include "SDL.h"
static SDL_Window* sdlWindow;

#include <GL/glew.h>

#include "com/d3d.h"
#include "com/ddraw.h"
#include "com/dinput.h"

uint32_t callId = 0;

typedef struct {
  const char* name;
  void(*callback)(void*, Address, void*);
  Address address;
} Export;

unsigned int exportCount = 0;
Export* exports = NULL;

void AddExport(const char* name, void* callback, Address address) {
  exports = realloc(exports, (exportCount + 1) * sizeof(Export));
  Export* export = &exports[exportCount];
  export->name = malloc(strlen(name) + 1);
  strcpy((char*)export->name, name);
  export->callback = callback;
  export->address = 0;
  exportCount++;
}


Export* LookupExportByName(const char* name) {
  for(unsigned int i = 0; i < exportCount; i++) {
    Export* export = &exports[i];
    if (!strcmp(export->name, name)) {
      return export;
    }
  }
  return NULL;
}

Export* LookupExportByOrdinal(const char* name, uint32_t ordinal) {
  if (!strcmp(name, "COMCTL32.dll")) {
    if (ordinal == 17) {
      return LookupExportByName("InitCommonControls");
    }
  }
  return NULL;
}

// HACK BLOCK!
#if 1 


const char** dirlisting = NULL;

Address clearEax = 0;

uint32_t tls[1000] = {0};

//FIXME: To be moved elsewhere
#include "shaders.h"
#include "shader.h"


#include "windows.h" // Hack while exports are not ready
// HACK:
#include <unicorn/unicorn.h>

static void UnknownImport(void* uc, Address address, void* user_data);
Address CreateInterface(const char* name, unsigned int slotCount) {
  //FIXME: Unsure about most terminology / inner workings here
  Address interfaceAddress = Allocate(100); //FIXME: Size of object
  Address vtableAddress = Allocate(4 * slotCount);
  uint32_t* vtable = (uint32_t*)Memory(vtableAddress);
  for(unsigned int i = 0; i < slotCount; i++) {
    // Point addresses to themself
    char* slotName = malloc(128);
    sprintf(slotName, "%s__%d", name, i);
    Export* export = LookupExportByName(slotName);

    Address hltAddress;
    if (export != NULL) {
      if (export->address == 0) {
        hltAddress = CreateHlt();
        AddHltHandler(hltAddress, export->callback, (void*)slotName);
        export->address = hltAddress;
      } else {
        hltAddress = export->address;
      }
    } else {
      hltAddress = CreateHlt();
      AddHltHandler(hltAddress, UnknownImport, (void*)slotName);
      AddExport(slotName, UnknownImport, hltAddress);
    }
    vtable[i] = hltAddress;
  }
  // First element in object is pointer to vtable
  *(uint32_t*)Memory(interfaceAddress) = vtableAddress;

  return interfaceAddress;
}


#endif






Exe* exe; //FIXME: This is hack. I feel this shouldn't be exposed aside from the loader
const char* exeName = "swep1rcr.exe";

static char* TranslatePath(const char* path) {
  size_t length = strlen(path) + 1;
  char* newPath = malloc(length);
  char* cursor = strcpy(newPath, path);
  while(*cursor != '\0') {
    if (*cursor == '\\') {
      *cursor = '/';
    } else {
      *cursor = tolower(*cursor);
    }
    cursor++;
  }

  // The CD install folder will be mapped back to our current directory.
  // This allows easier installation (by simply copying all files from disc)
  if ((length >= 9) && !memcmp(newPath, "d:/gnome/", 9)) {
    memcpy(newPath, "./", 2);
    memmove(&newPath[2], &newPath[9], length - 9);
  }

  return newPath;
}

void StackTrace(uint32_t base, unsigned int frames, unsigned int arguments) {
  uint32_t stackAddress = base;
  for(unsigned int i = 0; i < frames; i++) {
    printf("Base: 0x%" PRIX32 "\n", stackAddress);
    if (stackAddress == 0) {
      // End of stack trace!
      return;
    }
    uint32_t* stack = (uint32_t*)Memory(stackAddress);
    // stack[0] = EBP of calling function
    // stack[1] = Return address
    // stack[2..] = Arguments
    if (stack == NULL) {
      printf("Corrupt base in trace!\n");
      return;
    }
    printf("#%2d Returns to 0x%" PRIX32 " (", i, stack[1]);
    for(unsigned int j = 0; j < arguments; j++) {
      printf("@%d=0x%08" PRIX32 ", ", j, stack[j + 2]);
    }
    printf("...)\n");
    // Get the previous ebp
    stackAddress = stack[0];
  }
}

// FIXME: Move to exe functions
void RelocateSection(Exe* exe, unsigned int sectionIndex) { 
  // Relocate
	//reloc 21589 offset    0 [301d3017] ABSOLUTE
	//reloc 21590 offset   11 [301d3028] HIGHLOW
#if 0
  switch(relocation->type)
    case RelocationAbsolute:
      assert(0);
      break;
    case RelocationHighLow:
      assert(0);
      break;
    default:
      assert(0);
      break;
  }
#endif
}

void LoadSection(Exe* exe, unsigned int sectionIndex) {
  PeSection* section = &exe->sections[sectionIndex];

  // Map memory for section
  uint8_t* mem = (uint8_t*)aligned_malloc(0x1000, section->virtualSize);

  // Read data from exe and fill rest of space with zero
  fseek(exe->f, section->rawAddress, SEEK_SET);
  uint32_t rawSize = section->rawSize;
  if (rawSize > section->virtualSize) {
    rawSize = section->virtualSize;
  }
  fread(mem, 1, rawSize, exe->f);
  if (rawSize < section->virtualSize) {
    memset(&mem[rawSize], 0x00, section->virtualSize - rawSize);
  }

  // Write back address to the exe object
  exe->mappedSections[sectionIndex] = mem;
}

void UnloadSection(Exe* exe, unsigned int sectionIndex) {
  free(exe->mappedSections[sectionIndex]);
  exe->mappedSections[sectionIndex] = NULL;
}


static void UcTimerHook(void* uc, uint64_t address, uint32_t size, void* user_data) {
  printf("Time is %" PRIu64 "\n", SDL_GetTicks());
}

// This is strictly for debug purposes, it attempts to dump fscanf (internally used by sscanf too)
static void UcFscanfHook(void* uc, uint64_t address, uint32_t size, void* user_data) {
  printf("\nfscanf\n\n");

  int eip;
  uc_reg_read(uc, UC_X86_REG_EIP, &eip);
  int esp;
  uc_reg_read(uc, UC_X86_REG_ESP, &esp);
  int eax;
  uc_reg_read(uc, UC_X86_REG_EAX, &eax);
  
  Address stackAddress = esp;
  uint32_t* stack = (uint32_t*)Memory(stackAddress);


  // This is the FILE struct used by microsoft CRT
  typedef struct {
    Address _ptr;
    int32_t _cnt;
    Address _base;
    int32_t _flag;
    int32_t _file;
    int32_t _charbuf;
    int32_t _bufsiz;
    Address _tmpfname;
  } _iobuf;

  // Pop the return address
  Address returnAddress = stack[0];
  printf("Return at 0x%" PRIX32 "\n", returnAddress);
  _iobuf* iob = Memory(stack[1]); // Get FILE object
  char* buf = Memory(iob->_ptr);
  printf("stream: 0x%" PRIX32 " ('%.100s...')\n", stack[1], buf);
  char* fmt = (char*)Memory(stack[2]);
  printf("fmt: 0x%" PRIX32 " ('%s')\n", stack[2], fmt);

  // We'll let MS code handle buffer loads
  if (buf == NULL) {
    return;
  }

#if 0
  if (!strcmp(fmt, "%s")) {
    SetTracing(true);
  }
#endif

#if 0
  //FIXME: Hack.. for these to work, they'd have to consume the buffer
  if (!strcmp(fmt, "%s")) {
    eax = sscanf(buf, fmt, Memory(stack[3]));
    uc_reg_write(uc, UC_X86_REG_EAX, &eax);
    eip = returnAddress;
    uc_reg_write(uc, UC_X86_REG_EIP, &eip);
    esp += 4;
    uc_reg_write(uc, UC_X86_REG_ESP, &esp);
    return;
  }
  if (!strcmp(fmt, "%f %f %f")) {
    eax = 3; //sscanf(buf, fmt);
    uc_reg_write(uc, UC_X86_REG_EAX, &eax);
    eip = returnAddress;
    uc_reg_write(uc, UC_X86_REG_EIP, &eip);
    esp += 4;
    uc_reg_write(uc, UC_X86_REG_ESP, &esp);
    return;
  }
#endif


}

// This is strictly for debug purposes, it attempts to add messages to the log in case of the weird-crash
static void UcCrashHook(void* uc, uint64_t address, uint32_t size, void* user_data) {
  int eip;
  uc_reg_read(uc, UC_X86_REG_EIP, &eip);
  int esp;
  uc_reg_read(uc, UC_X86_REG_ESP, &esp);
  int eax;
  uc_reg_read(uc, UC_X86_REG_EAX, &eax);
  int ecx;
  uc_reg_read(uc, UC_X86_REG_ECX, &ecx);
  int edi;
  uc_reg_read(uc, UC_X86_REG_EDI, &edi);
  
  Address stackAddress = esp;
  uint32_t* stack = (uint32_t*)Memory(stackAddress);

  char buf[1024];
  sprintf(buf, "Bug: ecx=0x%08" PRIX32 ", at eip=0x%08" PRIX32 ". 0x%08" PRIX32 " 0x%08" PRIX32 " 0x%08" PRIX32 " 0x%08" PRIX32,
          ecx, eip,
          stack[0], stack[1], stack[2], stack[3]);
  glDebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_MARKER, 0, GL_DEBUG_SEVERITY_NOTIFICATION, -1, buf);
}

static void PrintVertices(unsigned int vertexFormat, Address address, unsigned int count) {
  unsigned int texCount = ((vertexFormat & 0xF00) >> 8);
  GLsizei stride = 4 * 4 + 4 + 4 + texCount * 8;

  uint32_t* p = (uint32_t*)Memory(address);
  for(unsigned int i = 0; i < count; i++) {
    float* f = (float*)p;
    printf("  %d.    %f %f %f %f 0x%08" PRIX32 " 0x%08" PRIX32, i, f[0], f[1], f[2], f[3], p[4], p[5]);
    p += 6;
    if (texCount >= 1) {
      printf(" %f %f", f[6], f[7]);
      p += 2;
    }
    if (texCount >= 2) {
      printf(" %f %f", f[8], f[9]);
      p += 2;
    }
    printf("\n");
  }
}

static void LoadIndices(Address address, unsigned int count) {
  static GLuint buffer = 0;
  if (buffer == 0) {
    glGenBuffers(1, &buffer);
  }
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * 2, Memory(address), GL_STREAM_DRAW);  
}

static void LoadVertices(unsigned int vertexFormat, Address address, unsigned int count) {
  unsigned int texCount = ((vertexFormat & 0xF00) >> 8);
  GLsizei stride = 4 * 4 + 4 + 4 + texCount * 8;

  static GLuint buffer = 0;
  if (buffer == 0) {
    glGenBuffers(1, &buffer);
  }
  glBindBuffer(GL_ARRAY_BUFFER, buffer);
  glBufferData(GL_ARRAY_BUFFER, count * stride, Memory(address), GL_STREAM_DRAW);
}

bool depthMask;
GLenum destBlend;
GLenum srcBlend;
bool alphaTest;
uint32_t fogColor; // ARGB
bool fogEnable;
float fogStart;
float fogEnd;
float projectionMatrix[16];
float clipScale[3];
float clipOffset[3];

static GLenum SetupRenderer(unsigned int primitiveType, unsigned int vertexFormat) {
  unsigned int texCount = ((vertexFormat & 0xF00) >> 8);
  GLsizei stride = 4 * 4 + 4 + 4 + texCount * 8;
  // Re-Volt only uses D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_SPECULAR OR'd with either D3DFVF_TEX{0,1,2}

  GLuint program = 0;
  glGetIntegerv(GL_CURRENT_PROGRAM, (GLint*)&program);

  GLint positionIn = glGetAttribLocation(program, "positionIn");
  if (positionIn != -1) {
    glEnableVertexAttribArray(positionIn);
    glVertexAttribPointer(positionIn, 4, GL_FLOAT, GL_FALSE, stride, (const GLvoid*)0);
  }

  GLint diffuseIn = glGetAttribLocation(program, "diffuseIn");
  if (diffuseIn != -1) {
    glEnableVertexAttribArray(diffuseIn);
    glVertexAttribPointer(diffuseIn, 4, GL_UNSIGNED_BYTE, GL_TRUE, stride, (const GLvoid*)16);
  }

  GLint specularIn = glGetAttribLocation(program, "specularIn");
  if (specularIn != -1) {
    glEnableVertexAttribArray(specularIn);
    glVertexAttribPointer(specularIn, 4, GL_UNSIGNED_BYTE, GL_TRUE, stride, (const GLvoid*)20);
  }

  GLint uv0In = glGetAttribLocation(program, "uv0In");
  if (uv0In != -1) {
    glEnableVertexAttribArray(uv0In);
    glVertexAttribPointer(uv0In, 2, GL_FLOAT, GL_TRUE, stride, (const GLvoid*)24);
  }

  glUniform1i(glGetUniformLocation(program, "tex0"), 0);
  glUniformMatrix4fv(glGetUniformLocation(program, "projectionMatrix"), 1, GL_FALSE, projectionMatrix);

  glUniform1i(glGetUniformLocation(program, "alphaTest"), alphaTest);

  glUniform1i(glGetUniformLocation(program, "fogEnable"), fogEnable);
  glUniform1f(glGetUniformLocation(program, "fogStart"), fogStart);
  glUniform1f(glGetUniformLocation(program, "fogEnd"), fogEnd);
  glUniform3f(glGetUniformLocation(program, "fogColor"),
              ((fogColor >> 16) & 0xFF) / 255.0,
              ((fogColor >> 8) & 0xFF) / 255.0,
              (fogColor & 0xFF) / 255.0);

  glUniform3fv(glGetUniformLocation(program, "clipScale"), 1, clipScale);
  glUniform3fv(glGetUniformLocation(program, "clipOffset"), 1, clipOffset);

#if 1
  // Hack to disable texture if tex0 is used - doesn't work?!
  if (texCount == 0) {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_R, GL_ONE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_G, GL_ONE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, GL_ONE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_A, GL_ONE);
  } else {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_R, GL_RED);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_G, GL_GREEN);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, GL_BLUE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_A, GL_ALPHA);
  }
#endif

  glBlendFunc(srcBlend, destBlend);

#if 0
  // Wireframe mode
  glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
#endif

  glDepthMask(depthMask ? GL_TRUE : GL_FALSE);

  GLenum mode;
  switch(primitiveType) {
    case 1: mode = GL_POINTS;         break; // D3DPT_POINTLIST
    case 2: mode = GL_LINES;          break; // D3DPT_LINELIST
    case 3: mode = GL_LINE_STRIP;     break; // D3DPT_LINESTRIP
    case 4: mode = GL_TRIANGLES;      break; // D3DPT_TRIANGLELIST
    case 5: mode = GL_TRIANGLE_STRIP; break; // D3DPT_TRIANGLESTRIP
    case 6: mode = GL_TRIANGLE_FAN;   break; // D3DPT_TRIANGLEFAN
    default:
      assert(false);
      break;
  }
  return mode;
}



// CRT-Startup / pre-WinMain:

HACKY_IMPORT_BEGIN(GetVersion)
  hacky_printf("(No parameters)\n");
  eax = 0x00010A04;
HACKY_IMPORT_END()

HACKY_IMPORT_BEGIN(HeapCreate)
  hacky_printf("flOptions 0x%" PRIX32 "\n", stack[1]);
  hacky_printf("dwInitialSize 0x%" PRIX32 "\n", stack[2]);
  hacky_printf("dwMaximumSize 0x%" PRIX32 "\n", stack[3]);
  eax = 0x555;
  esp += 3 * 4;
HACKY_IMPORT_END()

HACKY_IMPORT_BEGIN(HeapAlloc)
  hacky_printf("hHeap 0x%" PRIX32 "\n", stack[1]);
  hacky_printf("dwFlags 0x%" PRIX32 "\n", stack[2]);
  hacky_printf("dwBytes 0x%" PRIX32 "\n", stack[3]);
  eax = Allocate(stack[3]);
  //FIXME: Only do this if flag is set..
  memset(Memory(eax), 0x00, stack[3]);
  esp += 3 * 4;
HACKY_IMPORT_END()

HACKY_IMPORT_BEGIN(InitializeCriticalSection)
  hacky_printf("lpCriticalSection 0x%" PRIX32 "\n", stack[1]);
  esp += 1 * 4;
HACKY_IMPORT_END()

HACKY_IMPORT_BEGIN(TlsAlloc)
  static int tlsIndex = 0;
  assert(tlsIndex < 500);
  eax = tlsIndex++; // TLS Index
HACKY_IMPORT_END()

HACKY_IMPORT_BEGIN(EnterCriticalSection)
  // Heavily impacts performance!
#if 1
  hacky_printf("lpCriticalSection 0x%" PRIX32 "\n", stack[1]);
#else
  silent = true;
#endif
  esp += 1 * 4;
HACKY_IMPORT_END()

HACKY_IMPORT_BEGIN(LeaveCriticalSection)
  // Heavily impacts performance!
#if 1
  hacky_printf("lpCriticalSection 0x%" PRIX32 "\n", stack[1]);
#else
  silent = true;
#endif
  esp += 1 * 4;
HACKY_IMPORT_END()

HACKY_IMPORT_BEGIN(VirtualAlloc)
  hacky_printf("lpAddress 0x%" PRIX32 "\n", stack[1]);
  hacky_printf("dwSize 0x%" PRIX32 "\n", stack[2]);
  hacky_printf("flAllocationType 0x%" PRIX32 "\n", stack[3]);
  hacky_printf("flProtect 0x%" PRIX32 "\n", stack[4]);
  eax = Allocate(stack[2]);
  memset(Memory(eax), 0x00, stack[2]);
  esp += 4 * 4;
HACKY_IMPORT_END()

HACKY_IMPORT_BEGIN(TlsSetValue)
  hacky_printf("dwTlsIndex 0x%" PRIX32 "\n", stack[1]);
  hacky_printf("lpTlsValue 0x%" PRIX32 "\n", stack[2]);
  tls[stack[1]] = stack[2];
  eax = 1; // nonzero if succeeds
  esp += 2 * 4;
HACKY_IMPORT_END()

HACKY_IMPORT_BEGIN(GetCurrentThreadId)
  eax = 666; // nonzero if succeeds
HACKY_IMPORT_END()

HACKY_IMPORT_BEGIN(GetACP)
  eax = 777; // nonzero if succeeds
HACKY_IMPORT_END()

HACKY_IMPORT_BEGIN(GetModuleFileNameA)
  hacky_printf("hModule 0x%" PRIX32 "\n", stack[1]);
  hacky_printf("lpFilename 0x%" PRIX32 "\n", stack[2]);
  hacky_printf("nSize 0x%" PRIX32 "\n", stack[3]);
  const char* path = "XYZ";
  assert(stack[3] >= (strlen(path) + 1));
  eax = sprintf((char*)Memory(stack[2]), "%s", path); // number of chars written
  esp += 3 * 4;
HACKY_IMPORT_END()

HACKY_IMPORT_BEGIN(GetCPInfo)
  hacky_printf("CodePage 0x%" PRIX32 "\n", stack[1]);
  hacky_printf("lpCPInfo 0x%" PRIX32 "\n", stack[2]);
  eax = 1; // Returns 1 if successful, or 0 otherwise.
  esp += 2 * 4;
HACKY_IMPORT_END()

HACKY_IMPORT_BEGIN(GetStartupInfoA)
  hacky_printf("lpStartupInfo 0x%" PRIX32 "\n", stack[1]);
  esp += 1 * 4;
HACKY_IMPORT_END()

HACKY_IMPORT_BEGIN(GetStdHandle)
  hacky_printf("nStdHandle 0x%" PRIX32 "\n", stack[1]);
  eax = 888;
  esp += 1 * 4;
HACKY_IMPORT_END()

HACKY_IMPORT_BEGIN(GetFileType)
  hacky_printf("hFile 0x%" PRIX32 "\n", stack[1]);
  //eax = 2; // FILE_TYPE_CHAR
  eax = 1; // FILE_TYPE_DISK
  esp += 1 * 4;
HACKY_IMPORT_END()

HACKY_IMPORT_BEGIN(SetHandleCount)
  hacky_printf("uNumber 0x%" PRIX32 "\n", stack[1]);
  eax = stack[1];
  esp += 1 * 4;
HACKY_IMPORT_END()

HACKY_IMPORT_BEGIN(GetCommandLineA)
  const char* cmd = "swep1rcr.exe";
  Address tmp = Allocate(strlen(cmd) + 1);
  strcpy((char*)Memory(tmp), cmd);
  eax = tmp;
HACKY_IMPORT_END()

HACKY_IMPORT_BEGIN(GetEnvironmentStringsW)
  Address tmp = Allocate(4);
  ((char*)Memory(tmp))[0] = '\0';
  ((char*)Memory(tmp))[1] = '\0';
  ((char*)Memory(tmp))[2] = '\0';
  ((char*)Memory(tmp))[3] = '\0';
  eax = tmp;
HACKY_IMPORT_END()

HACKY_IMPORT_BEGIN(WideCharToMultiByte)
  hacky_printf("CodePage 0x%" PRIX32 "\n", stack[1]);
  hacky_printf("dwFlags 0x%" PRIX32 "\n", stack[2]);
  hacky_printf("lpWideCharStr 0x%" PRIX32 "\n", stack[3]);
  hacky_printf("cchWideChar 0x%" PRIX32 "\n", stack[4]);
  hacky_printf("lpMultiByteStr 0x%" PRIX32 "\n", stack[5]);
  hacky_printf("cbMultiByte 0x%" PRIX32 "\n", stack[6]);
  hacky_printf("lpDefaultChar 0x%" PRIX32 "\n", stack[7]);
  hacky_printf("lpUsedDefaultChar 0x%" PRIX32 "\n", stack[8]);
  eax = 1; //FIXME: Number of chars written
  esp += 8 * 4;
HACKY_IMPORT_END()

HACKY_IMPORT_BEGIN(MultiByteToWideChar)
  hacky_printf("CodePage 0x%" PRIX32 "\n", stack[1]);
  hacky_printf("dwFlags 0x%" PRIX32 "\n", stack[2]);
  hacky_printf("lpMultiByteStr 0x%" PRIX32 "\n", stack[3]);
  hacky_printf("cbMultiByte 0x%" PRIX32 "\n", stack[4]);
  hacky_printf("lpWideCharStr 0x%" PRIX32 "\n", stack[5]);
  hacky_printf("cchWideChar 0x%" PRIX32 "\n", stack[6]);
//FIXME: MOVE SYMBOLS?!
  eax = 0; //FIXME: Number of chars written
  esp += 6 * 4;
HACKY_IMPORT_END()

HACKY_IMPORT_BEGIN(LCMapStringW)
  hacky_printf("Locale 0x%" PRIX32 "\n", stack[1]);
  hacky_printf("dwMapFlags 0x%" PRIX32 "\n", stack[2]);
  hacky_printf("lpSrcStr 0x%" PRIX32 "\n", stack[3]);
  hacky_printf("cchSrc 0x%" PRIX32 "\n", stack[4]);
  hacky_printf("lpDestStr 0x%" PRIX32 "\n", stack[5]);
  hacky_printf("cchDest 0x%" PRIX32 "\n", stack[6]);
//FIXME: MOVE SYMBOLS?!
  eax = 1 + 1; //FIXME: Number of chars in translated string including zero term
  esp += 6 * 4;
HACKY_IMPORT_END()

HACKY_IMPORT_BEGIN(GetModuleHandleA)
  hacky_printf("lpModuleName 0x%" PRIX32 " ('%s')\n", stack[1], Memory(stack[1]));
  eax = 999;
  esp += 1 * 4;
HACKY_IMPORT_END()

HACKY_IMPORT_BEGIN(GetProcAddress)
  Address lpProcName = stack[2];
  const char* procName = Memory(lpProcName);
  hacky_printf("hModule 0x%" PRIX32 "\n", stack[1]);
  hacky_printf("lpProcName 0x%" PRIX32 " ('%s')\n", lpProcName, procName);

  Export* export = LookupExportByName(procName);
  if (export == NULL) {
    printf("Export for '%s' could not be found\n", procName);
    eax = 0;
    assert(false);
  } else {
    //FIXME: Use existing address for export
    Address hltAddress = CreateHlt();
    AddHltHandler(hltAddress, export->callback, (void*)procName);
    eax = hltAddress;
    printf("Providing at 0x%08X\n", hltAddress);
  }

  esp += 2 * 4;
HACKY_IMPORT_END()

// This one is retrieved using GetProcAddress!
HACKY_IMPORT_BEGIN(IsProcessorFeaturePresent)
  hacky_printf("ProcessorFeature 0x%" PRIX32 "\n", stack[1]);
  eax = 0; // Feature not supported = zero; else nonzero
  esp += 1 * 4;
HACKY_IMPORT_END()

HACKY_IMPORT_BEGIN(SetUnhandledExceptionFilter)
  // (Only used in 1207 revolt.exe CRT)
  hacky_printf("lpTopLevelExceptionFilter 0x%" PRIX32 "\n", stack[1]);
  eax = 0; // Previous handler (NULL = none existed)
  esp += 1 * 4;
HACKY_IMPORT_END()

HACKY_IMPORT_BEGIN(QueryPerformanceCounter)
  hacky_printf("lpPerformanceCount 0x%" PRIX32 "\n", stack[1]);
  *(uint64_t*)Memory(stack[1]) = SDL_GetPerformanceCounter();
  eax = 1; // nonzero if succeeds
  esp += 1 * 4;
HACKY_IMPORT_END()

HACKY_IMPORT_BEGIN(FreeEnvironmentStringsW)
  hacky_printf("lpszEnvironmentBlock 0x%" PRIX32 "\n", stack[1]);
  eax = 1; // nonzero if succeeds
  esp += 1 * 4;
HACKY_IMPORT_END()

HACKY_IMPORT_BEGIN(GetStringTypeW)
  hacky_printf("dwInfoType 0x%" PRIX32 "\n", stack[1]);
  hacky_printf("lpSrcStr 0x%" PRIX32 "\n", stack[2]);
  hacky_printf("cchSrc 0x%" PRIX32 "\n", stack[3]);
  hacky_printf("lpCharType 0x%" PRIX32 "\n", stack[4]);
  eax = 1; // nonzero if succeeds
  esp += 4 * 4;
HACKY_IMPORT_END()



// Actual revolt.exe starts here, anything until this point was CRT-Startup / pre-WinMain:

HACKY_IMPORT_BEGIN(GetCursorPos)
  hacky_printf("lpPoint 0x%" PRIX32 "\n", stack[1]);
  int32_t* point = (int32_t*)Memory(stack[1]);
  int x;
  int y;
  SDL_GetMouseState(&x, &y);
  point[0] = x;
  point[1] = y;
  eax = 1; // nonzero if succeeds
  esp += 1 * 4;
HACKY_IMPORT_END()

HACKY_IMPORT_BEGIN(SetCursorPos)
  hacky_printf("x %" PRIu32 "\n", stack[1]);
  hacky_printf("y %" PRIu32 "\n", stack[2]);
  eax = 1; // nonzero if succeeds
  esp += 2 * 4;
HACKY_IMPORT_END()

HACKY_IMPORT_BEGIN(GetKeyNameTextA)
  hacky_printf("lParam 0x%" PRIX32 "\n", stack[1]);
  hacky_printf("lpString 0x%" PRIX32 " ('%s')\n", stack[2], (char*)Memory(stack[2]));
  hacky_printf("cchSize %" PRIu32 "\n", stack[3]);
  eax = snprintf(Memory(stack[2]), stack[3], "k%" PRIu32, stack[1]); // Cancel was selected
  esp += 3 * 4;
HACKY_IMPORT_END()

HACKY_IMPORT_BEGIN(GetComputerNameA)
  hacky_printf("lpBuffer 0x%" PRIX32 "\n", stack[1]);
  uint32_t* size = (uint32_t*)Memory(stack[2]);
  hacky_printf("lpnSize 0x%" PRIX32 " (%" PRIu32 ")\n", stack[2], *size);
  *size = snprintf(Memory(stack[1]), *size, "ComputerName"); // Cancel was selected
  eax = 1; // nonzero if succeeds
  esp += 2 * 4;
HACKY_IMPORT_END()

HACKY_IMPORT_BEGIN(timeGetTime)
  //FIXME: Avoid overflow?
  eax = SDL_GetTicks();
HACKY_IMPORT_END()

HACKY_IMPORT_BEGIN(GetLastError)
  silent = true;
  eax = 0; // no error
HACKY_IMPORT_END()

HACKY_IMPORT_BEGIN(TlsGetValue)
  silent = true;
  if (!silent) {
    hacky_printf("dwTlsIndex 0x%" PRIX32 "\n", stack[1]);
  }
  eax = tls[stack[1]]; // TLS value FIXME!
  esp += 1 * 4;
HACKY_IMPORT_END()

HACKY_IMPORT_BEGIN(SetLastError)
  silent = true;
  if (!silent) {
    hacky_printf("dwErrCode 0x%" PRIX32 "\n", stack[1]);
  }
  eax = 0; // TLS value FIXME!
  esp += 1 * 4;
HACKY_IMPORT_END()

HACKY_IMPORT_BEGIN(LocalAlloc)
  silent = true;
  if (!silent) {
    hacky_printf("uFlags 0x%" PRIX32 "\n", stack[1]);
    hacky_printf("uBytes 0x%" PRIX32 "\n", stack[2]);
  }
  eax = Allocate(stack[2]);
  // Only if zeroinit: clear
  if (stack[1] & 0x40) {
      memset(Memory(eax), 0x00, stack[2]);
  }
  esp += 2 * 4;
HACKY_IMPORT_END()

HACKY_IMPORT_BEGIN(LocalFree)
  silent = true;
  if (!silent) {
    hacky_printf("hMem 0x%" PRIX32 "\n", stack[1]);
  }
  Free(stack[1]);
  eax = 0; // NULL on success
  esp += 1 * 4;
HACKY_IMPORT_END()

HACKY_IMPORT_BEGIN(FindWindowA)
  hacky_printf("lpClassName 0x%" PRIX32 " ('%s')\n", stack[1], (char*)Memory(stack[1]));
  hacky_printf("lpWindowName 0x%" PRIX32 " ('%s')\n", stack[2], (char*)Memory(stack[2]));
  eax = 0; // NULL = window not found, else HWND
  esp += 2 * 4;
HACKY_IMPORT_END()

HACKY_IMPORT_BEGIN(MessageBoxA)
  hacky_printf("hWnd 0x%" PRIX32 "\n", stack[1]);
  hacky_printf("lpText 0x%" PRIX32 " ('%s')\n", stack[2], (char*)Memory(stack[2]));
  hacky_printf("lpCaption 0x%" PRIX32 " ('%s')\n", stack[3], (char*)Memory(stack[3]));
  hacky_printf("uType 0x%" PRIX32 "\n", stack[4]);
  SDL_Delay(5000);
  eax = 2; // Cancel was selected
  esp += 4 * 4;
HACKY_IMPORT_END()

HACKY_IMPORT_BEGIN(ExitProcess)
  hacky_printf("uExitCode 0x%" PRIX32 "\n", stack[1]);
  exit(EXIT_FAILURE); //FIXME: Instead, handle this gracefully somehow?!
  esp += 1 * 4;
HACKY_IMPORT_END()

HACKY_IMPORT_BEGIN(GetTempPathA)
  hacky_printf("nBufferLength 0x%" PRIX32 "\n", stack[1]);
  hacky_printf("lpBuffer 0x%" PRIX32 "\n", stack[2]);
  assert(stack[1] >= 6);
  eax = sprintf((char*)Memory(stack[2]), "%s", "/tmp/"); // number of chars writte
  esp += 2 * 4;
HACKY_IMPORT_END()

HACKY_IMPORT_BEGIN(wsprintfA)
  // __cdecl!
  hacky_printf("lpOut 0x%" PRIX32 "\n", stack[1]);
  hacky_printf("lpFmt 0x%" PRIX32 " ('%s')\n", stack[2], (char*)Memory(stack[2]));

  char* out = (char*)Memory(stack[1]);
  const char* in = (char*)Memory(stack[2]);

  unsigned int count = 0;
  unsigned int stackIndex = 3;
  while(*in != '\0') {
    const char* nextPercent = strchr(in, '%');
    if (nextPercent == NULL) {
      count += sprintf(&out[count], in); // Copy rest of the input to output
      break;
    }
    unsigned int length = nextPercent - in;
    memcpy(&out[count], in, length);
    in += length;
    count += length;
    in = nextPercent + 1;
    char type = *in++;
    switch(type) {
      case 'c':
        count += sprintf(&out[count], "%c", stack[stackIndex++]);
        break;
      case 's':
        count += sprintf(&out[count], "%s", (char*)Memory(stack[stackIndex++]));
        break;
      case 'd':
        count += sprintf(&out[count], "%d", stack[stackIndex++]);
        break;
      default:
        printf("Unknown format type '%c'\n", type);
        assert(false);
    }
  }
  eax = count;

  printf("Out: '%s'\n", out);
HACKY_IMPORT_END()

FILE* handles[10000];
uint32_t handle_index = 1;

HACKY_IMPORT_BEGIN(CreateFileA)
  const char* lpFileName = (char*)Memory(stack[1]);
  hacky_printf("lpFileName 0x%" PRIX32 " ('%s')\n", stack[1], lpFileName);
  hacky_printf("dwDesiredAccess 0x%" PRIX32 "\n", stack[2]);
  hacky_printf("dwShareMode 0x%" PRIX32 "\n", stack[3]);
  hacky_printf("lpSecurityAttributes 0x%" PRIX32 "\n", stack[4]);
  hacky_printf("dwCreationDisposition 0x%" PRIX32 "\n", stack[5]);
  hacky_printf("dwFlagsAndAttributes 0x%" PRIX32 "\n", stack[6]);
  hacky_printf("hTemplateFile 0x%" PRIX32 "\n", stack[7]);
  char* path = TranslatePath(lpFileName);
  FILE* f = fopen(path, stack[2] & 0x40000000 ? (stack[5] == 4 ? "ab" : "wb") : "rb");
  if (f != NULL) {
    printf("File handle is 0x%" PRIX32 "\n", handle_index);
    handles[handle_index] = f;
    eax = handle_index;
    handle_index++;
  } else {
    printf("Failed to open file ('%s' as '%s')\n", lpFileName, path);
    eax = 0xFFFFFFFF;
  }
  free(path);
  esp += 7 * 4;

HACKY_IMPORT_END()

HACKY_IMPORT_BEGIN(WriteFile)
  hacky_printf("hFile 0x%" PRIX32 "\n", stack[1]);
  hacky_printf("lpBuffer 0x%" PRIX32 "\n", stack[2]);
  hacky_printf("nNumberOfBytesToWrite 0x%" PRIX32 "\n", stack[3]);
  hacky_printf("lpNumberOfBytesWritten 0x%" PRIX32 "\n", stack[4]);
  hacky_printf("lpOverlapped 0x%" PRIX32 "\n", stack[5]);
  *(uint32_t*)Memory(stack[4]) = fwrite(Memory(stack[2]), 1, stack[3], handles[stack[1]]);
  eax = 1; // nonzero if succeeds
  esp += 5 * 4;
HACKY_IMPORT_END()

HACKY_IMPORT_BEGIN(HeapFree)
  hacky_printf("hHeap 0x%" PRIX32 "\n", stack[1]);
  hacky_printf("dwFlags 0x%" PRIX32 "\n", stack[2]);
  hacky_printf("lpMem 0x%" PRIX32 "\n", stack[3]);
  eax = 1; // nonzero if succeeds
  esp += 3 * 4;
HACKY_IMPORT_END()

HACKY_IMPORT_BEGIN(CloseHandle)
  hacky_printf("hObject 0x%" PRIX32 "\n", stack[1]);
  if (stack[1] == 5554321) { // Thread handle..
    eax = 1; // nonzero if succeeds
  } else if (stack[1] == 5551337) { // Thread handle..
    eax = 1; // nonzero if succeeds
  } else {
    eax = fclose(handles[stack[1]]) ? 0 : 1; // nonzero if succeeds
  }
  esp += 1 * 4;
HACKY_IMPORT_END()

HACKY_IMPORT_BEGIN(CoInitialize)
  hacky_printf("pvReserved 0x%" PRIX32 "\n", stack[1]);
  assert(stack[1] == 0x00000000);
  eax = 0; // S_OK
  esp += 1 * 4;
HACKY_IMPORT_END()

HACKY_IMPORT_BEGIN(CoUninitialize)
  eax = 0; // void
  esp += 0 * 4;
HACKY_IMPORT_END()

HACKY_IMPORT_BEGIN(CoCreateInstance)
  hacky_printf("rclsid 0x%" PRIX32 "\n", stack[1]);
  hacky_printf("pUnkOuter 0x%" PRIX32 "\n", stack[2]);
  hacky_printf("dwClsContext 0x%" PRIX32 "\n", stack[3]);
  hacky_printf("riid 0x%" PRIX32 "\n", stack[4]);
  hacky_printf("ppv 0x%" PRIX32 "\n", stack[5]);
  const API(CLSID)* clsid = (const API(CLSID)*)Memory(stack[1]);
  char clsidString[1024];
  sprintf(clsidString, "%08" PRIX32 "-%04" PRIX16 "-%04" PRIX16 "-%02" PRIX8 "%02" PRIX8 "-%02" PRIX8 "%02" PRIX8 "%02" PRIX8 "%02" PRIX8 "%02" PRIX8 "%02" PRIX8,
          clsid->Data1, clsid->Data2, clsid->Data3,
          clsid->Data4[0], clsid->Data4[1], clsid->Data4[2], clsid->Data4[3],
          clsid->Data4[4], clsid->Data4[5], clsid->Data4[6], clsid->Data4[7]);
  printf("  (read clsid: {%s})\n", clsidString);
  const API(IID)* iid = (const API(IID)*)Memory(stack[4]);
  char iidString[1024];
  sprintf(iidString, "%08" PRIX32 "-%04" PRIX16 "-%04" PRIX16 "-%02" PRIX8 "%02" PRIX8 "-%02" PRIX8 "%02" PRIX8 "%02" PRIX8 "%02" PRIX8 "%02" PRIX8 "%02" PRIX8,
          iid->Data1, iid->Data2, iid->Data3,
          iid->Data4[0], iid->Data4[1], iid->Data4[2], iid->Data4[3],
          iid->Data4[4], iid->Data4[5], iid->Data4[6], iid->Data4[7]);
  printf("  (read iid: {%s})\n", iidString);
  //FIXME: Unsure about most terminology / inner workings here
  static unsigned int comIndex = 0;
  char name[32];
  //FIXME: Add more classed / interfaces

  if (!strcmp(clsidString, "2FE8F810-B2A5-11D0-A787-0000F803ABFC")) {
    // DEFINE_GUID(CLSID_DirectPlayLobby, 0x2fe8f810, 0xb2a5, 0x11d0, 0xa7, 0x87, 0x0, 0x0, 0xf8, 0x3, 0xab, 0xfc);
    if (!strcmp(iidString, "2DB72491-652C-11D1-A7A8-0000F803ABFC")) {
      strcpy(name, "IDirectPlayLobby3A");
    } else {
      assert(false);
    }
  } else if (!strcmp(clsidString, "D7B70EE0-4340-11CF-B063-0020AFC2CD35")) {
    // DEFINE_GUID(CLSID_DirectDraw, 0xD7B70EE0,0x4340,0x11CF,0xB0,0x63,0x00,0x20,0xAF,0xC2,0xCD,0x35);
    if (!strcmp(iidString, "9C59509A-39BD-11D1-8C4A-00C04FD930C5")) {
      strcpy(name, "IDirectDraw4");
    } else {
      assert(false);
    }
  } else if (!strcmp(clsidString, "92FA2C24-253C-11D2-90FB-006008A1F441")) {
    if (!strcmp(iidString, "E4C40280-CCBA-11D2-9DCF-00500411582F")) {
      strcpy(name, "IA3d4");
    } else {
      assert(false);
    }
  } else if (!strcmp(clsidString, "D1EB6D20-8923-11D0-9D97-00A0C90A43CB")) {
    if (!strcmp(iidString, "0AB1C530-4745-11D1-A7A1-0000F803ABFC")) {
      strcpy(name, "IDirectPlay4");
    } else {
      assert(false);
    }
  } else {
    assert(false);
  }
  *(Address*)Memory(stack[5]) = CreateInterface(name, 200);
  comIndex++;
  eax = 0; // S_OK
  esp += 5 * 4;
HACKY_IMPORT_END()

HACKY_IMPORT_BEGIN(DirectDrawCreate)
  hacky_printf("lpGUID 0x%" PRIX32 "\n", stack[1]);
  hacky_printf("lplpDD 0x%" PRIX32 "\n", stack[2]);
  hacky_printf("pUnkOuter 0x%" PRIX32 "\n", stack[3]);
  *(Address*)Memory(stack[2]) = CreateInterface("IDirectDraw4", 200);
  eax = 0; // DD_OK
  esp += 3 * 4;
HACKY_IMPORT_END()

HACKY_IMPORT_BEGIN(PeekMessageA)
  hacky_printf("lpMsg 0x%" PRIX32 "\n", stack[1]);
  hacky_printf("hWnd 0x%" PRIX32 "\n", stack[2]);
  hacky_printf("wMsgFilterMin 0x%" PRIX32 "\n", stack[3]);
  hacky_printf("wMsgFilterMax 0x%" PRIX32 "\n", stack[4]);
  hacky_printf("wRemoveMsg 0x%" PRIX32 "\n", stack[5]);

  //FIXME: Translate SDL events to windows events
  SDL_Event event;
	while (SDL_PollEvent(&event)) {
		switch (event.type) {
		case SDL_QUIT:
      printf("\n\nSDL Exit!\n");
      exit(EXIT_FAILURE);
			break;
    }
  }

  eax = 0; // If a message is available, the return value is nonzero.
  esp += 5 * 4;
HACKY_IMPORT_END()

HACKY_IMPORT_BEGIN(GetCurrentDirectoryA)
  hacky_printf("nBufferLength 0x%" PRIX32 "\n", stack[1]);
  hacky_printf("lpBuffer 0x%" PRIX32 "\n", stack[2]);
  const char* path = "C:\\tmp\\";
  assert(stack[1] >= (strlen(path) + 1));
  eax = sprintf((char*)Memory(stack[2]), "%s", path); // number of chars written
  esp += 2 * 4;
HACKY_IMPORT_END()

HACKY_IMPORT_BEGIN(ReadFile)
  hacky_printf("hFile 0x%" PRIX32 "\n", stack[1]);
  hacky_printf("lpBuffer 0x%" PRIX32 "\n", stack[2]);
  hacky_printf("nNumberOfBytesToRead 0x%" PRIX32 "\n", stack[3]);
  hacky_printf("lpNumberOfBytesRead 0x%" PRIX32 "\n", stack[4]);
  hacky_printf("lpOverlapped 0x%" PRIX32 "\n", stack[5]);
  *(uint32_t*)Memory(stack[4]) = fread(Memory(stack[2]), 1, stack[3], handles[stack[1]]);
  eax = 1; // nonzero if succeeds
  esp += 5 * 4;
HACKY_IMPORT_END()

HACKY_IMPORT_BEGIN(RegCreateKeyExA)
  hacky_printf("hKey 0x%" PRIX32 "\n", stack[1]);
  hacky_printf("lpSubKey 0x%" PRIX32 " ('%s')\n", stack[2], (char*)Memory(stack[2]));
  hacky_printf("Reserved 0x%" PRIX32 "\n", stack[3]);
  hacky_printf("lpClass 0x%" PRIX32 "\n", stack[4]);
  hacky_printf("dwOptions 0x%" PRIX32 "\n", stack[5]);
  hacky_printf("samDesired 0x%" PRIX32 "\n", stack[6]);
  hacky_printf("lpSecurityAttributes 0x%" PRIX32 "\n", stack[7]);
  hacky_printf("phkResult 0x%" PRIX32 "\n", stack[8]);
  hacky_printf("lpdwDisposition 0x%" PRIX32 "\n", stack[9]);
  *(uint32_t*)Memory(stack[8]) = 222; //FIXME: Pointer!
//    eax = 0; // ERROR_SUCCESS
  eax = 1; // anything which is not ERROR_SUCCESS
  esp += 9 * 4;
HACKY_IMPORT_END()

HACKY_IMPORT_BEGIN(RegQueryValueExA)
  hacky_printf("hKey 0x%" PRIX32 "\n", stack[1]);
  hacky_printf("lpValueName 0x%" PRIX32 " ('%s')\n", stack[2], (char*)Memory(stack[2]));
  hacky_printf("lpReserved 0x%" PRIX32 "\n", stack[3]);
  hacky_printf("lpType 0x%" PRIX32 "\n", stack[4]);
  hacky_printf("lpData 0x%" PRIX32 "\n", stack[5]);
  hacky_printf("lpcbData 0x%" PRIX32 "\n", stack[6]);

  // Patch to accept the CD
  if (!strcmp((char*)Memory(stack[2]), "CD Path")) {
    //FIXME: Assert that there is enough room for the path
    strcpy(Memory(stack[5]), "D:");
    eax = 0; // ERROR_SUCCESS
  } else {
    eax = 2; // ERROR_FILE_NOT_FOUND
  }
  esp += 6 * 4;
HACKY_IMPORT_END()

HACKY_IMPORT_BEGIN(RegSetValueExA)
  hacky_printf("hKey 0x%" PRIX32 "\n", stack[1]);
  hacky_printf("lpValueName 0x%" PRIX32 " ('%s')\n", stack[2], (char*)Memory(stack[2]));
  hacky_printf("Reserved 0x%" PRIX32 "\n", stack[3]);
  hacky_printf("dwType 0x%" PRIX32 "\n", stack[4]);
  hacky_printf("lpData 0x%" PRIX32 " ('%.*s')\n", stack[5], stack[6], (char*)Memory(stack[5]));
  hacky_printf("cbData 0x%" PRIX32 "\n", stack[6]);
  eax = 0; // ERROR_SUCCESS
  esp += 6 * 4;
HACKY_IMPORT_END()

HACKY_IMPORT_BEGIN(RegCloseKey)
  hacky_printf("hKey 0x%" PRIX32 "\n", stack[1]);
  eax = 0; // ERROR_SUCCESS
  esp += 1 * 4;
HACKY_IMPORT_END()

HACKY_IMPORT_BEGIN(GetUserNameA)
  hacky_printf("lpBuffer 0x%" PRIX32 "\n", stack[1]);
  hacky_printf("lpnSize 0x%" PRIX32 "\n", stack[2]);
  const char* name = "OpenSWE1R";
  assert(stack[2] >= (strlen(name) + 1));
  sprintf((char*)Memory(stack[1]), "%s", name);
  eax = 1; // BOOL -> success = nonzero
  esp += 2 * 4;
HACKY_IMPORT_END()

HACKY_IMPORT_BEGIN(SetFilePointer)
  hacky_printf("hFile 0x%" PRIX32 "\n", stack[1]);
  hacky_printf("lDistanceToMove 0x%" PRIX32 "\n", stack[2]);
  hacky_printf("lpDistanceToMoveHigh 0x%" PRIX32 "\n", stack[3]);
  hacky_printf("dwMoveMethod 0x%" PRIX32 "\n", stack[4]);
  int moveMethods[] = { SEEK_SET, SEEK_CUR, SEEK_END };
  assert(stack[4] < 3);
  fseek(handles[stack[1]], stack[2], moveMethods[stack[4]]);
  eax = ftell(handles[stack[1]]);
  //FIXME: Higher word
  esp += 4 * 4;
HACKY_IMPORT_END()

HACKY_IMPORT_BEGIN(QueryPerformanceFrequency)
  hacky_printf("lpFrequency 0x%" PRIX32 "\n", stack[1]);
  *(uint64_t*)Memory(stack[1]) = SDL_GetPerformanceFrequency();
  eax = 1; // BOOL - but doc: hardware supports a high-resolution performance counter = nonzero return
  esp += 1 * 4;
HACKY_IMPORT_END()

HACKY_IMPORT_BEGIN(GetObjectA)
  hacky_printf("hgdiobj 0x%" PRIX32 "\n", stack[1]);
  hacky_printf("cbBuffer 0x%" PRIX32 "\n", stack[2]);
  hacky_printf("lpvObject 0x%" PRIX32 "\n", stack[3]);
  //If the function succeeds, and lpvObject is a valid pointer, the return value is the number of bytes stored into the buffer.
  //If the function succeeds, and lpvObject is NULL, the return value is the number of bytes required to hold the information the function would store into the buffer.
  //If the function fails, the return value is zero.

  // Copy BITMAP from memory
  memcpy(Memory(stack[3]), Memory(stack[1]), stack[2]);

  eax = 0; // int
  esp += 3 * 4;
HACKY_IMPORT_END()

HACKY_IMPORT_BEGIN(CreateCompatibleDC)
  hacky_printf("hdc 0x%" PRIX32 "\n", stack[1]);
  // Hack: This is large enough to fit a pointer to the object (SelectObject)
  eax = Allocate(sizeof(Address));
  esp += 1 * 4;
HACKY_IMPORT_END()

HACKY_IMPORT_BEGIN(SelectObject)
  hacky_printf("hdc 0x%" PRIX32 "\n", stack[1]);
  hacky_printf("hgdiobj 0x%" PRIX32 "\n", stack[2]);
  // Ugly hack.. let DC point at object
  *(Address*)Memory(stack[1]) = stack[2];
  eax = 0; //FIXME: It's complicated.. see docs instead!
  esp += 2 * 4;
HACKY_IMPORT_END()

HACKY_IMPORT_BEGIN(StretchBlt)
  hacky_printf("hdcDest 0x%" PRIX32 "\n", stack[1]);
  hacky_printf("nXOriginDest 0x%" PRIX32 "\n", stack[2]);
  hacky_printf("nYOriginDest 0x%" PRIX32 "\n", stack[3]);
  hacky_printf("nWidthDest 0x%" PRIX32 "\n", stack[4]);
  hacky_printf("nHeightDest 0x%" PRIX32 "\n", stack[5]);
  hacky_printf("hdcSrc 0x%" PRIX32 "\n", stack[6]);
  hacky_printf("nXOriginSrc 0x%" PRIX32 "\n", stack[7]);
  hacky_printf("nYOriginSrc 0x%" PRIX32 "\n", stack[8]);
  hacky_printf("nWidthSrc 0x%" PRIX32 "\n", stack[9]);
  hacky_printf("nHeightSrc 0x%" PRIX32 "\n", stack[10]);
  hacky_printf("dwRop 0x%" PRIX32 "\n", stack[11]);

  // Get the pointer to the object the DC points at, we'll assume that it is a BITMAP
  Address objectAddress = *(Address*)Memory(stack[6]);
  API(BITMAP)* bitmap = Memory(objectAddress);
  void* data = Memory(bitmap->bmBits);

  // Update the texture interface
  if (stack[1] != 0) {
    GLint previousTexture = 0;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &previousTexture);
    glBindTexture(GL_TEXTURE_2D, stack[1]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, stack[4], stack[5], 0, GL_BGR, GL_UNSIGNED_BYTE, data);
    glBindTexture(GL_TEXTURE_2D, previousTexture);
  } else {


    printf("\n\n\n\nblt!!!!!!\n\n\n\n");
    glClearColor(1.0f,0.0f,1.0f,1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    printf("Invalid destination!\n");
  }

  eax = 1; //  succeeds = return value is nonzero.
  esp += 11 * 4;
HACKY_IMPORT_END()

HACKY_IMPORT_BEGIN(DeleteDC)
  hacky_printf("hdc 0x%" PRIX32 "\n", stack[1]);
  eax = 1; //  succeeds = return value is nonzero
  esp += 1 * 4;
HACKY_IMPORT_END()

HACKY_IMPORT_BEGIN(DeleteObject)
  hacky_printf("hObject 0x%" PRIX32 "\n", stack[1]);
  eax = 1; //  succeeds = return value is nonzero
  esp += 1 * 4;
HACKY_IMPORT_END()

HACKY_IMPORT_BEGIN(GetPixel)
  int nXPos = stack[2];
  int nYPos = stack[3];
  // Heavily impacts performance!
#if 0
  hacky_printf("hdc 0x%" PRIX32 "\n", stack[1]);
  hacky_printf("nXPos 0x%" PRIX32 "\n", nXPos);
  hacky_printf("nYPos 0x%" PRIX32 "\n", nYPos);
#else
  silent = true;
#endif
  eax = 0xFF00FF00 ^ nXPos ^ nYPos; //  Color
  esp += 3 * 4;
HACKY_IMPORT_END()

// Thread related

HACKY_IMPORT_BEGIN(CreateThread)
  // Loading in a worker-thread during a loadscreen - Acclaim.. gj.. NOT!
  hacky_printf("lpThreadAttributes 0x%" PRIX32 "\n", stack[1]);
  hacky_printf("dwStackSize 0x%" PRIX32 "\n", stack[2]);
  hacky_printf("lpStartAddress 0x%" PRIX32 "\n", stack[3]);
  hacky_printf("lpParameter 0x%" PRIX32 "\n", stack[4]);
  hacky_printf("dwCreationFlags 0x%" PRIX32 "\n", stack[5]);
  hacky_printf("lpThreadId 0x%" PRIX32 "\n", stack[6]);

  //CreateEmulatedThread(stack[3]);

  eax = 5554321; //  handle to new thread
  esp += 6 * 4;

HACKY_IMPORT_END()

HACKY_IMPORT_BEGIN(CreateEventA)
  hacky_printf("lpEventAttributes 0x%" PRIX32 "\n", stack[1]);
  hacky_printf("bManualReset 0x%" PRIX32 "\n", stack[2]);
  hacky_printf("bInitialState 0x%" PRIX32 "\n", stack[3]);
  hacky_printf("lpName 0x%" PRIX32 " ('%s')\n", stack[4], (char*)Memory(stack[4]));

  eax = 5551337; // HANDLE
  esp += 4 * 4;
HACKY_IMPORT_END()

HACKY_IMPORT_BEGIN(SetEvent)
  hacky_printf("hEvent 0x%" PRIX32 "\n", stack[1]);

  eax = 1; //  succeeds = return value is nonzero
  esp += 1 * 4;
HACKY_IMPORT_END()

HACKY_IMPORT_BEGIN(WaitForSingleObject)
  hacky_printf("hHandle 0x%" PRIX32 "\n", stack[1]);
  hacky_printf("dwMilliseconds %" PRId32 "\n", stack[2]);

  eax = 0; // DWORD (0 = "The state of the specified object is signaled.")
  esp += 2 * 4;
HACKY_IMPORT_END()

//FIXME: Should be atomic by definition in OpenSWE1R?
HACKY_IMPORT_BEGIN(InterlockedIncrement)
  hacky_printf("Addend 0x%" PRIX32 "\n", stack[1]);
  uint32_t* Addend = (uint32_t*)Memory(stack[1]);
  *Addend = *Addend + 1;
  eax = *Addend;
  esp += 1 * 4; // FIXME: MSDN claims cdecl?! asm looks like stdcall
HACKY_IMPORT_END()

//FIXME: Should be atomic by definition in OpenSWE1R?
HACKY_IMPORT_BEGIN(InterlockedDecrement)
  hacky_printf("Addend 0x%" PRIX32 "\n", stack[1]);
  uint32_t* Addend = (uint32_t*)Memory(stack[1]);
  *Addend = *Addend - 1;
  eax = *Addend;
  esp += 1 * 4; // FIXME: MSDN claims cdecl?! asm looks like stdcall
HACKY_IMPORT_END()

HACKY_IMPORT_BEGIN(GetCurrentThread)
  eax = 12345; // nonzero if succeeds
HACKY_IMPORT_END()

HACKY_IMPORT_BEGIN(SetThreadPriority)
  hacky_printf("hThread 0x%" PRIX32 "\n", stack[1]);
  hacky_printf("nPriority 0x%" PRIX32 "\n", stack[2]);
  eax = 1; // success = the return value is nonzero.
  esp += 2 * 4;
HACKY_IMPORT_END()

HACKY_IMPORT_BEGIN(Sleep)
  hacky_printf("dwMilliseconds 0x%" PRIX32 "\n", stack[1]);
  SleepThread(stack[1]);
  esp += 1 * 4;
HACKY_IMPORT_END()

HACKY_IMPORT_BEGIN(DeleteCriticalSection)
  hacky_printf("lpCriticalSection 0x%" PRIX32 "\n", stack[1]);
  esp += 1 * 4;
HACKY_IMPORT_END()

HACKY_IMPORT_BEGIN(SuspendThread)
  hacky_printf("hThread 0x%" PRIX32 "\n", stack[1]);
  eax = 0; // FIXME: Suspend count or -1 in case of error
  esp += 1 * 4;
HACKY_IMPORT_END()

HACKY_IMPORT_BEGIN(ResumeThread)
  hacky_printf("hThread 0x%" PRIX32 "\n", stack[1]);
  eax = 0; // FIXME: Suspend count or -1 in case of error
  esp += 1 * 4;
HACKY_IMPORT_END()

HACKY_IMPORT_BEGIN(TerminateThread)
  hacky_printf("hThread 0x%" PRIX32 "\n", stack[1]);
  hacky_printf("dwExitCode 0x%" PRIX32 "\n", stack[2]);
  //FIXME: This should exit the particular thread
  eax = 1; // BOOL; non-zero on success
  esp += 2 * 4;
HACKY_IMPORT_END()

HACKY_IMPORT_BEGIN(ExitThread)
  hacky_printf("dwExitCode 0x%" PRIX32 "\n", stack[1]);
  printf("\n\n\n\n\nMASSIVE HACK! STARTING NOW!\n\n\n\n\n");
  SleepThread(0xFFFFFFFFFFFFFFFFLLU);
  // Spinlock this thread..
  eip = Allocate(2);
  uint8_t* code = Memory(eip);
  code[0] = 0xEB; // a: jmp a
  code[1] = 0xFE;
  esp += 1 * 4;
HACKY_IMPORT_END()

// Window creation function
HACKY_IMPORT_BEGIN(LoadIconA)
  hacky_printf("hInstance 0x%" PRIX32 "\n", stack[1]);
  hacky_printf("lpIconName 0x%" PRIX32 " ('%s')\n", stack[2], (char*)Memory(stack[2]));
  eax = 0; // NULL, pretend we failed
  esp += 2 * 4;
HACKY_IMPORT_END()

HACKY_IMPORT_BEGIN(LoadCursorA)
  hacky_printf("hInstance 0x%" PRIX32 "\n", stack[1]);
  hacky_printf("lpCursorName 0x%" PRIX32 " ('%s')\n", stack[2], (char*)Memory(stack[2]));
  eax = 0; // NULL, pretend we failed
  esp += 2 * 4;
HACKY_IMPORT_END()

HACKY_IMPORT_BEGIN(GetStockObject)
  hacky_printf("fnObject 0x%" PRIX32 "\n", stack[1]);
  eax = 0; // NULL, pretend we failed
  esp += 1 * 4;
HACKY_IMPORT_END()

HACKY_IMPORT_BEGIN(GetSystemMetrics)
  hacky_printf("nIndex %" PRId32 "\n", stack[1]);
  switch(stack[1]) {
    case 0:
      eax = 640; // Horizontal resolution
      break;
    case 1:
      eax = 480; // Vertical resolution
      break;
    case 15:
      eax = 0; //FIXME
      break;
    case 32:
      eax = 0; //FIXME
      break;
    default:
      eax = 16;
      printf("Unknown metric\n");
      assert(false);
      break;
  }
  esp += 1 * 4;
HACKY_IMPORT_END()

HACKY_IMPORT_BEGIN(RegisterClassA)
  hacky_printf("lpWndClass 0x%" PRIX32 "\n", stack[1]);
  eax = 444; //FIXME: ATOM, whatever that is?!
  esp += 1 * 4;
HACKY_IMPORT_END()

HACKY_IMPORT_BEGIN(RegisterClassExA)
  hacky_printf("lpWndClass 0x%" PRIX32 "\n", stack[1]);
  eax = 444; //FIXME: ATOM, whatever that is?!
  esp += 1 * 4;
HACKY_IMPORT_END()

HACKY_IMPORT_BEGIN(CreateWindowExA)
  hacky_printf("dwExStyle 0x%" PRIX32 "\n", stack[1]);
  hacky_printf("lpClassName 0x%" PRIX32 " ('%s')\n", stack[2], (char*)Memory(stack[2]));
  hacky_printf("lpWindowName 0x%" PRIX32 " ('%s')\n", stack[3], (char*)Memory(stack[3]));
  hacky_printf("dwStyle 0x%" PRIX32 "\n", stack[4]);
  hacky_printf("x 0x%" PRIX32 "\n", stack[5]);
  hacky_printf("y 0x%" PRIX32 "\n", stack[6]);
  hacky_printf("nWidth 0x%" PRIX32 "\n", stack[7]);
  hacky_printf("nHeight 0x%" PRIX32 "\n", stack[8]);
  hacky_printf("hWndParent 0x%" PRIX32 "\n", stack[9]);
  hacky_printf("hMenu 0x%" PRIX32 "\n", stack[10]);
  hacky_printf("hInstance 0x%" PRIX32 "\n", stack[11]);
  hacky_printf("lpParam 0x%" PRIX32 "\n", stack[12]);
  eax = 333; // HWND
  esp += 12 * 4;
HACKY_IMPORT_END()

HACKY_IMPORT_BEGIN(SetWindowPos)
  hacky_printf("hWnd 0x%" PRIX32 "\n", stack[1]);
  hacky_printf("hWndInsertAfter 0x%" PRIX32 "\n", stack[2]);
  hacky_printf("X 0x%" PRIX32 "\n", stack[3]);
  hacky_printf("Y 0x%" PRIX32 "\n", stack[4]);
  hacky_printf("cx 0x%" PRIX32 "\n", stack[5]);
  hacky_printf("cy 0x%" PRIX32 "\n", stack[6]);
  hacky_printf("uFlags 0x%" PRIX32 "\n", stack[7]);
  eax = 1; // nonzero if succeeds
  esp += 7 * 4;
HACKY_IMPORT_END()

HACKY_IMPORT_BEGIN(ShowWindow)
  hacky_printf("hWnd 0x%" PRIX32 "\n", stack[1]);
  hacky_printf("nCmdShow 0x%" PRIX32 "\n", stack[2]);
  eax = 0; // Previously hidden
  esp += 2 * 4;
HACKY_IMPORT_END()

HACKY_IMPORT_BEGIN(UpdateWindow)
  hacky_printf("hWnd 0x%" PRIX32 "\n", stack[1]);
  eax = 1; // nonzero if succeeds
  esp += 1 * 4;
HACKY_IMPORT_END()

HACKY_IMPORT_BEGIN(SetCursor)
  hacky_printf("hCursor 0x%" PRIX32 "\n", stack[1]);
  eax = 0; // NULL = there was no previous cursor
  esp += 1 * 4;
HACKY_IMPORT_END()


// WTF.. why?! COMCTL ordinal import which does nothing

HACKY_IMPORT_BEGIN(InitCommonControls)
HACKY_IMPORT_END()

// Weird font stuff

HACKY_IMPORT_BEGIN(CreateFontA)
  hacky_printf("nHeight 0x%" PRIX32 "\n", stack[1]);
  hacky_printf("nWidth 0x%" PRIX32 "\n", stack[2]);
  hacky_printf("nEscapement 0x%" PRIX32 "\n", stack[3]);
  hacky_printf("nOrientation 0x%" PRIX32 "\n", stack[4]);
  hacky_printf("fnWeight 0x%" PRIX32 "\n", stack[5]);
  hacky_printf("fdwItalic 0x%" PRIX32 "\n", stack[6]);
  hacky_printf("fdwUnderline 0x%" PRIX32 "\n", stack[7]);
  hacky_printf("fdwStrikeOut 0x%" PRIX32 "\n", stack[8]);
  hacky_printf("fdwCharSet 0x%" PRIX32 "\n", stack[9]);
  hacky_printf("fdwOutputPrecision 0x%" PRIX32 "\n", stack[10]);
  hacky_printf("fdwClipPrecision 0x%" PRIX32 "\n", stack[11]);
  hacky_printf("fdwQuality 0x%" PRIX32 "\n", stack[12]);
  hacky_printf("fdwPitchAndFamily 0x%" PRIX32 "\n", stack[13]);
  const char* lpszFace = (const char*)Memory(stack[14]);
  hacky_printf("lpszFace 0x%" PRIX32 " ('%s')\n", stack[14], lpszFace);
  eax = 0x1337C0DE; // HFONT Handle
  esp += 14 * 4;
HACKY_IMPORT_END()

// Console stuff

HACKY_IMPORT_BEGIN(SetConsoleTextAttribute)
  hacky_printf("hConsoleOutput 0x%" PRIX32 "\n", stack[1]);
  hacky_printf("wAttributes 0x%" PRIX32 "\n", stack[2]);
  eax = 1; // nonzero if succeeds
  esp += 2 * 4;
HACKY_IMPORT_END()

HACKY_IMPORT_BEGIN(WriteConsoleA)
  hacky_printf("hConsoleOutput 0x%" PRIX32 "\n", stack[1]);
  hacky_printf("lpBuffer 0x%" PRIX32 " ('%.*s')\n", stack[2], stack[3], Memory(stack[2]));
  hacky_printf("nNumberOfCharsToWrite 0x%" PRIX32 "\n", stack[3]);
  hacky_printf("lpNumberOfCharsWritten 0x%" PRIX32 "\n", stack[4]);
  hacky_printf("lpReserved 0x%" PRIX32 "\n", stack[5]);
  eax = 1; // nonzero if succeeds
  esp += 5 * 4;
HACKY_IMPORT_END()

// Function to find files

HACKY_IMPORT_BEGIN(CreateDirectoryA)
  const char* lpPathName = (const char*)Memory(stack[1]);
  hacky_printf("lpPathName 0x%" PRIX32 " ('%s')\n", stack[1], lpPathName);
  hacky_printf("lpSecurityAttributes 0x%" PRIX32 "\n", stack[2]);
  eax = 1; // nonzero if succeeds
  esp += 2 * 4;
HACKY_IMPORT_END()

HACKY_IMPORT_BEGIN(SetFileAttributesA)
  const char* lpFileName = (const char*)Memory(stack[1]);
  hacky_printf("lpFileName 0x%" PRIX32 " ('%s')\n", stack[1], lpFileName);
  hacky_printf("dwFileAttributes 0x%" PRIX32 "\n", stack[2]);
  eax = 1; // nonzero if succeeds
  esp += 2 * 4;
HACKY_IMPORT_END()

HACKY_IMPORT_BEGIN(FileTimeToLocalFileTime)
  hacky_printf("lpFileTime 0x%" PRIX32 "\n", stack[1]);
  hacky_printf("lpLocalFileTime 0x%" PRIX32 "\n", stack[2]);
  //FIXME
  eax = 1; // nonzero if succeeds
  esp += 2 * 4;
HACKY_IMPORT_END()

HACKY_IMPORT_BEGIN(FileTimeToSystemTime)
  hacky_printf("lpFileTime 0x%" PRIX32 "\n", stack[1]);
  hacky_printf("lpSystemTime 0x%" PRIX32 "\n", stack[2]);
  //FIXME
  eax = 1; // nonzero if succeeds
  esp += 2 * 4;
HACKY_IMPORT_END()


HACKY_IMPORT_BEGIN(FindFirstFileA)
  const char* pattern = (const char*)Memory(stack[1]);
  hacky_printf("lpFileName 0x%" PRIX32 " ('%s')\n", stack[1], pattern);
  hacky_printf("lpFindFileData 0x%" PRIX32 "\n", stack[2]);
//dwFileAttributes
  char* wildcard1 = strchr(pattern, '*');
  char* wildcard2 = strchr(pattern, '?');
  if ((wildcard1 == NULL) && (wildcard2 == NULL)) {
    // It's asking explicitly for one file..
    static char foundFile[128];
    if ((pattern[0] == '.') && (pattern[1] == '\\')) {
      pattern = &pattern[2];
    } else {
      assert(false);
    }
    strcpy(foundFile, pattern);
    static const char* passthrough[] = {
      ".", "..",
      foundFile,
      NULL
    };
    dirlisting = passthrough;
  } else if (!strcmp(".\\data\\player\\*.sav", pattern)) {
    static const char* profiles[] = {
      ".", "..",
      "anakin.sav",
      NULL
    };
    dirlisting = profiles;
  } else {
    const char* none[] = { NULL };
    dirlisting = none;
    printf("Unknown pattern: '%s'\n", pattern);
    SDL_Delay(3000);
  }

  if (*dirlisting) {
    API(WIN32_FIND_DATA)* data = Memory(stack[2]);
    data->dwFileAttributes = strchr(*dirlisting,'.') ? 0x80 : 0x10; // FILE_ATTRIBUTE_NORMAL or FILE_ATTRIBUTE_DIRECTORY
    sprintf(data->cFileName, "%s", *dirlisting);
    dirlisting++;
    eax = 123; // File found
  } else {
    eax = 0xFFFFFFFF; // INVALID_HANDLE_VALUE = No files found
  }

  esp += 2 * 4;
HACKY_IMPORT_END()

HACKY_IMPORT_BEGIN(FindNextFileA)
  hacky_printf("hFindFile 0x%" PRIX32 "\n", stack[1]);
  hacky_printf("lpFindFileData 0x%" PRIX32 "\n", stack[2]);

  if (*dirlisting) {
    API(WIN32_FIND_DATA)* data = Memory(stack[2]);
    data->dwFileAttributes = strchr(*dirlisting,'.') ? 0x80 : 0x10; // FILE_ATTRIBUTE_NORMAL or FILE_ATTRIBUTE_DIRECTORY
    sprintf(data->cFileName, "%s", *dirlisting);
    dirlisting++;
    eax = 1; // File found
  } else {
    eax = 0; // No file found
  }

  esp += 2 * 4;
HACKY_IMPORT_END()

HACKY_IMPORT_BEGIN(FindClose)
  hacky_printf("hFindFile 0x%" PRIX32 "\n", stack[1]);
  eax = 1; // nonzero if succeeds
  esp += 1 * 4;
HACKY_IMPORT_END()




// Name entry screen

HACKY_IMPORT_BEGIN(GetKeyState)
  API(SHORT) pressed = 0x8000; // high order bit = pressed
  API(SHORT) toggled = 0x0001; // low order bit = toggled
  API(SHORT) returnValue = 0; // default: unpressed
  int nVirtKey = stack[1];
  switch(nVirtKey) {
    case API(VK_CAPITAL):
      returnValue = 0;
      break;
    default:
      break;
  }
  eax = returnValue;
  esp += 1 * 4;
HACKY_IMPORT_END()

HACKY_IMPORT_BEGIN(MapVirtualKeyA)
  API(UINT) uCode = stack[1];
  API(UINT) uMapType = stack[2];

  hacky_printf("uCode 0x%" PRIX32 "\n", uCode);
  hacky_printf("uMapType 0x%" PRIX32 "\n", uMapType);

  API(UINT) returnValue = 0; // 0 = no map
  switch(uMapType) {
    case 1: // MAPVK_VSC_TO_VK: uCode is a scan code and is translated into a virtual-key code that does not distinguish between left- and right-hand keys. If there is no translation, the function returns 0.
      if (uCode == API(VK_LSHIFT) || uCode == API(VK_RSHIFT)) {
        returnValue = API(VK_SHIFT);
      } else if (uCode == API(VK_LCONTROL) || uCode == API(VK_RCONTROL)) {
        returnValue = API(VK_CONTROL);
      } else if (uCode == API(VK_LMENU) || uCode == API(VK_RMENU)) {
        returnValue = API(VK_MENU);
      } else {
        returnValue = uCode; // FIXME: is this okay?
      }
      break;
    case 2: // MAPVK_VK_TO_CHAR: uCode is a virtual-key code and is translated into an unshifted character value in the low-order word of the return value. Dead keys (diacritics) are indicated by setting the top bit of the return value. If there is no translation, the function returns 0.
      if (uCode >= 0x41 && uCode <= 0x5A) { // A to Z key
        returnValue = 'a' + uCode - 0x41;
      }
      //FIXME: Other symbols
      // Assume unmapped otherwise
      break;
    default:
      printf("Unknown key map mode in MapVirtualKeyA\n");
      assert(false);
      break;
  }
  eax = returnValue;
  esp += 2 * 4;
HACKY_IMPORT_END()



  // Copy protection

HACKY_IMPORT_BEGIN(RegOpenKeyExA)
  hacky_printf("hKey 0x%" PRIX32 "\n", stack[1]);
  hacky_printf("lpSubKey 0x%" PRIX32 " ('%s')\n", stack[2], (char*)Memory(stack[2]));
  hacky_printf("ulOptions 0x%" PRIX32 "\n", stack[3]);
  hacky_printf("samDesired 0x%" PRIX32 "\n", stack[4]);
  hacky_printf("phkResult 0x%" PRIX32 "\n", stack[5]);
  eax = 0; // ERROR_SUCCESS
  esp += 5 * 4;
HACKY_IMPORT_END()

HACKY_IMPORT_BEGIN(GetLogicalDrives)
  eax = (1 << 0) | (1 << 2) | (1 << 3); // A, C, D
HACKY_IMPORT_END()

HACKY_IMPORT_BEGIN(GetDriveTypeA)
  hacky_printf("lpSubKey 0x%" PRIX32 " ('%s')\n", stack[1], (char*)Memory(stack[1]));
  eax = 5; // Claim everything is CDROM
  esp += 1 * 4;
HACKY_IMPORT_END()

HACKY_IMPORT_BEGIN(DeleteFileA)
  hacky_printf("lpFileName 0x%" PRIX32 " ('%s')\n", stack[1], (char*)Memory(stack[1]));
  //FIXME: Only stubbed for security reasons
  eax = 1; // nonzero if succeeds
  esp += 1 * 4;
HACKY_IMPORT_END()

HACKY_IMPORT_BEGIN(SetErrorMode)
  hacky_printf("uMode 0x%" PRIX32 "\n", stack[1]);
  //FIXME: Only stubbed for security reasons
  eax = 0; // Previous mode
  esp += 1 * 4;
HACKY_IMPORT_END()



// CD check

HACKY_IMPORT_BEGIN(GetFileVersionInfoSizeA)
  hacky_printf("lptstrFilename 0x%" PRIX32 " (%s)\n", stack[1], Memory(stack[1]));
  hacky_printf("lpdwHandle 0x%" PRIX32 "\n", stack[2]);

  if (stack[2]) {
    *(uint32_t*)Memory(stack[2]) = 0;
  }

  eax = 0; // Size of file version info [0 means error]
  esp += 2 * 4;
HACKY_IMPORT_END()

HACKY_IMPORT_BEGIN(GetVolumeInformationA)
  hacky_printf("lpRootPathName 0x%" PRIX32 " (%s)\n", stack[1], Memory(stack[1]));
  hacky_printf("lpVolumeNameBuffer 0x%" PRIX32 "\n", stack[2]);
  hacky_printf("nVolumeNameSize %" PRIu32 "\n", stack[3]);
  hacky_printf("lpVolumeSerialNumber 0x%" PRIX32 "\n", stack[4]);
  hacky_printf("lpMaximumComponentLength 0x%" PRIX32 "\n", stack[5]);
  hacky_printf("lpFileSystemFlags 0x%" PRIX32 "\n", stack[6]);
  hacky_printf("lpFileSystemNameBuffer 0x%" PRIX32 "\n", stack[7]);
  hacky_printf("nFileSystemNameSize %" PRIu32 "\n", stack[8]);

  strcpy(Memory(stack[2]), "racer100_0");

  eax = 1; // BOOL, non-zero means all requested info was available
  esp += 8 * 4;
HACKY_IMPORT_END()







// Smush

HACKY_IMPORT_BEGIN(SmushGetFrameCount)
  eax = 0; // int
  // cdecl
HACKY_IMPORT_END()

HACKY_IMPORT_BEGIN(SmushPlay)
  hacky_printf("filename 0x%" PRIX32 " ('%s')\n", stack[1], (char*)Memory(stack[1]));
  hacky_printf("arg2 0x%" PRIX32 "\n", stack[2]);
  hacky_printf("arg3 0x%" PRIX32 "\n", stack[3]);
  hacky_printf("arg4 0x%" PRIX32 "\n", stack[4]);
  hacky_printf("arg5 0x%" PRIX32 "\n", stack[5]);
  hacky_printf("width %" PRIu32 "\n", stack[6]);
  hacky_printf("height %" PRIu32 "\n", stack[7]);
  hacky_printf("arg8 0x%" PRIX32 "\n", stack[8]);
  hacky_printf("arg9 0x%" PRIX32 "\n", stack[9]);
  hacky_printf("frameRenderCallback 0x%" PRIX32 "\n", stack[10]);
  hacky_printf("arg11 0x%" PRIX32 "\n", stack[11]);
  hacky_printf("arg12 0x%" PRIX32 "\n", stack[12]);
  hacky_printf("arg13 0x%" PRIX32 "\n", stack[13]);

  eax = 0; // int
  // cdecl
HACKY_IMPORT_END()

HACKY_IMPORT_BEGIN(SmushSetVolume)
  hacky_printf("volume 0x%" PRIX32 "\n", stack[1]);
  eax = 0; // void
  // cdecl
HACKY_IMPORT_END()


HACKY_IMPORT_BEGIN(SmushShutdown)
  eax = 0; // void
  // cdecl
HACKY_IMPORT_END()

HACKY_IMPORT_BEGIN(SmushStartup)
  hacky_printf("hwnd 0x%" PRIX32 "\n", stack[1]);
  hacky_printf("directSound 0x%" PRIX32 "\n", stack[2]);
  eax = 1; // int
  // cdecl
HACKY_IMPORT_END()




// IDirectPlayLobby3A 


// IDirectPlayLobby3A -> STDMETHOD_(ULONG,Release)       (THIS) PURE; //2
HACKY_COM_BEGIN(IDirectPlayLobby3A, 2)
  hacky_printf("p 0x%" PRIX32 "\n", stack[1]);
  eax = 0; // FIXME: No idea what this expects to return..
  esp += 1 * 4;
HACKY_COM_END()

// IDirectPlayLobby3A -> STDMETHOD(GetConnectionSettings)(THIS_ DWORD, LPVOID, LPDWORD) PURE; //8
HACKY_COM_BEGIN(IDirectPlayLobby3A, 8)
  hacky_printf("p 0x%" PRIX32 "\n", stack[1]);
  hacky_printf("a 0x%" PRIX32 "\n", stack[2]);
  hacky_printf("b 0x%" PRIX32 "\n", stack[3]);
  hacky_printf("c 0x%" PRIX32 "\n", stack[4]);
  *(uint32_t*)Memory(stack[4]) = 1000; //FIXME: Return proper size (revolt.exe assumes this is a certain minimum size and operates on it directly without checking)
  //      eax = 0; // HRESULT -> non-negative means success
  //HACK: Return NOTLOBBIED (?) which revolt thinks is fine = lobby already created?!
  eax = 0x8877042e;
  esp += 4 * 4;
HACKY_COM_END()

// IDirectPlayLobby3A -> STDMETHOD(RegisterApplication)  (THIS_ DWORD, LPVOID) PURE; //16
HACKY_COM_BEGIN(IDirectPlayLobby3A, 16)
  hacky_printf("p 0x%" PRIX32 "\n", stack[1]);
  hacky_printf("a 0x%" PRIX32 "\n", stack[2]);
  hacky_printf("b 0x%" PRIX32 "\n", stack[3]);
  eax = 0; // HRESULT -> non-negative means success
  esp += 3 * 4;
HACKY_COM_END()








// IID_IDirectDraw4


// IID_IDirectDraw4 -> STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR * ppvObj) PURE; // 0
HACKY_COM_BEGIN(IDirectDraw4, 0)
  hacky_printf("p 0x%" PRIX32 "\n", stack[1]);
  hacky_printf("riid 0x%" PRIX32 "\n", stack[2]);
  hacky_printf("ppvObj 0x%" PRIX32 "\n", stack[3]);
  const API(IID)* iid = (const API(IID)*)Memory(stack[2]);

  char iidString[1024];
  sprintf(iidString, "%08" PRIX32 "-%04" PRIX16 "-%04" PRIX16 "-%02" PRIX8 "%02" PRIX8 "-%02" PRIX8 "%02" PRIX8 "%02" PRIX8 "%02" PRIX8 "%02" PRIX8 "%02" PRIX8,
          iid->Data1, iid->Data2, iid->Data3,
          iid->Data4[0], iid->Data4[1], iid->Data4[2], iid->Data4[3],
          iid->Data4[4], iid->Data4[5], iid->Data4[6], iid->Data4[7]);
  printf("  (read iid: {%s})\n", iidString);

  char name[32];  

  if (!strcmp(iidString, "9C59509A-39BD-11D1-8C4A-00C04FD930C5")) {
    strcpy(name, "IDirectDraw4");
  } else if (!strcmp(iidString, "BB223240-E72B-11D0-A9B4-00AA00C0993E")) {
    strcpy(name, "IDirect3D3");
  } else {
    assert(false);
  }

  *(Address*)Memory(stack[3]) = CreateInterface(name, 200);
  eax = 0; // FIXME: No idea what this expects to return..
  esp += 3 * 4;
HACKY_COM_END()

// IID_IDirectDraw4 -> STDMETHOD_(ULONG,Release) (THIS) PURE; // 2
HACKY_COM_BEGIN(IDirectDraw4, 2)
  hacky_printf("p 0x%" PRIX32 "\n", stack[1]);
  eax = 0; // FIXME: No idea what this expects to return..
  esp += 1 * 4;
HACKY_COM_END()

// IID_IDirectDraw4 -> STDMETHOD(CreatePalette)(THIS_ DWORD, LPPALETTEENTRY, LPDIRECTDRAWPALETTE FAR*, IUnknown FAR * ) PURE; // 5
HACKY_COM_BEGIN(IDirectDraw4, 5)
  hacky_printf("p 0x%" PRIX32 "\n", stack[1]);
  hacky_printf("a 0x%" PRIX32 "\n", stack[2]);
  hacky_printf("b 0x%" PRIX32 "\n", stack[3]);
  hacky_printf("c 0x%" PRIX32 "\n", stack[4]);
  hacky_printf("d 0x%" PRIX32 "\n", stack[5]);
  *(Address*)Memory(stack[4]) = CreateInterface("IDirectDrawPalette", 200);
  eax = 0; // FIXME: No idea what this expects to return..
  esp += 5 * 4;
HACKY_COM_END()

// IID_IDirectDraw4 -> STDMETHOD(CreateSurface)(THIS_  LPDDSURFACEDESC2, LPDIRECTDRAWSURFACE4 FAR *, IUnknown FAR *) PURE; // 6
HACKY_COM_BEGIN(IDirectDraw4, 6)
  hacky_printf("p 0x%" PRIX32 "\n", stack[1]);
  hacky_printf("a 0x%" PRIX32 "\n", stack[2]);
  hacky_printf("b 0x%" PRIX32 "\n", stack[3]);
  hacky_printf("c 0x%" PRIX32 "\n", stack[4]);

  Address surfaceAddress = CreateInterface("IDirectDrawSurface4", 50);
  API(DirectDrawSurface4)* surface = (API(DirectDrawSurface4)*)Memory(surfaceAddress);

  *(Address*)Memory(stack[3]) = surfaceAddress;

  API(DDSURFACEDESC2)* desc = (API(DDSURFACEDESC2)*)Memory(stack[2]);

  printf("dwSize = %" PRIu32 "\n", desc->dwSize);
  printf("dwFlags = 0x%08" PRIX32 "\n", desc->dwFlags);
  printf("ddsCaps.dwCaps = 0x%08" PRIX32 "\n", desc->ddsCaps.dwCaps);
  printf("dwWidth = %" PRIu32 "\n", desc->dwWidth);
  printf("dwHeight = %" PRIu32 "\n", desc->dwHeight);

  printf("ddpfPixelFormat.dwSize = %" PRIu32 "\n", desc->ddpfPixelFormat.dwSize);
  printf("ddpfPixelFormat.dwFlags = 0x%08" PRIX32 "\n", desc->ddpfPixelFormat.dwFlags);

  printf("ddpfPixelFormat.dwRGBBitCount = %" PRIu32 "\n", desc->ddpfPixelFormat.dwRGBBitCount);
  printf("ddpfPixelFormat.dwRBitMask =        0x%08" PRIX32 "\n", desc->ddpfPixelFormat.dwRBitMask);
  printf("ddpfPixelFormat.dwGBitMask =        0x%08" PRIX32 "\n", desc->ddpfPixelFormat.dwGBitMask);
  printf("ddpfPixelFormat.dwBBitMask =        0x%08" PRIX32 "\n", desc->ddpfPixelFormat.dwBBitMask);
  printf("ddpfPixelFormat.dwRGBAlphaBitMask = 0x%08" PRIX32 "\n", desc->ddpfPixelFormat.dwRGBAlphaBitMask);


  memcpy(&surface->desc, desc, sizeof(API(DDSURFACEDESC2)));

enum {
  API(DDSD_PITCH) = 0x00000008l
};

  surface->desc.dwFlags = API(DDSD_PITCH);
  surface->desc.lPitch = surface->desc.dwWidth * desc->ddpfPixelFormat.dwRGBBitCount / 8;

  if (desc->ddsCaps.dwCaps & API(DDSCAPS_TEXTURE)) {
    // FIXME: Delay this until the interface is queried the first time?!
    surface->texture = CreateInterface("IDirect3DTexture2", 20);
    API(Direct3DTexture2)* texture = (API(Direct3DTexture2)*)Memory(surface->texture);
    texture->surface = surfaceAddress;
    glGenTextures(1, &texture->handle);
    printf("GL handle is %d\n", texture->handle);
  } else {
    //FIXME: only added to catch bugs, null pointer should work
    surface->texture = CreateInterface("invalid", 200);

    //FIXME: WTF is this shit?!
    API(Direct3DTexture2)* texture = (API(Direct3DTexture2)*)Memory(surface->texture);
    glGenTextures(1, &texture->handle);
    //assert(false);
  }

  eax = 0; // FIXME: No idea what this expects to return..
  esp += 4 * 4;
HACKY_COM_END()

// IID_IDirectDraw4 -> STDMETHOD(EnumDisplayModes)( THIS_ DWORD, LPDDSURFACEDESC2, LPVOID, LPDDENUMMODESCALLBACK2 ) PURE; // 8
HACKY_COM_BEGIN(IDirectDraw4, 8)
  hacky_printf("EnumDisplayModes\n");
  uint32_t a = stack[2];
  uint32_t b = stack[3];
  uint32_t c = stack[4];
  Address d = stack[5];
  hacky_printf("p 0x%" PRIX32 "\n", stack[1]);
  hacky_printf("a 0x%" PRIX32 "\n", stack[2]);
  hacky_printf("b 0x%" PRIX32 "\n", stack[3]);
  hacky_printf("c 0x%" PRIX32 "\n", stack[4]);
  hacky_printf("d 0x%" PRIX32 "\n", d);
  eax = 0; // FIXME: No idea what this expects to return..
  esp += 5 * 4;
  // Push a call to the callback onto the stack.. this is some ugly hack..

  // Convention is PASCAL

  esp -= 4;
  *(uint32_t*)Memory(esp) = returnAddress;

  {
    esp -= 4;
    *(uint32_t*)Memory(esp) = c; // user pointer

    esp -= 4;
    Address descAddress = Allocate(sizeof(API(DDSURFACEDESC2)));
    API(DDSURFACEDESC2)* desc = Memory(descAddress);
    desc->ddpfPixelFormat.dwFlags = API(DDPF_RGB);
    desc->ddpfPixelFormat.dwRGBBitCount = 24;
    desc->dwWidth = 640;
    desc->dwHeight = 480;
    desc->lpSurface = 0x01010101;
    *(uint32_t*)Memory(esp) = descAddress; // API(DDSURFACEDESC2)*

    // Emulate the call
    esp -= 4;
    *(uint32_t*)Memory(esp) = clearEax; // Return to clear eax
    eip = d;
    printf("  Callback at 0x%" PRIX32 "\n", eip);
    //FIXME: Add a hook which returns 0
  }
HACKY_COM_END()

// IID_IDirectDraw4 -> STDMETHOD(FlipToGDISurface)(THIS) PURE; // 10
HACKY_COM_BEGIN(IDirectDraw4, 10)
  hacky_printf("FlipToGDISurface\n");
  hacky_printf("p 0x%" PRIX32 "\n", stack[1]);

  SDL_GL_SwapWindow(sdlWindow);

  eax = 0; // FIXME: No idea what this expects to return..
  esp += 1 * 4;
HACKY_COM_END()

// IID_IDirectDraw4 -> STDMETHOD(GetCaps)( THIS_ LPDDCAPS, LPDDCAPS) PURE; // 11
HACKY_COM_BEGIN(IDirectDraw4, 11)
  hacky_printf("p 0x%" PRIX32 "\n", stack[1]);
  hacky_printf("a 0x%" PRIX32 "\n", stack[2]); // HAL
  hacky_printf("b 0x%" PRIX32 "\n", stack[3]); // SW

//FIXME: !!!!!!!!! Set some caps?!

// (+8) & 0x80000 ? 0 : 1; // #define DDCAPS2_CANRENDERWINDOWED 0x00080000
// (+4) & 1 ? 1 : 0; // #define DDCAPS_3D	0x00000001
// (+64)
// (+60)

#if 1
  API(DDCAPS)* halCaps = Memory(stack[2]);
  API(DDCAPS)* swCaps = Memory(stack[3]);

  printf("halCaps is %d bytes (known: %d bytes)\n", halCaps->dwSize, sizeof(API(DDCAPS)));

  halCaps->dwCaps = API(DDCAPS_3D) | API(DDCAPS_BLTDEPTHFILL);
  halCaps->dwCaps2 = API(DDCAPS2_CANRENDERWINDOWED);

  halCaps->dwVidMemTotal = 16*1024*1024; // 16MiB VRAM free :)
  halCaps->dwVidMemFree = 12*1024*1024; // 12MiB VRAM free :(
  
#endif

  eax = 0; // FIXME: No idea what this expects to return..
  esp += 3 * 4;
HACKY_COM_END()

// IID_IDirectDraw4 -> STDMETHOD(Initialize)(THIS_ GUID FAR *) PURE; // 18
HACKY_COM_BEGIN(IDirectDraw4, 18)
  hacky_printf("p 0x%" PRIX32 "\n", stack[1]);
  hacky_printf("a 0x%" PRIX32 "\n", stack[2]);
  eax = 0; // FIXME: No idea what this expects to return..
  esp += 2 * 4;
HACKY_COM_END()

// IID_IDirectDraw4 ->    STDMETHOD(RestoreDisplayMode)(THIS) PURE; // 19
HACKY_COM_BEGIN(IDirectDraw4, 19)
  hacky_printf("p 0x%" PRIX32 "\n", stack[1]);
  eax = 0; // FIXME: No idea what this expects to return..
  esp += 1 * 4;
HACKY_COM_END()

// IID_IDirectDraw4 -> STDMETHOD(SetCooperativeLevel)(THIS_ HWND, DWORD) PURE; // 20
HACKY_COM_BEGIN(IDirectDraw4, 20)
  hacky_printf("p 0x%" PRIX32 "\n", stack[1]);
  hacky_printf("a 0x%" PRIX32 "\n", stack[2]);
  hacky_printf("b 0x%" PRIX32 "\n", stack[3]);
  eax = 0; // FIXME: No idea what this expects to return..
  esp += 3 * 4;
HACKY_COM_END()

// IID_IDirectDraw4 -> STDMETHOD(SetDisplayMode)(THIS_ DWORD, DWORD,DWORD, DWORD, DWORD) PURE; // 21
HACKY_COM_BEGIN(IDirectDraw4, 21)
  hacky_printf("p 0x%" PRIX32 "\n", stack[1]);
  hacky_printf("a 0x%" PRIX32 "\n", stack[2]);
  hacky_printf("b 0x%" PRIX32 "\n", stack[3]);
  hacky_printf("c 0x%" PRIX32 "\n", stack[4]);
  hacky_printf("d 0x%" PRIX32 "\n", stack[5]);
  hacky_printf("e 0x%" PRIX32 "\n", stack[6]);
  eax = 0; // FIXME: No idea what this expects to return..
  esp += 6 * 4;
HACKY_COM_END()

// IID_IDirectDraw4 -> STDMETHOD(GetAvailableVidMem)(THIS_ LPDDSCAPS2, LPDWORD, LPDWORD) PURE; // 23
HACKY_COM_BEGIN(IDirectDraw4, 23)
  hacky_printf("p 0x%" PRIX32 "\n", stack[1]);
  hacky_printf("a 0x%" PRIX32 "\n", stack[2]);
  hacky_printf("b 0x%" PRIX32 "\n", stack[3]);
  hacky_printf("c 0x%" PRIX32 "\n", stack[4]);
  eax = 0; // FIXME: No idea what this expects to return..
  esp += 4 * 4;
HACKY_COM_END()

// IID_IDirectDraw4 -> STDMETHOD(GetDeviceIdentifier)(THIS_ LPDDDEVICEIDENTIFIER, DWORD ) PURE; // 27
HACKY_COM_BEGIN(IDirectDraw4, 27)
  hacky_printf("p 0x%" PRIX32 "\n", stack[1]);
  hacky_printf("a 0x%" PRIX32 "\n", stack[2]);
  hacky_printf("b 0x%" PRIX32 "\n", stack[3]);
  eax = 0; // FIXME: No idea what this expects to return..
  esp += 3 * 4;
HACKY_COM_END()








// IDirectDrawSurface4

// IDirectDrawSurface4 -> STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR * ppvObj) PURE; // 0
HACKY_COM_BEGIN(IDirectDrawSurface4, 0)
  hacky_printf("p 0x%" PRIX32 "\n", stack[1]);
  hacky_printf("a 0x%" PRIX32 "\n", stack[2]);
  hacky_printf("b 0x%" PRIX32 "\n", stack[3]);
  API(DirectDrawSurface4)* this = (API(DirectDrawSurface4)*)Memory(stack[1]);
  const API(IID)* iid = (const API(IID)*)Memory(stack[2]);
  printf("  (read iid: {%08" PRIX32 "-%04" PRIX16 "-%04" PRIX16 "-%02" PRIX8 "%02" PRIX8 "-%02" PRIX8 "%02" PRIX8 "%02" PRIX8 "%02" PRIX8 "%02" PRIX8 "%02" PRIX8 "})\n",
         iid->Data1, iid->Data2, iid->Data3,
         iid->Data4[0], iid->Data4[1], iid->Data4[2], iid->Data4[3],
         iid->Data4[4], iid->Data4[5], iid->Data4[6], iid->Data4[7]);
  if (iid->Data1 == 0x93281502) { //FIXME: Check for full GUID (Direct3DTexture2)
    printf("Returning texture 0x%" PRIX32 "\n", this->texture);
    *(Address*)Memory(stack[3]) = this->texture;
  } else {
    assert(false);
  }
  eax = 0; // FIXME: No idea what this expects to return..
  esp += 3 * 4;
HACKY_COM_END()

// IDirectDrawSurface4 -> STDMETHOD_(ULONG,AddRef) (THIS)  PURE; // 1
HACKY_COM_BEGIN(IDirectDrawSurface4, 1)
  hacky_printf("p 0x%" PRIX32 "\n", stack[1]);
  eax = 1; // New reference count
  esp += 1 * 4;
HACKY_COM_END()

// IDirectDrawSurface4 -> STDMETHOD_(ULONG,Release)       (THIS) PURE; //2
HACKY_COM_BEGIN(IDirectDrawSurface4, 2)
  hacky_printf("p 0x%" PRIX32 "\n", stack[1]);
  eax = 0; // FIXME: No idea what this expects to return..
  esp += 1 * 4;
HACKY_COM_END()

// IDirectDrawSurface4 -> STDMETHOD(AddAttachedSurface)(THIS_ LPDIRECTDRAWSURFACE4) PURE; // 3
HACKY_COM_BEGIN(IDirectDrawSurface4, 3)
  hacky_printf("p 0x%" PRIX32 "\n", stack[1]);
  hacky_printf("a 0x%" PRIX32 "\n", stack[2]);
  eax = 0; // FIXME: No idea what this expects to return..
  esp += 2 * 4;
HACKY_COM_END()

// IDirectDrawSurface4 -> STDMETHOD(Blt)(THIS_ LPRECT,LPDIRECTDRAWSURFACE4, LPRECT,DWORD, LPDDBLTFX) PURE; // 5
HACKY_COM_BEGIN(IDirectDrawSurface4, 5)
  hacky_printf("Blt\n");
  hacky_printf("p 0x%" PRIX32 "\n", stack[1]);
  hacky_printf("a 0x%" PRIX32 "\n", stack[2]);
  hacky_printf("b 0x%" PRIX32 "\n", stack[3]);
  hacky_printf("c 0x%" PRIX32 "\n", stack[4]);
  uint32_t d = stack[5];
  uint32_t e = stack[6];
  hacky_printf("d 0x%08" PRIX32 "\n", d);
  hacky_printf("e 0x%" PRIX32 "\n", e);

  API(DirectDrawSurface4)* this = (API(DirectDrawSurface4)*)Memory(stack[1]);

  API(DDBLTFX)* bltfx = Memory(e);

  assert((d & ~(API(DDBLT_COLORFILL) | API(DDBLT_WAIT) | API(DDBLT_DEPTHFILL))) == 0);

  if (d & API(DDBLT_WAIT)) {
    // nop
  }

  if (d & API(DDBLT_COLORFILL)) {
    assert(!(this->desc.ddsCaps.dwCaps & API(DDSCAPS_ZBUFFER)));

    //FIXME: Why is this zero during startup?!
    if (this->desc.ddpfPixelFormat.dwRGBBitCount != 0) {
      assert(this->desc.ddpfPixelFormat.dwRGBBitCount == 16);
    }

    glClearColor(((bltfx->dwFillColor >> 11) & 0x1F) / 31.0f,
                 ((bltfx->dwFillColor >> 5) & 0x3F) / 63.0f,
                 (bltfx->dwFillColor & 0x1F) / 31.0f,
                 255.0f);
    glClear(GL_COLOR_BUFFER_BIT);
  }

  if (d & API(DDBLT_DEPTHFILL)) {
    assert(this->desc.ddsCaps.dwCaps & API(DDSCAPS_ZBUFFER));
    assert(this->desc.ddpfPixelFormat.dwZBufferBitDepth == 16);

    glDepthMask(GL_TRUE);
    assert(bltfx->dwFillDepth = 0xFFFF);
    glClearDepthf(1.0f); //FIXME!!
    glClear(GL_DEPTH_BUFFER_BIT);
  }

  eax = 0; // FIXME: No idea what this expects to return..
  esp += 6 * 4;
HACKY_COM_END()

// IDirectDrawSurface4 -> STDMETHOD(DeleteAttachedSurface)(THIS_ DWORD,LPDIRECTDRAWSURFACE4) PURE; // 8
HACKY_COM_BEGIN(IDirectDrawSurface4, 8)
  hacky_printf("p 0x%" PRIX32 "\n", stack[1]);
  hacky_printf("a 0x%" PRIX32 "\n", stack[2]);
  hacky_printf("b 0x%" PRIX32 "\n", stack[3]);

  eax = 0; // FIXME: No idea what this expects to return..
  esp += 3 * 4;
HACKY_COM_END()

// IDirectDrawSurface4 -> STDMETHOD(Flip)(THIS_ LPDIRECTDRAWSURFACE4, DWORD) PURE; // 11
HACKY_COM_BEGIN(IDirectDrawSurface4, 11)
  hacky_printf("Flip\n");
  hacky_printf("p 0x%" PRIX32 "\n", stack[1]);
  hacky_printf("a 0x%" PRIX32 "\n", stack[2]);
  hacky_printf("b 0x%" PRIX32 "\n", stack[3]);

  SDL_GL_SwapWindow(sdlWindow);

  eax = 0; // FIXME: No idea what this expects to return..
  esp += 3 * 4;
HACKY_COM_END()

// IDirectDrawSurface4 -> STDMETHOD(GetAttachedSurface)(THIS_ LPDDSCAPS2, LPDIRECTDRAWSURFACE4 FAR *) PURE; // 12
HACKY_COM_BEGIN(IDirectDrawSurface4, 12)
  //FIXME: Don't allow this to run unlimited times, it wastes memory like crazy.
  //       Instead, do it properly..
  hacky_printf("p 0x%" PRIX32 "\n", stack[1]);
  hacky_printf("a 0x%" PRIX32 "\n", stack[2]);
  hacky_printf("b 0x%" PRIX32 "\n", stack[3]);
  API(DDSCAPS2)* caps = (API(DDSCAPS2)*)Memory(stack[2]);

  printf("dwCaps = 0x%08" PRIX32 "\n", caps->dwCaps);

  if (caps->dwCaps & API(DDSCAPS_MIPMAP)) {
    //FIXME: This is probably BAD!
    printf("Redirecting to itself\n");
    *(Address*)Memory(stack[3]) = stack[1];
  } else {
    printf("Creating new dummy surface\n");
    Address surfaceAddress = CreateInterface("IDirectDrawSurface4", 50);
    API(DirectDrawSurface4)* surface = (API(DirectDrawSurface4)*)Memory(surfaceAddress);
    memset(&surface->desc, 0x00, sizeof(surface->desc));

    // FIXME: 32 bit would be ignored by the game. Keeps falling back to 16
    surface->desc.ddpfPixelFormat.dwRGBBitCount = 16;

    surface->texture = 0;
    *(Address*)Memory(stack[3]) = surfaceAddress;
  }
  //FIXME: Used to retrieve surface for mipmaps?!
  eax = 0; // FIXME: No idea what this expects to return..
  esp += 3 * 4;
HACKY_COM_END()

// IDirectDrawSurface4 -> STDMETHOD(GetDC)(THIS_ HDC FAR *) PURE; // 17
HACKY_COM_BEGIN(IDirectDrawSurface4, 17)
  hacky_printf("p 0x%" PRIX32 "\n", stack[1]);
  hacky_printf("a 0x%" PRIX32 "\n", stack[2]);

  API(DirectDrawSurface4)* this = (API(DirectDrawSurface4)*)Memory(stack[1]);
  if (this->texture != 0) {
    API(Direct3DTexture2)* texture = (API(Direct3DTexture2)*)Memory(this->texture);
    printf("Returning GL tex handle %d\n", texture->handle);
    *(Address*)Memory(stack[2]) = texture->handle;
  } else {
    printf("Invalid GL tex handle\n");
    *(Address*)Memory(stack[2]) = 0;
  }

  eax = 0; // FIXME: No idea what this expects to return..
  esp += 2 * 4;
HACKY_COM_END()

// IDirectDrawSurface4 -> STDMETHOD(GetSurfaceDesc)(THIS_ LPDDSURFACEDESC2) PURE; // 22
HACKY_COM_BEGIN(IDirectDrawSurface4, 22)
  hacky_printf("p 0x%" PRIX32 "\n", stack[1]);
  hacky_printf("a 0x%" PRIX32 "\n", stack[2]);

  API(DDSURFACEDESC2)* desc = (API(DDSURFACEDESC2)*)Memory(stack[2]);
  //FIXME?!  

  eax = 0; // FIXME: No idea what this expects to return..
  esp += 2 * 4;
HACKY_COM_END()

// IDirectDrawSurface4 -> STDMETHOD(Lock)(THIS_ LPRECT,LPDDSURFACEDESC2,DWORD,HANDLE) PURE; // 25
HACKY_COM_BEGIN(IDirectDrawSurface4, 25)
  hacky_printf("p 0x%" PRIX32 "\n", stack[1]);
  hacky_printf("a 0x%" PRIX32 "\n", stack[2]);
  hacky_printf("b 0x%" PRIX32 "\n", stack[3]);
  hacky_printf("c 0x%" PRIX32 "\n", stack[4]);
  hacky_printf("d 0x%" PRIX32 "\n", stack[5]);

  API(DirectDrawSurface4)* this = (API(DirectDrawSurface4)*)Memory(stack[1]);

  assert(stack[2] == 0);
  assert(stack[5] == 0);

  //Hack: Part 1: check if we already have this surface in RAM
  if (this->desc.lpSurface == 0) {
    this->desc.lpSurface = Allocate(this->desc.dwHeight * this->desc.lPitch);
    memset(Memory(this->desc.lpSurface), 0x77, this->desc.dwHeight * this->desc.lPitch);
  }


  if (this->desc.ddsCaps.dwCaps & API(DDSCAPS_ZBUFFER)) {
    assert(this->desc.lPitch == 2 * this->desc.dwWidth);

    uint8_t* pixel_buffer = Memory(this->desc.lpSurface);
    glReadPixels(0, 0, this->desc.dwWidth, this->desc.dwHeight, GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT, pixel_buffer);

    // Reserve a buffer for swapping lines during image flipping
    static size_t line_buffer_size = 0;
    static uint8_t* line_buffer = NULL;
    size_t line_size = this->desc.dwWidth * 2;
    if (line_size > line_buffer_size) {
      line_buffer = realloc(line_buffer, line_size);
      line_buffer_size = line_size;
    }

    // Do the vertical flip
    for(unsigned int i = 0; i < this->desc.dwHeight / 2; i++) {
      uint8_t* line_a = &pixel_buffer[this->desc.lPitch * i];
      uint8_t* line_b = &pixel_buffer[this->desc.lPitch * (this->desc.dwHeight - 1 - i)];
      memcpy(line_buffer, line_a, line_size);
      memcpy(line_a, line_b, line_size);
      memcpy(line_b, line_buffer, line_size);
    }

  }


  API(DDSURFACEDESC2)* desc = Memory(stack[3]);
  memcpy(desc, &this->desc, sizeof(API(DDSURFACEDESC2)));
  
  printf("%d x %d (pitch: %d); bpp = %d; at 0x%08X\n", desc->dwWidth, desc->dwHeight, desc->lPitch, desc->ddpfPixelFormat.dwRGBBitCount, desc->lpSurface);
#if 0
  desc->dwWidth = 16;
  desc->dwHeight = 16;
  desc->lPitch = desc->dwWidth * 4; // 2 = number of bytes per pixel
#endif



  eax = 0; // FIXME: No idea what this expects to return..
  esp += 5 * 4;
HACKY_COM_END()

// IDirectDrawSurface4 -> STDMETHOD(ReleaseDC)(THIS_ HDC) PURE; // 26
HACKY_COM_BEGIN(IDirectDrawSurface4, 26)
  hacky_printf("p 0x%" PRIX32 "\n", stack[1]);
  hacky_printf("a 0x%" PRIX32 "\n", stack[2]);
  eax = 0; // FIXME: No idea what this expects to return..
  esp += 2 * 4;
HACKY_COM_END()

// IDirectDrawSurface4 -> STDMETHOD(Unlock)(THIS_ LPRECT) PURE; // 32
HACKY_COM_BEGIN(IDirectDrawSurface4, 32)
  hacky_printf("p 0x%" PRIX32 "\n", stack[1]);
  hacky_printf("a 0x%" PRIX32 "\n", stack[2]);

  assert(stack[2] == 0);

  API(DirectDrawSurface4)* this = (API(DirectDrawSurface4)*)Memory(stack[1]);

  API(DDSURFACEDESC2)* desc = &this->desc;

  API(Direct3DTexture2)* texture = (API(Direct3DTexture2)*)Memory(this->texture);

  GLint previousTexture = 0;
  glGetIntegerv(GL_TEXTURE_BINDING_2D, &previousTexture);
  glBindTexture(GL_TEXTURE_2D, texture->handle);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  if (desc->ddpfPixelFormat.dwRGBBitCount == 32) {
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, desc->dwWidth, desc->dwHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, Memory(desc->lpSurface));
  } else {
    if (desc->ddpfPixelFormat.dwRGBAlphaBitMask == 0x8000) {
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, desc->dwWidth, desc->dwHeight, 0, GL_BGRA, GL_UNSIGNED_SHORT_1_5_5_5_REV, Memory(desc->lpSurface));
    } else {
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, desc->dwWidth, desc->dwHeight, 0, GL_BGRA, GL_UNSIGNED_SHORT_4_4_4_4_REV, Memory(desc->lpSurface));
    }
  }
  glBindTexture(GL_TEXTURE_2D, previousTexture);

//Hack: part 2: don't free this to keep data in RAM. see lock for part 1
#if 0
  Free(desc->lpSurface);
  desc->lpSurface = 0;
#endif

  eax = 0; // FIXME: No idea what this expects to return..
  esp += 2 * 4;
HACKY_COM_END()



// IDirectDrawPalette -> STDMETHOD_(ULONG,Release)(THIS) PURE; // 2
HACKY_COM_BEGIN(IDirectDrawPalette, 2)
  hacky_printf("p 0x%" PRIX32 "\n", stack[1]);
  eax = 0; // FIXME: No idea what this expects to return..
  esp += 1 * 4;
HACKY_COM_END()










// IDirect3D3 -> STDMETHOD_(ULONG,Release)(THIS) PURE; // 2
HACKY_COM_BEGIN(IDirect3D3, 2)
  hacky_printf("p 0x%" PRIX32 "\n", stack[1]);
  eax = 0; // FIXME: No idea what this expects to return..
  esp += 1 * 4;
HACKY_COM_END()

// IDirect3D3 -> STDMETHOD(EnumDevices)(THIS_ LPD3DENUMDEVICESCALLBACK,LPVOID) PURE; // 3
HACKY_COM_BEGIN(IDirect3D3, 3)
  hacky_printf("EnumDevices\n");
  uint32_t a = stack[2];
  uint32_t b = stack[3];
  hacky_printf("p 0x%" PRIX32 "\n", stack[1]);
  hacky_printf("a 0x%" PRIX32 "\n", a);
  hacky_printf("b 0x%" PRIX32 "\n", b);
  eax = 0; // FIXME: No idea what this expects to return..
  esp += 3 * 4;
  // Push a call to the callback onto the stack.. this is some ugly hack..

  // Convention not specified -> stdcall?!

  esp -= 4;
  *(uint32_t*)Memory(esp) = returnAddress;

  {
    esp -= 4;
    *(uint32_t*)Memory(esp) = b; // lpContext

    Address desc_addr = Allocate(sizeof(API(D3DDEVICEDESC)));
    API(D3DDEVICEDESC)* desc = (API(D3DDEVICEDESC)*)Memory(desc_addr);
    memset(desc, 0x00, sizeof(API(D3DDEVICEDESC)));
    desc->dwSize = sizeof(API(D3DDEVICEDESC));
    desc->dwFlags = 0xFFFFFFFF;

    desc->dwDeviceZBufferBitDepth = 16;

enum {
  API(D3DPTEXTURECAPS_PERSPECTIVE) =   0x00000001L,
  API(D3DPTEXTURECAPS_ALPHA) =         0x00000004L,
  API(D3DPTEXTURECAPS_TRANSPARENCY) =  0x00000008L
};

    desc->dpcTriCaps.dwTextureCaps = 0;
    desc->dpcTriCaps.dwTextureCaps |= API(D3DPTEXTURECAPS_PERSPECTIVE);
    desc->dpcTriCaps.dwTextureCaps |= API(D3DPTEXTURECAPS_ALPHA);
    desc->dpcTriCaps.dwTextureCaps |= API(D3DPTEXTURECAPS_TRANSPARENCY);

enum {
  API(D3DPSHADECAPS_ALPHAGOURAUDBLEND) = 0x00004000L
};

    desc->dpcTriCaps.dwShadeCaps = 0;
    desc->dpcTriCaps.dwShadeCaps |= API(D3DPSHADECAPS_ALPHAGOURAUDBLEND);

enum {
  API(D3DPTBLENDCAPS_MODULATEALPHA) = 0x00000008L
};

    desc->dpcTriCaps.dwTextureBlendCaps = 0;    
    desc->dpcTriCaps.dwTextureBlendCaps |= API(D3DPTBLENDCAPS_MODULATEALPHA);

    desc->dwMaxVertexCount = 0xFFFF;

    desc->dwMinTextureWidth = 1;
    desc->dwMinTextureHeight = 1;
    desc->dwMaxTextureWidth = 2048;
    desc->dwMaxTextureHeight = 2048;

    esp -= 4;
    *(uint32_t*)Memory(esp) = desc_addr; // LPD3DDEVICEDESC

    esp -= 4;
    *(uint32_t*)Memory(esp) = desc_addr; // LPD3DDEVICEDESC

    esp -= 4;
    Address lpDeviceName = Allocate(200);
    sprintf((char*)Memory(lpDeviceName), "%s", "OpenSWE1R D3D");
    *(uint32_t*)Memory(esp) = lpDeviceName;

    // Used to identifty drawing device
    esp -= 4;
    Address lpDeviceDescription = Allocate(200);
    sprintf((char*)Memory(lpDeviceDescription), "%s", "OpenSWE1R D3D");
    *(uint32_t*)Memory(esp) = lpDeviceDescription;

    // Used as parameter in Direct Draw `Initialize`
    esp -= 4;
    Address guid_addr = Allocate(sizeof(API(IID)));
    API(IID)* guid = (API(IID)*)Memory(guid_addr);
API(IID)* iid = guid;

// IDirect3DHALDevice
iid->Data1 = 0x84E63DE0;
iid->Data2 = 0x46AA;
iid->Data3 = 0x11CF;
iid->Data4[0] = 0x81;
iid->Data4[1] = 0x6F;
iid->Data4[2] = 0x00;
iid->Data4[3] = 0x00;
iid->Data4[4] = 0xC0;
iid->Data4[5] = 0x20;
iid->Data4[6] = 0x15;
iid->Data4[7] = 0x6E;
  printf("\n\n\n\n\n\n\n  (planned iid: {%08" PRIX32 "-%04" PRIX16 "-%04" PRIX16 "-%02" PRIX8 "%02" PRIX8 "-%02" PRIX8 "%02" PRIX8 "%02" PRIX8 "%02" PRIX8 "%02" PRIX8 "%02" PRIX8 "})\n\n\n\n\n\n",
     iid->Data1, iid->Data2, iid->Data3,
     iid->Data4[0], iid->Data4[1], iid->Data4[2], iid->Data4[3],
     iid->Data4[4], iid->Data4[5], iid->Data4[6], iid->Data4[7]);
    *(uint32_t*)Memory(esp) = guid_addr; // lpGUID

    // Emulate the call
    esp -= 4;
    *(uint32_t*)Memory(esp) = clearEax; // Return to clear eax
    eip = a;
    printf("  Callback at 0x%" PRIX32 "\n", eip);
    //FIXME: Add a hook which returns 0
  }
HACKY_COM_END()


// IDirect3D3 -> STDMETHOD(CreateViewport)(THIS_ LPDIRECT3DVIEWPORT3*,LPUNKNOWN) PURE; // 6
HACKY_COM_BEGIN(IDirect3D3, 6)
  hacky_printf("p 0x%" PRIX32 "\n", stack[1]);
  hacky_printf("a 0x%" PRIX32 "\n", stack[2]);
  hacky_printf("b 0x%" PRIX32 "\n", stack[3]);
  *(Address*)Memory(stack[2]) = CreateInterface("IDirect3DViewport3", 200);
  eax = 0; // FIXME: No idea what this expects to return..
  esp += 3 * 4;
HACKY_COM_END()

// IDirect3D3 -> STDMETHOD(CreateDevice)(THIS_ REFCLSID,LPDIRECTDRAWSURFACE4,LPDIRECT3DDEVICE3*,LPUNKNOWN) PURE; // 8
HACKY_COM_BEGIN(IDirect3D3, 8)
  hacky_printf("p 0x%" PRIX32 "\n", stack[1]);
  hacky_printf("a 0x%" PRIX32 "\n", stack[2]);
  hacky_printf("b 0x%" PRIX32 "\n", stack[3]);
  hacky_printf("c 0x%" PRIX32 "\n", stack[4]);
  hacky_printf("d 0x%" PRIX32 "\n", stack[5]);
  *(Address*)Memory(stack[4]) = CreateInterface("IDirect3DDevice3", 200);
  eax = 0; // FIXME: No idea what this expects to return..
  esp += 5 * 4;
HACKY_COM_END()

// IDirect3D3 -> STDMETHOD(EnumZBufferFormats)(THIS_ REFCLSID,LPD3DENUMPIXELFORMATSCALLBACK,LPVOID) PURE; // 10
HACKY_COM_BEGIN(IDirect3D3, 10)
  hacky_printf("EnumZBufferFormats\n");
  uint32_t b = stack[3];
  uint32_t c = stack[4];
  printf("p 0x%" PRIX32 "\n", stack[1]);
  hacky_printf("a 0x%" PRIX32 "\n", stack[2]);
  hacky_printf("b 0x%" PRIX32 "\n", b);
  hacky_printf("c 0x%" PRIX32 "\n", c);
  eax = 0; // FIXME: No idea what this expects to return..
  esp += 4 * 4;
  // Push a call to the callback onto the stack.. this is some ugly hack..

  // Convention not specified -> stdcall?!

  esp -= 4;
  *(uint32_t*)Memory(esp) = returnAddress;

  {
    Address formatAddress = Allocate(sizeof(API(DDPIXELFORMAT)));
    API(DDPIXELFORMAT)* format = (API(DDPIXELFORMAT)*)Memory(formatAddress);
    format->dwSize = sizeof(API(DDPIXELFORMAT));
    format->dwFlags = 0x400; // DDPF_ZBUFFER;
    format->dwZBufferBitDepth = 16;

    esp -= 4;
    *(uint32_t*)Memory(esp) = c; // user pointer
    esp -= 4;
    *(uint32_t*)Memory(esp) = formatAddress; // API(DDPIXELFORMAT)*

    // Emulate the call
    esp -= 4;
    *(uint32_t*)Memory(esp) = clearEax; // Return to clear eax
    eip = b;
    printf("  Callback at 0x%" PRIX32 "\n", eip);
    //FIXME: Add a hook which returns 0
  }
HACKY_COM_END()















// IDirect3DDevice3 -> STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID * ppvObj) PURE; // 0
HACKY_COM_BEGIN(IDirect3DDevice3, 0)
  hacky_printf("p 0x%" PRIX32 "\n", stack[1]);
  hacky_printf("a 0x%" PRIX32 "\n", stack[2]);
  hacky_printf("b 0x%" PRIX32 "\n", stack[3]);
  API(DirectDrawSurface4)* this = (API(DirectDrawSurface4)*)Memory(stack[1]);
  const API(IID)* iid = (const API(IID)*)Memory(stack[2]);
  printf("  (read iid: {%08" PRIX32 "-%04" PRIX16 "-%04" PRIX16 "-%02" PRIX8 "%02" PRIX8 "-%02" PRIX8 "%02" PRIX8 "%02" PRIX8 "%02" PRIX8 "%02" PRIX8 "%02" PRIX8 "})\n",
     iid->Data1, iid->Data2, iid->Data3,
     iid->Data4[0], iid->Data4[1], iid->Data4[2], iid->Data4[3],
     iid->Data4[4], iid->Data4[5], iid->Data4[6], iid->Data4[7]);
  #if 0
  if (iid->Data1 == 0x93281502) { //FIXME: Check for full GUID (Direct3DTexture2)
  printf("Returning texture 0x%" PRIX32 "\n", this->texture);
  *(Address*)Memory(stack[3]) = this->texture;
  } else {
  assert(false);
  }
  #endif
  assert(false);
  eax = 0; // FIXME: No idea what this expects to return..
  esp += 3 * 4;
HACKY_COM_END()

// IDirect3DDevice3 -> STDMETHOD_(ULONG,Release)       (THIS) PURE; //2
HACKY_COM_BEGIN(IDirect3DDevice3, 2)
  hacky_printf("p 0x%" PRIX32 "\n", stack[1]);
  eax = 0; // FIXME: No idea what this expects to return..
  esp += 1 * 4;
HACKY_COM_END()

// IDirect3DDevice3 -> STDMETHOD(GetCaps)(THIS_ LPD3DDEVICEDESC,LPD3DDEVICEDESC) PURE; // 3
HACKY_COM_BEGIN(IDirect3DDevice3, 3)
  hacky_printf("p 0x%" PRIX32 "\n", stack[1]);
  hacky_printf("a 0x%" PRIX32 "\n", stack[2]);
  hacky_printf("b 0x%" PRIX32 "\n", stack[3]);

  API(D3DDEVICEDESC)* desc = (API(D3DDEVICEDESC)*)Memory(stack[2]);
  uint32_t size = desc->dwSize;
  memset(desc, 0x00, size);
  desc->dwSize = size;
  desc->dpcTriCaps.dwTextureBlendCaps = 0;
  desc->dpcTriCaps.dwTextureBlendCaps |= API(D3DPTBLENDCAPS_MODULATEALPHA);

  eax = 0; // FIXME: No idea what this expects to return..
  esp += 3 * 4;
HACKY_COM_END()

// IDirect3DDevice3 -> STDMETHOD(AddViewport)(THIS_ LPDIRECT3DVIEWPORT3) PURE; // 5
HACKY_COM_BEGIN(IDirect3DDevice3, 5)
  hacky_printf("p 0x%" PRIX32 "\n", stack[1]);
  hacky_printf("a 0x%" PRIX32 "\n", stack[2]);
  eax = 0; // FIXME: No idea what this expects to return..
  esp += 2 * 4;
HACKY_COM_END()

// IDirect3DDevice3 -> STDMETHOD(EnumTextureFormats)(THIS_ LPD3DENUMPIXELFORMATSCALLBACK,LPVOID) PURE; // 8
HACKY_COM_BEGIN(IDirect3DDevice3, 8)
  hacky_printf("EnumTextureFormats\n");
  uint32_t a = stack[2];
  uint32_t b = stack[3];
  hacky_printf("p 0x%" PRIX32 "\n", stack[1]);
  hacky_printf("a 0x%" PRIX32 "\n", a);
  hacky_printf("b 0x%" PRIX32 "\n", b);
  eax = 0; // FIXME: No idea what this expects to return..
  esp += 3 * 4;
  // Push a call to the callback onto the stack.. this is some ugly hack..

  // Convention not specified -> stdcall?!

  esp -= 4;
  *(uint32_t*)Memory(esp) = returnAddress;

  {
    {
      Address formatAddress = Allocate(sizeof(API(DDPIXELFORMAT)));
      API(DDPIXELFORMAT)* format = (API(DDPIXELFORMAT)*)Memory(formatAddress);
      memset(format, 0x00, sizeof(API(DDPIXELFORMAT)));
      format->dwSize = sizeof(API(DDPIXELFORMAT));
      format->dwFlags = API(DDPF_RGB) | API(DDPF_ALPHAPIXELS);
      format->dwRGBBitCount = 16;
      format->dwRBitMask = 0x0F00;
      format->dwGBitMask = 0x00F0;
      format->dwBBitMask = 0x000F;
      format->dwRGBAlphaBitMask = 0xF000;

      esp -= 4;
      *(uint32_t*)Memory(esp) = b; // user pointer
      esp -= 4;
      *(uint32_t*)Memory(esp) = formatAddress; // API(DDPIXELFORMAT)*

      // Emulate a call by setting return address to where we want to go.
      esp -= 4;
      *(uint32_t*)Memory(esp) = clearEax; // Return to clear eax
    }
    {
      Address formatAddress = Allocate(sizeof(API(DDPIXELFORMAT)));
      API(DDPIXELFORMAT)* format = (API(DDPIXELFORMAT)*)Memory(formatAddress);
      memset(format, 0x00, sizeof(API(DDPIXELFORMAT)));
      format->dwSize = sizeof(API(DDPIXELFORMAT));
      format->dwFlags = API(DDPF_RGB) | API(DDPF_ALPHAPIXELS);
      format->dwRGBBitCount = 16;
      format->dwRBitMask = 0x7C00;
      format->dwGBitMask = 0x03E0;
      format->dwBBitMask = 0x001F;
      format->dwRGBAlphaBitMask = 0x8000;

      esp -= 4;
      *(uint32_t*)Memory(esp) = b; // user pointer
      esp -= 4;
      *(uint32_t*)Memory(esp) = formatAddress; // API(DDPIXELFORMAT)*

      // Emulate a call by setting return address to the callback.
      esp -= 4;
      *(uint32_t*)Memory(esp) = a; // Continue with next format
    }
#if 1
    {
      Address formatAddress = Allocate(sizeof(API(DDPIXELFORMAT)));
      API(DDPIXELFORMAT)* format = (API(DDPIXELFORMAT)*)Memory(formatAddress);
      memset(format, 0x00, sizeof(API(DDPIXELFORMAT)));
      format->dwSize = sizeof(API(DDPIXELFORMAT));
      format->dwFlags = API(DDPF_RGB) | API(DDPF_ALPHAPIXELS);
      format->dwRGBBitCount = 32;
      format->dwRBitMask = 0x00FF0000;
      format->dwGBitMask = 0x0000FF00;
      format->dwBBitMask = 0x000000FF;
      format->dwRGBAlphaBitMask = 0xFF000000;

      esp -= 4;
      *(uint32_t*)Memory(esp) = b; // user pointer
      esp -= 4;
      *(uint32_t*)Memory(esp) = formatAddress; // API(DDPIXELFORMAT)*

      // Emulate the call. We are calling the callback.
      // We also set the return address to the callback.
      esp -= 4;
      *(uint32_t*)Memory(esp) = a; // Continue with next format
      eip = a;
    }
#endif


    printf("  Callback at 0x%" PRIX32 "\n", eip);
    //FIXME: Add a hook which returns 0
  }
HACKY_COM_END()

// IDirect3DDevice3 -> STDMETHOD(BeginScene)(THIS) PURE; // 9
HACKY_COM_BEGIN(IDirect3DDevice3, 9)
  hacky_printf("p 0x%" PRIX32 "\n", stack[1]);
  eax = 0; // FIXME: No idea what this expects to return..
  esp += 1 * 4;
HACKY_COM_END()

// IDirect3DDevice3 -> STDMETHOD(EndScene)(THIS) PURE; // 10
HACKY_COM_BEGIN(IDirect3DDevice3, 10)
  hacky_printf("p 0x%" PRIX32 "\n", stack[1]);
  eax = 0; // FIXME: No idea what this expects to return..
  esp += 1 * 4;
HACKY_COM_END()

// IDirect3DDevice3 -> STDMETHOD(SetCurrentViewport)(THIS_ LPDIRECT3DVIEWPORT3) PURE; // 12
HACKY_COM_BEGIN(IDirect3DDevice3, 12)
  hacky_printf("p 0x%" PRIX32 "\n", stack[1]);
  hacky_printf("a 0x%" PRIX32 "\n", stack[2]);
  eax = 0; // FIXME: No idea what this expects to return..
  esp += 2 * 4;
HACKY_COM_END()

static void glSet(GLenum state, bool set) {
  if (set) {
    glEnable(state);
  } else {
    glDisable(state);
  }
}

GLenum mapBlend(API(D3DBLEND) blend) {
  switch(blend) {
  case API(D3DBLEND_SRCALPHA):
    return GL_SRC_ALPHA;
  case API(D3DBLEND_INVSRCALPHA):
    return GL_ONE_MINUS_SRC_ALPHA;
  default:
    assert(false);
    return GL_ZERO;
  }
}

// IDirect3DDevice3 -> STDMETHOD(SetRenderState)(THIS_ D3DRENDERSTATETYPE,DWORD) PURE; // 22
HACKY_COM_BEGIN(IDirect3DDevice3, 22)
  silent = false;
  if (!silent) {
    hacky_printf("p 0x%" PRIX32 "\n", stack[1]);
    hacky_printf("a 0x%" PRIX32 "\n", stack[2]);
    hacky_printf("b 0x%" PRIX32 "\n", stack[3]);
  }
  uint32_t a = stack[2];
  uint32_t b = stack[3];
  switch(a) {
    case API(D3DRENDERSTATE_TEXTUREHANDLE):
      assert(b == 0); // Texture handle for legacy interfaces (Texture,Texture2)
      // FIXME
      break;

    case API(D3DRENDERSTATE_ANTIALIAS):
      assert(b == API(D3DANTIALIAS_NONE)); // D3DANTIALIASMODE
      // FIXME
      break;

    case API(D3DRENDERSTATE_TEXTUREPERSPECTIVE):
      assert(b == 1); // TRUE for perspective correction
      // FIXME
      break;

    case API(D3DRENDERSTATE_ZENABLE):
      assert(b == 1); // D3DZBUFFERTYPE (or TRUE/FALSE for legacy)
      glSet(GL_DEPTH_TEST, b);
      break;

    case API(D3DRENDERSTATE_FILLMODE):
      assert(b == API(D3DFILL_SOLID)); // D3DFILLMODE
      // FIXME
      break;

    case API(D3DRENDERSTATE_SHADEMODE):
      assert(b == API(D3DSHADE_GOURAUD)); // D3DSHADEMODE
      // FIXME
      break;

    case API(D3DRENDERSTATE_MONOENABLE):
      assert(b == 0); // TRUE to enable mono rasterization
      // FIXME
      break;

    case API(D3DRENDERSTATE_ZWRITEENABLE):
      assert((b == 0) || (b == 1)); // TRUE to enable z writes
      depthMask = b;
      break;

    case API(D3DRENDERSTATE_ALPHATESTENABLE):
      assert(b == 1); // TRUE to enable alpha tests
      alphaTest = b;
      break;

    case API(D3DRENDERSTATE_TEXTUREMIN):
      assert(b == API(D3DFILTER_MIPNEAREST)); // D3DTEXTUREFILTER
      // FIXME
      break;

    case API(D3DRENDERSTATE_SRCBLEND):
      assert(b == API(D3DBLEND_SRCALPHA)); // D3DBLEND
      srcBlend = mapBlend(b);
      break;

    case API(D3DRENDERSTATE_DESTBLEND):
      assert(b == API(D3DBLEND_INVSRCALPHA)); // D3DBLEND
      destBlend = mapBlend(b);
      break;

    case API(D3DRENDERSTATE_TEXTUREMAPBLEND):
      assert((b == API(D3DTBLEND_MODULATE)) || (b == API(D3DTBLEND_MODULATEALPHA))); // D3DTEXTUREBLEND
      // FIXME
      break;

    case API(D3DRENDERSTATE_CULLMODE):
      assert(b == API(D3DCULL_NONE)); // D3DCULL
      // FIXME
      break;

    case API(D3DRENDERSTATE_ZFUNC):
      assert(b == API(D3DCMP_LESSEQUAL)); // D3DCMPFUNC
      glDepthFunc(GL_LEQUAL);
      break;

    case API(D3DRENDERSTATE_ALPHAFUNC):
      assert(b == API(D3DCMP_NOTEQUAL)); // D3DCMPFUNC
      // FIXME
      break;

    case API(D3DRENDERSTATE_DITHERENABLE):
      assert(b == 1); // TRUE to enable dithering
      glSet(GL_DITHER, b);
      break;

    case API(D3DRENDERSTATE_ALPHABLENDENABLE):
      assert((b == 0) || (b == 1)); // TRUE to enable alpha blending
      glSet(GL_BLEND, b);
      break;

    case API(D3DRENDERSTATE_FOGENABLE):
      assert((b == 0) || (b == 1)); // TRUE to enable fog blending
      fogEnable = b;
      break;

    case API(D3DRENDERSTATE_SPECULARENABLE):
      assert(b == 0); // TRUE to enable specular
      // FIXME
      break;

    case API(D3DRENDERSTATE_SUBPIXEL):
      assert(b == 1); // TRUE to enable subpixel correction
      // FIXME
      break;

    case API(D3DRENDERSTATE_STIPPLEDALPHA):
      assert((b == 0) || (b == 1)); // TRUE to enable stippled alpha (RGB device only)
      // FIXME
      break;

    case API(D3DRENDERSTATE_FOGCOLOR):
      // D3DCOLOR
      fogColor = b;
      break;

    case API(D3DRENDERSTATE_FOGTABLEMODE):
      assert((b == API(D3DFOG_NONE)) || (b == API(D3DFOG_LINEAR))); // D3DFOGMODE
      // FIXME
      break;

    case API(D3DRENDERSTATE_FOGTABLESTART):
      // Fog start (for both vertex and pixel fog)
      fogStart = *(float*)&b;
      break;

    case API(D3DRENDERSTATE_FOGTABLEEND):
      // Fog end
      fogEnd = *(float*)&b;
      break;

    case API(D3DRENDERSTATE_COLORKEYENABLE):
      assert(b == 1); // TRUE to enable source colorkeyed textures
      // FIXME
      break;

    default:
      printf("Unknown render-state %d set to 0x%08" PRIX32 " (%f)\n", a, b, *(float*)&b);
      assert(false);
      break;
  }
  eax = 0; // FIXME: No idea what this expects to return..
  esp += 3 * 4;
HACKY_COM_END()

// IDirect3DDevice3 -> STDMETHOD(SetTransform)(THIS_ D3DTRANSFORMSTATETYPE,LPD3DMATRIX) PURE; // 25
HACKY_COM_BEGIN(IDirect3DDevice3, 25)
  hacky_printf("p 0x%" PRIX32 "\n", stack[1]);
  hacky_printf("a 0x%" PRIX32 "\n", stack[2]);
  hacky_printf("b 0x%" PRIX32 "\n", stack[3]);
  uint32_t a = stack[2];
  float* m = Memory(stack[3]);
  switch(a) {
    case 3: // Projection
      memcpy(projectionMatrix, m, 4 * 4 * sizeof(float));
      break;
    default:
      printf("Unknown matrix %d\n", a);
      //FIXME: assert(false) once this runs faster
      break;
  }
  printf("Matrix %d:\n", a);
  printf("  %f\t%f\t%f\t%f\n", m[ 0], m[ 1], m[ 2], m[ 3]);
  printf("  %f\t%f\t%f\t%f\n", m[ 4], m[ 5], m[ 6], m[ 7]);
  printf("  %f\t%f\t%f\t%f\n", m[ 8], m[ 9], m[10], m[11]);
  printf("  %f\t%f\t%f\t%f\n", m[12], m[13], m[14], m[15]);

  eax = 0; // FIXME: No idea what this expects to return..
  esp += 3 * 4;
HACKY_COM_END()


// IDirect3DDevice3 -> STDMETHOD(DrawPrimitive)(THIS_ D3DPRIMITIVETYPE,DWORD,LPVOID,DWORD,DWORD) PURE; // 28
HACKY_COM_BEGIN(IDirect3DDevice3, 28)
  hacky_printf("p 0x%" PRIX32 "\n", stack[1]);
  hacky_printf("a (primitive type) 0x%" PRIX32 "\n", stack[2]);
  hacky_printf("b (vertex format = fvf) 0x%" PRIX32 "\n", stack[3]);
  hacky_printf("c (buffer) 0x%" PRIX32 "\n", stack[4]);
  hacky_printf("d (vertex-count) 0x%" PRIX32 "\n", stack[5]);
  hacky_printf("e (flags) 0x%" PRIX32 "\n", stack[6]);

  PrintVertices(stack[3], stack[4], stack[5]);
  LoadVertices(stack[3], stack[4], stack[5]);
  GLenum mode = SetupRenderer(stack[2], stack[3]);
  glDrawArrays(mode, 0, stack[5]);

  eax = 0; // FIXME: No idea what this expects to return..
  esp += 6 * 4;
HACKY_COM_END()

// IDirect3DDevice3 -> STDMETHOD(DrawIndexedPrimitive)(THIS_ D3DPRIMITIVETYPE,DWORD,LPVOID,DWORD,LPWORD,DWORD,DWORD) PURE; // 29
HACKY_COM_BEGIN(IDirect3DDevice3, 29)
  hacky_printf("p 0x%" PRIX32 "\n", stack[1]);
  hacky_printf("d3dptPrimitiveType 0x%" PRIX32 "\n", stack[2]);
  hacky_printf("dwVertexTypeDesc 0x%" PRIX32 "\n", stack[3]);
  hacky_printf("lpvVertices 0x%" PRIX32 "\n", stack[4]);
  hacky_printf("dwVertexCount 0x%" PRIX32 "\n", stack[5]);
  hacky_printf("lpwIndices 0x%" PRIX32 "\n", stack[6]);
  hacky_printf("dwIndexCount 0x%" PRIX32 "\n", stack[7]);
  hacky_printf("dwFlags 0x%" PRIX32 "\n", stack[8]);

  LoadIndices(stack[6], stack[7]);
  LoadVertices(stack[3], stack[4], stack[5]);
  GLenum mode = SetupRenderer(stack[2], stack[3]);
  glDrawElements(mode, stack[7], GL_UNSIGNED_SHORT, NULL);

  eax = 0; // FIXME: No idea what this expects to return..
  esp += 8 * 4;
HACKY_COM_END()

// IDirect3DDevice3 -> STDMETHOD(SetTexture)(THIS_ DWORD,LPDIRECT3DTEXTURE2) PURE; // 38
HACKY_COM_BEGIN(IDirect3DDevice3, 38)
  uint32_t a = stack[2];
  uint32_t b = stack[3];
  hacky_printf("p 0x%" PRIX32 "\n", stack[1]);
  hacky_printf("a 0x%" PRIX32 "\n", a);
  hacky_printf("b 0x%" PRIX32 "\n", b);

  if (b != 0) {
    API(Direct3DTexture2)* texture = Memory(b);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture->handle);
  } else {
    glBindTexture(GL_TEXTURE_2D, 0); // FIXME: I believe this is supposed to be white?!
  }

  eax = 0; // FIXME: No idea what this expects to return..
  esp += 3 * 4;
HACKY_COM_END()

// IDirect3DDevice3 -> STDMETHOD(SetTextureStageState)(THIS_ DWORD,D3DTEXTURESTAGESTATETYPE,DWORD) PURE; // 40
HACKY_COM_BEGIN(IDirect3DDevice3, 40)

  uint32_t a = stack[2];
  uint32_t b = stack[3];
  uint32_t c = stack[4];

  hacky_printf("p 0x%" PRIX32 "\n", stack[1]);
  hacky_printf("a 0x%" PRIX32 "\n", a);
  hacky_printf("b 0x%" PRIX32 "\n", b);
  hacky_printf("c 0x%" PRIX32 "\n", c);

  assert(a == 0);

  switch(b) {

  case API(D3DTSS_ADDRESSU):
    assert((c == API(D3DTADDRESS_WRAP)) || (c == API(D3DTADDRESS_CLAMP))); // D3DTEXTUREADDRESS
    //FIXME
    break;
  case API(D3DTSS_ADDRESSV):
    assert((c == API(D3DTADDRESS_WRAP)) || (c == API(D3DTADDRESS_CLAMP))); // D3DTEXTUREADDRESS
    //FIXME
    break;
  case API(D3DTSS_MAGFILTER): // D3DTEXTUREMAGFILTER
    assert(c == API(D3DTFG_LINEAR));
    //FIXME
    break;
  case API(D3DTSS_MINFILTER): // D3DTEXTUREMINFILTER
    assert(c == API(D3DTFN_LINEAR));
    //FIXME
    break;
  case API(D3DTSS_MIPFILTER): // D3DTEXTUREMIPFILTER
    assert(c == API(D3DTFP_NONE));
    //FIXME
    break;
  default:
    assert(false);
    break;
  }

  eax = 0; // FIXME: No idea what this expects to return..
  esp += 4 * 4;
HACKY_COM_END()










// IDirect3DTexture2

// IDirect3DTexture2 -> STDMETHOD(QueryInterface)				(THIS_ REFIID, LPVOID FAR *) PURE; // 0
HACKY_COM_BEGIN(IDirect3DTexture2, 0)
  hacky_printf("QueryInterface\n");
  hacky_printf("p 0x%" PRIX32 "\n", stack[1]);
  hacky_printf("a 0x%" PRIX32 "\n", stack[2]);
  hacky_printf("b 0x%" PRIX32 "\n", stack[3]);

  const API(IID)* iid = (const API(IID)*)Memory(stack[2]);

  char iidString[1024];
  sprintf(iidString, "%08" PRIX32 "-%04" PRIX16 "-%04" PRIX16 "-%02" PRIX8 "%02" PRIX8 "-%02" PRIX8 "%02" PRIX8 "%02" PRIX8 "%02" PRIX8 "%02" PRIX8 "%02" PRIX8,
          iid->Data1, iid->Data2, iid->Data3,
          iid->Data4[0], iid->Data4[1], iid->Data4[2], iid->Data4[3],
          iid->Data4[4], iid->Data4[5], iid->Data4[6], iid->Data4[7]);
  printf("  (read iid: {%s})\n", iidString);

  char name[32];
  //FIXME: Add more classed / interfaces

  if (!strcmp(iidString, "0B2B8630-AD35-11D0-8EA6-00609797EA5B")) {
    API(Direct3DTexture2)* this = Memory(stack[1]);
    *(Address*)Memory(stack[3]) = this->surface;
  } else {
    assert(false);
  }
 
  eax = 0;
  esp += 3 * 4;
HACKY_COM_END()


// IDirect3DTexture2 -> STDMETHOD_(ULONG,Release)       (THIS) PURE; //2
HACKY_COM_BEGIN(IDirect3DTexture2, 2)
  hacky_printf("p 0x%" PRIX32 "\n", stack[1]);
  eax = 0; // FIXME: No idea what this expects to return..
  esp += 1 * 4;
HACKY_COM_END()

// IDirect3DTexture2 -> STDMETHOD(GetHandle)(THIS_ LPDIRECT3DDEVICE2,LPD3DTEXTUREHANDLE) PURE; // 3
HACKY_COM_BEGIN(IDirect3DTexture2, 3)
  hacky_printf("p 0x%" PRIX32 "\n", stack[1]);
  hacky_printf("a 0x%" PRIX32 "\n", stack[2]);
  hacky_printf("b 0x%" PRIX32 "\n", stack[3]);
  *(uint32_t*)Memory(stack[3]) = 1248;
  eax = 0; // FIXME: No idea what this expects to return..
  esp += 3 * 4;
HACKY_COM_END()

// IDirect3DTexture2 -> STDMETHOD(Load)(THIS_ LPDIRECT3DTEXTURE2) PURE; // 5
HACKY_COM_BEGIN(IDirect3DTexture2, 5)
  hacky_printf("p 0x%" PRIX32 "\n", stack[1]);
  hacky_printf("a 0x%" PRIX32 "\n", stack[2]);

  API(Direct3DTexture2)* this = Memory(stack[1]);
  API(Direct3DTexture2)* a = Memory(stack[2]);
  //FIXME: Dirty hack..
  this->handle = a->handle;
  eax = 0; // FIXME: No idea what this expects to return..
  esp += 2 * 4;
HACKY_COM_END()






// IDirect3DViewport3 

// IDirect3DViewport3 -> STDMETHOD_(ULONG,Release)       (THIS) PURE; //2
HACKY_COM_BEGIN(IDirect3DViewport3, 2)
  hacky_printf("p 0x%" PRIX32 "\n", stack[1]);
  eax = 0; // FIXME: No idea what this expects to return..
  esp += 1 * 4;
HACKY_COM_END()

// IDirect3DViewport3 -> STDMETHOD(SetViewport2)(THIS_ LPD3DVIEWPORT2) PURE; // 17
HACKY_COM_BEGIN(IDirect3DViewport3, 17)
  hacky_printf("p 0x%" PRIX32 "\n", stack[1]);
  hacky_printf("a 0x%" PRIX32 "\n", stack[2]);
  API(D3DVIEWPORT2)* vp = (API(D3DVIEWPORT2)*)Memory(stack[2]);
  assert(vp->dwSize == sizeof(API(D3DVIEWPORT2)));

  clipScale[0] = 2.0f / vp->dvClipWidth;
  clipScale[1] = 2.0f / vp->dvClipHeight;
  clipScale[2] = 2.0f / (vp->dvMaxZ - vp->dvMinZ);
  clipOffset[0] = -vp->dvClipX * clipScale[0] - 1.0f;
  clipOffset[1] = -vp->dvClipY * clipScale[1] - 1.0f;
  clipOffset[2] = -vp->dvMinZ * clipScale[2] - 1.0f;
  glViewport(vp->dwX, vp->dwY, vp->dwWidth, vp->dwHeight);

  eax = 0; // FIXME: No idea what this expects to return..
  esp += 2 * 4;
HACKY_COM_END()

// IDirect3DViewport3 -> STDMETHOD(Clear2)(THIS_ DWORD,LPD3DRECT,DWORD,D3DCOLOR,D3DVALUE,DWORD) PURE; // 20
HACKY_COM_BEGIN(IDirect3DViewport3, 20)
  hacky_printf("p 0x%" PRIX32 "\n", stack[1]);
  hacky_printf("a 0x%" PRIX32 "\n", stack[2]);
  hacky_printf("b 0x%" PRIX32 "\n", stack[3]);
  hacky_printf("c 0x%" PRIX32 "\n", stack[4]);
  hacky_printf("d 0x%" PRIX32 "\n", stack[5]);
  hacky_printf("e 0x%" PRIX32 "\n", stack[6]);
  hacky_printf("f 0x%" PRIX32 "\n", stack[7]);

  unsigned int rectCount = stack[2];
  API(D3DRECT)* rects = Memory(stack[3]);

  glEnable(GL_SCISSOR_TEST);
  GLint viewport[4];
  glGetIntegerv(GL_VIEWPORT, viewport);
  for(unsigned int i = 0; i < rectCount; i++) {
    API(D3DRECT)* rect = &rects[i];
    //FIXME: Clip to viewport..
    int width = rect->x2 - rect->x1;
    int height = rect->y2 -  rect->y1;
    glScissor(rect->x1, viewport[3] - rect->y2, width, height);

    unsigned int flags = stack[4];
    uint32_t clearColor = stack[5];
    float zValue = *(float*)&stack[6];
    uint32_t stencilValue = stack[7];

    float a = (clearColor >> 24) / 255.0f;
    float r = ((clearColor >> 24) & 0xFF) / 255.0f;
    float g = ((clearColor >> 16) & 0xFF) / 255.0f;
    float b = (clearColor & 0xFF) / 255.0f;

    glClearStencil(stencilValue);
    glClearDepth(zValue);
    glClearColor(r, g, b, a);
    glClear(((flags & API(D3DCLEAR_TARGET)) ? GL_COLOR_BUFFER_BIT : 0) |
            ((flags & API(D3DCLEAR_ZBUFFER)) ? GL_DEPTH_BUFFER_BIT : 0) |
            ((flags & API(D3DCLEAR_STENCIL)) ? GL_STENCIL_BUFFER_BIT : 0));
  }
  glDisable(GL_SCISSOR_TEST);

  eax = 0; // FIXME: No idea what this expects to return..
  esp += 7 * 4;
HACKY_COM_END()








// IDirectInputDeviceA

// IDirectInputDeviceA -> STDMETHOD_(ULONG,Release)       (THIS) PURE; //2
HACKY_COM_BEGIN(IDirectInputDeviceA, 2)
  hacky_printf("p 0x%" PRIX32 "\n", stack[1]);
  eax = 0; // FIXME: No idea what this expects to return..
  esp += 1 * 4;
HACKY_COM_END()

// IDirectInputDeviceA -> STDMETHOD(GetCapabilities)(THIS_ LPDIDEVCAPS) PURE; // 3
HACKY_COM_BEGIN(IDirectInputDeviceA, 3)
  hacky_printf("p 0x%" PRIX32 "\n", stack[1]);
  hacky_printf("a 0x%" PRIX32 "\n", stack[2]);

  //FIXME!

  eax = 0; // FIXME: No idea what this expects to return..
  esp += 2 * 4;
HACKY_COM_END()

// IDirectInputDeviceA -> STDMETHOD(SetProperty)(THIS_ REFGUID,LPCDIPROPHEADER) PURE; // 6
HACKY_COM_BEGIN(IDirectInputDeviceA, 6)
  hacky_printf("p 0x%" PRIX32 "\n", stack[1]);
  hacky_printf("a 0x%" PRIX32 "\n", stack[2]);
  hacky_printf("b 0x%" PRIX32 "\n", stack[3]);

  //FIXME!

  eax = 0; // FIXME: No idea what this expects to return..
  esp += 3 * 4;
HACKY_COM_END()

// IDirectInputDeviceA -> STDMETHOD(Acquire)(THIS) PURE; // 7
HACKY_COM_BEGIN(IDirectInputDeviceA, 7)
  hacky_printf("p 0x%" PRIX32 "\n", stack[1]);
  eax = 0; // FIXME: No idea what this expects to return..
  esp += 1 * 4;
HACKY_COM_END()

// IDirectInputDeviceA -> STDMETHOD(Unacquire)(THIS) PURE; // 8
HACKY_COM_BEGIN(IDirectInputDeviceA, 8)
  hacky_printf("p 0x%" PRIX32 "\n", stack[1]);
  eax = 0; // HRESULT -> non-negative means success
  esp += 1 * 4;
HACKY_COM_END()

uint8_t keyboardState[256];
void UpdateKeyboardState() {
  const Uint8 *sdlState = SDL_GetKeyboardState(NULL);
  const uint8_t pressed = 0x80; // This is the only requirement for pressed keys
  const uint8_t unpressed = 0x00;
  memset(keyboardState, 0x00, 256);
  keyboardState[API(DIK_ESCAPE)] = sdlState[SDL_SCANCODE_ESCAPE] ? pressed : unpressed;
  keyboardState[API(DIK_RETURN)] = sdlState[SDL_SCANCODE_RETURN] ? pressed : unpressed;
  keyboardState[API(DIK_SPACE)] = sdlState[SDL_SCANCODE_SPACE] ? pressed : unpressed;
  keyboardState[API(DIK_UP)] = sdlState[SDL_SCANCODE_UP] ? pressed : unpressed;
  keyboardState[API(DIK_DOWN)] = sdlState[SDL_SCANCODE_DOWN] ? pressed : unpressed;
  keyboardState[API(DIK_LEFT)] = sdlState[SDL_SCANCODE_LEFT] ? pressed : unpressed;
  keyboardState[API(DIK_RIGHT)] = sdlState[SDL_SCANCODE_RIGHT] ? pressed : unpressed;
  keyboardState[API(DIK_Q)] = sdlState[SDL_SCANCODE_Q] ? pressed : unpressed;
  keyboardState[API(DIK_W)] = sdlState[SDL_SCANCODE_W] ? pressed : unpressed;
  keyboardState[API(DIK_E)] = sdlState[SDL_SCANCODE_E] ? pressed : unpressed;
  keyboardState[API(DIK_R)] = sdlState[SDL_SCANCODE_R] ? pressed : unpressed;
  keyboardState[API(DIK_I)] = sdlState[SDL_SCANCODE_I] ? pressed : unpressed;
  keyboardState[API(DIK_A)] = sdlState[SDL_SCANCODE_A] ? pressed : unpressed;
  keyboardState[API(DIK_S)] = sdlState[SDL_SCANCODE_S] ? pressed : unpressed;
  keyboardState[API(DIK_D)] = sdlState[SDL_SCANCODE_D] ? pressed : unpressed;
  keyboardState[API(DIK_F)] = sdlState[SDL_SCANCODE_F] ? pressed : unpressed;
  keyboardState[API(DIK_J)] = sdlState[SDL_SCANCODE_J] ? pressed : unpressed;
  keyboardState[API(DIK_K)] = sdlState[SDL_SCANCODE_K] ? pressed : unpressed;
  keyboardState[API(DIK_L)] = sdlState[SDL_SCANCODE_L] ? pressed : unpressed;
  keyboardState[API(DIK_M)] = sdlState[SDL_SCANCODE_M] ? pressed : unpressed;
  keyboardState[API(DIK_F1)] = sdlState[SDL_SCANCODE_F1] ? pressed : unpressed;
  keyboardState[API(DIK_F2)] = sdlState[SDL_SCANCODE_F2] ? pressed : unpressed;
  keyboardState[API(DIK_F3)] = sdlState[SDL_SCANCODE_F3] ? pressed : unpressed;
  keyboardState[API(DIK_F4)] = sdlState[SDL_SCANCODE_F4] ? pressed : unpressed;
  keyboardState[API(DIK_F5)] = sdlState[SDL_SCANCODE_F5] ? pressed : unpressed;
  keyboardState[API(DIK_F6)] = sdlState[SDL_SCANCODE_F6] ? pressed : unpressed;
  keyboardState[API(DIK_F7)] = sdlState[SDL_SCANCODE_F7] ? pressed : unpressed;
  keyboardState[API(DIK_F12)] = sdlState[SDL_SCANCODE_F12] ? pressed : unpressed;
  keyboardState[API(DIK_GRAVE)] = sdlState[SDL_SCANCODE_GRAVE] ? pressed : unpressed;
  keyboardState[API(DIK_EQUALS )] = sdlState[SDL_SCANCODE_EQUALS] ? pressed : unpressed;
  keyboardState[API(DIK_MINUS)] = sdlState[SDL_SCANCODE_MINUS] ? pressed : unpressed;
  keyboardState[API(DIK_TAB)] = sdlState[SDL_SCANCODE_TAB] ? pressed : unpressed;
  keyboardState[API(DIK_CAPSLOCK)] = sdlState[SDL_SCANCODE_CAPSLOCK] ? pressed : unpressed;
  keyboardState[API(DIK_LSHIFT)] = sdlState[SDL_SCANCODE_LSHIFT] ? pressed : unpressed;
  keyboardState[API(DIK_RSHIFT)] = sdlState[SDL_SCANCODE_RSHIFT] ? pressed : unpressed;
  keyboardState[API(DIK_LCONTROL)] = sdlState[SDL_SCANCODE_LCTRL] ? pressed : unpressed;
  keyboardState[API(DIK_RCONTROL)] = sdlState[SDL_SCANCODE_RCTRL] ? pressed : unpressed;
  keyboardState[API(DIK_LALT)] = sdlState[SDL_SCANCODE_LALT] ? pressed : unpressed;
  keyboardState[API(DIK_RALT)] = sdlState[SDL_SCANCODE_RALT] ? pressed : unpressed;
}

// IDirectInputDeviceA -> STDMETHOD(GetDeviceState)(THIS_ DWORD,LPVOID) PURE; // 9
HACKY_COM_BEGIN(IDirectInputDeviceA, 9)
  hacky_printf("p 0x%" PRIX32 "\n", stack[1]);
  hacky_printf("a 0x%" PRIX32 "\n", stack[2]);
  hacky_printf("b 0x%" PRIX32 "\n", stack[3]);
  UpdateKeyboardState();
  memcpy(Memory(stack[3]), keyboardState, stack[2]);
  eax = 0; // FIXME: No idea what this expects to return..
  esp += 3 * 4;
HACKY_COM_END()

// IDirectInputDeviceA -> STDMETHOD(GetDeviceData)(THIS_ DWORD,LPDIDEVICEOBJECTDATA,LPDWORD,DWORD) PURE; // 10
HACKY_COM_BEGIN(IDirectInputDeviceA, 10)
  hacky_printf("p 0x%" PRIX32 "\n", stack[1]);
  hacky_printf("a 0x%" PRIX32 "\n", stack[2]);
  hacky_printf("b 0x%" PRIX32 "\n", stack[3]);
  hacky_printf("c 0x%" PRIX32 "\n", stack[4]);
  hacky_printf("d 0x%" PRIX32 "\n", stack[5]);

  // Don't allow PEEK flag
  assert(stack[5] == 0);

  // Diff the keyboard input between calls
  static uint8_t previousState[256] = {0};
  assert(sizeof(previousState) == sizeof(keyboardState));
  UpdateKeyboardState();
  uint32_t* count = (uint32_t*)Memory(stack[4]);
  unsigned int max_count = *count;
  printf("max count is %d\n", max_count);
  *count = 0;
  unsigned int objectSize = stack[2];
  assert(objectSize == sizeof(API(DIDEVICEOBJECTDATA)));
  for(unsigned int i = 0; i < 256; i++) {
    if (keyboardState[i] != previousState[i]) {
      if (*count < max_count) {
        API(DIDEVICEOBJECTDATA) objectData;
        memset(&objectData, 0x00, sizeof(objectData));
        objectData.dwOfs = i;
        objectData.dwData = keyboardState[i];
        printf("Adding %d: %d\n", objectData.dwOfs, objectData.dwData);
        memcpy(Memory(stack[3] + *count * objectSize), &objectData, objectSize);
        *count = *count + 1;
      }
    }
  }
  memcpy(previousState, keyboardState, sizeof(keyboardState));
  printf("returning %d entries\n", *count);

  eax = 0; // FIXME: No idea what this expects to return..
  esp += 5 * 4;
HACKY_COM_END()

// IDirectInputDeviceA -> STDMETHOD(SetDataFormat)(THIS_ LPCDIDATAFORMAT) PURE;
HACKY_COM_BEGIN(IDirectInputDeviceA, 11)
  hacky_printf("p 0x%" PRIX32 "\n", stack[1]);
  hacky_printf("a 0x%" PRIX32 "\n", stack[2]);
  eax = 0; // HRESULT -> non-negative means success
  esp += 2 * 4;
HACKY_COM_END()

// IDirectInputDeviceA -> STDMETHOD(SetCooperativeLevel)(THIS_ HWND,DWORD) PURE;
HACKY_COM_BEGIN(IDirectInputDeviceA, 13)
  hacky_printf("p 0x%" PRIX32 "\n", stack[1]);
  hacky_printf("a 0x%" PRIX32 "\n", stack[2]);
  hacky_printf("b 0x%" PRIX32 "\n", stack[3]);
  eax = 0; // HRESULT -> non-negative means success
  esp += 3 * 4;
HACKY_COM_END()







// DirectDraw

HACKY_IMPORT_BEGIN(DirectDrawEnumerateA)
  Address lpCallback = stack[1];
  Address lpContext = stack[2];
  hacky_printf("lpCallback 0x%" PRIX32 "\n", lpCallback); // LPDDENUMCALLBACKA
  hacky_printf("lpContext 0x%" PRIX32 "\n", lpContext);
  eax = 0; // HRESULT -> non-negative means success
  esp += 2 * 4;
#if 1
  // Push a call to the callback onto the stack.. this is some ugly hack..

  // Convention is PASCAL

  esp -= 4;
  *(uint32_t*)Memory(esp) = returnAddress;

  {
    esp -= 4;
    *(uint32_t*)Memory(esp) = lpContext; // lpContext

    esp -= 4;
    *(uint32_t*)Memory(esp) = Allocate(200); // lpDriverName

    // Used to identifty drawing device
    esp -= 4;
    Address lpDriverDescription = Allocate(200);
    sprintf((char*)Memory(lpDriverDescription), "%s", "OpenSWE1R D3D");
    *(uint32_t*)Memory(esp) = lpDriverDescription;

    // Used as parameter in Direct Draw `Initialize`
    esp -= 4;
    *(uint32_t*)Memory(esp) = Allocate(200); // lpGUID

    // Emulate the call
    esp -= 4;
    *(uint32_t*)Memory(esp) = clearEax;
    eip = lpCallback;

    printf("  Callback at 0x%" PRIX32 "\n", eip);
    //FIXME: Add a hook which returns 0
  }
#endif
HACKY_IMPORT_END()










// DirectInput

HACKY_IMPORT_BEGIN(DirectInputCreateA)
  hacky_printf("hinst 0x%" PRIX32 "\n", stack[1]);
  hacky_printf("dwVersion 0x%" PRIX32 "\n", stack[2]);
  hacky_printf("lplpDirectInput 0x%" PRIX32 "\n", stack[3]);
  hacky_printf("punkOuter 0x%" PRIX32 "\n", stack[4]);
  //FIXME: Unsure about most terminology / inner workings here
  *(Address*)Memory(stack[3]) = CreateInterface("IDirectInputA", 200);
  eax = 0; // HRESULT -> non-negative means success
  esp += 4 * 4;
HACKY_IMPORT_END()




// IDirectInputA

// IDirectInputA -> STDMETHOD_(ULONG,Release)       (THIS) PURE; //2
HACKY_COM_BEGIN(IDirectInputA, 2)
  hacky_printf("p 0x%" PRIX32 "\n", stack[1]);
  eax = 0; // FIXME: No idea what this expects to return..
  esp += 1 * 4;
HACKY_COM_END()

// IDirectInputA -> STDMETHOD(CreateDevice)(THIS_ REFGUID,LPDIRECTINPUTDEVICEA *,LPUNKNOWN) PURE;
HACKY_COM_BEGIN(IDirectInputA, 3)
  hacky_printf("p 0x%" PRIX32 "\n", stack[1]);
  hacky_printf("rguid 0x%" PRIX32 "\n", stack[2]);
  hacky_printf("lpIDD 0x%" PRIX32 "\n", stack[3]);
  hacky_printf("pUnkOuter 0x%" PRIX32 "\n", stack[4]);
  *(Address*)Memory(stack[3]) = CreateInterface("IDirectInputDeviceA", 200);
  eax = 0; // HRESULT -> non-negative means success
  esp += 4 * 4;
HACKY_COM_END()

// IDirectInputA -> STDMETHOD(EnumDevices)(THIS_ DWORD,LPDIENUMDEVICESCALLBACKA,LPVOID,DWORD) PURE;
HACKY_COM_BEGIN(IDirectInputA, 4)
  hacky_printf("EnumDevices\n");
  hacky_printf("p 0x%" PRIX32 "\n", stack[1]);
  uint32_t a = stack[2];
  uint32_t b = stack[3];
  uint32_t c = stack[4];
  uint32_t d = stack[5];
  hacky_printf("a 0x%" PRIX32 "\n", a);
  hacky_printf("b 0x%" PRIX32 "\n", b);
  hacky_printf("c 0x%" PRIX32 "\n", c);
  hacky_printf("d 0x%" PRIX32 "\n", d);
  //FIXME: Do some callback stuff
  eax = 0; // HRESULT -> non-negative means success
  esp += 5 * 4;
  // Push a call to the callback onto the stack.. this is some ugly hack..

  // Convention is PASCAL

  esp -= 4;
  *(uint32_t*)Memory(esp) = returnAddress;

  {
    esp -= 4;
    *(uint32_t*)Memory(esp) = c; // pvRef

    Address ddiAddress = Allocate(sizeof(API(DIDEVICEINSTANCEA)));
    API(DIDEVICEINSTANCEA)* ddi = Memory(ddiAddress);
    memset(ddi, 0x00, sizeof(API(DIDEVICEINSTANCEA)));

    ddi->dwSize = sizeof(API(DIDEVICEINSTANCEA));
    //FIXME:    GUID guidInstance;
    //FIXME:    GUID guidProduct;
    enum {
      API(DIDEVTYPE_KEYBOARD) = 3
    };
    ddi->dwDevType = API(DIDEVTYPE_KEYBOARD); // or something
    sprintf(ddi->tszInstanceName, "OpenSWE1R Keyboard 1"); // TCHAR tszInstanceName[MAX_PATH];
    sprintf(ddi->tszProductName, "OpenSWE1R Keyboard"); // TCHAR tszProductName[MAX_PATH];
    //FIXME:    GUID guidFFDriver;
    ddi->wUsagePage = 0; //FIXME look at usb spec?
    ddi->wUsage = 0; //FIXME look at usb spec?

    esp -= 4;
    *(uint32_t*)Memory(esp) = ddiAddress; // LPCDIDEVICEINSTANCEA

    // Emulate the call
    esp -= 4;
    *(uint32_t*)Memory(esp) = clearEax; // Return to clear eax
    eip = b;

    printf("  Callback at 0x%" PRIX32 "\n", eip);
    //FIXME: Add a hook which returns 0
  }
HACKY_COM_END()





static void UcMallocHook(void* uc, uint64_t address, uint32_t size, void* user_data) {
  int eip;
  uc_reg_read(uc, UC_X86_REG_EIP, &eip);
  int esp;
  uc_reg_read(uc, UC_X86_REG_ESP, &esp);
  int eax;
  uc_reg_read(uc, UC_X86_REG_EAX, &eax);
  
  Address stackAddress = esp;
  uint32_t* stack = (uint32_t*)Memory(stackAddress);

  // Pop the return address
  Address returnAddress = stack[0];
  printf("Return at 0x%" PRIX32 "\n", returnAddress);

  eax = Allocate(stack[1]);
  printf("malloc(%d) -> 0x%08X\n", stack[1], eax);

  uc_reg_write(uc, UC_X86_REG_EAX, &eax);
  eip = returnAddress;
  uc_reg_write(uc, UC_X86_REG_EIP, &eip);
  esp += 4;
  uc_reg_write(uc, UC_X86_REG_ESP, &esp);
}

static void UcFreeHook(void* uc, uint64_t address, uint32_t size, void* user_data) {
  int eip;
  uc_reg_read(uc, UC_X86_REG_EIP, &eip);
  int esp;
  uc_reg_read(uc, UC_X86_REG_ESP, &esp);
  int eax;
  uc_reg_read(uc, UC_X86_REG_EAX, &eax);
  
  Address stackAddress = esp;
  uint32_t* stack = (uint32_t*)Memory(stackAddress);

  // Pop the return address
  Address returnAddress = stack[0];
  printf("Return at 0x%" PRIX32 "\n", returnAddress);

  printf("free(0x%08X)\n", stack[1]);
  Free(stack[1]);

  eax = 0;
  uc_reg_write(uc, UC_X86_REG_EAX, &eax);
  eip = returnAddress;
  uc_reg_write(uc, UC_X86_REG_EIP, &eip);
  esp += 4;
  uc_reg_write(uc, UC_X86_REG_ESP, &esp);
}


// Some TGA loading function

static void UcTGAHook(void* uc, uint64_t address, uint32_t size, void* user_data) {
  int eip;
  uc_reg_read(uc, UC_X86_REG_EIP, &eip);
  int esp;
  uc_reg_read(uc, UC_X86_REG_ESP, &esp);
  int eax;
  uc_reg_read(uc, UC_X86_REG_EAX, &eax);
  
  Address stackAddress = esp;
  uint32_t* stack = (uint32_t*)Memory(stackAddress);

  // Pop the return address
  Address returnAddress = stack[0];
  printf("Return at 0x%" PRIX32 "\n", returnAddress);

  //int __cdecl sub_48A230(int a1, char *a2, _DWORD *a3, _DWORD *a4)
  printf("\n\n\n[ 48A230 ] TGAHook(0x%08X, 0x%08X, 0x%08X, 0x%08X)\n\n\n\n", stack[1], stack[2], stack[3], stack[4]);

  // Emulate instruction we overwrote
  eax = stack[1];
  uc_reg_write(uc, UC_X86_REG_EAX, &eax);
  eip = 0x48a234;
  uc_reg_write(uc, UC_X86_REG_EIP, &eip);
}



// Generic hook


// Callback for tracing instructions
static void UnknownImport(void* uc, Address address, void* user_data) {
  int eip;
  uc_reg_read(uc, UC_X86_REG_EIP, &eip);
  int esp;
  uc_reg_read(uc, UC_X86_REG_ESP, &esp);
  int eax;
  uc_reg_read(uc, UC_X86_REG_EAX, &eax);
  
  Address stackAddress = esp;
  uint32_t* stack = (uint32_t*)Memory(stackAddress);

#if 0
  // This lists a stack trace.
  // It's a failed attempt because most functions omit the frame pointer
  int ebp;
  uc_reg_read(uc, UC_X86_REG_EBP, &ebp);
  StackTrace(ebp, 10, 4);
#endif

  // Pop the return address
  Address returnAddress = stack[0];
  eip = returnAddress;
  esp += 4;
  
  printf("\nUnknown function!\n\n");

  printf("Stack at 0x%" PRIX32 "; returning EAX: 0x%08" PRIX32 "\n", stackAddress, eax);
  printf("%7" PRIu32 " Emulation at %X ('%s') from %X\n\n", callId, eip, (char*)user_data, returnAddress);

  callId++;

  assert(false);

  uc_reg_write(uc, UC_X86_REG_ESP, &esp);
  uc_reg_write(uc, UC_X86_REG_EIP, &eip);
  uc_reg_write(uc, UC_X86_REG_EAX, &eax);
}

// This loads the exe into memory, even on Linux
// NOTE: This purposely does not map the file into memory for portability
Exe* LoadExe(const char* path) {
  exe = (Exe*)malloc(sizeof(Exe)); //FIXME: Hack to make this global!
  memset(exe, 0x00, sizeof(exe));

  // Load the exe file and skip the DOS header
  exe->f = fopen(path, "rb");
  if (exe->f == NULL) {
    perror(path);
    return NULL;
  }
  char dosMagic[2];
  fread(dosMagic, 1, 2, exe->f);
  assert(memcmp(dosMagic, "MZ", 2) == 0);
  fseek(exe->f, 60, SEEK_SET);
  uint32_t peAddress;
  fread(&peAddress, 1, 4, exe->f);

  // Read COFF header
  fseek(exe->f, peAddress, SEEK_SET);
  char peMagic[4];
  fread(peMagic, 1, 4, exe->f);
  assert(memcmp(peMagic, "PE\x00\x00", 4) == 0);
  fread(&exe->coffHeader, 1, sizeof(exe->coffHeader), exe->f);
  printf("Machine type: 0x%" PRIX16 "\n", exe->coffHeader.machine);
  printf("Number of sections: %" PRIu16 "\n", exe->coffHeader.numberOfSections);
  
  // Read optional PE header
  assert(exe->coffHeader.sizeOfOptionalHeader >= sizeof(exe->peHeader));
  fread(&exe->peHeader, 1, sizeof(exe->peHeader), exe->f);
  printf("Entry point: 0x%" PRIX32 "\n", exe->peHeader.imageBase + exe->peHeader.addressOfEntryPoint);

  //FIXME: Parse data dictionaries
  exe->dataDirectories = malloc(exe->peHeader.numberOfRvaAndSizes * sizeof(PeDataDirectory));
  fread(exe->dataDirectories, exe->peHeader.numberOfRvaAndSizes, sizeof(PeDataDirectory), exe->f);

  // Load sections
  exe->mappedSections = malloc(exe->coffHeader.numberOfSections * sizeof(uint8_t*));
  exe->sections = malloc(exe->coffHeader.numberOfSections * sizeof(PeSection));
  for(unsigned int sectionIndex = 0; sectionIndex < exe->coffHeader.numberOfSections; sectionIndex++) {
    PeSection* section = &exe->sections[sectionIndex];

    // Load actual data
    //FIXME: Use/skip proper alignment etc. from header
    fread(section, 1, sizeof(PeSection), exe->f);

    //FIXME: Check if section must be loaded
    if (1) {
      size_t cur = ftell(exe->f);
      LoadSection(exe, sectionIndex);
      fseek(exe->f, cur, SEEK_SET);
    } else {
      exe->mappedSections[sectionIndex] = NULL;
    }

    // Debug printing
    printf("Section %u: Virtual: 0x%" PRIX32 " - 0x%" PRIX32 " Initialized: 0x%" PRIX32 " - 0x%" PRIX32 " ('%.8s')\n", sectionIndex,
           exe->peHeader.imageBase + section->virtualAddress,
           exe->peHeader.imageBase + section->virtualAddress + section->virtualSize - 1,
           exe->peHeader.imageBase + section->virtualAddress,
           exe->peHeader.imageBase + section->virtualAddress + section->rawSize - 1,
           section->name);

  }

  // Do relocations (not necessary as we currently load to preferred address)
  if (false) {
    uint32_t relocationRva = exe->peHeader.imageBase + exe->dataDirectories[5].virtualAddress;
    uint32_t remainingSize = exe->dataDirectories[5].size;

    while(remainingSize >= sizeof(API(IMAGE_BASE_RELOCATION))) {
      API(IMAGE_BASE_RELOCATION)* baseRelocation = Memory(relocationRva);
      assert(baseRelocation->sizeOfBlock >= sizeof(API(IMAGE_BASE_RELOCATION)));

      unsigned int relocationCount = (baseRelocation->sizeOfBlock - sizeof(API(IMAGE_BASE_RELOCATION))) / 2;
      printf("Base relocation: 0x%" PRIX32 " (%d relocations)\n", baseRelocation->virtualAddress, relocationCount);
      uint16_t* relocations = Memory(relocationRva);
      for(unsigned int i = 0; i < relocationCount; i++) {
        uint16_t relocation = relocations[i];
        unsigned int type = relocation >> 12;
        unsigned int offset = relocation & 0xFFF;
    
        printf("  Relocation (type %d) at 0x%" PRIX32 "\n", type, exe->peHeader.imageBase + baseRelocation->virtualAddress + offset);
        switch(type) {
          case 0: // IMAGE_REL_BASED_ABSOLUTE
            // "This relocation is meaningless and is only used as a place holder to round relocation blocks up to a DWORD multiple size."
            break;
          case 3: // IMAGE_REL_BASED_HIGHLOW
            // "This relocation means add both the high and low 16 bits of the delta to the DWORD specified by the calculated RVA."
            break;
          default:
            assert(false);
            break;
        }
      }

      relocationRva += baseRelocation->sizeOfBlock;
      remainingSize -= baseRelocation->sizeOfBlock;
    }    
  }


  // Check imports
  // IMAGE_DIRECTORY_ENTRY_IMPORT = 1
  // IMAGE_DIRECTORY_ENTRY_IAT = 12
  {
    uint32_t importRva = exe->peHeader.imageBase + exe->dataDirectories[1].virtualAddress;
    uint32_t remainingSize = exe->dataDirectories[1].size;
    printf("Import table located at 0x%" PRIX32 "\n", importRva);
    //FIXME: Should be done differently. Import table expects zero element at end which is not checked yet! (it's optional here)
    while(remainingSize >= sizeof(API(IMAGE_IMPORT_DESCRIPTOR))) {

      // Access import and check if it is valid
      API(IMAGE_IMPORT_DESCRIPTOR)* imports = Memory(importRva);
      if (IsZero(imports, sizeof(API(IMAGE_IMPORT_DESCRIPTOR)))) {
        break;
      }

      // Dump imports
      const char* name = Memory(exe->peHeader.imageBase + imports->name);
      //FIXME: Bound checking?
      uint32_t originalThunkAddress = exe->peHeader.imageBase + imports->originalFirstThunk;
      uint32_t thunkAddress = exe->peHeader.imageBase + imports->firstThunk;
      printf("Imports for '%s' (0x%" PRIX32 " / 0x%" PRIX32 ")\n", name, originalThunkAddress, thunkAddress);
      while(1) {
        uint32_t importByNameAddress = *(uint32_t*)Memory(originalThunkAddress);
        uint32_t* symbolAddress = (uint32_t*)Memory(thunkAddress);
        if (importByNameAddress == 0) {
          break;
        }

        // Dump entry
        char* label;
        if (importByNameAddress & 0x80000000) {
          unsigned int ordinal = importByNameAddress & 0x7FFFFFFF;
          printf("  0x%" PRIX32 ": @%" PRIu32 " ..", thunkAddress, ordinal);
          label = malloc(128);
          sprintf(label, "<%s@%d>", name, ordinal);
        } else {
          API(IMAGE_IMPORT_BY_NAME)* importByName = Memory(exe->peHeader.imageBase + importByNameAddress);
          printf("  0x%" PRIX32 ": 0x%" PRIX16 " '%s' ..", thunkAddress, importByName->hint, importByName->name);
          label = importByName->name;
        }

        //FIXME: This is a hack.. these calls were WAY too slow because UC is really bad at switching contexts
#if 1
        if (!strcmp("EnterCriticalSection", label) || !strcmp("LeaveCriticalSection", label)) {
          Address codeAddress = Allocate(20);
          uint8_t* code = (uint8_t*)Memory(codeAddress);
          //FIXME:
          *code++ = 0x58; // pop eax // return address
          *code++ = 0x59; // pop ecx // argument
          // test count, 0
          // jne break
          // inc $count for entercrit // dec $count for leavecrit
          // ret
          // break: < Place a breakpoint here which switches threads
          *code++ = 0x50; // push eax
          *code++ = 0xC3; // retn
          *symbolAddress = codeAddress;
          printf("patched\n");
        } else
#endif
        {
          Export* export = NULL;
          if (importByNameAddress & 0x80000000) {
            export = LookupExportByOrdinal(name, importByNameAddress & 0x7FFFFFFF);
          } else {
            export = LookupExportByName(label);
          }

          if (export == NULL) {
            Address hltAddress = CreateHlt();
            AddHltHandler(hltAddress, UnknownImport, (void*)label);
            AddExport(label, UnknownImport, hltAddress);
            *symbolAddress = hltAddress;
            printf("missing at 0x%08X\n", hltAddress);
            //FIXME: Report error and assert false
          } else {
            if (true) { //(export->isVariable == false) {
              Address hltAddress = CreateHlt();
              AddHltHandler(hltAddress, export->callback, (void*)label);
              *symbolAddress = hltAddress;
              printf("found at 0x%08X\n", hltAddress);
            } else {
              printf("found.. is variable\n");
              assert(false);
            }
          }

        }
        
        // Jump to next imported symbol
        originalThunkAddress += 4;
        thunkAddress += 4;
      }

      // Jump to next entry
      importRva += sizeof(API(IMAGE_IMPORT_DESCRIPTOR));
      remainingSize -= sizeof(API(IMAGE_IMPORT_DESCRIPTOR));
    }
  }

  return exe;
}

void RelocateExe(Exe* exe) {
  //FIXME
}

void UnloadExe(Exe* exe) {
  if (exe->mappedSections != NULL) {
    for(unsigned int sectionIndex = 0; sectionIndex < exe->coffHeader.numberOfSections; sectionIndex++) {
      if (exe->mappedSections[sectionIndex] != NULL) {
        UnloadSection(exe, sectionIndex);
      }
    }
    free(exe->mappedSections);
    exe->mappedSections = NULL;
  }
  if (exe->sections != NULL) {
    free(exe->sections);
    exe->sections = NULL;
  }
  if (exe->dataDirectories != NULL) {
    free(exe->dataDirectories);
    exe->dataDirectories = NULL;
  }
  fclose(exe->f);
  free(exe);
}

//FIXME: Abstract exe mapping and context creation from emu kickoff
void RunX86(Exe* exe) {

  // Map the important exe parts into emu memory
  for(unsigned int sectionIndex = 0; sectionIndex < exe->coffHeader.numberOfSections; sectionIndex++) {
    PeSection* section = &exe->sections[sectionIndex];
    void* mappedSection = (void*)exe->mappedSections[sectionIndex];
    if (mappedSection != NULL) {
      uint32_t base = exe->peHeader.imageBase + section->virtualAddress;
      printf("Mapping 0x%" PRIX32 " - 0x%" PRIX32 "\n", base, base + section->virtualSize - 1);
      MapMemory(mappedSection, base, AlignUp(section->virtualSize, exe->peHeader.sectionAlignment), true, true, true);
    }
  }

  //FIXME: Schedule a virtual main-thread
  printf("Emulation starting\n");
  CreateEmulatedThread(exe->peHeader.imageBase + exe->peHeader.addressOfEntryPoint);
  RunEmulation();

  CleanupEmulation();
}

int main(int argc, char* argv[]) {
  printf("-- Initializing\n");
  InitializeEmulation();
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_EVENTS) < 0) {
		  printf("Failed to initialize SDL2!\n");
  }
  printf("-- Creating window\n");
  {
    bool fullscreen = false;
    int w = 640;
    int h = 480;

  	Uint32 style = SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN;
	  if (fullscreen) {
		  style |= SDL_WINDOW_FULLSCREEN;
    }

	  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	  SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    sdlWindow = SDL_CreateWindow("OpenSWE1R", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, w, h, style);
	  assert(sdlWindow != NULL);

	  SDL_GLContext glcontext = SDL_GL_CreateContext(sdlWindow);
	  assert(glcontext != NULL);

    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (err != GLEW_OK) {
      fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
      return 1;
    }


    //FIXME: This is ugly but gets the job done.. for now
    static GLuint vao = 0;
    if (vao == 0) {
      glGenVertexArrays(1, &vao);
    }
    glBindVertexArray(vao);


    glDisable(GL_CULL_FACE);
//    glDepthFunc(GL_GEQUAL);
    glCullFace(GL_FRONT);    


  	SDL_ShowWindow(sdlWindow);
  }
  printf("-- Compiling shaders\n");
  GLuint shader1Texture = 0;
  {
    GLuint vertexShader = CreateShader(VertexShader1Texture, GL_VERTEX_SHADER);
    GLuint fragmentShader = CreateShader(FragmentShader1Texture, GL_FRAGMENT_SHADER);
    shader1Texture = CreateShaderProgram(vertexShader, fragmentShader);
  }
  bool linked = LinkShaderProgram(shader1Texture);
  PrintShaderProgramLog(shader1Texture);
  assert(linked);
  glUseProgram(shader1Texture); //FIXME: Hack..
  printf("-- Loading exe\n");
  Exe* exe = LoadExe(exeName);
  if (exe == NULL) {
    printf("Couldn't load '%s'\n", exeName);
    exit(EXIT_FAILURE);
  }
  RelocateExe(exe);

  // Attempt to identify the game version using the COFF timestamp
  if (exe->coffHeader.timeDateStamp == 0x3727ce0e) {
    printf("Game version: Retail, English\n");
  } else if (exe->coffHeader.timeDateStamp == 0x3738c552) {
    printf("Game version: Retail, German\n"); // International?
  } else if (exe->coffHeader.timeDateStamp == 0x37582659) {
    printf("Game version: Webdemo, English\n");
  } else if (exe->coffHeader.timeDateStamp == 0x3c60692c) {
    printf("Game version: Patched, English\n");
  } else if (exe->coffHeader.timeDateStamp == 0x3c6321d1) {
    printf("Game version: Patched, International\n");
  } else {
    printf("Game version: Unknown (COFF timestamp: 0x%08X)\n", exe->coffHeader.timeDateStamp);
    assert(false);
  }

  clearEax = Allocate(3);
  uint8_t* p = Memory(clearEax);
  *p++ = 0x31; *p++ = 0xC0; // xor eax, eax
  *p++ = 0xC3;              // ret

// 0x90 = nop (used to disable code)
// 0xC3 = ret (used to skip function)
// 0x84 = je (probably used to be `jne`, used to invert condition)
// 0x75 = jne (probably used to be `je`, used to invert condition)

// These functions access internal FILE* data I belive; crashes our emu
#if 0
*(uint8_t*)Memory(0x4A1670) = 0xC3; // _lock
*(uint8_t*)Memory(0x4A16F0) = 0xC3; // _unlock
*(uint8_t*)Memory(0x4A1710) = 0xC3; // _lock_file
*(uint8_t*)Memory(0x4A1780) = 0xC3; // _unlock_file
#endif

#if 0 //FIXME FIXME FIXME FIXME FIXME
  // These do something bad internally
  CreateBreakpoint(0x49f270, UcMallocHook, "<malloc>");
  CreateBreakpoint(0x49f200, UcFreeHook, "<free>");

  // This function used to crash with SIGSEGV, so I wanted to peek at the parameters.
  CreateBreakpoint(0x48A230, UcTGAHook, "<TGAHook>");
#endif

#if 0
*(uint8_t*)Memory(0x487d71) = 0x75; // Invert the check for eax after "DirectDrawEnumerate" (ours will always fail)
*(uint8_t*)Memory(0x488ce2) = 0x75; // Invert the check for eax after "EnumDisplayModes" (ours will always fail)
*(uint8_t*)Memory(0x489e20) = 0x75; // Invert the check for eax after "EnumDevices" [graphics] (ours will always fail)
*(uint8_t*)Memory(0x48a013) = 0x84; // Invert the check for eax after "EnumTextureFormats" (ours will always fail)
*(uint8_t*)Memory(0x485433) = 0x75; // Invert the check for eax after "EnumDevices" [input] (ours will always fail)
#endif

//memset(Memory(0x423cd9), 0x90, 5); // Disable command line arg scanning

  printf("-- Switching mode\n");
  RunX86(exe);
  printf("-- Exiting\n");
  UnloadExe(exe);
  return EXIT_SUCCESS;
}
