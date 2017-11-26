// Stolen from ftp://ftp.physik.hu-berlin.de/pub/useful/dx7asdk/DXF/include/dplobby.h

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
#endif

//typedef struct {
//}

//COM_INTERFACE(, sizeof() / sizeof(void*))
