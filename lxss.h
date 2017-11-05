#pragma once

#include <initguid.h>

DEFINE_GUID(lxGuid, 0x4F476546, 0xB412, 0x4579, 0xB6, 0x4C, 0x12, 0x3D, 0xF3, 0x31, 0xE3, 0xD6);
DEFINE_GUID(lxSessionGuid, 0x536A6BCF, 0xFE04, 0x41D9, 0xB9, 0x78, 0xDC, 0xAC, 0xA9, 0xA9, 0xB5, 0xB9);
DEFINE_GUID(lxInstanceGuid, 0x8F9E8123, 0x58D4, 0x484A, 0xAC, 0x25, 0x7E, 0xF7, 0xD5, 0xF7, 0x44, 0x8F);

//should be enum.
typedef int _LxssDistributionState;

MIDL_INTERFACE("536A6BCF-FE04-41D9-B978-DCACA9A9B5B9")
ILxssUserSession : public IUnknown
{
	virtual HRESULT STDMETHODCALLTYPE CreateInstance(_GUID const *,_GUID const &,void * *) = 0;
	virtual HRESULT STDMETHODCALLTYPE RegisterDistribution(USHORT const *,DWORD,USHORT const *,USHORT const *,_GUID *) = 0;
	virtual HRESULT STDMETHODCALLTYPE GetDistributionId(USHORT const *,DWORD,_GUID *) = 0;
	virtual HRESULT STDMETHODCALLTYPE InitializeFileSystem(USHORT const *) = 0;
	virtual HRESULT STDMETHODCALLTYPE SetDistributionState(_GUID const *,_LxssDistributionState,_LxssDistributionState *) = 0;
	virtual HRESULT STDMETHODCALLTYPE QueryDistributionState(_GUID const *,_LxssDistributionState *) = 0;
	virtual HRESULT STDMETHODCALLTYPE UnregisterDistribution(_GUID const *) = 0;
	virtual HRESULT STDMETHODCALLTYPE ConfigureDistribution(_GUID const *,char const *,DWORD,DWORD,char const * *,DWORD) = 0;
	virtual HRESULT STDMETHODCALLTYPE GetDistributionConfiguration(_GUID const *,USHORT * *,DWORD *,USHORT * *,char * *,DWORD *,DWORD *,char * * *,DWORD *) = 0;
	virtual HRESULT STDMETHODCALLTYPE GetDefaultDistribution(_GUID *) = 0;
	virtual HRESULT STDMETHODCALLTYPE SetDefaultDistribution(_GUID const *) = 0;
	virtual HRESULT STDMETHODCALLTYPE EnumerateDistributions(DWORD,DWORD *,_GUID * *) = 0;
};

enum _LXSS_STD_HANDLE_TYPE {
	HANDLE_TYPE_NONE = 0,
	HANDLE_TYPE_INPUT = 1,
	HANDLE_TYPE_OUTPUT = 2,
};

struct _LXSS_STD_HANDLE
{
	ULONG Handle;
	ULONG HandleType;
};

struct _LXSS_STD_HANDLES
{
	_LXSS_STD_HANDLE STDIN;
	_LXSS_STD_HANDLE STDOUT;
	_LXSS_STD_HANDLE STDERR;
};

struct _LXSS_CONSOLE_DATA
{
	DWORD ConsoleHandle;
};

MIDL_INTERFACE("8F9E8123-58D4-484A-AC25-7EF7D5F7448F")
ILxssInstance : public IUnknown
{
	virtual HRESULT STDMETHODCALLTYPE RegisterAdssBusServer(char const *,DWORD *) = 0;
	virtual HRESULT STDMETHODCALLTYPE GetId(_GUID *) = 0;
	virtual HRESULT STDMETHODCALLTYPE CreateLxProcess(char const * exec_path, unsigned long argc, char const * * argv,
		unsigned long env_cnt,char const * * env, LPCOLESTR cwd, LPCOLESTR search_paths, DWORD sync_io,
		_LXSS_STD_HANDLES * hds, _LXSS_CONSOLE_DATA * data,
		DWORD user_id, DWORD * pObjProcess,DWORD * pObjMessage) = 0;
	virtual HRESULT STDMETHODCALLTYPE ConnectAdssBusServer(char const *,DWORD *) = 0;
	//virtual void vdtor(unsigned int v) = 0;
	//virtual _LxssDistributionState GetState(void) = 0;
	//virtual void Start(void) = 0;
	//virtual void Stop(void) = 0;
	//virtual void _Terminate(void) = 0;
};
