#include "stdafx.h"
#include "util.h"


HRESULT AdssBusClientpIoctl(HANDLE a1, ULONG iocode, const void* input, size_t ilen, void * output, size_t olen)
{
	IO_STATUS_BLOCK iob = { 0 };
	if (!a1)
		return STATUS_DEVICE_NOT_CONNECTED;
	else
		return ZwDeviceIoControlFile(a1, NULL/*event*/, NULL /*user apc*/, NULL/*ctx*/,
			&iob, iocode, (void*)input, (ULONG)ilen, output, (ULONG)olen);
}

HRESULT AdssBusClientReceiveMessageAsync(HANDLE hf, void *buf, ULONG bflen, DWORD *a4, PIO_STATUS_BLOCK iob, HANDLE hEvent)
{
	LARGE_INTEGER loff;
	loff.QuadPart = 0;
	NTSTATUS nt = ZwReadFile(hf, hEvent, NULL/*apc*/, NULL, iob, buf, bflen, &loff, NULL);
	if (nt == 0)
		*a4 = (DWORD)iob->Information;
	return nt;
}

bool read_file_all(HANDLE hf, string & rdo)
{
	HANDLE hEvent = CreateEventEx(NULL, NULL, 0, EVENT_ALL_ACCESS);

	char buf[4096];
	HRESULT hr = 0;

	do
	{
		ULONG olen = 0;
		IO_STATUS_BLOCK iob = { 0 };
		hr = AdssBusClientReceiveMessageAsync(hf, buf, sizeof(buf), &olen, &iob, hEvent);
		if (hr == STATUS_PENDING)
		{
			WaitForSingleObject(hEvent, INFINITE);
			hr = iob.Status;
			olen = (ULONG)iob.Information;
		}
		rdo.append(buf, olen);
	} while (hr == STATUS_BUFFER_TOO_SMALL);

	CloseHandle(hEvent);
	return hr == STATUS_SUCCESS;
}


wstring a2w(const char* src, size_t len)
{
	wstring sb;
	if (len == -1)len = strlen(src);
	size_t nsz = len * 2 + 10;
	sb.resize(nsz);
	int olen = MultiByteToWideChar(CP_UTF8, 0, src, (int) len, &sb[0], (int) nsz);
	sb.resize(olen < 0 ? 0 : olen);
	return sb;
}

string w2a(const wchar_t * src, size_t len)
{
	string sa;
	if (len == -1)len = wcslen(src);
	size_t osz = len * 3 + 10;
	sa.resize(osz);
	int olen = WideCharToMultiByte(CP_UTF8, 0, src, (int)len, &sa[0], (int) osz, 0, 0);
	sa.resize(olen < 0 ? 0 : olen);
	return sa;
}

CAnsiConsole::CAnsiConsole()
{
	m_st = 0;
	m_oldin = 0;
	m_oldout = 0;

	HANDLE hi = GetStdHandle(STD_INPUT_HANDLE);
	GetConsoleMode(hi, &m_oldin);
	DWORD mode = m_oldin;
	mode &= ~(ENABLE_INSERT_MODE | ENABLE_PROCESSED_INPUT | ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT);
	mode |= ENABLE_VIRTUAL_TERMINAL_INPUT | ENABLE_WINDOW_INPUT;
	if (SetConsoleMode(hi, mode))
		m_st |= 1;

	hi = GetStdHandle(STD_OUTPUT_HANDLE);
	GetConsoleMode(hi, &m_oldout);
	mode = m_oldout;
	mode |= ENABLE_PROCESSED_OUTPUT | ENABLE_VIRTUAL_TERMINAL_PROCESSING | DISABLE_NEWLINE_AUTO_RETURN;
	DWORD mode1 = mode & ~DISABLE_NEWLINE_AUTO_RETURN;
	if (SetConsoleMode(hi, mode) || SetConsoleMode(hi, mode1))
		m_st |= 2;

	m_cpin = GetConsoleCP();
	m_cpout = GetConsoleOutputCP();
	SetConsoleCP(CP_UTF8);
	SetConsoleOutputCP(CP_UTF8);

	const wchar_t * buf = L"1033\0\0";
	ULONG no = 0;
	SetThreadPreferredUILanguages(MUI_LANGUAGE_ID, buf, &no);
}

CAnsiConsole::~CAnsiConsole()
{
	SetConsoleCP(m_cpin);
	SetConsoleOutputCP(m_cpout);
	SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), m_oldin);
	SetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), m_oldout);
}
