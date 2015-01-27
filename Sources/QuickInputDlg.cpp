/*
 * Copyright (C) 2015 Andrey Rychkov <wholegroup@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "stdafx.h"
#include "resource.h"
#include "QuickInput.h"
#include "HyperLink.h"
#include "DialogName.h"
#include "DialogList.h"
#include "ListBoxWords.h"
#include "ListWords.h"
#include "DlgColors.h"
#include "DlgPrograms.h"
#include "DlgAbout.h"
#include "DlgOptions.h"
#include "DlgEditDict.h"
#include "ClipboardBackup.h"
#include "QuickInputDlg.h"
#include "HookDll/HookDll.h"
#include <psapi.h>
#include <locale.h>

#ifdef _DEBUG
	#define new DEBUG_NEW
#undef THIS_FILE
	static char THIS_FILE[] = __FILE__;
#endif

static UINT UWM_CHAR          = ::RegisterWindowMessage(UWM_CHAR_MSG);
static UINT UWM_SETFOCUS      = ::RegisterWindowMessage(UWM_SETFOCUS_MSG);
static UINT UWM_KILLFOCUS     = ::RegisterWindowMessage(UWM_KILLFOCUS_MSG);
static UINT UWM_KEYDOWN       = ::RegisterWindowMessage(UWM_KEYDOWN_MSG);
static UINT UWM_KEYUP         = ::RegisterWindowMessage(UWM_KEYUP_MSG);
static UINT WM_TASKBARCREATED = ::RegisterWindowMessage(L"TaskbarCreated");

BEGIN_MESSAGE_MAP(CQuickInputDlg, CDialog)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDB_TESTFEAUTURE, TestFeatures)
	ON_MESSAGE(WM_TRAY, NotifyTrayIcon)
	ON_COMMAND(ID_TRAY_CLOSE, Close)
	ON_COMMAND(ID_TRAY_OPTION, ShowOptionsDlg)
	ON_COMMAND(ID_TRAY_ABOUT, ShowAboutDlg)
	ON_COMMAND(ID_TRAY_COLORS, ShowColorsDlg)
	ON_COMMAND(ID_TRAY_PROGRAMS, ShowProgramsDlg)
	ON_COMMAND(ID_TRAY_EDITDICT, ShowEditDictDlg)
	ON_COMMAND(ID_TRAY_WORKING, ChangeUserWorking)
	ON_MESSAGE(WM_STARTHIDE, HideDialog)
	ON_REGISTERED_MESSAGE(UWM_CHAR, ProcessChar)
	ON_REGISTERED_MESSAGE(UWM_SETFOCUS, ProcessSetFocus)
	ON_REGISTERED_MESSAGE(UWM_KILLFOCUS, ProcessKillFocus)
	ON_REGISTERED_MESSAGE(UWM_KEYDOWN, ProcessKeyDown)
	ON_REGISTERED_MESSAGE(UWM_KEYUP, ProcessKeyUp)
	ON_WM_WINDOWPOSCHANGED()
	ON_WM_ACTIVATE()
	ON_REGISTERED_MESSAGE(WM_TASKBARCREATED, TaskbarCreated)
	ON_MESSAGE(WM_HOTKEY, ProcessHotKey)
	ON_WM_DRAWCLIPBOARD()
	ON_WM_CHANGECBCHAIN()
END_MESSAGE_MAP()

// тред для копирование из clipboard выбраного слова/строки
INT CopyThread(LPVOID lpParam);

// тред для скрытия balloon tip через BALLONTIPTIME мс.
INT HideBalloonTip(LPVOID lpParam);

//////////////////////////////////////////////////////////////////////////
// конструктор
//
CQuickInputDlg::CQuickInputDlg(CWnd* pParent /*=NULL*/)
	: CDialog()
{
	m_pParentWnd  = pParent;

	m_hSMemory        = NULL;
	m_pSMemory        = NULL;
	m_pDB             = NULL;
	m_listAllWords    = NULL;
	m_wndListWords    = NULL;
	m_hEventClipboard = NULL;

	m_dlgOptions  = NULL;
	m_dlgAbout    = NULL;
	m_dlgColors   = NULL;
	m_dlgPrograms = NULL;
	m_dlgEditDict = NULL;

	m_dwLanguage    = GetUserDefaultLangID();
	m_uiTransparent = 75;
	m_colorBG       = 15581078;
	m_colorSelect   = 6478259;
	m_cChosenWork   = 0;
	m_cRunStart     = 1;
	m_cRunMin       = 0;
	m_uiMinLetter   = 4;
	m_cInsSpace     = 1;
	m_cAutoAdd      = 1;
	m_cSpaceDown    = 0;
	m_cHotkeyAdd    = 0;
	m_uiHKAddVK     = 0;
	m_uiHKAddMod    = 0;
	m_uiLettersShow = 2;
	m_uiListWordsX  = 0;
	m_uiListWordsY  = 0;

	ZeroMemory(&m_charFormat, sizeof(m_charFormat));
	m_charFormat.cbSize          = 0;
	m_charFormat.dwMask          = 3892314127;
	m_charFormat.dwEffects       = 1;
	m_charFormat.yHeight         = 200;
	m_charFormat.yOffset         = 0;
	m_charFormat.crTextColor     = 0;
	m_charFormat.bCharSet        = 204;
	m_charFormat.bPitchAndFamily = 34;
	_tcscpy(m_charFormat.szFaceName, _T("Tahoma"));

	m_bWork     = TRUE;
	m_bUserWork = TRUE;

	m_pcbBackup = NULL;
}

//////////////////////////////////////////////////////////////////////////
//
//
VOID CQuickInputDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

//////////////////////////////////////////////////////////////////////////
// инициализация диалога
//
BOOL CQuickInputDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// проверка запущенного экземпляра
	BOOL bRun = FALSE;

	m_hSMemory = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, 
		SHMEMSIZE, _T(PROGRAM_NAME));
	if (NULL == m_hSMemory)
	{
		MessageBox(_T("CreateFileMapping::Error"), _T("ERROR"), MB_OK | MB_ICONERROR);

		EndDialog(0);

		return FALSE;
	}
	bRun = (ERROR_ALREADY_EXISTS == GetLastError());

	m_pSMemory = MapViewOfFile(m_hSMemory, FILE_MAP_ALL_ACCESS, 0, 0, 0);
	if (NULL == m_pSMemory)
	{
		EndDialog(0);

		return FALSE;
	}

	if (bRun)
	{
		HWND hWnd = ((HWND*)m_pSMemory)[0];

		if (hWnd)
		{
			::PostMessage(hWnd, WM_COMMAND, ID_TRAY_OPTION, CN_COMMAND);
		}

		EndDialog(0);

		return FALSE;
	}

	// запись в шаредмемори хендла окна
	memset(m_pSMemory, 0, SHMEMSIZE);
	memcpy(m_pSMemory, &(this->m_hWnd), sizeof(this->m_hWnd));

	// установка иконки
	SetIcon(AfxGetApp()->LoadIcon(IDI_MAINFRAME), TRUE);
	if ((WV_2000 == GetWindowsVersion()) || (WV_2003 == GetWindowsVersion()))
	{
		SetIcon(AfxGetApp()->LoadIcon(IDI_TRAY_256), FALSE);
	}
	else
	{
		SetIcon(AfxGetApp()->LoadIcon(IDI_TRAY_32BPP), FALSE);
	}

	// установка заголовка
	SetWindowText(CString(_T(PROGRAM_NAME)) + GetProductVersion());

	// установка иконки в трее
	ShowTrayIcon();

	// минимизация при старте
	if (m_cRunMin)
	{
		ShowWindow(SW_SHOWMINNOACTIVE);
		PostMessage(WM_STARTHIDE);
	}

	// создание диалогов
	m_dlgAbout = new CDlgAbout();
	m_dlgAbout->CreateIndirect((LPDLGTEMPLATE)LoadResourceLang(RT_DIALOG, IDD_ABOUT), 
		GetDlgItem(IDC_DIALOGVIEW));
	m_dlgAbout->ShowWindow(SW_HIDE);

	m_dlgOptions = new CDlgOptions();
	m_dlgOptions->CreateIndirect((LPDLGTEMPLATE)LoadResourceLang(RT_DIALOG, IDD_OPTIONS), 
		GetDlgItem(IDC_DIALOGVIEW));
	m_dlgOptions->ShowWindow(SW_SHOW);

	m_dlgColors = new CDlgColors();
	m_dlgColors->CreateIndirect((LPDLGTEMPLATE)LoadResourceLang(RT_DIALOG, IDD_COLORS), 
		GetDlgItem(IDC_DIALOGVIEW));
	m_dlgColors->ShowWindow(SW_HIDE);

	m_dlgPrograms = new CDlgPrograms();
	m_dlgPrograms->CreateIndirect((LPDLGTEMPLATE)LoadResourceLang(RT_DIALOG, IDD_PROGRAMS), 
		GetDlgItem(IDC_DIALOGVIEW));
	m_dlgPrograms->ShowWindow(SW_HIDE);
	LoadChosenPrograms();

	m_dlgEditDict = new CDlgEditDict();
	m_dlgEditDict->CreateIndirect((LPDLGTEMPLATE)LoadResourceLang(RT_DIALOG, IDD_EDITDICT), 
		GetDlgItem(IDC_DIALOGVIEW));
	m_dlgEditDict->ShowWindow(SW_HIDE);

	// список диалогов
	m_cDlgList.SubclassDlgItem(IDC_DIALOGLIST, this);
	m_cDlgList.SetItemsHeight(70);
	m_cDlgList.SetCaptionWnd(&m_cDlgName);

	m_cDlgList.AddItem(LoadStringLang(IDS_DLG_LIST_OPTIONS).GetBuffer(0), m_dlgOptions, 
		LoadStringLang(IDS_DLG_CAPTION_OPTIONS).GetBuffer(0), IDI_CONFIG);
	m_cDlgList.AddItem(LoadStringLang(IDS_DLG_LIST_COLORS).GetBuffer(0), m_dlgColors, 
		LoadStringLang(IDS_DLG_CAPTION_COLORS).GetBuffer(0), IDI_COLORS);
	m_cDlgList.AddItem(LoadStringLang(IDS_DLG_LIST_PROGRAMS).GetBuffer(0), m_dlgPrograms, 
		LoadStringLang(IDS_DLG_CAPTION_PROGRAMS).GetBuffer(0), IDI_PROGRAMS);
	m_cDlgList.AddItem(LoadStringLang(IDS_DLG_LIST_EDITDICT).GetBuffer(0), m_dlgEditDict, 
		LoadStringLang(IDS_DLG_CAPTION_EDITDICT).GetBuffer(0), IDI_EDITDICT);
	m_cDlgList.AddItem(LoadStringLang(IDS_DLG_LIST_ABOUT).GetBuffer(0), m_dlgAbout, 
		(LoadStringLang(IDS_DLG_CAPTION_ABOUT) + GetProductVersion()).GetBuffer(0), IDI_INFO);

	m_cDlgList.SetCurSel(0);

	// заголовок диалогов
	m_cDlgName.SubclassDlgItem(IDC_DIALOGNAME, this);
	m_cDlgName.SetWindowText(LoadStringLang(IDS_DLG_CAPTION_OPTIONS).GetBuffer(0));

	// создание окна ускоренного ввода
	m_wndListWords = new CListWords(this);
	m_wndListWords->CreateEx(WS_EX_TOOLWINDOW, NULL, NULL, WS_POPUP, 
		CRect(0, 0, 200, 100), GetDesktopWindow(), 0);
	
	// инициализация начальных параметров
	m_wndListWords->SetBackground(m_colorBG);
	m_wndListWords->SetBGSelection(m_colorSelect);
	SetCharFormat(m_charFormat);
	SetTransparent(m_uiTransparent);

	// скрываем кнопку тест в релизе
	#ifndef _DEBUG
	GetDlgItem(IDB_TESTFEAUTURE)->ShowWindow(SW_HIDE);
	#endif

	// установка хука
	if (FALSE == SetHook(this->m_hWnd))
	{
		MessageBox(_T("SetHook::Error"), _T("ERROR"), MB_OK | MB_ICONERROR);

		EndDialog(0);

		return FALSE;
	}

	// создание event для работы с clipboard
	m_hEventClipboard = CreateEvent(NULL, TRUE, FALSE, L"{F1174574-7C5C-46e2-9EBB-ED4A24167133}");
	if (NULL == m_hEventClipboard)
	{
		MessageBox(_T("CreateEvent::Error"), _T("ERROR"), MB_OK | MB_ICONERROR);

		EndDialog(0);

		return FALSE;
	}

	// регистрируемся наблюдателем clipboard
	m_hNextViewer = SetClipboardViewer();

	// регистрация hotkey для добавления выделенного слова в словарь
	if (0 != m_cHotkeyAdd)
	{
		RegisterHotkeyAdd(TRUE);
	}

	m_cToolTip.Create(this, TTS_NOPREFIX | TTS_BALLOON | TTS_ALWAYSTIP);
	m_cToolTip.SendMessage(TTM_SETTITLE, TTI_INFO, (LPARAM)L"test title");

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
// инициализация до создания диалога
//
INT CQuickInputDlg::DoModal() 
{
	// соединение с БД
	TCHAR tcPathDB[MAX_PATH];

	ZeroMemory(tcPathDB, MAX_PATH);

	SHGetSpecialFolderPath(this->m_hWnd, tcPathDB, CSIDL_APPDATA, 0);
	if (tcPathDB[_tcslen(tcPathDB) - 1] != _T('\\'))
	{
		_tcscat(tcPathDB, _T("\\"));
	}
	_tcscat(tcPathDB, _T(PROGRAM_NAME));

	CreateDirectory(tcPathDB, NULL);

	_tcscat(tcPathDB, _T("\\")_T(DB_NAME));

	if (sqlite3_open(tcPathDB, &m_pDB))
	{
		MessageBox(_T("sqlite3_open"), _T("ERROR"), MB_OK | MB_ICONERROR);

		return IDCANCEL;
	}

	// создание таблиц
	CreateTables();

	// чтение настроек
	LoadSettings();

	// установка локали установленная по умолчанию в системе
	TCHAR tcTmp[128], tcAnsiCodePage[128];
	
	ZeroMemory(tcTmp, sizeof(TCHAR) * 128);
	ZeroMemory(tcAnsiCodePage, sizeof(TCHAR) * 128);
	GetLocaleInfo(GetUserDefaultLangID(), LOCALE_IDEFAULTANSICODEPAGE, tcTmp, 128);
	tcAnsiCodePage[0] = _T('.');
	CopyMemory(&tcAnsiCodePage[1], tcTmp, sizeof(TCHAR) * 127);

	_tsetlocale(LC_ALL, tcAnsiCodePage);

	// создание локализованного диалога
	if (!InitModalIndirect((LPDLGTEMPLATE)LoadResourceLang(RT_DIALOG, this->IDD), m_pParentWnd))
			return IDCANCEL;

	return CDialog::DoModal();
}

//////////////////////////////////////////////////////////////////////////
// скрывает диалог
//
LRESULT CQuickInputDlg::HideDialog(WPARAM wParam, LPARAM lParam)
{
	SetMinimized();

	return 0;
}

//////////////////////////////////////////////////////////////////////////
// обработка WM_DESTROY
//
VOID CQuickInputDlg::OnDestroy() 
{
	// удаление хоткея для добавления выделенного слова в словарь
	RegisterHotkeyAdd(FALSE);

	// удаление наблюдения за clipboard
	ChangeClipboardChain(m_hNextViewer);

	// закрытие event'а
	CloseHandle(m_hEventClipboard);

	// удаление backup clipboard
	if (NULL != m_pcbBackup)
	{
		delete m_pcbBackup;
	}

	// удаление хука
	UnSetHook();
	::PostMessage(HWND_BROADCAST, WM_NULL, 0, 0);

	// сохранение настроек программы
	if (NULL != m_wndListWords)
	{
		SaveSettings();
	}

	// удаление окна списка слов
	if (NULL != m_wndListWords)
	{
		m_wndListWords->DestroyWindow();
	}

	// удаление списка слов
	if (NULL != m_listAllWords)
	{
		delete m_listAllWords;
	}

	// удаление диалога выбора программ
	if (NULL != m_dlgPrograms)
	{
		m_dlgPrograms->DestroyWindow();
		delete m_dlgPrograms;
	}

	// удаление диалога выбора цветов
	if (NULL != m_dlgColors)
	{
		m_dlgColors->DestroyWindow();
		delete m_dlgColors;
	}

	// удаление диалога с информацией о программе
	if (NULL != m_dlgAbout)
	{
		m_dlgAbout->DestroyWindow();
		delete m_dlgAbout;
	}

	// удаление диалога с настройками программы
	if (NULL != m_dlgOptions)
	{
		m_dlgOptions->DestroyWindow();
		delete m_dlgOptions;
	}

	// удаление диалога редактирования словаря
	if (NULL != m_dlgEditDict)
	{
		m_dlgEditDict->DestroyWindow();
		delete m_dlgEditDict;
	}

	if (NULL != m_pSMemory)
	{
		UnmapViewOfFile(m_pSMemory);
	}

	if (NULL != m_hSMemory)
	{
		CloseHandle(m_hSMemory);
	}

	if (NULL != m_pDB)
	{
		sqlite3_close(m_pDB);
	}

	CDialog::OnDestroy();
}

//////////////////////////////////////////////////////////////////////////
// обработка IDOK
//
VOID CQuickInputDlg::OnOK()
{
	if (NULL != m_dlgOptions)
	{
		m_dlgOptions->OnOK();
	}

	// установка хоткея
	if (0 != m_cHotkeyAdd)
	{
		RegisterHotkeyAdd(TRUE);
	}

	// минимизация окна
	SetMinimized(TRUE);

	return;
}

//////////////////////////////////////////////////////////////////////////
// обработка IDCANCEL
//
VOID CQuickInputDlg::OnCancel()
{
	SetMinimized(TRUE);

	#ifdef _DEBUG
	Close();
	#endif

	return;
}

//////////////////////////////////////////////////////////////////////////
// выход из программы
//
VOID CQuickInputDlg::Close() 
{
	ShowTrayIcon(FALSE);

	EndDialog(IDOK);
}

//////////////////////////////////////////////////////////////////////////
// минимизирование окна
//
VOID CQuickInputDlg::SetMinimized(BOOL bMinimized)
{
	if (TRUE == bMinimized)
	{
		ShowWindow(SW_HIDE);
	}
	else
	{
		SetForegroundWindow();

		ShowWindow(SW_SHOWNORMAL);
	}
}

//////////////////////////////////////////////////////////////////////////
// тестирование какой-либо фичи
//
VOID CQuickInputDlg::TestFeatures() 
{
	if (m_wndListWords->IsWindowVisible())
	{
		ListWordsShow(FALSE);
	}
	else
	{
		m_wndListWords->ShowTitleBar();
		m_wndListWords->SetPositionList(CPoint(m_uiListWordsX, m_uiListWordsY));
		m_wndListWords->ShowWindow(TRUE);

		// удаление старого списка и создание нового
		if (NULL != m_listAllWords)
		{
			delete m_listAllWords;

			m_listAllWords = NULL;
		}

		m_listAllWords = new CStringList(MAX_ROWS);

		m_listAllWords->AddTail(LoadStringLang(IDS_TESTLIST_1).GetBuffer(0));
		m_listAllWords->AddTail(LoadStringLang(IDS_TESTLIST_2).GetBuffer(0));
		m_listAllWords->AddTail(LoadStringLang(IDS_TESTLIST_3).GetBuffer(0));
		m_listAllWords->AddTail(LoadStringLang(IDS_TESTLIST_4).GetBuffer(0));
		m_listAllWords->AddTail(LoadStringLang(IDS_TESTLIST_5).GetBuffer(0));
		m_listAllWords->AddTail(LoadStringLang(IDS_TESTLIST_6).GetBuffer(0));
		m_listAllWords->AddTail(LoadStringLang(IDS_TESTLIST_7).GetBuffer(0));
		m_listAllWords->AddTail(LoadStringLang(IDS_TESTLIST_8).GetBuffer(0));
		m_listAllWords->AddTail(LoadStringLang(IDS_TESTLIST_9).GetBuffer(0));
		m_listAllWords->AddTail(LoadStringLang(IDS_TESTLIST_10).GetBuffer(0));
		m_listAllWords->AddTail(LoadStringLang(IDS_TESTLIST_11).GetBuffer(0));
		m_listAllWords->AddTail(LoadStringLang(IDS_TESTLIST_12).GetBuffer(0));

		m_wndListWords->RefreshList(m_listAllWords, &m_strWord);

		//m_wndListWords->ShowWindow(SW_SHOWNOACTIVATE);
		ListWordsShow(TRUE);
	}

	//wnd->UpdateWindow();
	//Close();
	//EndDialog(IDRESTART);
}

//////////////////////////////////////////////////////////////////////////
// установка/удаление иконки в системном трее
// установить, если (TRUE == bShow)
//
VOID CQuickInputDlg::ShowTrayIcon(BOOL bShow/* = TRUE*/, BOOL bModify/* = FALSE*/, CString strText/* = L""*/)
{
	NOTIFYICONDATA notifyIconData;

	ZeroMemory(&notifyIconData, sizeof(NOTIFYICONDATA));

	notifyIconData.cbSize = sizeof(notifyIconData);
	notifyIconData.hWnd   = this->m_hWnd;
	notifyIconData.uID    = 0;

	if (TRUE == bShow)
	{
		notifyIconData.uFlags           = NIF_MESSAGE | NIF_ICON | NIF_TIP | NIF_INFO;
		notifyIconData.uCallbackMessage = WM_TRAY;
		wcscpy(notifyIconData.szTip, _T(PROGRAM_NAME));

		if (TRUE != strText.IsEmpty())
		{
			// обрезаем strText до BALLOONTIPSIZE символов
			if (BALLOONTIPSIZE < strText.GetLength())
			{
				strText.Delete(BALLOONTIPSIZE, strText.GetLength() - BALLOONTIPSIZE);
				strText += L"[...]";
			}

			strText = LoadStringLang(IDS_BALLOONTIP_ADD) + strText;

			// заполняем инфу для balloon tip
			notifyIconData.dwInfoFlags = NIIF_INFO;
			wcscpy(notifyIconData.szInfo, strText);
			wcscpy(notifyIconData.szInfoTitle, _T(PROGRAM_NAME) + GetProductVersion());

			AfxBeginThread((AFX_THREADPROC)HideBalloonTip, this, THREAD_PRIORITY_NORMAL, 0, 0);
		}

		if ((WV_2000 == GetWindowsVersion()) || (WV_2003 == GetWindowsVersion()))
		{
			if (TRUE == m_bUserWork)
			{
				notifyIconData.hIcon = AfxGetApp()->LoadIcon(IDI_TRAY_256);
			}
			else
			{
				notifyIconData.hIcon = AfxGetApp()->LoadIcon(IDI_TRAY_256_GR);
			}
		}
		else
		{
			if (TRUE == m_bUserWork)
			{
				notifyIconData.hIcon = AfxGetApp()->LoadIcon(IDI_TRAY_32BPP);
			}
			else
			{
				notifyIconData.hIcon = AfxGetApp()->LoadIcon(IDI_TRAY_32BPP_GR);
			}
		}

		if (TRUE == bModify)
		{
			Shell_NotifyIcon(NIM_MODIFY, &notifyIconData);
		}
		else
		{
			Shell_NotifyIcon(NIM_ADD, &notifyIconData);
		}
	}
	else
	{
		Shell_NotifyIcon(NIM_DELETE, &notifyIconData);
	}
}

//////////////////////////////////////////////////////////////////////////
// обработка сообщений из трея
//
LRESULT CQuickInputDlg::NotifyTrayIcon(WPARAM wParam, LPARAM lParam)
{
	UINT uiID;
	UINT uiMouseMsg;

	uiID       = (UINT)wParam;
	uiMouseMsg = (UINT)lParam;

	// вызов всплывающего меню
	if ((0 == uiID) && (WM_RBUTTONUP == uiMouseMsg))
	{
		POINT  point;
		CMenu* menuPopUp;
		CMenu* menu;

		SetForegroundWindow();

		// получаем нужное меню
		menuPopUp = new CMenu();
		menuPopUp->LoadMenuIndirect(LoadResourceLang(RT_MENU, IDM_POPUPMENU));

		menu = menuPopUp->GetSubMenu(0);

		// устанавливаем check на пункт работы программы
		if (TRUE == m_bUserWork)
		{
			menu->CheckMenuItem(ID_TRAY_WORKING, MF_BYCOMMAND | MF_CHECKED);
		}
		else
		{
			menu->CheckMenuItem(ID_TRAY_WORKING, MF_BYCOMMAND | MF_UNCHECKED);
		}

		// вывод меню
		GetCursorPos(&point);
		menu->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this);

		delete menuPopUp;
	}
	else

	// вызов диалога настроек
	if ((0 == uiID) && (WM_LBUTTONDBLCLK == uiMouseMsg))
	{
		SetMinimized(FALSE);
	}
	else

	// на передний план
	if ((0 == uiID) && (WM_LBUTTONUP == uiMouseMsg))
	{
		ChangeUserWorking();

		SetForegroundWindow();
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////
// загрузка настроек
//
VOID CQuickInputDlg::LoadSettings()
{
	m_dwLanguage                 = theApp.GetProfileInt(_T(""), _T("m_dwLanguage"), m_dwLanguage);
	m_uiTransparent              = theApp.GetProfileInt(_T(""), _T("m_uiTransparent"), m_uiTransparent);
	m_charFormat.yHeight         = theApp.GetProfileInt(_T(""), _T("m_charFormat.yHeight"), m_charFormat.yHeight);
	m_charFormat.crTextColor     = theApp.GetProfileInt(_T(""), _T("m_charFormat.crTextColor"), m_charFormat.crTextColor);
	m_charFormat.dwEffects       = theApp.GetProfileInt(_T(""), _T("m_charFormat.dwEffects"), m_charFormat.dwEffects);
	m_charFormat.dwMask          = theApp.GetProfileInt(_T(""), _T("m_charFormat.dwMask"), m_charFormat.dwMask);
	m_charFormat.bCharSet        = theApp.GetProfileInt(_T(""), _T("m_charFormat.bCharSet"), m_charFormat.bCharSet);
	m_charFormat.bPitchAndFamily = theApp.GetProfileInt(_T(""), _T("m_charFormat.bPitchAndFamily"), m_charFormat.bPitchAndFamily);
	m_colorBG                    = theApp.GetProfileInt(_T(""), _T("m_colorBG"), m_colorBG);
	m_colorSelect                = theApp.GetProfileInt(_T(""), _T("m_colorSelect"), m_colorSelect);
	m_cChosenWork                = theApp.GetProfileInt(_T(""), _T("m_cChosenWork"), m_cChosenWork);
	m_cRunStart                  = theApp.GetProfileInt(_T(""), _T("m_cRunStart"), m_cRunStart);
	m_cRunMin                    = theApp.GetProfileInt(_T(""), _T("m_cRunMin"), m_cRunMin);
	m_uiMinLetter                = theApp.GetProfileInt(_T(""), _T("m_uiMinLetter"), m_uiMinLetter);
	m_cInsSpace                  = theApp.GetProfileInt(_T(""), _T("m_cInsSpace"), m_cInsSpace);
	m_cAutoAdd                   = theApp.GetProfileInt(_T(""), _T("m_cAutoAdd"), m_cAutoAdd);
	m_cSpaceDown                 = theApp.GetProfileInt(_T(""), _T("m_cSpaceDown"), m_cSpaceDown);
	m_cHotkeyAdd                 = theApp.GetProfileInt(_T(""), _T("m_cHotkeyAdd"), m_cHotkeyAdd);
	m_uiHKAddVK                  = theApp.GetProfileInt(_T(""), _T("m_uiHKAddVK"), m_uiHKAddVK);
	m_uiHKAddMod                 = theApp.GetProfileInt(_T(""), _T("m_uiHKAddMod"), m_uiHKAddMod);
	m_uiLettersShow              = theApp.GetProfileInt(_T(""), _T("m_uiLettersShow"), m_uiLettersShow);
	m_uiListWordsX               = theApp.GetProfileInt(_T(""), _T("m_uiListWordsX"), m_uiListWordsX);
	m_uiListWordsY               = theApp.GetProfileInt(_T(""), _T("m_uiListWordsY"), m_uiListWordsY);

	CString strTmp;

	strTmp = theApp.GetProfileString(_T(""), _T("m_charFormat.szFaceName"), m_charFormat.szFaceName);
	_tcscpy(m_charFormat.szFaceName, strTmp.GetBuffer(0));

	SetRunStart(m_cRunStart);
}

//////////////////////////////////////////////////////////////////////////
// сохранение настроек
//
VOID CQuickInputDlg::SaveSettings()
{
	// сохранение настроек программы
	theApp.WriteProfileInt(_T(""), _T("m_dwLanguage"), m_dwLanguage);
	theApp.WriteProfileInt(_T(""), _T("m_uiTransparent"), m_uiTransparent);
	theApp.WriteProfileString(_T(""), _T("m_charFormat.szFaceName"), m_charFormat.szFaceName);
	theApp.WriteProfileInt(_T(""), _T("m_charFormat.yHeight"), m_charFormat.yHeight);
	theApp.WriteProfileInt(_T(""), _T("m_charFormat.crTextColor"), m_charFormat.crTextColor);
	theApp.WriteProfileInt(_T(""), _T("m_charFormat.dwEffects"), m_charFormat.dwEffects);
	theApp.WriteProfileInt(_T(""), _T("m_charFormat.dwMask"), m_charFormat.dwMask);
	theApp.WriteProfileInt(_T(""), _T("m_charFormat.bCharSet"), m_charFormat.bCharSet);
	theApp.WriteProfileInt(_T(""), _T("m_charFormat.bPitchAndFamily"), m_charFormat.bPitchAndFamily);
	theApp.WriteProfileInt(_T(""), _T("m_colorBG"), m_colorBG);
	theApp.WriteProfileInt(_T(""), _T("m_colorSelect"), m_colorSelect);
	theApp.WriteProfileInt(_T(""), _T("m_cChosenWork"), m_cChosenWork);
	theApp.WriteProfileInt(_T(""), _T("m_cRunStart"), m_cRunStart);
	theApp.WriteProfileInt(_T(""), _T("m_cRunMin"), m_cRunMin);
	theApp.WriteProfileInt(_T(""), _T("m_uiMinLetter"), m_uiMinLetter);
	theApp.WriteProfileInt(_T(""), _T("m_cInsSpace"), m_cInsSpace);
	theApp.WriteProfileInt(_T(""), _T("m_cAutoAdd"), m_cAutoAdd);
	theApp.WriteProfileInt(_T(""), _T("m_cSpaceDown"), m_cSpaceDown);
	theApp.WriteProfileInt(_T(""), _T("m_cHotkeyAdd"), m_cHotkeyAdd);
	theApp.WriteProfileInt(_T(""), _T("m_uiHKAddVK"), m_uiHKAddVK);
	theApp.WriteProfileInt(_T(""), _T("m_uiHKAddMod"), m_uiHKAddMod);
	theApp.WriteProfileInt(_T(""), _T("m_uiLettersShow"), m_uiLettersShow);
	theApp.WriteProfileInt(_T(""), _T("m_uiListWordsX"), m_uiListWordsX);
	theApp.WriteProfileInt(_T(""), _T("m_uiListWordsY"), m_uiListWordsY);

	// сохранение списка программ
	TCHAR        tcSQLQuery[SQL_MAXSIZE];
	SQLITE3_STMT *pStmt;
	INT          iReturn;
	CString      strReturn, strPath, strTmp;

	// удаление списка выбранных программ
	ZeroMemory(tcSQLQuery, SQL_MAXSIZE);

	_tcscpy(tcSQLQuery, _T("DELETE FROM ")_T(TABLE_PROGRAMS));

	iReturn = sqlite3_prepare(m_pDB, tcSQLQuery, -1, &pStmt, NULL);
	if (SQLITE_OK != iReturn)
	{
		MessageBox(_T("sqlite3_prepare::delete::")_T(TABLE_PROGRAMS), _T("ERROR"), MB_OK | MB_ICONERROR);

		sqlite3_finalize(pStmt);

		return;
	}

	iReturn = sqlite3_step(pStmt);
	if (SQLITE_DONE != iReturn)
	{
		MessageBox(_T("sqlite3_step::delete::")_T(TABLE_PROGRAMS), _T("ERROR"), MB_OK | MB_ICONERROR);

		sqlite3_finalize(pStmt);

		return;
	}

	sqlite3_finalize(pStmt);

	// сохранение списка выбранных программ
	for (UINT i = 0; i < m_dlgPrograms->GetCount(); ++i)
	{
		m_dlgPrograms->GetString(i, strReturn, strPath);

		strReturn.Replace(_T("\'"),_T("\'\'"));
		strPath.Replace(_T("\'"),_T("\'\'"));
		strTmp.Format(_T("INSERT INTO ")_T(TABLE_PROGRAMS)_T(" VALUES(NULL, '%s', '%s'); "), strReturn, strPath);

		ZeroMemory(tcSQLQuery, SQL_MAXSIZE);
		_tcscat(tcSQLQuery, strTmp);

		iReturn = sqlite3_prepare(m_pDB, tcSQLQuery, -1, &pStmt, NULL);
		if (SQLITE_OK != iReturn)
		{
			MessageBox(_T("sqlite3_prepare::insert::")_T(TABLE_PROGRAMS), _T("ERROR"), MB_OK | MB_ICONERROR);

			sqlite3_finalize(pStmt);

			return;
		}

		iReturn = sqlite3_step(pStmt);
		if (SQLITE_DONE != iReturn)
		{
			MessageBox(_T("sqlite3_step::insert::")_T(TABLE_PROGRAMS), _T("ERROR"), MB_OK | MB_ICONERROR);

			sqlite3_finalize(pStmt);

			return;
		}

		sqlite3_finalize(pStmt);
	}
}

//////////////////////////////////////////////////////////////////////////
// создание таблиц в БД
//
BOOL CQuickInputDlg::CreateTables()
{
	TCHAR        tcSQLQuery[SQL_MAXSIZE];
	SQLITE3_STMT *pStmt;
	INT          iReturn;

	// таблица - словарь
	ZeroMemory(tcSQLQuery, SQL_MAXSIZE);
	
	_tcscpy(tcSQLQuery, _T("CREATE TABLE ")_T(TABLE_WORDS)_T(" (")
		_T("ID    INTEGER PRIMARY KEY AUTOINCREMENT,") // уникальный идентификатор
		_T("CID   INTEGER DEFAULT 0,")                 // ИД контрола
		_T("CLASS VARCHAR(100) DEFAULT '',")           // имя класса контрола
		_T("PATH  VARCHAR(260) DEFAULT '',")           // путь к процессу
		_T("WORD  VARCHAR(100),")		                 // введенное слово
		_T("COUNT INTEGER DEFAULT 1")                  // счетчик
		_T(");"));

	iReturn = sqlite3_prepare(m_pDB, tcSQLQuery, -1, &pStmt, NULL);
	iReturn = sqlite3_step(pStmt);

	sqlite3_finalize(pStmt);

	// таблица - список программ
	ZeroMemory(tcSQLQuery, SQL_MAXSIZE);

	_tcscpy(tcSQLQuery, _T("CREATE TABLE ")_T(TABLE_PROGRAMS)_T(" (")
		_T("ID      INTEGER PRIMARY KEY AUTOINCREMENT,") // уникальный идентификатор
		_T("MODULE  VARCHAR(260),")	                   // наименование программы
		_T("PATH    VARCHAR(260)")		                   // полный путь к программе
		_T(");"));

	iReturn = sqlite3_prepare(m_pDB, tcSQLQuery, -1, &pStmt, NULL);
	iReturn = sqlite3_step(pStmt);

	sqlite3_finalize(pStmt);

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
// загружает указанный ресурс
//
HGLOBAL CQuickInputDlg::LoadResourceLang(LPCTSTR lpResType, DWORD dwResID)
{
	HINSTANCE hAppInstance;
	HRSRC     hrRes;

	hAppInstance = GetModuleHandle(NULL);

	hrRes = FindResourceEx(hAppInstance, lpResType, MAKEINTRESOURCE(dwResID), (WORD)m_dwLanguage);
	if (!hrRes)
	{
		//hrRes = FindResourceEx(hAppInstance, lpResType, MAKEINTRESOURCE(dwResID), GetUserDefaultLangID());
		hrRes = FindResourceEx(hAppInstance, lpResType, MAKEINTRESOURCE(dwResID), 
			MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US));
	}

	return LoadResource(hAppInstance, hrRes);	
}

//////////////////////////////////////////////////////////////////////////
// загружает строковой ресурс
//
CString CQuickInputDlg::LoadStringLang(DWORD dwResID)
{
	CString strReturn = _T("");
	LPCWSTR lpStr;
	TCHAR   *tcDst;

	lpStr = (LPCWSTR)LoadResourceLang(RT_STRING, 1 + (dwResID / 16));
	if (!lpStr)
	{
		return strReturn;
	}

	for (DWORD dwStrPos = 0; dwStrPos < (dwResID & 0x000F); dwStrPos++)
	{
		lpStr += *lpStr + 1;
	}

	tcDst = new TCHAR[*lpStr+1];

	#ifdef _UNICODE
	lstrcpyn(tcDst, lpStr + 1, *lpStr+1);
	#else
	WideCharToMultiByte(CP_ACP, 0, lpStr+1, *lpStr+1, tcDst, *lpStr+1, NULL, NULL);
	#endif

	strReturn = tcDst;

	delete [] tcDst;

	return strReturn;
}

//////////////////////////////////////////////////////////////////////////
// Обработка сообщения WM_CHAR от хука
//
LRESULT CQuickInputDlg::ProcessChar(WPARAM wParam, LPARAM lParam)
	{
	UCHAR   *uiPointer = (UCHAR*)m_pSMemory;
	MSG     msg;

	uiPointer += sizeof(HWND);
	msg = ((LPMSG)uiPointer)[wParam];

	// перекодировка в UTF-16 при необходимости
	if (!(0xFF00 & msg.wParam))
	{
		TCHAR tcTmp;
		CHAR  cTmp = msg.wParam;

		MultiByteToWideChar(CP_ACP, 0, &cTmp, 1, (LPWSTR)&tcTmp, 1);

		msg.wParam = tcTmp;
	}

	if (!m_wndListWords->IsWindowVisible() && 
		m_strWord.IsEmpty() &&
		(0x08 != msg.wParam) &&
		(0x0A != msg.wParam) &&
		(0x1B != msg.wParam) &&
		(0x09 != msg.wParam) &&
		(0x0D != msg.wParam) &&
		(0x20 != msg.wParam))
	{
		CPoint ptNewCaret; 
		INT    iHeightLetter = 0;

		// получение координат
		GetQICaretPos(ptNewCaret, iHeightLetter);

		if (CPoint(0,0) == ptNewCaret)
		{
			ptNewCaret.x = m_uiListWordsX;
			ptNewCaret.y = m_uiListWordsY;

			m_wndListWords->ShowTitleBar();
		}
		else
		{
			m_wndListWords->ShowTitleBar(FALSE);
		}

		m_strWord += TCHAR(msg.wParam);

		// устанавливаем позицию списка слов
		m_wndListWords->SetPositionList(ptNewCaret, iHeightLetter);

		// чтение списка слов из БД
		ReadWordsFromDB();

		// обновление слов в списке быстрого ввода
		m_wndListWords->RefreshList(m_listAllWords, &m_strWord);

		// вывод окна со списком слов
		if (m_listAllWords->GetCount())
		{
			ListWordsShow(TRUE);
		}
	}
	else

	if (m_wndListWords->IsWindowVisible() || (0x20 == msg.wParam) || !m_strWord.IsEmpty())
	{
		switch (msg.wParam) 
		{ 
			case 0x08: // backspace
				if (!m_strWord.IsEmpty())
				{
					m_strWord.Delete(m_strWord.GetLength() - 1, 1);

					if (m_strWord.IsEmpty())
					{
						ListWordsShow(FALSE);
					}
					else
					{
						// чтение списка слов из БД
						ReadWordsFromDB();

						// обновление вывода
						m_wndListWords->RefreshList(m_listAllWords, &m_strWord);

						// показываем список, если он не пустой
						CString strSelected;

						m_wndListWords->GetSelected(strSelected);
						if (!strSelected.IsEmpty())
						{
							ListWordsShow(TRUE);
						}
					}
				}
				else
				{
					ListWordsShow(FALSE);
				}
				break; 

			case 0x20: // пробел
				// запись слова в БД
				SaveWordInDB(m_strWord);

				ListWordsShow(FALSE);

				break; 

			default: // Process displayable characters. 
				m_strWord += TCHAR(msg.wParam);

				// чтение списка слов из БД
				ReadWordsFromDB();

				// обновление слов в списке быстрого ввода
				m_wndListWords->RefreshList(m_listAllWords, &m_strWord);

				// скрываем список, если он пустой
				if (m_wndListWords->GetCount())
				{
					CString strSelected;

					m_wndListWords->GetSelected(strSelected);

					if ((m_wndListWords->GetCount() > 1) || (strSelected != m_strWord))
					{
						ListWordsShow(TRUE);
					}
					else
					{
						ListWordsShow(FALSE, FALSE);
					}
				}
				else
				{
					ListWordsShow(FALSE, FALSE);
				}

				break; 
		}
	}

	m_bKeyDown = FALSE;

	return 0;
}

//////////////////////////////////////////////////////////////////////////
// Обработка сообщения WM_SETFOCUS от хука
//
LRESULT CQuickInputDlg::ProcessSetFocus(WPARAM wParam, LPARAM lParam)
{
	if (((HWND)wParam == m_wndListWords->m_hWnd) || ::IsChild(m_wndListWords->m_hWnd, (HWND)wParam))
	{
		// запрещаем в нем работу программы
		EmulatePressButton(VK_HOOK_OFF, FALSE);

		return 0;
	}

	// убираем список слов при смене фокуса
	ListWordsShow(FALSE);

	// если это окно Quick Input'а
	if (((HWND)wParam == m_hWnd) || ::IsChild(m_hWnd, (HWND)wParam))
	{
		// обновляем список программ в диалоге настроек
		if (m_dlgPrograms->IsWindowVisible() && !::IsChild(m_hWnd, m_hActiveWnd))
		{
			m_dlgPrograms->FillLists();
		}

		// запрещаем в нем работу программы
		EmulatePressButton(VK_HOOK_OFF, FALSE);

		m_hActiveWnd = (HWND)wParam;

		return 0;
	}

	m_hActiveWnd = (HWND)wParam;

	// получение информации о окне
	HANDLE hProcess    = NULL;
	DWORD  dwProcessID = 0;

	GetWindowThreadProcessId(m_hActiveWnd, &dwProcessID);
	hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwProcessID);

	// ИД элемента
	m_lActiveWndID = ::GetWindowLong(m_hActiveWnd, GWL_ID);

	// имя класса окна
	::GetClassName(m_hActiveWnd, m_strActiveWndClass.GetBuffer(128), 128);

	// полный путь к процессу которому принадлежит окно
	GetModuleFileNameEx(hProcess, (HINSTANCE)GetWindowLong(m_hActiveWnd, GWL_HINSTANCE), 
		m_strActiveWndPath.GetBuffer(MAX_PATH), MAX_PATH);

	CloseHandle(hProcess);

	// проверяем разрешено или запрещено работать в программе на которой фокус
	BOOL    bFind = FALSE;
	CString strReturn, strPath;

	for (UINT i = 0; i < m_dlgPrograms->GetCount(); i++)
	{
		m_dlgPrograms->GetString(i, strReturn, strPath);

		if (strPath == m_strActiveWndPath)
		{
			bFind = TRUE;

			break;
		}
	}
	
	if (m_cChosenWork)
	{
		if (bFind)
		{
			m_bWork = TRUE;
		}
		else
		{
			m_bWork = FALSE;
		}
	}
	else
	{
		if (bFind)
		{
			m_bWork = FALSE;
		}
		else
		{
			m_bWork = TRUE;
		}
	}

	if (FALSE == m_bUserWork)
	{
		m_bWork = FALSE;
	}

	if (m_bWork)
	{
		EmulatePressButton(VK_HOOK_ON, FALSE);
	}
	else
	{
		EmulatePressButton(VK_HOOK_OFF, FALSE);
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////
// Обработка сообщения WM_KEYDOWN от хука
//
LRESULT CQuickInputDlg::ProcessKeyDown(WPARAM wParam, LPARAM lParam)
{
	switch((DWORD)lParam)
	{
		//
		case VK_RETURN:
		case VK_TAB:
			if (m_wndListWords->IsWindowVisible() || !m_strWord.IsEmpty())
			{
				// получение выбранного слова
				CString strSelWord;
				m_wndListWords->GetSelected(strSelWord);

				// вставка выбранного слова
				if (m_wndListWords->IsWindowVisible() && !strSelWord.IsEmpty())
				{
					// проверяем большими символами осуществлять ввод или нет
					if (m_strWord.GetLength() >= 2)
					{
						CString strOne = m_strWord;
						CString strTwo;

						strOne.Delete(2, strOne.GetLength() - 2);
						strTwo = strOne;
						strOne.MakeUpper();

						if (strOne == strTwo)
						{
							strSelWord.MakeUpper();
						}
					}

					EmulatePressButton(VK_HOOK_OFF, FALSE);

					// удаляем все введенные символы, кроме первого
					for (INT i = (m_strWord.GetLength() - 1); i > 0; i--)
					{
						EmulatePressButton(VK_BACK, FALSE);
					}

					// посылаем окну выбранную строку посимвольно, кроме первого символа
					for (INT i = 1; i < strSelWord.GetLength(); i++)
					{
						EmulatePressButton(strSelWord.GetAt(i));
					}

					// завершающий пробел при необходимости
					if (m_cInsSpace)
					{
						EmulatePressButton(L' ');
					}

					EmulatePressButton(VK_HOOK_ON, FALSE);

					AddCountWordsInDB(strSelWord);

					// скрываем окно быстрого ввода
					ListWordsShow(FALSE);

					return FALSE;
				}
				else
				{
					// запись слова в БД
					if (!m_strWord.IsEmpty())
					{
						SaveWordInDB(m_strWord);
					}

					ListWordsShow(FALSE);
				}
			}

			EmulatePressButton(VK_HOOK_OFF, FALSE);
			EmulatePressButton((WORD)lParam, FALSE);
			EmulatePressButton(VK_HOOK_ON, FALSE);

			return FALSE;

			break;

		//
		case VK_ESCAPE:
			if (m_wndListWords->IsWindowVisible())
			{
				ListWordsShow(FALSE);

				// запись слова в БД
				SaveWordInDB(m_strWord);

				return FALSE;
			}

			EmulatePressButton(VK_HOOK_OFF, FALSE);
			EmulatePressButton(VK_ESCAPE, FALSE);
			EmulatePressButton(VK_HOOK_ON, FALSE);

			return FALSE;

			break; 

		// вверх 
		case VK_UP: 
			if (m_wndListWords->IsWindowVisible())
			{
				m_wndListWords->SelectedUp();

				return FALSE;
			}

			EmulatePressButton(VK_HOOK_OFF, FALSE);
			EmulatePressButton(VK_UP, FALSE);
			EmulatePressButton(VK_HOOK_ON, FALSE);

			return FALSE;

			break;

		// вниз
		case VK_DOWN: 
			if (m_wndListWords->IsWindowVisible())
			{
				m_wndListWords->SelectedDown();

				return FALSE;
			}

			EmulatePressButton(VK_HOOK_OFF, FALSE);
			EmulatePressButton(VK_DOWN, FALSE);
			EmulatePressButton(VK_HOOK_ON, FALSE);

			return FALSE;

			break;

		// пробел
		case VK_SPACE: 
			if (m_cSpaceDown && m_wndListWords->IsWindowVisible())
			{
				if (0 == (GetKeyState(VK_CONTROL) & 0x7000))
				{
					m_wndListWords->SelectedDownCycle();

					return FALSE;
				}
			}

			EmulatePressButton(VK_HOOK_OFF, FALSE);
			EmulatePressButton(VK_SPACE, FALSE);
			EmulatePressButton(VK_HOOK_ON, FALSE);

			// посылаем ProcessChar
			UCHAR *uiPointer = (UCHAR*)m_pSMemory;
			MSG   msg;

			uiPointer             += sizeof(HWND); // в начале шаред мемори находится хендл программы
			msg.wParam             = VK_SPACE;
			((LPMSG)uiPointer)[65] = msg;

			ProcessChar(65, (LPARAM)m_hActiveWnd);

			return FALSE;

			break;
	}

	m_bKeyDown = TRUE;

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
// обработка сообщения WM_KEYUP от хука
//
LRESULT CQuickInputDlg::ProcessKeyUp(WPARAM wParam, LPARAM lParam)
{
	if (TRUE == m_bKeyDown)
	{
		ListWordsShow(FALSE);

		m_bKeyDown = FALSE;
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
// функция возвращает позицию каретки в экранных координатах
//
VOID CQuickInputDlg::GetQICaretPos(CPoint &ptCaret, INT &iHeightLetter)
{
	// Edit
	if (_T("Edit") == m_strActiveWndClass)
	{
		// получение координаты каретки
		AttachThreadInput(GetCurrentThreadId(), GetWindowThreadProcessId(m_hActiveWnd, NULL), TRUE);
		::GetCaretPos(&ptCaret);
		AttachThreadInput(GetCurrentThreadId(), GetWindowThreadProcessId(m_hActiveWnd, NULL), FALSE);

		//if (CPoint(0,0) != ptCaret) 
		{
			HDC        hDCActiveWnd;	
			HGDIOBJ    hNewFont;
			TEXTMETRIC textMetric;
			HGDIOBJ    hOldFont;

			hDCActiveWnd = ::GetDC(m_hActiveWnd);

			hNewFont = (HGDIOBJ)::SendMessage(m_hActiveWnd, WM_GETFONT, NULL, NULL);
			if (NULL != hNewFont)
			{
				hOldFont = ::SelectObject(hDCActiveWnd, hNewFont);
				::GetTextMetrics(hDCActiveWnd, &textMetric);
				::SelectObject(hDCActiveWnd, hOldFont);
			}
			else
			{
				::GetTextMetrics(hDCActiveWnd, &textMetric);
			}
			//ptCaret.y += textMetric.tmHeight;
			iHeightLetter = textMetric.tmHeight;

			::ReleaseDC (m_hActiveWnd, hDCActiveWnd);

			::ClientToScreen(m_hActiveWnd, &ptCaret);
		}
	}
	else

	// RichEdit
	if (-1 != m_strActiveWndClass.Find(_T("RichEdit20W")))
	{
		// получение координаты каретки
		AttachThreadInput(GetCurrentThreadId(), GetWindowThreadProcessId(m_hActiveWnd, NULL), TRUE);
		::GetCaretPos(&ptCaret);
		AttachThreadInput(GetCurrentThreadId(), GetWindowThreadProcessId(m_hActiveWnd, NULL), FALSE);

		if (CPoint(0,0) != ptCaret)
		{
			HANDLE     hProcess    = NULL;
			DWORD      dwProcessID = 0;
			CHARFORMAT2 *_charFormat, charFormat;
			DWORD      dwBR;

			// инициализация
			ZeroMemory(&charFormat, sizeof(CHARFORMAT2));
			charFormat.cbSize = sizeof(CHARFORMAT2);

			// открытие процесса
			GetWindowThreadProcessId(m_hActiveWnd, &dwProcessID);
			hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwProcessID);

			// параметры символов
			_charFormat = (CHARFORMAT2*)VirtualAllocEx(hProcess, NULL, sizeof(CHARFORMAT2), 
				MEM_COMMIT, PAGE_READWRITE);
			WriteProcessMemory(hProcess, _charFormat, &charFormat, sizeof(CHARFORMAT2), &dwBR);

			::SendMessage(m_hActiveWnd, EM_GETCHARFORMAT, SCF_SELECTION, (LPARAM)_charFormat);

			ReadProcessMemory(hProcess, _charFormat, &charFormat, sizeof(CHARFORMAT2), &dwBR);
			VirtualFreeEx(hProcess, _charFormat, 0, MEM_RELEASE);

			CloseHandle(hProcess);

			if ((charFormat.dwMask & CFM_SIZE) == CFM_SIZE)
			{
				//ptCaret.y += (charFormat.yHeight + charFormat.yOffset) / 12;
				iHeightLetter = (charFormat.yHeight + charFormat.yOffset) / 12;
			}

			::ClientToScreen(m_hActiveWnd, &ptCaret);
		}
	}
	else

	// все остальное
	{
		// получение координаты каретки
		AttachThreadInput(GetCurrentThreadId(), GetWindowThreadProcessId(m_hActiveWnd, NULL), TRUE);
		::GetCaretPos(&ptCaret);
		AttachThreadInput(GetCurrentThreadId(), GetWindowThreadProcessId(m_hActiveWnd, NULL), FALSE);

		if (CPoint(0,0) != ptCaret) 
		{
			HDC        hDCActiveWnd;	
			HGDIOBJ    hNewFont;
			TEXTMETRIC textMetric;
			HGDIOBJ    hOldFont;

			hDCActiveWnd = ::GetDC(m_hActiveWnd);

			hNewFont = (HGDIOBJ)::SendMessage(m_hActiveWnd, WM_GETFONT, NULL, NULL);
			if (NULL != hNewFont)
			{
				hOldFont = ::SelectObject(hDCActiveWnd, hNewFont);
				::GetTextMetrics(hDCActiveWnd, &textMetric);
				::SelectObject(hDCActiveWnd, hOldFont);
			}
			else
			{
				::GetTextMetrics(hDCActiveWnd, &textMetric);
			}
			//ptCaret.y += textMetric.tmHeight;
			iHeightLetter = textMetric.tmHeight;

			::ReleaseDC (m_hActiveWnd, hDCActiveWnd);

			::ClientToScreen(m_hActiveWnd, &ptCaret);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// функция записывает слово в БД
//
VOID CQuickInputDlg::SaveWordInDB(CString strSaveWord, BOOL bForceAdd/* = FALSE*/)
{
	if (!bForceAdd && !m_cAutoAdd)
	{
		return;
	}

	// удаление знаков пунктуации
	INT iLength;

	do 
	{
		iLength = strSaveWord.GetLength();

		strSaveWord.TrimRight(_T(".,;!?-:"));
	} while(iLength != strSaveWord.GetLength());

	// проверка на минимальное количество символов в слове
	if (strSaveWord.GetLength() < (INT)m_uiMinLetter)
	{
		return;
	}

	// перевод в нижний регистр
	strSaveWord.MakeLower();

	// поиск слова в БД
	if (TRUE == SearchWordInDB(strSaveWord))
	{
		return;
	}

	// вставка в бд
	TCHAR        tcSQLQuery[SQL_MAXSIZE];
	SQLITE3_STMT *pStmt;
	INT          iReturn;
	CString      strTmp;

	ZeroMemory(tcSQLQuery, SQL_MAXSIZE);

	_tcscpy(tcSQLQuery, _T("INSERT INTO ")_T(TABLE_WORDS)_T("(WORD) VALUES("));

	strSaveWord.Replace(_T("\'"), _T("\'\'"));
	strTmp.Format(_T("'%s');"), strSaveWord);
	_tcscat(tcSQLQuery, strTmp);

	iReturn = sqlite3_prepare(m_pDB, tcSQLQuery, -1, &pStmt, NULL);
	if (SQLITE_OK != iReturn)
	{
		MessageBox(_T("sqlite3_prepare::insert::")_T(TABLE_WORDS), _T("ERROR"), MB_OK | MB_ICONERROR);

		sqlite3_finalize(pStmt);

		return;
	}

	iReturn = sqlite3_step(pStmt);
	if (SQLITE_DONE != iReturn)
	{
		MessageBox(_T("sqlite3_step::insert::")_T(TABLE_WORDS), _T("ERROR"), MB_OK | MB_ICONERROR);

		sqlite3_finalize(pStmt);

		return;
	}

	sqlite3_finalize(pStmt);
}

//////////////////////////////////////////////////////////////////////////
// функция считывает список слов из базы данных
//
VOID CQuickInputDlg::ReadWordsFromDB()
{
	CString strWord = m_strWord;
	strWord.MakeLower();

	// удаление старого списка и создание нового
	if (NULL != m_listAllWords)
	{
		delete m_listAllWords;

		m_listAllWords = NULL;
	}

	m_listAllWords = new CStringList(MAX_ROWS);

	// формирование и выполнение запроса
	TCHAR        tcSQLQuery[SQL_MAXSIZE];
	SQLITE3_STMT *pStmt;
	INT          iReturn;
	INT          iCount;
	CString      strTmp;
	
	strWord.Replace(_T("\'"), _T("\'\'"));
	strTmp.Format(_T("SELECT DISTINCT WORD FROM ")_T(TABLE_WORDS)
		_T(" WHERE WORD LIKE '%s%%' AND LENGTH(WORD) >= %d ORDER BY COUNT DESC, WORD;"), 
		strWord, m_uiMinLetter);

	ZeroMemory(tcSQLQuery, SQL_MAXSIZE);
	_tcscpy(tcSQLQuery, strTmp);

	iReturn = sqlite3_prepare(m_pDB, tcSQLQuery, -1, &pStmt, NULL);
	if (SQLITE_OK != iReturn)
	{
		MessageBox(_T("sqlite3_prepare::select::")_T(TABLE_WORDS), _T("ERROR"), MB_OK | MB_ICONERROR);

		sqlite3_finalize(pStmt);

		return;
	}

	iCount  = 0;
	iReturn = sqlite3_step(pStmt);
	while ((SQLITE_ROW == iReturn) && (++iCount <= MAX_ROWS))
	{
		m_listAllWords->AddTail((TCHAR*)sqlite3_column_text(pStmt, 0));

		iReturn = sqlite3_step(pStmt);
	}

	sqlite3_finalize(pStmt);
}

//////////////////////////////////////////////////////////////////////////
// функция увеличивается счетчик использования слова
//
VOID CQuickInputDlg::AddCountWordsInDB(CString strWord)
{
	// формирование и выполнение запроса
	TCHAR        tcSQLQuery[SQL_MAXSIZE];
	SQLITE3_STMT *pStmt;
	INT          iReturn;
	CString      strTmp;

	strWord.Replace(_T("\'"), _T("\'\'"));
	strTmp.Format(_T("UPDATE ")_T(TABLE_WORDS)_T(" SET COUNT = COUNT + 1 WHERE WORD LIKE '%s';"), strWord);

	ZeroMemory(tcSQLQuery, SQL_MAXSIZE);
	_tcscpy(tcSQLQuery, strTmp);

	iReturn = sqlite3_prepare(m_pDB, tcSQLQuery, -1, &pStmt, NULL);
	if (SQLITE_OK != iReturn)
	{
		MessageBox(_T("sqlite3_prepare::update::")_T(TABLE_WORDS), _T("ERROR"), MB_OK | MB_ICONERROR);

		sqlite3_finalize(pStmt);

		return;
	}

	iReturn = sqlite3_step(pStmt);
	if (SQLITE_DONE != iReturn)
	{
		MessageBox(_T("sqlite3_step::update::")_T(TABLE_WORDS), _T("ERROR"), MB_OK | MB_ICONERROR);

		sqlite3_finalize(pStmt);

		return;
	}

	sqlite3_finalize(pStmt);
}

//////////////////////////////////////////////////////////////////////////
// Обработка изменения положения окна
//
VOID CQuickInputDlg::OnWindowPosChanged(WINDOWPOS* lpwndpos)
{
	CDialog::OnWindowPosChanged(lpwndpos);

	if (NULL != m_dlgColors)
	{
		m_dlgColors->OnWindowPosChanged(NULL);
		m_dlgColors->ActivateList(TRUE, NULL, FALSE);
	}
}

//////////////////////////////////////////////////////////////////////////
// Обработка смены активного окна
//
VOID CQuickInputDlg::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized)
{
	CDialog::OnActivate(nState, pWndOther, bMinimized);

	if (NULL != m_dlgColors)
	{
		if (WA_INACTIVE == nState) 
		{
			m_dlgColors->ActivateList(FALSE, pWndOther, bMinimized);
		}
		else
		{
			m_dlgColors->ActivateList(TRUE, pWndOther, bMinimized);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// загрузить список программ из БД
//
VOID CQuickInputDlg::LoadChosenPrograms()
{
	if (NULL == m_dlgPrograms)
	{
		return;
	}

	TCHAR        tcSQLQuery[SQL_MAXSIZE];
	SQLITE3_STMT *pStmt;
	INT          iReturn;

	ZeroMemory(tcSQLQuery, SQL_MAXSIZE);
	_tcscpy(tcSQLQuery, _T("SELECT MODULE, PATH FROM ")_T(TABLE_PROGRAMS));

	iReturn = sqlite3_prepare(m_pDB, tcSQLQuery, -1, &pStmt, NULL);
	if (SQLITE_OK != iReturn)
	{
		MessageBox(_T("sqlite3_prepare::select::")_T(TABLE_PROGRAMS), _T("ERROR"), MB_OK | MB_ICONERROR);

		sqlite3_finalize(pStmt);

		return;
	}

	iReturn = sqlite3_step(pStmt);
	while (SQLITE_ROW == iReturn)
	{
		m_dlgPrograms->AddChosenProgram(CString((TCHAR*)sqlite3_column_text(pStmt, 0)), 
			CString((TCHAR*)sqlite3_column_text(pStmt, 1)));

		iReturn = sqlite3_step(pStmt);
	}

	sqlite3_finalize(pStmt);
}

//////////////////////////////////////////////////////////////////////////
// формат шрифта окна быстрого ввода
//
VOID CQuickInputDlg::SetCharFormat(CHARFORMAT &charFormat)
{
	m_charFormat = charFormat;

	LOGFONT   lf;
	CClientDC dc(this);

	ZeroMemory(&lf, sizeof(LOGFONT));

	_tcscpy(lf.lfFaceName, m_charFormat.szFaceName);
	lf.lfHeight         = -MulDiv(m_charFormat.yHeight / 20, dc.GetDeviceCaps(LOGPIXELSY), 72);
	lf.lfPitchAndFamily = m_charFormat.bPitchAndFamily;
	lf.lfUnderline      = m_charFormat.dwEffects & CFM_UNDERLINE ? TRUE : FALSE;
	lf.lfStrikeOut      = m_charFormat.dwEffects & CFM_STRIKEOUT ? TRUE : FALSE;
	lf.lfItalic         = m_charFormat.dwEffects & CFM_ITALIC ? TRUE : FALSE;
	lf.lfWeight         = m_charFormat.dwEffects & CFM_BOLD ? 700 : 400;
	lf.lfCharSet        = m_charFormat.bCharSet;

	m_fontList.DeleteObject();
	m_fontList.CreateFontIndirect(&lf);

	m_wndListWords->SetFont(&m_fontList);
	m_wndListWords->SetTextColor(GetCharFormat().crTextColor);
}

//////////////////////////////////////////////////////////////////////////
// прозрачность окна быстрого ввода
//
VOID CQuickInputDlg::SetTransparent(UINT uiTransparent)
{
	m_uiTransparent = uiTransparent;

	::SetWindowLong(m_wndListWords->m_hWnd, GWL_EXSTYLE, 
		GetWindowLong(m_wndListWords->m_hWnd, GWL_EXSTYLE) | WS_EX_LAYERED);
	::SetLayeredWindowAttributes(m_wndListWords->m_hWnd, 0, (255 * m_uiTransparent) / 100, LWA_ALPHA);
}

//////////////////////////////////////////////////////////////////////////
// функция скрывает окно со списком слов
//
VOID CQuickInputDlg::ListWordsShow(BOOL bShow /*= TRUE*/, BOOL bErase/* = TRUE*/)
{
	if (NULL == m_wndListWords)
	{
		return;
	}

	if (bShow)
	{
		if (m_wndListWords->IsWindowVisible())
		{
			return;
		}

		if (m_strWord.GetLength() >= (INT)m_uiLettersShow)
		{
			m_wndListWords->ShowWindow(SW_SHOWNOACTIVATE);
		}
	} 
	else
	{
		m_wndListWords->ShowWindow(SW_HIDE);

		if (bErase)
		{
			m_strWord = _T("");
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// рестарт программы - для смены языка
//
VOID CQuickInputDlg::Restart()
{
	ShowTrayIcon(FALSE);

	EndDialog(IDRESTART);
}

//////////////////////////////////////////////////////////////////////////
// показывает диалог "настройки"
//
VOID CQuickInputDlg::ShowOptionsDlg() 
{
	SetMinimized(FALSE);

	m_cDlgList.SetCurSel(0);
	m_cDlgList.SelectDialog();
}

//////////////////////////////////////////////////////////////////////////
// показывает диалог "О Quick Input"
//
VOID CQuickInputDlg::ShowAboutDlg()
{
	SetMinimized(FALSE);

	m_cDlgList.SetCurSel(4);
	m_cDlgList.SelectDialog();
}

//////////////////////////////////////////////////////////////////////////
// показывает диалог "Внешний вид"
//
VOID CQuickInputDlg::ShowColorsDlg()
{
	SetMinimized(FALSE);

	m_cDlgList.SetCurSel(1);
	m_cDlgList.SelectDialog();
}

//////////////////////////////////////////////////////////////////////////
// показывает диалог "Программы"
//
VOID CQuickInputDlg::ShowProgramsDlg()
{
	SetMinimized(FALSE);

	m_cDlgList.SetCurSel(2);
	m_cDlgList.SelectDialog();
}

//////////////////////////////////////////////////////////////////////////
// показывает диалог "Редактирование словаря"
//
VOID CQuickInputDlg::ShowEditDictDlg()
{
	SetMinimized(FALSE);

	m_cDlgList.SetCurSel(3);
	m_cDlgList.SelectDialog();
}

//////////////////////////////////////////////////////////////////////////
// установка запуска программы при старте ОС
//
VOID CQuickInputDlg::SetRunStart(CHAR cRunStart)
{
	m_cRunStart = cRunStart;

	HKEY  hKey;
	TCHAR ptcFileName[MAX_PATH];

	if (ERROR_SUCCESS == RegOpenKey(HKEY_CURRENT_USER, 
		_T("Software\\Microsoft\\Windows\\CurrentVersion\\Run"), &hKey))
	{
		if (cRunStart)
		{
			ZeroMemory(ptcFileName, sizeof(TCHAR) * MAX_PATH);
			GetModuleFileName(AfxGetApp()->m_hInstance, ptcFileName, sizeof(TCHAR) * (MAX_PATH - 1));
			RegSetValueEx(hKey, _T("Quick Input"), 0, REG_SZ, (BYTE*)ptcFileName, _tcslen(ptcFileName) * sizeof(TCHAR));
		}
		else
		{
			RegDeleteValue(hKey, _T("Quick Input"));
		}

		RegCloseKey(hKey);
	}
}

//////////////////////////////////////////////////////////////////////////
// эмулирует нажатие кнопки на клаве
//
VOID CQuickInputDlg::EmulatePressButton(WORD wCode, BOOL bUnicode/* = TRUE*/)
{
	INPUT inputChar;

	ZeroMemory(&inputChar, sizeof(inputChar));

	// пропуск управляющих
	if ((VK_HOOK_OFF == wCode) || (VK_HOOK_ON == wCode))
	{
		inputChar.type       = INPUT_KEYBOARD;
		inputChar.ki.wVk     = wCode;
		inputChar.ki.wScan   = 0;
		inputChar.ki.dwFlags = 0;
		inputChar.ki.time    = 0;
	
		SendInput(1, &inputChar, sizeof(inputChar));

		return;
	}

	// нажатие
	if (bUnicode)
	{
		inputChar.type       = INPUT_KEYBOARD;
		inputChar.ki.wVk     = 0;
		inputChar.ki.wScan   = wCode;
		inputChar.ki.dwFlags = KEYEVENTF_UNICODE;
		inputChar.ki.time    = 0;
	}
	else
	{
		inputChar.type       = INPUT_KEYBOARD;
		inputChar.ki.wVk     = wCode;
		inputChar.ki.wScan   = MapVirtualKey(inputChar.ki.wVk, 0);
		inputChar.ki.dwFlags = KEYEVENTF_EXTENDEDKEY;
		inputChar.ki.time    = 0;
	}

	SendInput(1, &inputChar, sizeof(inputChar));

	// отжатие
	if (bUnicode)
	{
		inputChar.type       = INPUT_KEYBOARD;
		inputChar.ki.wVk     = 0;
		inputChar.ki.wScan   = wCode;
		inputChar.ki.dwFlags = KEYEVENTF_UNICODE | KEYEVENTF_KEYUP;
		inputChar.ki.time    = 0;
	}
	else
	{
		inputChar.type       = INPUT_KEYBOARD;
		inputChar.ki.wVk     = wCode;
		inputChar.ki.wScan   = MapVirtualKey(inputChar.ki.wVk, 0);
		inputChar.ki.dwFlags = KEYEVENTF_KEYUP | KEYEVENTF_EXTENDEDKEY;
		inputChar.ki.time    = 0;
	}

	SendInput(1, &inputChar, sizeof(inputChar));
}

//////////////////////////////////////////////////////////////////////////
// обработка сообщения WM_TASKBARCREATED
//
LRESULT CQuickInputDlg::TaskbarCreated(WPARAM wParam, LPARAM lParam)
{
	ShowTrayIcon(FALSE);
	ShowTrayIcon(TRUE);

	return 0;
}

//////////////////////////////////////////////////////////////////////////
// обработка сообщения WM_KILLFOCUS от хука
//
LRESULT CQuickInputDlg::ProcessKillFocus(WPARAM wParam, LPARAM lParam)
{
	if (lParam != GetWindowThreadProcessId(m_wndListWords->m_hWnd, NULL))
	{
		ListWordsShow(FALSE);
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////
// поиск слова в БД
//
BOOL CQuickInputDlg::SearchWordInDB(CString strWord)
{
	CString        strQuery;
	TCHAR          tcSQLQuery[SQL_MAXSIZE];
	INT            iReturn;
	SQLITE3_STMT   *pStmt;

	// формирование запроса к БД
	strWord.Replace(_T("\'"),_T("\'\'"));
	strQuery.Format(_T("SELECT WORD FROM ")_T(TABLE_WORDS)_T(" WHERE WORD LIKE '%s';"), strWord);

	ZeroMemory(tcSQLQuery, SQL_MAXSIZE);
	_tcscpy(tcSQLQuery, strQuery);

	// заполнение списка слов
	iReturn = sqlite3_prepare(GetSQLiteDB(), tcSQLQuery, -1, &pStmt, NULL);
	if (SQLITE_OK != iReturn)
	{
		MessageBox(_T("search::sqlite3_prepare::select::")_T(TABLE_WORDS), _T("ERROR"), MB_OK | MB_ICONERROR);

		sqlite3_finalize(pStmt);

		return FALSE;
	}

	iReturn = sqlite3_step(pStmt);
	if (SQLITE_ROW == iReturn)
	{
		sqlite3_finalize(pStmt);

		return TRUE;
	}

	sqlite3_finalize(pStmt);

	return FALSE;
}

//////////////////////////////////////////////////////////////////////////
// возвращает версию продукта
//
CString CQuickInputDlg::GetProductVersion()
{
	CString strVersion;
	CString strPath;
	DWORD   dwDummy = 0;
	DWORD   dwSize  = 0;

	GetModuleFileName(theApp.m_hInstance, strPath.GetBufferSetLength(MAX_PATH), MAX_PATH);
	if (dwSize = GetFileVersionInfoSize(strPath, &dwDummy))
	{
		BYTE* pBuf = new BYTE[dwSize];

		if (GetFileVersionInfo(strPath, 0, dwSize, pBuf))
		{
			UINT   uiLen;
			LPVOID lpVSFFI;

			if (VerQueryValue(pBuf, L"\\", &lpVSFFI, &uiLen))
			{
				VS_FIXEDFILEINFO vsFFI;

				CopyMemory(&vsFFI, lpVSFFI, sizeof(VS_FIXEDFILEINFO));
				if (VS_FFI_SIGNATURE == vsFFI.dwSignature)
				{
					strVersion.Format(L" v%d.%d", vsFFI.dwProductVersionMS / 1000, vsFFI.dwProductVersionMS);
				}
			}
		}

		delete [] pBuf;
	}

	return strVersion;
}

//////////////////////////////////////////////////////////////////////////
// возвращает версию Windows
//
EWindowsVersion CQuickInputDlg::GetWindowsVersion()
{
	OSVERSIONINFO osVersion;

	if (GetVersionEx(&osVersion))
	{
		if (5 < osVersion.dwMajorVersion)
		{
			return WV_NONE;
		}

		switch (osVersion.dwMinorVersion)
		{
			case 0:
				return WV_2000;
				break;

			case 1:
				return WV_XP;
				break;

			case 2:
				return WV_2003;
				break;

			default:
				return WV_VISTA;
		}
	}

	return WV_NONE;
}

//////////////////////////////////////////////////////////////////////////
// переключение режима работы программы
//
VOID CQuickInputDlg::ChangeUserWorking()
{
	if (TRUE == m_bUserWork)
	{
		m_bUserWork = FALSE;
	}
	else
	{
		m_bUserWork = TRUE;
	}

	ShowTrayIcon(TRUE, TRUE);
}

//////////////////////////////////////////////////////////////////////////
// обработка хоткея
//
LRESULT CQuickInputDlg::ProcessHotKey(WPARAM wParam, LPARAM lParam)
{
	// проверка обрабатываемого хоткея
	TRACE(L"ProcessHotKey!\n");

	// сохранение клипбоарда
	if (NULL != m_pcbBackup)
	{
		delete m_pcbBackup;
	}
	m_pcbBackup = new CClipboardBackup();

	// запуск треда ожидания изменения clipboard
	ResetEvent(m_hEventClipboard);
	AfxBeginThread((AFX_THREADPROC)CopyThread, this, THREAD_PRIORITY_NORMAL, 0, 0);

	// посылаем CTRL + Ins активному приложению
	INPUT in[4];

	ZeroMemory (&in, sizeof (in));

	in[0].type = in[1].type = in[2].type = in[3].type = INPUT_KEYBOARD;
	in[0].ki.wVk = in[2].ki.wVk = VK_CONTROL;
	in[1].ki.wVk = in[3].ki.wVk = VK_INSERT;
	in[0].ki.time = in[1].ki.time = in[2].ki.time = in[3].ki.time = GetTickCount();
	in[2].ki.dwFlags = in[3].ki.dwFlags = KEYEVENTF_KEYUP;
	SendInput (4, in, sizeof (INPUT));

	return 0;
}

//////////////////////////////////////////////////////////////////////////
// обработка WM_DRAWCLIPBOARD - изменение clipboard
//
VOID CQuickInputDlg::OnDrawClipboard()
{
	CDialog::OnDrawClipboard();

	SetEvent(m_hEventClipboard);

	if (IsWindow(m_hNextViewer))
	{
		::PostMessage(m_hNextViewer, WM_DRAWCLIPBOARD, 0, 0);
	}
}

//////////////////////////////////////////////////////////////////////////
// обработка WM_CHANGECBCHAIN - удаление наблюдателя clipboard
//
VOID CQuickInputDlg::OnChangeCbChain(HWND hWndRemove, HWND hWndAfter)
{
	CDialog::OnChangeCbChain(hWndRemove, hWndAfter);

	if (m_hNextViewer == hWndRemove)
	{
		m_hNextViewer = hWndAfter;
	}
	else
	{
		if (IsWindow(m_hNextViewer))
		{
			::PostMessage(m_hNextViewer, WM_CHANGECBCHAIN, (WPARAM)hWndRemove, (LPARAM)hWndAfter);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// копирование из clipboard выбраного слова/строки
//
INT CopyThread(LPVOID lpParam)
{
	CQuickInputDlg   *pDlg           = (CQuickInputDlg*)lpParam;
	HANDLE           hEventClipboard = pDlg->GetEventClipboard();
	CClipboardBackup *pcbBackup      = pDlg->GetCLipboardBackup();

	TRACE(L"CopyThread\n");

	// ожидаем обновления клипбоарда
	if (WAIT_OBJECT_0 == WaitForSingleObject(hEventClipboard, WAITCLIPBOARD))
	{
		// выдираем из буфера обмена строку
		CString strAdd;

		if (OpenClipboard(NULL))
		{
			HGLOBAL hG = GetClipboardData(CF_UNICODETEXT);

			if (NULL != hG)
			{
				LPWSTR lpWStr = (LPWSTR)GlobalLock(hG);

				if (NULL != lpWStr)
				{
					strAdd = lpWStr;

					GlobalUnlock(hG);
				}
			}

			CloseClipboard();
		}

		// заносим строку в БД
		pDlg->ShowBalloonTip(strAdd);
		pDlg->SaveWordInDB(strAdd, TRUE);

		// восстановление clipboard
		pcbBackup->Restore();
	}

	ExitThread(0);

	return 0;
}

//////////////////////////////////////////////////////////////////////////
// регистрация хоткея для добавления слова в словарь
//
BOOL CQuickInputDlg::RegisterHotkeyAdd(BOOL bRegister/* = TRUE*/)
{
	UnregisterHotKey(GetSafeHwnd(), 0);

	if (FALSE == bRegister)
	{
		return true;
	}

	// установка хоткея
	UINT uiMod = 0;

	if (HOTKEYF_ALT & m_uiHKAddMod)
	{
		uiMod |= MOD_ALT;
	}
	if (HOTKEYF_SHIFT & m_uiHKAddMod)
	{
		uiMod |= MOD_SHIFT;
	}
	if (HOTKEYF_CONTROL & m_uiHKAddMod)
	{
		uiMod |= MOD_CONTROL;
	}

	if (!RegisterHotKey(GetSafeHwnd(), 0, uiMod, m_uiHKAddVK))
	{
		MessageBox(L"ERROR::RegisterHotKey");
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
// тред для скрытия balloon tip через BALLONTIPTIME мс.
//
INT HideBalloonTip(LPVOID lpParam)
{
	CQuickInputDlg *pDlg = (CQuickInputDlg*)lpParam;

	Sleep(BALLOONTIPTIME);

	pDlg->ShowBalloonTip(L"");

	return 0;
}