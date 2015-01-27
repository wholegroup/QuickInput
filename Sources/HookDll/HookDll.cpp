#include "stdafx.h"
#include "HookDll.h"

// #define NOTWORK

#define PROGRAM_NAME "Quick Input"
#define SHMEMSIZE    2048

#pragma data_seg(".WholeGroupQI")
HHOOK  hMsgHook1    = NULL;  // хендл кука 1
HHOOK  hMsgHook2    = NULL;  // хендл кука 2
HHOOK  hMsgHook3    = NULL;  // хендл кука 3
HWND   hParentWnd   = NULL;  // хендл родительского окна
UINT   uiNumMsg     = 0;     // от 1 до 64
BOOL   bPause       = FALSE; // если ЛОЖЬ, то обработка сообщений выключена
#pragma data_seg()
#pragma comment(linker, "/SECTION:.WholeGroupQI,RWS")

#ifdef _MANAGED
#pragma managed(push, off)
#endif

HINSTANCE hInstance = NULL;  // хендл контекста
UINT      UWM_CHAR;          // ИД сообщения
UINT      UWM_SETFOCUS;      // ИД сообщения
UINT      UWM_KILLFOCUS;     // ИД сообщения
UINT      UWM_KEYUP;         // ИД сообщения
UINT      UWM_KEYDOWN;       // ИД сообщения
HANDLE    hSMemory  = NULL;  // хендл шаредмемори
LPVOID    pSMemory  = NULL;  // указатель на шаредмемори


LRESULT CALLBACK GetMsgProc(INT iCode, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK CallWndProc(INT iCode, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WndProc(INT iCode, WPARAM wParam, LPARAM lParam);


BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
	switch(dwReason)
	{
		case DLL_PROCESS_ATTACH:
			hInstance     = (HINSTANCE)hModule;
			UWM_CHAR      = RegisterWindowMessage(UWM_CHAR_MSG);
			UWM_SETFOCUS  = RegisterWindowMessage(UWM_SETFOCUS_MSG);
			UWM_KILLFOCUS = RegisterWindowMessage(UWM_KILLFOCUS_MSG);
			UWM_KEYUP     = RegisterWindowMessage(UWM_KEYUP_MSG);
			UWM_KEYDOWN   = RegisterWindowMessage(UWM_KEYDOWN_MSG);
			break;

		case DLL_PROCESS_DETACH:
			break;
	}
		
	return TRUE;
}


HOOKAPI BOOL SetHook(HWND hWnd)
{
	// если хук создан
	if (NULL != hParentWnd)
	{
		return FALSE;
	}

	// если не указан хендл окна, куда отправлять сообщения
	if (NULL == hWnd)
	{
		return FALSE;
	}

	// установка хука
	hMsgHook1 = SetWindowsHookEx(WH_GETMESSAGE, (HOOKPROC)GetMsgProc, hInstance, 0);
	if (NULL == hMsgHook1)
	{
		return FALSE;
	}

	hMsgHook2 = SetWindowsHookEx(WH_CALLWNDPROC, (HOOKPROC)CallWndProc, hInstance, 0);
	if (NULL == hMsgHook2)
	{
		return FALSE;
	}

	hParentWnd = hWnd;

	return TRUE;
}


HOOKAPI BOOL UnSetHook()
{
	if (NULL != hMsgHook1)
	{
		UnhookWindowsHookEx(hMsgHook1);
	}

	if (NULL != hMsgHook2)
	{
		UnhookWindowsHookEx(hMsgHook2);
	}

	hMsgHook1  = NULL;
	hMsgHook2  = NULL;
	hParentWnd = NULL;

	return TRUE;
}


LRESULT CALLBACK GetMsgProc(INT iCode, WPARAM wParam, LPARAM lParam)
{
	#ifdef NOTWORK
	return CallNextHookEx(hMsgHook1, iCode, wParam, lParam);
	#endif

	// обработка
	if (HC_ACTION == iCode)
	{
		LPMSG lpMsg = (LPMSG)lParam;

		if ((WM_KEYUP != lpMsg->message) && (WM_KEYDOWN != lpMsg->message) && (WM_CHAR != lpMsg->message))
		{
			return CallNextHookEx(hMsgHook1, iCode, wParam, lParam);
		}

		// проверка флагов - включить/выключить обработку хуком
		if (VK_HOOK_OFF == lpMsg->wParam)
		{
			bPause = TRUE;
			lpMsg->message = WM_NULL;
		}
		else
		if (VK_HOOK_ON == lpMsg->wParam)
		{
			bPause = FALSE;
			lpMsg->message = WM_NULL;
		}

		// выходим, если обработка выключена
		if (TRUE == bPause)
		{
			return CallNextHookEx(hMsgHook1, iCode, wParam, lParam);
		}

		// обработка WM_KEYDOWN
		if (WM_KEYDOWN == lpMsg->message)
		{
			// блокировка клавишей
			if ((VK_RETURN == lpMsg->wParam) || 
				(VK_TAB     == lpMsg->wParam) || 
				(VK_UP      == lpMsg->wParam) || 
				(VK_DOWN    == lpMsg->wParam) ||
				(VK_SPACE   == lpMsg->wParam) ||
				(VK_ESCAPE  == lpMsg->wParam))
			{
				if ((PM_REMOVE != wParam))
				{
					lpMsg->message = WM_NULL;
				}
				else
				{
					LRESULT lResult = SendMessage(hParentWnd, UWM_KEYDOWN, (WPARAM)lpMsg->hwnd, (LPARAM)lpMsg->wParam);
					if (FALSE == lResult)
					{
						lpMsg->message = WM_NULL;
					}
				}
			}
		}
		else

		// обработка WM_KEYUP
		if (WM_KEYUP == lpMsg->message)
		{
			if ((VK_RETURN == lpMsg->wParam) || 
				(VK_TAB     == lpMsg->wParam) || 
				(VK_UP      == lpMsg->wParam) || 
				(VK_DOWN    == lpMsg->wParam) ||
				(VK_SPACE   == lpMsg->wParam) ||
				(VK_ESCAPE  == lpMsg->wParam))
			{
				lpMsg->message = WM_NULL;
			}
			else
			{
				LRESULT lResult = SendMessage(hParentWnd, UWM_KEYUP, (WPARAM)lpMsg->hwnd, (LPARAM)lpMsg->wParam);
				if (FALSE == lResult)
				{
					lpMsg->message = WM_NULL;
				}
			}
		}
		else

		// обработка WM_CHAR
		if ((WM_CHAR == lpMsg->message) && (PM_REMOVE == wParam))
		{
			hSMemory = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, SHMEMSIZE, _T(PROGRAM_NAME));
			if (NULL != hSMemory)
			{
				pSMemory = MapViewOfFile(hSMemory, FILE_MAP_ALL_ACCESS, 0, 0, 0);
				if (NULL != pSMemory)
				{
					UCHAR *uiPointer = (UCHAR*)pSMemory;

					uiPointer += sizeof(HWND); // в начале шаред мемори находится хендл программы

					((LPMSG)uiPointer)[uiNumMsg++] = *lpMsg;

					SendMessage(hParentWnd, UWM_CHAR, (WPARAM)(uiNumMsg - 1), (LPARAM)lpMsg->hwnd);
					if (uiNumMsg == 64)
					{
						uiNumMsg = 0;
					}

					UnmapViewOfFile(pSMemory);
				}

				CloseHandle(hSMemory);
			}
		}
	}

	return CallNextHookEx(hMsgHook1, iCode, wParam, lParam);
}


LRESULT CALLBACK CallWndProc(INT iCode, WPARAM wParam, LPARAM lParam)
{
	#ifdef NOTWORK
	return CallNextHookEx(hMsgHook2, iCode, wParam, lParam);
	#endif

	if (HC_ACTION == iCode)
	{
		PCWPSTRUCT lpWPStruct = (PCWPSTRUCT)lParam;

		if (lParam && (WM_SETFOCUS == lpWPStruct->message))
		{
			bPause = FALSE;

			PostMessage(hParentWnd, UWM_SETFOCUS, (WPARAM)lpWPStruct->hwnd, (LPARAM)lpWPStruct->hwnd);
		}
		else

		/*
		if (lParam && (WM_KILLFOCUS == lpWPStruct->message))
		{
			PostMessage(hParentWnd, UWM_KILLFOCUS, (WPARAM)lpWPStruct->wParam, (LPARAM)lpWPStruct->lParam);
		}
		*/
		if (lParam && (WM_ACTIVATEAPP == lpWPStruct->message))
		{
			if (FALSE == lpWPStruct->wParam)
			{
				PostMessage(hParentWnd, UWM_KILLFOCUS, (WPARAM)lpWPStruct->wParam, (LPARAM)lpWPStruct->lParam);
			}
		}
	}

	return CallNextHookEx(hMsgHook2, iCode, wParam, lParam);
}

#ifdef _MANAGED
#pragma managed(pop)
#endif

