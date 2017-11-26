// Stolen from https://github.com/sekishi1259/SimpleROHook/blob/master/Injection/ProxyIDirectInput.h

#if 0

class CProxyIDirectInput7 : public IDirectInput7
{
private:
	IDirectInput7*			m_Instance;
public:
	CProxyIDirectInput7(IDirectInput7* ptr) : m_Instance(ptr) {}

	/*** IUnknown methods ***/
	STDMETHOD(QueryInterface)(THIS_ REFIID p1, LPVOID * p2) PROXY2(QueryInterface) //0
	STDMETHOD_(ULONG,AddRef) (THIS)		PROXY0(AddRef) // 1
	STDMETHOD_(ULONG,Release) (THIS)	PROXY_RELEASE // 2

	/*** IDirectInput2A methods ***/
	//
	STDMETHOD(CreateDevice)(THIS_ REFGUID rguid,LPDIRECTINPUTDEVICEA *lpIDD,LPUNKNOWN pUnkOuter) // 3
	{
		return Proxy_CreateDevice(rguid, lpIDD,pUnkOuter);
	}
	STDMETHOD(EnumDevices)(THIS_ DWORD p1,LPDIENUMDEVICESCALLBACKA p2,LPVOID p3,DWORD p4) PROXY4(EnumDevices) // 4
	STDMETHOD(GetDeviceStatus)(THIS_ REFGUID p1)                                 PROXY1(GetDeviceStatus) // 5
	STDMETHOD(RunControlPanel)(THIS_ HWND p1,DWORD p2)                           PROXY2(RunControlPanel) // 6
	STDMETHOD(Initialize)(THIS_ HINSTANCE p1,DWORD p2)                           PROXY2(Initialize) // 7
	STDMETHOD(FindDevice)(THIS_ REFGUID p1,LPCSTR p2,LPGUID p3)                  PROXY3(FindDevice) // 8

	/*** IDirectInput7A methods ***/
	STDMETHOD(CreateDeviceEx)(THIS_ REFGUID p1,REFIID p2,LPVOID *p3,LPUNKNOWN p4)        PROXY4(CreateDeviceEx) // 9

	//
	// Proxy Functions
	//
	HRESULT Proxy_CreateDevice(THIS_ REFGUID rguid,LPDIRECTINPUTDEVICEA *lpIDD,LPUNKNOWN pUnkOuter); // 10?
};
#undef CLASSNAME

#endif



#if 0


DECLARE_INTERFACE_(IDirectInputDeviceA, IUnknown)
{
    /*** IUnknown methods ***/
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID * ppvObj) PURE; // 0
    STDMETHOD_(ULONG,AddRef)(THIS) PURE; // 1
    STDMETHOD_(ULONG,Release)(THIS) PURE; // 2

    /*** IDirectInputDeviceA methods ***/
    STDMETHOD(GetCapabilities)(THIS_ LPDIDEVCAPS) PURE; // 3
    STDMETHOD(EnumObjects)(THIS_ LPDIENUMDEVICEOBJECTSCALLBACKA,LPVOID,DWORD) PURE; // 4
    STDMETHOD(GetProperty)(THIS_ REFGUID,LPDIPROPHEADER) PURE;
    STDMETHOD(SetProperty)(THIS_ REFGUID,LPCDIPROPHEADER) PURE; // 6
    STDMETHOD(Acquire)(THIS) PURE;
    STDMETHOD(Unacquire)(THIS) PURE; // 8
    STDMETHOD(GetDeviceState)(THIS_ DWORD,LPVOID) PURE;
    STDMETHOD(GetDeviceData)(THIS_ DWORD,LPDIDEVICEOBJECTDATA,LPDWORD,DWORD) PURE; // 10
    STDMETHOD(SetDataFormat)(THIS_ LPCDIDATAFORMAT) PURE;
    STDMETHOD(SetEventNotification)(THIS_ HANDLE) PURE; // 12
    STDMETHOD(SetCooperativeLevel)(THIS_ HWND,DWORD) PURE;
    STDMETHOD(GetObjectInfo)(THIS_ LPDIDEVICEOBJECTINSTANCEA,DWORD,DWORD) PURE; // 14
    STDMETHOD(GetDeviceInfo)(THIS_ LPDIDEVICEINSTANCEA) PURE;
    STDMETHOD(RunControlPanel)(THIS_ HWND,DWORD) PURE; // 16
    STDMETHOD(Initialize)(THIS_ HINSTANCE,DWORD,REFGUID) PURE;
};


#endif
