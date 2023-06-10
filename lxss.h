#pragma once

#include <initguid.h>

DEFINE_GUID(lxGuid, 0x4F476546, 0xB412, 0x4579, 0xB6, 0x4C, 0x12, 0x3D, 0xF3, 0x31, 0xE3, 0xD6);
DEFINE_GUID(lxSessionGuid, 0x536A6BCF, 0xFE04, 0x41D9, 0xB9, 0x78, 0xDC, 0xAC, 0xA9, 0xA9, 0xB5, 0xB9);
DEFINE_GUID(lxInstanceGuid, 0x8F9E8123, 0x58D4, 0x484A, 0xAC, 0x25, 0x7E, 0xF7, 0xD5, 0xF7, 0x44, 0x8F);

//should be enum.
typedef int _LxssDistributionState;
typedef struct {
} _LXSS_STD_HANDLE_TYPES;

MIDL_INTERFACE("536A6BCF-FE04-41D9-B978-DCACA9A9B5B9")
ILxssUserSession : public IUnknown
{
	virtual HRESULT STDMETHODCALLTYPE CreateInstance(_GUID const *, DWORD idx,  _GUID const &,void * *) = 0;
	virtual HRESULT STDMETHODCALLTYPE RegisterDistribution(USHORT const *,DWORD,USHORT const *,USHORT const *,_GUID *) = 0;
	virtual HRESULT STDMETHODCALLTYPE GetDistributionId(USHORT const *,DWORD,_GUID *) = 0;
	virtual HRESULT STDMETHODCALLTYPE TerminateDistribution(_GUID const *)=0;
	virtual HRESULT STDMETHODCALLTYPE UnregisterDistribution(_GUID const *) = 0;
	virtual HRESULT STDMETHODCALLTYPE ConfigureDistribution(_GUID const *,char const *,DWORD,DWORD,char const * *,DWORD) = 0;
	virtual HRESULT STDMETHODCALLTYPE GetDistributionConfiguration(_GUID const *,USHORT * *,DWORD *,USHORT * *,char * *,DWORD *,DWORD *,char * * *,DWORD *) = 0;
	virtual HRESULT STDMETHODCALLTYPE GetDefaultDistribution(_GUID *) = 0;
	virtual HRESULT STDMETHODCALLTYPE SetDefaultDistribution(_GUID const *) = 0;
	virtual HRESULT STDMETHODCALLTYPE EnumerateDistributions(DWORD,DWORD *,_GUID * *) = 0;
	virtual HRESULT STDMETHODCALLTYPE CreateLxProcess(_GUID const *, char const *, DWORD, char const * *, USHORT const *, USHORT const *, USHORT *, DWORD, USHORT const *, short, short, DWORD, _LXSS_STD_HANDLE_TYPES *, _GUID *, void * *, void * *, void * *, void * *)=0;
	virtual HRESULT STDMETHODCALLTYPE BeginUpgradeDistribution(_GUID const *, DWORD *, USHORT * *)=0;
	virtual HRESULT STDMETHODCALLTYPE FinishUpgradeDistribution(_GUID const *, DWORD)=0;
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
	virtual HRESULT STDMETHODCALLTYPE RegisterAdssBusServer(char const *, void**) = 0;
	virtual HRESULT STDMETHODCALLTYPE GetId(_GUID *) = 0;
	virtual HRESULT STDMETHODCALLTYPE GetDistributionId(_GUID *) = 0;
	virtual HRESULT STDMETHODCALLTYPE CreateLxProcess(
		char const * exec_path, unsigned long argc, char const * * argv,
		LPCOLESTR cwd, LPCOLESTR search_paths, LPOLESTR envs,
		DWORD sync_io, _LXSS_STD_HANDLES * hds,
		DWORD user_id, BSTR xx, void**, void**) = 0;
	virtual HRESULT STDMETHODCALLTYPE ConnectAdssBusServer(char const *, void**) = 0;
};
/*
::RegisterAdssBusServer(char const *,void * *)
::GetId(_GUID *)
::GetDistributionId(_GUID *)
::CreateLxProcess(char const *,ulong,char const * *,
ushort const * cwd,ushort const * s,ushort * env,
ulong,_LXSS_STD_HANDLES *,ulong,ushort const *,void * *,void * *)
::ConnectAdssBusServer(char const *,void * *)
*/