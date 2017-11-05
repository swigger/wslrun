#pragma once

HRESULT AdssBusClientpIoctl(HANDLE a1, ULONG iocode, const void* input, size_t ilen, void * output, size_t olen);
HRESULT AdssBusClientReceiveMessageAsync(HANDLE hf, void *buf, ULONG bflen, DWORD *a4, PIO_STATUS_BLOCK iob, HANDLE hEvent);
bool read_file_all(HANDLE hf, string & rdo);

wstring a2w(const char* src, size_t len = -1);
string w2a(const wchar_t * src, size_t len = -1);
extern "C" DWORD get_console_handle();


class CAnsiConsole
{
	int m_st;
	DWORD m_oldin, m_oldout;
	DWORD m_cpin, m_cpout;
public:
	CAnsiConsole();
	~CAnsiConsole();
	bool isOK()
	{
		return m_st == 3;
	}
};
