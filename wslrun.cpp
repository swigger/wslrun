// wslrun.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "lxss.h"
#include "adss.h"
#include "util.h"
#include "childrun.h"


void adss_process_msg(HANDLE hdmsg, string & rdo)
{
	//dump_hex(0, rdo.data(), rdo.length(), stdout);
	if (rdo.size() < sizeof(adss_msg)) return;
	adss_msg * m = (adss_msg*)rdo.data();

	string cmdline;
	const char * argv = rdo.data() + m->args_off;
	for (int i = 0; i < m->argc; ++i)
	{
		if (strpbrk(argv, " \r\n\t"))
		{
			cmdline += '"';
			cmdline += argv;
			cmdline += "\" ";
		}
		else
		{
			cmdline += argv;
			cmdline += " ";
		}
		argv += strlen(argv) + 1;
	}
	if (!cmdline.empty()) cmdline.resize(cmdline.size() - 1);
	adss_msg_out mo = { 0 };
	wstring wcmd = a2w(cmdline.c_str(), cmdline.length());
	wstring wdir = a2w(rdo.c_str() + m->dir_off);
	wstring wexe = a2w(rdo.c_str() + m->exename_off);
	HANDLE ios[3];
	memset(ios, 0xff, sizeof(ios));

	int mark_fail = 0;
	for (int i = 0; i < 3; ++i)
	{
		HRESULT hr =  AdssBusClientpIoctl(hdmsg, IOCTL_ADSS_IPC_CONNECTION_UNMARSHAL_VFS_FILE/*0x2200BB*/, 
			&m->hds[i], sizeof(m->hds[i]), &ios[i], sizeof(ios[i]));
		if (FAILED(hr))
		{
			mark_fail = 1;
			break;
		}
		SetHandleInformation(ios[i], HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);
	}

	PROCESS_INFORMATION pinfo = { 0 };
	STARTUPINFO sinfo = { sizeof(sinfo) };

	if (mark_fail)
		mo.errno_ = EIO; //input&output error.
	else
	{
		//launch the process.
		sinfo.dwFlags = STARTF_USESTDHANDLES;
		sinfo.hStdInput = ios[0];
		sinfo.hStdOutput = ios[1];
		sinfo.hStdError = ios[2];
		//sinfo.lpDesktop = L"winsta0\\default";
		LPCWSTR dir = wdir.c_str();
		if (!*dir) dir = 0;
		PROC_THREAD_ATTRIBUTE_GROUP_AFFINITY;
		BOOL b = CreateProcess(wexe.c_str(), &wcmd[0], NULL, NULL, TRUE, CREATE_UNICODE_ENVIRONMENT, 0, dir, &sinfo, &pinfo);
		if (b)
		{
			mo.errno_ = 0;
			AdssBusClientpIoctl(hdmsg, IOCTL_ADSS_IPC_CONNECTION_MARSHAL_PID/* 0x220097 */, 
				&pinfo.hProcess, 8, &mo.fd, 8);
		}
		else
		{
			switch (GetLastError())
			{
			case ERROR_FILE_NOT_FOUND:
				mo.errno_ = ENOENT;
				break;
			case ERROR_ELEVATION_REQUIRED:
				mo.errno_ = EACCES; //#define EACCES      13  /* Permission denied */
				break;
			default:
				mo.errno_ = 22; //#define EINVAL      22
				break;
			}
		}
	}

	LARGE_INTEGER bo;
	IO_STATUS_BLOCK iob = { 0 };
	bo.QuadPart = 0;
	NtWriteFile(hdmsg, NULL, NULL, NULL, &iob, &mo, sizeof(mo), &bo, 0);
	if (mo.errno_ == 0)
	{
		string rdo1;
		read_file_all(hdmsg, rdo1);
		//assert(rdo1.length() == 32);
	}

	for (int i = 0; i < 3; ++i)
	{
		if (ios[i] != INVALID_HANDLE_VALUE)
			CloseHandle(ios[i]);
	}
	if (pinfo.hProcess != INVALID_HANDLE_VALUE) CloseHandle(pinfo.hProcess);
	if (pinfo.hThread != INVALID_HANDLE_VALUE) CloseHandle(pinfo.hThread);
}

uint32_t __stdcall adss_msg_io(void * v)
{
	HRESULT hr;
	HANDLE hdmsg = (HANDLE)v;
	for (;;)
	{
		uint32_t dw = -1; //timeout, handle value.
		hr = AdssBusClientpIoctl(hdmsg, IOCTL_ADSS_IPC_SERVER_WAIT_FOR_CONNECTION, &dw, sizeof(dw), &dw, sizeof(dw));
		if (FAILED(hr)) break;
		string rdo;
		if (read_file_all((HANDLE)(uintptr_t)dw, rdo))
		{
			adss_process_msg((HANDLE)(uintptr_t)dw, rdo);
		}
		CloseHandle((HANDLE)(uintptr_t)dw);
	}

	return 0;
}

static void usage()
{
	fprintf(stderr, "usage:\n"
		"wslrun [OPTIONS] [cmd [args...]]\n"
		"\nOPTIONS:\n"
		"  ~: run in home dir (home)\n"
		"  --nopath: no nt path\n"
		"  --sudo: run with root\n"
		"  --help: show this help\n");
}

static void set_hd(_LXSS_STD_HANDLE & hd, int index)
{
	HANDLE hf = INVALID_HANDLE_VALUE;
	BOOL isOK = false;
	DWORD mode;
	CONSOLE_SCREEN_BUFFER_INFO bi;

	switch (index)
	{
	case 0:
		hf = GetStdHandle(STD_INPUT_HANDLE);
		isOK = GetFileType(hf) == FILE_TYPE_CHAR && GetConsoleMode(hf, &mode);
		break;
	case 1:
		hf = GetStdHandle(STD_OUTPUT_HANDLE);
		isOK = GetFileType(hf) == FILE_TYPE_CHAR && GetConsoleScreenBufferInfo(hf, &bi);
		break;
	case 2:
		hf = GetStdHandle(STD_ERROR_HANDLE);
		isOK = GetFileType(hf) == FILE_TYPE_CHAR && GetConsoleScreenBufferInfo(hf, &bi);
		break;
	default:
		return;
	}
	if (isOK)
	{
		hd.Handle = 0;
		hd.HandleType = 0;
	}
	else
	{
		// if (hf == 0) hf = CreateFile(L"nul", index==0 ? GENERIC_READ : GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0);
		if (hf && hf != INVALID_HANDLE_VALUE)
			SetHandleInformation(hf, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);
		hd.Handle = (DWORD)(DWORD_PTR)hf;
		hd.HandleType = (index == 0) ? HANDLE_TYPE_INPUT : HANDLE_TYPE_OUTPUT;
	}
}

struct wslrun_args
{
	uint32_t keep_run;
	DWORD user_id;
	TCHAR cwd[MAX_PATH];
	WCHAR path[4096];
	WCHAR envs[4096];
	DWORD envs_sz;
	int argc;
	char argvs[8192];
};

const WCHAR * ptr_v(const WCHAR * x)
{
	if (x && !*x) return 0;
	return x;
}

int do_run(wslrun_args * args)
{
	CoInitializeEx(NULL, 0);
	CoInitializeSecurity(NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_DEFAULT, 
		SecurityDelegation, NULL, EOAC_STATIC_CLOAKING, NULL);

	CComPtr<ILxssUserSession> session;
	HRESULT rst = CoCreateInstance(lxGuid, NULL, CLSCTX_LOCAL_SERVER, __uuidof(ILxssUserSession), (void**)&session);
	if (!session)
	{
		fprintf(stderr, "failed to create lxss session. Check dev mode.\n");
		return 1;
	}

	CComPtr<ILxssInstance> inst;
	session->CreateInstance(0/*installed linux selection*/, 1/*idx*/, __uuidof(ILxssInstance), (void**)&inst);
	if (!inst)
	{
		fprintf(stderr, "failed to create lxss instance. Install at least one dist of linux plz.\n");
		return 1;
	}

	CAnsiConsole console;
	_LXSS_STD_HANDLES hds;
	_LXSS_CONSOLE_DATA cd;
	set_hd(hds.STDIN, 0);
	set_hd(hds.STDOUT, 1);
	set_hd(hds.STDERR, 2);
	cd.ConsoleHandle = get_console_handle();

	const char ** argvo = (const char**)malloc(args->argc * sizeof(char*));
	char * p = args->argvs;
	p += strlen(p) + 1;
	for (int i=0; i<args->argc; ++i)
	{
		argvo[i] = p;
		p += strlen(p) + 1;
	}

	DWORD sync_io = 1;
	DWORD oblow=0, obmsg=0;
	HRESULT hr = inst->CreateLxProcess(args->argvs, args->argc, argvo, 0, NULL,
		ptr_v(args->cwd), ptr_v(args->path), ptr_v(args->envs), args->envs_sz, sync_io,
		&hds, &cd, args->user_id, &oblow, &obmsg);

	if (FAILED(hr))
	{
		fprintf(stderr, "failed to create lxprocess, error %#x\n", hr);
		return 1;
	}

	SetHandleInformation((HANDLE)(DWORD_PTR)oblow, HANDLE_FLAG_INHERIT, 0);
	if (obmsg)
	{
		SetHandleInformation((HANDLE)(DWORD_PTR)obmsg, HANDLE_FLAG_INHERIT, 0);
		HANDLE ht = (HANDLE) _beginthreadex(NULL, 0, adss_msg_io, (void*)(uintptr_t)obmsg, 0, 0);
		(void)ht;
		CloseHandle(ht);
	}

	//wait for process exit.
	if (sync_io)
	{
		// sync_io mode, the created lxss process will take care of IO, make it more efficent to communicate with native processes' IO pipes.
		// and we can take the nt pid.
		// DWORD ntpid = 0;
		// AdssBusClientpIoctl((HANDLE)(uintptr_t)oblow, IOCTL_ADSS_LX_PROCESS_HANDLE_GET_NT_PID, 0, 0, &ntpid, sizeof(ntpid));
	}
	DWORD dw = -1;
	AdssBusClientpIoctl((HANDLE)(uintptr_t)oblow, IOCTL_ADSS_LX_PROCESS_HANDLE_WAIT_FOR_SIGNAL, &dw, sizeof(dw), &dw, sizeof(dw));

	if (args->keep_run)
	{
		for (;;)
			Sleep(86400000);
	}

    return dw >> 8;
}


int main(int argc, const char ** argv)
{
	CHILDRUN_HOOK_MAIN();

	wslrun_args args;
	memset(&args, 0, sizeof(args));
	args.user_id = -1;
	GetCurrentDirectory(_countof(args.cwd), args.cwd) || (args.cwd[0] = 0);
	ExpandEnvironmentStrings(L"%PATH%", args.path, _countof(args.path)) || (args.path[0] = 0);
	
	LPCWSTR ss = GetEnvironmentStrings();
	wstring envs;
	while (*ss)
	{
		envs += ss;
		envs += L'\0';
		ss += wcslen(ss) + 1;
	}
	envs += L'\0';
	if (envs.size() > _countof(args.envs))
	{
		fprintf(stderr, "internal error: buffer too small\n");
		return -1;
	}
	memcpy(args.envs, envs.data(), envs.length() * 2);
	args.envs_sz = envs.length();

	int ecase = 0;
	int i = 1;
	bool daemon = 0;
	for (; i < argc; ++i)
	{
		if (strcmp(argv[i], "~") == 0)
		{
			args.cwd[0] = 0;
		}
		else if (strcmp(argv[i], "--sudo") == 0 || strcmp(argv[i], "--root") == 0)
		{
			args.user_id = 0;
		}
		else if (strcmp(argv[i], "--keep") == 0)
		{
			args.keep_run = 1;
		}
		else if (strcmp(argv[i], "--nopath") == 0)
		{
			args.path[0] = 0;
		}
		else if (strcmp(argv[i], "--daemon") == 0)
		{
			daemon = 1;
		}
		else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "-?") == 0)
		{
			return usage(), 1;
		}
		else if (strcmp(argv[i], "--") == 0)
		{
			++i;
			break;
		}
		else if (argv[i][0] == '-')
		{
			fprintf(stderr, "Unknown option: %s\n", argv[i]);
			return 1;
		}
		else
			break;
	}

	//TODO: check size...
	char * p = args.argvs;
	if (i < argc)
	{
		strcpy(p, argv[i]); p += strlen(argv[i]) + 1;
		for (int j = i; j < argc; ++j)
		{
			strcpy(p, argv[j]); p += strlen(argv[j]) + 1;
		}
		args.argc = argc - i;
	}
	else
	{
		strcpy(p, "/bin/bash"); p += strlen(p) + 1;
		strcpy(p, "-bash"); p += strlen(p) + 1;
		args.argc = 1;
	}

	if (daemon)
		return childrun(do_run, &args);
	else
		return do_run(&args);
}
