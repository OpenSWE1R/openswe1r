#ifndef __OPENSWE1R_WINDOWS_H__
#define __OPENSWE1R_WINDOWS_H__

#include <stdint.h>

#include "emulation.h"

typedef uint32_t DWORD;
typedef void VOID;

typedef uint32_t UINT; // FIXME: Assumption
typedef int16_t SHORT; // FIXME: Assumption

typedef struct {
  uint32_t Data1;
  uint16_t Data2;
  uint16_t Data3;
  uint8_t Data4[8];
} GUID;
typedef GUID CLSID;
typedef GUID IID;

typedef struct {
  uint32_t bmType;
  uint32_t bmWidth;
  uint32_t bmHeight;
  uint32_t bmWidthBytes;
  uint16_t bmPlanes;
  uint16_t bmBitsPixel;
  Address bmBits;
} BITMAP;

typedef DWORD COLORREF;

typedef uint32_t SIZE_T;

typedef struct {
  DWORD dwLowDateTime;
  DWORD dwHighDateTime;
} FILETIME;

#define MAX_PATH 260

typedef char TCHAR;
typedef struct {
  DWORD    dwFileAttributes;
  FILETIME ftCreationTime;
  FILETIME ftLastAccessTime;
  FILETIME ftLastWriteTime;
  DWORD    nFileSizeHigh;
  DWORD    nFileSizeLow;
  DWORD    dwReserved0;
  DWORD    dwReserved1;
  TCHAR    cFileName[MAX_PATH];
  TCHAR    cAlternateFileName[14];
} WIN32_FIND_DATA;

typedef struct {
  int x;
} STR;
typedef Address LPTSTR;


#define VK_SHIFT 0x10
#define VK_CONTROL 0x11
#define VK_MENU 0x12

#define VK_LSHIFT 0xA0
#define VK_RSHIFT 0xA1
#define VK_LCONTROL 0xA2
#define VK_RCONTROL 0xA3
#define VK_LMENU 0xA4
#define VK_RMENU 0xA5

#endif
