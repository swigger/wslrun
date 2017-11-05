#include "stdafx.h"
#include "childrun.h"

#ifdef _WIN32
#include <ndk/ntndk.h>
#pragma comment(lib, "ntdll.lib")

void * g_xblock = 0, *g_xblock2=0;
int (*g_xmain)(void*) = 0;
static string g_hooks;

extern "C" IMAGE_DOS_HEADER __ImageBase;

bool childrun_addhook_i(void(*f)(const char*, size_t), const char* str, size_t slen)
{
	if (!f) return false;
	string o1;
	unsigned int exlen = (unsigned int) slen;
	unsigned int padlen = (exlen + 7) & ~7;

	uint64_t zero = 0;
	o1.append((char*)&padlen, sizeof(padlen));
	o1.append((char*)&f, sizeof(void*));
	o1.append(str, exlen);
	o1.append((char*)&zero, padlen - exlen);
	g_hooks += o1;
	return true;
}

void childrun_runhooks()
{
	char * ptr = (char*)g_xblock2;
	if (!ptr) return;
	for (;;)
	{
		unsigned int len = *(unsigned int*)ptr;
		if (len == 0) break;
		ptr += sizeof(len);

		void(*f)(const char*, size_t);
		(void*&)f = *(void**)ptr;
		ptr += sizeof(void*);

		f(ptr, len);
		ptr += len;
	}
}

int childrun(int (*f)(void*), void * mem, int len)
{
	wchar_t thisname[260];
	STARTUPINFO sinfo = {sizeof(sinfo)};
	PROCESS_INFORMATION pinfo = {0};
	sinfo.dwFlags = STARTF_USESHOWWINDOW;
	sinfo.wShowWindow = SW_HIDE;
	GetModuleFileName(0, thisname, _countof(thisname));
	wstring cmdline(GetCommandLineW());
	BOOL b = CreateProcess(thisname, (LPWSTR)cmdline.c_str(), 0, 0, TRUE, CREATE_SUSPENDED|CREATE_NEW_CONSOLE, 0, 0, &sinfo, &pinfo);
	if (!b) return -1;

	ULONG rlen = 0;
	SIZE_T rlen1 = 0;
	PVOID ImageBase = 0;
	PROCESS_BASIC_INFORMATION pbi;
	if (ZwQueryInformationProcess(pinfo.hProcess, ProcessBasicInformation, &pbi, sizeof(pbi), &rlen) == 0 &&
		ReadProcessMemory(pinfo.hProcess, (char*)pbi.PebBaseAddress + offsetof(_PEB, ImageBaseAddress), &ImageBase, sizeof(ImageBase), &rlen1))
	{
		string hooks = g_hooks;
		unsigned int zero = 0;
		hooks.append((char*)&zero, sizeof(zero));

#define rp(v) (((char*)v)-(uintptr_t)&__ImageBase + (uintptr_t)ImageBase)
		char * pmem = (char*)VirtualAllocEx(pinfo.hProcess, 0, len + hooks.length(), MEM_COMMIT, PAGE_READWRITE);
		char * pmem2 = pmem + len;
		WriteProcessMemory(pinfo.hProcess, pmem, mem, len, 0);
		WriteProcessMemory(pinfo.hProcess, pmem2, hooks.data(), hooks.length(), 0);

		WriteProcessMemory(pinfo.hProcess, rp(&g_xmain), &f, sizeof(f), 0);
		WriteProcessMemory(pinfo.hProcess, rp(&g_xblock), &pmem, sizeof(pmem), 0);
		WriteProcessMemory(pinfo.hProcess, rp(&g_xblock2), &pmem2, sizeof(pmem2), 0);

		ResumeThread(pinfo.hThread);
		CloseHandle(pinfo.hProcess);
		CloseHandle(pinfo.hThread);
		return (int)pinfo.dwProcessId;
	}
	else
	{
		TerminateProcess(pinfo.hProcess, 1);
		CloseHandle(pinfo.hProcess);
		CloseHandle(pinfo.hThread);
		return -1;
	}
}
#else
static void close_io()
{
	const char * fn = "/dev/null";
	int oid = open(fn, O_WRONLY);
	int iid = open(fn, O_RDONLY);
	dup2(iid, 0);
	dup2(oid, 1);
	dup2(oid, 2);
	close(oid);
	close(iid);
}

int childrun(int (*f)(void*), void * mem, int len)
{
	int ix = fork();
	if (ix == 0)
	{
		//daemon(1, 0);
		close_io();
		int v = f(mem);
		exit(v);
		return 0;
	}
	return ix;
}
#endif
