#include "../windows.h"

#include "../export.h"
#include "../emulation.h"

// 0x8704A0
EXPORT_STDCALL(kernel32, API(DWORD), GetVersion) { // WINAPI
// Windows 98 (https://support.microsoft.com/en-us/kb/189249)
  uint16_t platformId = 1;
  uint8_t majorVersion = 4;
  uint8_t minorVersion = 10;
  return (platformId << 16) | (minorVersion << 8) | majorVersion;
}

// 0x8704F8
EXPORT_STDCALL(kernel32, API(LPTSTR), GetCommandLineA) { //WINAPI
  //return "program"; //FIXME!
  return 0;
}

// 0x8704F4
EXPORT_STDCALL(kernel32, API(VOID), GetStartupInfo, API(LPSTARTUPINFO),lpStartupInfo) { //WINAPI
  //lpStartupInfo->
}

