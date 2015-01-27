#pragma once

#define HOOKAPI __declspec(dllexport)

#define UWM_CHAR_MSG      _T("UWM_CHAR_MSG-623C8B6B-5A5B-45d5-8EA0-666FCC47D7C8")
#define UWM_SETFOCUS_MSG  _T("UWM_SETFOCUS_MSG-9022D43D-378B-46b3-8777-77852A3142E2")
#define UWM_KILLFOCUS_MSG _T("UWM_KILLFOCUS_MSG-AD478CDB-9DF7-4117-9529-97AA0849D555")
#define UWM_KEYUP_MSG     _T("UWM_KEYUP_MSG-FBC47B15-B3CD-4d02-890B-79F7D112D388")
#define UWM_KEYDOWN_MSG   _T("UWM_KEYDOWN_MSG-03EE20AF-23B8-4fe9-B375-5699EE0F0387")

#define VK_HOOK_ON  0xD8
#define VK_HOOK_OFF 0xD9

extern "C"
{
	HOOKAPI BOOL SetHook(HWND hWnd);
	HOOKAPI BOOL UnSetHook();
}


//////////////////////////////////////////////////////////////////////////
// для отладки
//
#if _DEBUG

#include <stdio.h>

#define MYDEBUGMSG(...)                                    \
{                                                          \
	FILE *hFile;                                            \
	hFile = _tfopen(_T("c:\\hookout.txt"), _T("a"));        \
	SYSTEMTIME st;                                          \
	ZeroMemory(&st, sizeof(st));                            \
	GetLocalTime(&st);                                      \
	_ftprintf(hFile,                                        \
	TEXT("[%02i.%02i.%04i %02i:%02i.%02i] "),               \
	st.wDay,                                                \
	st.wMonth,                                              \
	st.wYear,                                               \
	st.wHour,                                               \
	st.wMinute,                                             \
	st.wSecond);                                            \
	_ftprintf(hFile, __VA_ARGS__);                          \
	_ftprintf(hFile, _T("\n"));                             \
	fclose(hFile);                                          \
}

#else

#define MYDEBUGMSG(...)

#endif