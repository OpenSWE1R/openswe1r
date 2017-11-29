// Copyright 2017 OpenSWE1R Maintainers
// Licensed under GPLv2 or any later version
// Refer to the included LICENSE.txt file.

#include "../main.h"
#include "../windows.h"

#include <inttypes.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>

#ifdef DPLAY_ENET
#include <enet/enet.h>
#endif

typedef struct {
  API(DWORD)   dwSize;             // Size of structure
  API(DWORD)   dwFlags;            // Not used. Must be zero.
  union {                           // The short or friendly name
    Address  lpszShortName;  // Unicode
    Address   lpszShortNameA; // ANSI
  };
  union {                           // The long or formal name
    Address  lpszLongName;   // Unicode
    Address   lpszLongNameA;  // ANSI
  };

} API(DPNAME);

typedef struct {
  API(DWORD)   dwSize;             // Size of structure
  API(DWORD)   dwFlags;            // DPSESSION_xxx flags
  API(GUID)    guidInstance;       // ID for the session instance
  API(GUID)    guidApplication;    // GUID of the DirectPlay application.
                              // GUID_NULL for all applications.
  API(DWORD)   dwMaxPlayers;       // Maximum # players allowed in session
  API(DWORD)   dwCurrentPlayers;   // Current # players in session (read only)
  union {                           // Name of the session
    Address  lpszSessionName;    // Unicode
    Address   lpszSessionNameA;   // ANSI
  };
  union {                           // Password of the session (optional)
    Address  lpszPassword;       // Unicode
    Address   lpszPasswordA;      // ANSI
  };
  API(DWORD)   dwReserved1;        // Reserved for future MS use.
  API(DWORD)   dwReserved2;
  API(DWORD)   dwUser1;            // For use by the application
  API(DWORD)   dwUser2;
  API(DWORD)   dwUser3;
  API(DWORD)   dwUser4;
} API(DPSESSIONDESC2);

// IDirectPlayLobby3 (and IDirectPlayLobby3A) Interface
#if 0
DECLARE_INTERFACE_( IDirectPlayLobby3, IDirectPlayLobby )
{
    /*  IUnknown Methods	*/
    STDMETHOD(QueryInterface)       (THIS_ REFIID riid, LPVOID * ppvObj) PURE; //0
    STDMETHOD_(ULONG,AddRef)        (THIS) PURE;
    STDMETHOD_(ULONG,Release)       (THIS) PURE; //2

    /*  IDirectPlayLobby Methods	*/
    STDMETHOD(Connect)              (THIS_ DWORD, LPDIRECTPLAY2 *, IUnknown FAR *) PURE;
    STDMETHOD(CreateAddress)        (THIS_ REFGUID, REFGUID, LPCVOID, DWORD, LPVOID, LPDWORD) PURE; //4
    STDMETHOD(EnumAddress)          (THIS_ LPDPENUMADDRESSCALLBACK, LPCVOID, DWORD, LPVOID) PURE;
    STDMETHOD(EnumAddressTypes)     (THIS_ LPDPLENUMADDRESSTYPESCALLBACK, REFGUID, LPVOID, DWORD) PURE; //6
    STDMETHOD(EnumLocalApplications)(THIS_ LPDPLENUMLOCALAPPLICATIONSCALLBACK, LPVOID, DWORD) PURE;
    STDMETHOD(GetConnectionSettings)(THIS_ DWORD, LPVOID, LPDWORD) PURE; //8
    STDMETHOD(ReceiveLobbyMessage)  (THIS_ DWORD, DWORD, LPDWORD, LPVOID, LPDWORD) PURE;
    STDMETHOD(RunApplication)       (THIS_ DWORD, LPDWORD, LPDPLCONNECTION, HANDLE) PURE; //10
    STDMETHOD(SendLobbyMessage)     (THIS_ DWORD, DWORD, LPVOID, DWORD) PURE;
    STDMETHOD(SetConnectionSettings)(THIS_ DWORD, DWORD, LPDPLCONNECTION) PURE;
    STDMETHOD(SetLobbyMessageEvent) (THIS_ DWORD, DWORD, HANDLE) PURE; //13

    /*  IDirectPlayLobby2 Methods	*/
    STDMETHOD(CreateCompoundAddress)(THIS_ LPCDPCOMPOUNDADDRESSELEMENT,DWORD,LPVOID,LPDWORD) PURE;

    /*  IDirectPlayLobby3 Methods	*/
    STDMETHOD(ConnectEx)            (THIS_ DWORD, REFIID, LPVOID *, IUnknown FAR *) PURE;
    STDMETHOD(RegisterApplication)  (THIS_ DWORD, LPVOID) PURE; //16
    STDMETHOD(UnregisterApplication)(THIS_ DWORD, REFGUID) PURE;
    STDMETHOD(WaitForConnectionSettings)(THIS_ DWORD) PURE;
};





DECLARE_INTERFACE_( IDirectPlay4, IDirectPlay3 )
{
    /*** IUnknown methods ***/
    STDMETHOD(QueryInterface)       (THIS_ REFIID riid, LPVOID * ppvObj) PURE; // 0
    STDMETHOD_(ULONG,AddRef)        (THIS)  PURE; // 1
    STDMETHOD_(ULONG,Release)       (THIS) PURE; // 2
    /*** IDirectPlay2 methods ***/
    STDMETHOD(AddPlayerToGroup)     (THIS_ DPID, DPID) PURE; // 3
    STDMETHOD(Close)                (THIS) PURE; // 4
    STDMETHOD(CreateGroup)          (THIS_ LPDPID,LPAPI(DPNAME),LPVOID,DWORD,DWORD) PURE; // 5
    STDMETHOD(CreatePlayer)         (THIS_ LPDPID,LPAPI(DPNAME),HANDLE,LPVOID,DWORD,DWORD) PURE; // 6
    STDMETHOD(DeletePlayerFromGroup)(THIS_ DPID,DPID) PURE; // 7
    STDMETHOD(DestroyGroup)         (THIS_ DPID) PURE; // 8
    STDMETHOD(DestroyPlayer)        (THIS_ DPID) PURE; // 9
    STDMETHOD(EnumGroupPlayers)     (THIS_ DPID,LPGUID,LPDPENUMPLAYERSCALLBACK2,LPVOID,DWORD) PURE; // 10
    STDMETHOD(EnumGroups)           (THIS_ LPGUID,LPDPENUMPLAYERSCALLBACK2,LPVOID,DWORD) PURE; // 11
    STDMETHOD(EnumPlayers)          (THIS_ LPGUID,LPDPENUMPLAYERSCALLBACK2,LPVOID,DWORD) PURE; // 12
    STDMETHOD(EnumSessions)         (THIS_ LPAPI(DPSESSIONDESC2),DWORD,LPDPENUMSESSIONSCALLBACK2,LPVOID,DWORD) PURE; // 13
    STDMETHOD(GetCaps)              (THIS_ LPDPCAPS,DWORD) PURE; // 14
    STDMETHOD(GetGroupData)         (THIS_ DPID,LPVOID,LPDWORD,DWORD) PURE; // 15
    STDMETHOD(GetGroupName)         (THIS_ DPID,LPVOID,LPDWORD) PURE; // 16
    STDMETHOD(GetMessageCount)      (THIS_ DPID, LPDWORD) PURE; // 17
    STDMETHOD(GetPlayerAddress)     (THIS_ DPID,LPVOID,LPDWORD) PURE; // 18
    STDMETHOD(GetPlayerCaps)        (THIS_ DPID,LPDPCAPS,DWORD) PURE; // 19
    STDMETHOD(GetPlayerData)        (THIS_ DPID,LPVOID,LPDWORD,DWORD) PURE; // 20
    STDMETHOD(GetPlayerName)        (THIS_ DPID,LPVOID,LPDWORD) PURE; // 21
    STDMETHOD(GetSessionDesc)       (THIS_ LPVOID,LPDWORD) PURE; // 22
    STDMETHOD(Initialize)           (THIS_ LPGUID) PURE; // 23
    STDMETHOD(Open)                 (THIS_ LPAPI(DPSESSIONDESC2),DWORD) PURE; // 24
    STDMETHOD(Receive)              (THIS_ LPDPID,LPDPID,DWORD,LPVOID,LPDWORD) PURE; // 25
    STDMETHOD(Send)                 (THIS_ DPID, DPID, DWORD, LPVOID, DWORD) PURE; // 26
    STDMETHOD(SetGroupData)         (THIS_ DPID,LPVOID,DWORD,DWORD) PURE; // 27
    STDMETHOD(SetGroupName)         (THIS_ DPID,LPAPI(DPNAME),DWORD) PURE; // 28
    STDMETHOD(SetPlayerData)        (THIS_ DPID,LPVOID,DWORD,DWORD) PURE; // 29
    STDMETHOD(SetPlayerName)        (THIS_ DPID,LPAPI(DPNAME),DWORD) PURE; // 30
    STDMETHOD(SetSessionDesc)       (THIS_ LPAPI(DPSESSIONDESC2),DWORD) PURE; // 31
    /*** IDirectPlay3 methods ***/
    STDMETHOD(AddGroupToGroup)      (THIS_ DPID, DPID) PURE; // 32
    STDMETHOD(CreateGroupInGroup)   (THIS_ DPID,LPDPID,LPAPI(DPNAME),LPVOID,DWORD,DWORD) PURE; // 33
    STDMETHOD(DeleteGroupFromGroup)	(THIS_ DPID,DPID) PURE;	// 34
    STDMETHOD(EnumConnections)     	(THIS_ LPCGUID,LPDPENUMCONNECTIONSCALLBACK,LPVOID,DWORD) PURE; // 35
    STDMETHOD(EnumGroupsInGroup)	(THIS_ DPID,LPGUID,LPDPENUMPLAYERSCALLBACK2,LPVOID,DWORD) PURE; // 36
	STDMETHOD(GetGroupConnectionSettings)(THIS_ DWORD, DPID, LPVOID, LPDWORD) PURE; // 37
	STDMETHOD(InitializeConnection) (THIS_ LPVOID,DWORD) PURE; // 38
    STDMETHOD(SecureOpen)           (THIS_ LPCAPI(DPSESSIONDESC2),DWORD,LPCDPSECURITYDESC,LPCDPCREDENTIALS) PURE; // 39
    STDMETHOD(SendChatMessage)      (THIS_ DPID,DPID,DWORD,LPDPCHAT) PURE; // 40
    STDMETHOD(SetGroupConnectionSettings)(THIS_ DWORD,DPID,LPDPLCONNECTION) PURE; // 41
    STDMETHOD(StartSession)         (THIS_ DWORD,DPID) PURE; // 42
    STDMETHOD(GetGroupFlags)        (THIS_ DPID,LPDWORD) PURE; // 43
    STDMETHOD(GetGroupParent)       (THIS_ DPID,LPDPID) PURE; // 44
    STDMETHOD(GetPlayerAccount)     (THIS_ DPID, DWORD, LPVOID, LPDWORD) PURE; // 45
    STDMETHOD(GetPlayerFlags)       (THIS_ DPID,LPDWORD) PURE; // 46
    /*** IDirectPlay4 methods ***/
    STDMETHOD(GetGroupOwner)        (THIS_ DPID, LPDPID) PURE; // 47
    STDMETHOD(SetGroupOwner)        (THIS_ DPID, DPID) PURE;  // 48
    STDMETHOD(SendEx)               (THIS_ DPID, DPID, DWORD, LPVOID, DWORD, DWORD, DWORD, LPVOID, LPDWORD) PURE; // 49
    STDMETHOD(GetMessageQueue)      (THIS_ DPID, DPID, DWORD, LPDWORD, LPDWORD) PURE; // 50
    STDMETHOD(CancelMessage)        (THIS_ DWORD, DWORD) PURE; // 51
    STDMETHOD(CancelPriority)       (THIS_ DWORD, DWORD, DWORD) PURE; // 52
};



#endif

//typedef struct {
//}

//COM_INTERFACE(, sizeof() / sizeof(void*))

// IDirectPlay4 -> STDMETHOD_(ULONG,Release)       (THIS) PURE; // 2
HACKY_COM_BEGIN(IDirectPlay4, 2)
  hacky_printf("Release\n");
  hacky_printf("p 0x%" PRIX32 "\n", stack[1]);
  eax = 0;
  esp += 1 * 4;
HACKY_COM_END()

// IDirectPlay4 -> STDMETHOD(Close)                (THIS) PURE; // 4
HACKY_COM_BEGIN(IDirectPlay4, 4)
  hacky_printf("Close\n");
  hacky_printf("p 0x%" PRIX32 "\n", stack[1]);
  eax = 0;
  esp += 1 * 4;
HACKY_COM_END()

// IDirectPlay4 -> STDMETHOD(CreatePlayer)         (THIS_ LPDPID,LPAPI(DPNAME),HANDLE,LPVOID,DWORD,DWORD) PURE; // 6
HACKY_COM_BEGIN(IDirectPlay4, 6)
  hacky_printf("CreatePlayer\n");
  uint32_t a = stack[2];
  uint32_t b = stack[3];
  uint32_t c = stack[4];
  uint32_t d = stack[5];
  uint32_t e = stack[6];
  uint32_t f = stack[7];
  hacky_printf("p 0x%" PRIX32 "\n", stack[1]);
  hacky_printf("a 0x%" PRIX32 "\n", a);
  hacky_printf("b 0x%" PRIX32 "\n", b);
  hacky_printf("c 0x%" PRIX32 "\n", c);
  hacky_printf("d 0x%" PRIX32 "\n", d);
  hacky_printf("e 0x%" PRIX32 "\n", e);
  hacky_printf("f 0x%" PRIX32 "\n", f);
  eax = 0;
  esp += 7 * 4;
HACKY_COM_END()

// IDirectPlay4 -> STDMETHOD(DestroyPlayer)        (THIS_ DPID) PURE; // 9
HACKY_COM_BEGIN(IDirectPlay4, 9)
  hacky_printf("DestroyPlayer\n");
  uint32_t a = stack[2];
  hacky_printf("p 0x%" PRIX32 "\n", stack[1]);
  hacky_printf("a 0x%" PRIX32 "\n", a);
  eax = 0;
  esp += 2 * 4;
HACKY_COM_END()

API(DPSESSIONDESC2) globalDesc;

//FIXME: I can't get EnumPlayers to display any people on the menu >.<

// IDirectPlay4 -> STDMETHOD(EnumPlayers)          (THIS_ LPGUID,LPDPENUMPLAYERSCALLBACK2,LPVOID,DWORD) PURE; // 12
HACKY_COM_BEGIN(IDirectPlay4, 12)
  hacky_printf("EnumPlayers\n");
  uint32_t a = stack[2];
  uint32_t b = stack[3];
  uint32_t c = stack[4];
  uint32_t d = stack[5];
  hacky_printf("p 0x%" PRIX32 "\n", stack[1]);
  hacky_printf("a 0x%" PRIX32 "\n", a);
  hacky_printf("b 0x%" PRIX32 "\n", b);
  hacky_printf("c 0x%" PRIX32 "\n", c);
  hacky_printf("d 0x%" PRIX32 "\n", d);
  eax = 0;
  esp += 5 * 4;

  // Push calls to the callback onto the stack.. this is some ugly hack..
  if (globalDesc.dwCurrentPlayers > 0) {
    esp -= 4;
    *(uint32_t*)Memory(esp) = returnAddress; // Return where this was supposed to return to
    for(unsigned int i = 0; i < globalDesc.dwCurrentPlayers; i++) {
      esp -= 4;
      *(uint32_t*)Memory(esp) = c; // lpContext

      esp -= 4;
      *(uint32_t*)Memory(esp) = 0; // dwFlags

      esp -= 4;
      Address lpNameAddr = Allocate(sizeof(API(DPNAME)));
      API(DPNAME)* lpName = Memory(lpNameAddr);
      memset(lpName, 0x00, sizeof(API(DPNAME)));
      lpName->dwSize = sizeof(API(DPNAME));
      Address str = Allocate(128);
      //FIXME: max length
      sprintf_ucs2(Memory(str), "OpenSWE1R Player %d", i);
      lpName->lpszShortName = str;
      lpName->lpszLongName = str;
      *(uint32_t*)Memory(esp) = lpNameAddr; // lpName

enum {
  API(DPPLAYERTYPE_GROUP)  = 0x00000000,
  API(DPPLAYERTYPE_PLAYER) = 0x00000001
};

      esp -= 4;
      *(uint32_t*)Memory(esp) = API(DPPLAYERTYPE_PLAYER); // dwPlayerType

      esp -= 4;
      *(uint32_t*)Memory(esp) = i + 1; // dpId

      // Emulate the call
      esp -= 4;
      if (i == 0) {
        *(uint32_t*)Memory(esp) = clearEax; // Return where this was supposed to return to
      } else {
        *(uint32_t*)Memory(esp) = b; // Return to the next callback
      }
      eip = b;
      printf("  Callback at 0x%" PRIX32 "\n", eip);
      //FIXME: Add a hook which returns 0
    }
  }
HACKY_COM_END()

// IDirectPlay4 -> STDMETHOD(EnumSessions)         (THIS_ LPAPI(DPSESSIONDESC2),DWORD,LPDPENUMSESSIONSCALLBACK2,LPVOID,DWORD) PURE; // 13
HACKY_COM_BEGIN(IDirectPlay4, 13)
  hacky_printf("EnumSessions\n");
  uint32_t a = stack[2];
  uint32_t b = stack[3];
  uint32_t c = stack[4];
  uint32_t d = stack[5];
  uint32_t e = stack[6];
  hacky_printf("p 0x%" PRIX32 "\n", stack[1]);
  hacky_printf("a 0x%" PRIX32 "\n", a);
  hacky_printf("b 0x%" PRIX32 "\n", b);
  hacky_printf("c 0x%" PRIX32 "\n", c);
  hacky_printf("d 0x%" PRIX32 "\n", d);
  hacky_printf("e 0x%" PRIX32 "\n", e);
  eax = 0;
  esp += 6 * 4;

  // Push calls to the callback onto the stack.. this is some ugly hack..
  unsigned int sessionCount = 5;
  if (sessionCount > 0) {
    esp -= 4;
    *(uint32_t*)Memory(esp) = returnAddress; // Return where this was supposed to return to
    for (unsigned int i = 0; i < sessionCount; i++) {
      esp -= 4;
      *(uint32_t*)Memory(esp) = d; // lpContext

      esp -= 4;
      *(uint32_t*)Memory(esp) = 0; // dwFlags

      esp -= 4;
      *(uint32_t*)Memory(esp) = Allocate(4); // lpdwTimeOut

      Address lpThisSD = Allocate(sizeof(API(DPSESSIONDESC2)));
      API(DPSESSIONDESC2)* s = Memory(lpThisSD);
      memset(s, 0x00, sizeof(API(DPSESSIONDESC2)));
      s->dwSize = sizeof(API(DPSESSIONDESC2));
      s->lpszSessionName = Allocate(128);
      sprintf_ucs2(Memory(s->lpszSessionName), "OpenSWE1R Session %d:foo\n", i);

      esp -= 4;
      *(uint32_t*)Memory(esp) = lpThisSD; // lpThisSD

      // Emulate the call
      esp -= 4;
      if (i == 0) {
        *(uint32_t*)Memory(esp) = clearEax; // Return where this was supposed to return to
      } else {
        *(uint32_t*)Memory(esp) = c; // Return to the next callback
      }
      eip = c;
      printf("  Callback at 0x%" PRIX32 "\n", eip);
      //FIXME: Add a hook which returns 0
    }
  }
HACKY_COM_END()

// IDirectPlay4 -> STDMETHOD(GetSessionDesc)       (THIS_ LPVOID,LPDWORD) PURE; // 22
HACKY_COM_BEGIN(IDirectPlay4, 22)
  hacky_printf("GetSessionDesc\n");
  uint32_t a = stack[2];
  uint32_t b = stack[3];
  hacky_printf("p 0x%" PRIX32 "\n", stack[1]);
  hacky_printf("a 0x%" PRIX32 "\n", a);
  hacky_printf("b 0x%" PRIX32 "\n", b);
  uint32_t* size = Memory(b);
  printf("Size was %" PRIu32 "\n", *size);
  if ((a == 0) || (*size < sizeof(API(DPSESSIONDESC2)))) {
    *size = sizeof(API(DPSESSIONDESC2));
    eax = 0x8877001E; // probably DPERR_BUFFERTOOSMALL, I just copied this from disassembly
  } else {
    memcpy(Memory(a), &globalDesc, sizeof(API(DPSESSIONDESC2)));
    *size = sizeof(API(DPSESSIONDESC2));
    eax = 0;
  }
  printf("Size is %" PRIu32 "\n", *size);
  esp += 3 * 4;
HACKY_COM_END()

static void copySession(API(DPSESSIONDESC2)* dest, API(DPSESSIONDESC2)* src, bool copy_strings) {
  memcpy(dest, src, sizeof(API(DPSESSIONDESC2)));
  if (copy_strings) {
    // Fixup pointers:
    dest->lpszSessionName = Allocate(128);
    strcpy_ucs2(Memory(dest->lpszSessionName), Memory(src->lpszSessionName));
    if (dest->lpszPassword != 0) {
      dest->lpszPassword = Allocate(128);
      strcpy_ucs2(Memory(dest->lpszPassword), Memory(src->lpszPassword));
    }
  }
}

// IDirectPlay4 -> STDMETHOD(SetSessionDesc)       (THIS_ LPDPSESSIONDESC2,DWORD) PURE; // 31
HACKY_COM_BEGIN(IDirectPlay4, 31)
  hacky_printf("SetSessionDesc\n");
  uint32_t a = stack[2];
  uint32_t b = stack[3];
  hacky_printf("p 0x%" PRIX32 "\n", stack[1]);
  hacky_printf("a 0x%" PRIX32 "\n", a);
  hacky_printf("b 0x%" PRIX32 "\n", b);
  API(DPSESSIONDESC2)* desc = Memory(a);

  copySession(&globalDesc, Memory(a), true);

  eax = 0;
  esp += 3 * 4;
HACKY_COM_END()


// IDirectPlay4 -> STDMETHOD(Open)                 (THIS_ LPAPI(DPSESSIONDESC2),DWORD) PURE; // 24
HACKY_COM_BEGIN(IDirectPlay4, 24)
  hacky_printf("Open\n");
  uint32_t a = stack[2];
  uint32_t b = stack[3];
  hacky_printf("p 0x%" PRIX32 "\n", stack[1]);
  hacky_printf("a 0x%" PRIX32 "\n", a);
  hacky_printf("b 0x%" PRIX32 "\n", b);

enum {
  API(DPOPEN_JOIN) =                 0x00000001,
  API(DPOPEN_CREATE) =               0x00000002
};

  if (b == API(DPOPEN_JOIN)) {
    //FIXME: Set session to "good" values?
  } else if (b == API(DPOPEN_CREATE)) {
    copySession(&globalDesc, Memory(a), true);
  } else {
    assert(false);
  }

  eax = 0;
  esp += 3 * 4;
HACKY_COM_END()

// IDirectPlay4 -> STDMETHOD(Receive)              (THIS_ LPDPID,LPDPID,DWORD,LPVOID,LPDWORD) PURE; // 25
HACKY_COM_BEGIN(IDirectPlay4, 25)
  hacky_printf("Receive\n");
  uint32_t a = stack[2];
  uint32_t b = stack[3];
  uint32_t c = stack[4];
  uint32_t d = stack[5];
  uint32_t e = stack[6];
  hacky_printf("p 0x%" PRIX32 "\n", stack[1]);
  hacky_printf("a 0x%" PRIX32 "\n", a);
  hacky_printf("b 0x%" PRIX32 "\n", b);
  hacky_printf("c 0x%" PRIX32 "\n", c);
  hacky_printf("d 0x%" PRIX32 "\n", d);
  hacky_printf("e 0x%" PRIX32 "\n", e);
  eax = 0x887700BE; // probably DPERR_NOMESSAGES
  esp += 6 * 4;
HACKY_COM_END()

// IDirectPlay4 -> STDMETHOD(Send)                 (THIS_ DPID, DPID, DWORD, LPVOID, DWORD) PURE; // 26
HACKY_COM_BEGIN(IDirectPlay4, 26)
  hacky_printf("Send\n");
  uint32_t a = stack[2];
  uint32_t b = stack[3];
  uint32_t c = stack[4];
  uint32_t d = stack[5];
  uint32_t e = stack[6];
  hacky_printf("p 0x%" PRIX32 "\n", stack[1]);
  hacky_printf("a 0x%" PRIX32 "\n", a);
  hacky_printf("b 0x%" PRIX32 "\n", b);
  hacky_printf("c 0x%" PRIX32 "\n", c);
  hacky_printf("d 0x%" PRIX32 "\n", d);
  hacky_printf("e 0x%" PRIX32 "\n", e);
  eax = 0;
  esp += 6 * 4;
HACKY_COM_END()

// IDirectPlay4 -> STDMETHOD(EnumConnections)     	(THIS_ LPCGUID,LPDPENUMCONNECTIONSCALLBACK,LPVOID,DWORD) PURE; // 35
HACKY_COM_BEGIN(IDirectPlay4, 35)
  hacky_printf("EnumConnections\n");
  uint32_t a = stack[2];
  uint32_t b = stack[3];
  uint32_t c = stack[4];
  uint32_t d = stack[5];
  hacky_printf("p 0x%" PRIX32 "\n", stack[1]);
  hacky_printf("a 0x%" PRIX32 "\n", a);
  hacky_printf("b 0x%" PRIX32 "\n", b);
  hacky_printf("c 0x%" PRIX32 "\n", c);
  hacky_printf("d 0x%" PRIX32 "\n", d);
  eax = 0;
  esp += 5 * 4;

  // Push a call to the callback onto the stack.. this is some ugly hack..
#ifdef DPLAY_ENET
  esp -= 4;
  *(uint32_t*)Memory(esp) = returnAddress; // Return where this was supposed to return to
  {
    esp -= 4;
    *(uint32_t*)Memory(esp) = c; // lpContext

    esp -= 4;
    *(uint32_t*)Memory(esp) = 0; // dwFlags

    esp -= 4;
    Address lpNameAddr = Allocate(sizeof(API(DPNAME)));
    API(DPNAME)* lpName = Memory(lpNameAddr);
    memset(lpName, 0x00, sizeof(API(DPNAME)));
    lpName->dwSize = sizeof(API(DPNAME));
    Address str = Allocate(128);
    sprintf_ucs2(Memory(str), "OpenSWE1R Connection");
    lpName->lpszShortName = str;
    lpName->lpszLongName = str;
    *(uint32_t*)Memory(esp) = lpNameAddr; // lpName

    esp -= 4;
    *(uint32_t*)Memory(esp) = 0; // dwConnectionSize

    esp -= 4;
    Address lpConnection = Allocate(128);
    memset(Memory(lpConnection), 0x00, 128);
    *(uint32_t*)Memory(esp) = lpConnection; // lpConnection

    //FIXME: Use proper GUID
    Address lpguidSP = Allocate(sizeof(API(IID)));
    API(IID)* iid = Memory(lpguidSP);

    // DPSPGUID_IPX = {685BC400-9D2C-11cf-A9CD-00AA006886E3}
    iid->Data1 = 0x685BC400;
    iid->Data2 = 0x9D2C;
    iid->Data3 = 0x11CF;
    iid->Data4[0] = 0xA9;
    iid->Data4[1] = 0xCD;
    iid->Data4[2] = 0x00;
    iid->Data4[3] = 0xAA;
    iid->Data4[4] = 0x00;
    iid->Data4[5] = 0x68;
    iid->Data4[6] = 0x86;
    iid->Data4[7] = 0xE3;

    esp -= 4;
    *(uint32_t*)Memory(esp) = lpguidSP; // lpguidSP

    // Emulate the call
    esp -= 4;
    *(uint32_t*)Memory(esp) = clearEax;
    eip = b;
    printf("  Callback at 0x%" PRIX32 "\n", eip);
    //FIXME: Add a hook which returns 0
  }
#endif
HACKY_COM_END()

// IDirectPlay4 -> STDMETHOD(InitializeConnection) (THIS_ LPVOID,DWORD) PURE; // 38
HACKY_COM_BEGIN(IDirectPlay4, 38)
  hacky_printf("InitializeConnection\n");
  uint32_t a = stack[2];
  uint32_t b = stack[3];
  hacky_printf("p 0x%" PRIX32 "\n", stack[1]);
  hacky_printf("a 0x%" PRIX32 "\n", a);
  hacky_printf("b 0x%" PRIX32 "\n", b);

#ifdef DPLAY_ENET
  if (enet_initialize () != 0) {
    fprintf (stderr, "An error occurred while initializing ENet.\n");
    assert(false);
  }
  atexit (enet_deinitialize);
#endif

  eax = 0;
  esp += 3 * 4;
HACKY_COM_END()

