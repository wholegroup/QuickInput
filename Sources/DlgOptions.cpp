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

IMPLEMENT_DYNAMIC(CDlgOptions, CDialog)

BEGIN_MESSAGE_MAP(CDlgOptions, CDialog)
	ON_CBN_SELCHANGE(IDC_CBLANGUAGES, ChangeLanguage)
	ON_BN_CLICKED(IDC_CHECKRUNSTART, CheckRunStart)
	ON_BN_CLICKED(IDC_CHECKRUNMIN, CheckRunMin)
	ON_CBN_SELCHANGE(IDC_CBMINLETTER, ChangeMinLetter)
	ON_BN_CLICKED(IDC_CHECKINSSPACE, CheckInsSpace)
	ON_BN_CLICKED(IDC_CHECKAUTOADD, CheckAutoAdd)
	ON_BN_CLICKED(IDC_CHECKSPACEDOWN, CheckSpaceDown)
	ON_CBN_SELCHANGE(IDC_CBLETTERSSHOW, ChangeLettersShow)
	ON_BN_CLICKED(IDC_CHECKHOTKEYADD, CheckHotkeyAdd)
END_MESSAGE_MAP()

CDlgOptions *dlgOptions = NULL;

//////////////////////////////////////////////////////////////////////////
// перебор всех доступных локалей
//
BOOL CALLBACK CDlgOptions::EnumLocalesProc(LPTSTR lpLocaleString)
{
	dlgOptions->strLocale.AddTail(lpLocaleString);

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
// конструктор
//
CDlgOptions::CDlgOptions(CWnd* pParent)
	: CDialog(CDlgOptions::IDD, pParent)
{
	dlgOptions = this;
}

//////////////////////////////////////////////////////////////////////////
// деструктор
//
CDlgOptions::~CDlgOptions()
{
	dlgOptions = NULL;
}

//////////////////////////////////////////////////////////////////////////
// инициализация диалога
//
BOOL CDlgOptions::OnInitDialog()
{
	CDialog::OnInitDialog();

	CQuickInputDlg *pDlgQI = ((CQuickInputDlg*)(GetParent()->GetParent()));

	// получение списка всех возможных языков
	if (0 == strLocale.GetCount())
	{
		EnumSystemLocales(&CDlgOptions::EnumLocalesProc, LCID_INSTALLED);
	}

	// поиск всех возможных языков программы
	CComboBox *cbLanguages = (CComboBox*)GetDlgItem(IDC_CBLANGUAGES);
	HINSTANCE hAppInstance;
	HRSRC     hrRes;
	TCHAR     tcName[128]; 
	INT       iLocale = 0;

	hAppInstance = GetModuleHandle(NULL);;

	for (WORD i = 0; i < strLocale.GetCount(); ++i)
	{
		_stscanf(strLocale.GetAt(strLocale.FindIndex(i)), _T("%x"), &iLocale);

		hrRes = FindResourceEx(hAppInstance, RT_DIALOG, MAKEINTRESOURCE(IDD_QUICKINPUT_DIALOG), iLocale);
		if (hrRes)
		{
			iLanguageID.AddTail(iLocale);

			ZeroMemory(tcName, sizeof(TCHAR) * 128);
			GetLocaleInfo(iLocale, LOCALE_SNATIVELANGNAME, tcName, 128);

			cbLanguages->AddString(tcName);

			hrRes = NULL;

			if (pDlgQI->GetLanguage() == iLocale)
			{
				cbLanguages->SetCurSel(cbLanguages->GetCount() - 1);
			}
		}
	}

	if (CB_ERR == cbLanguages->GetCurSel())
	{
		cbLanguages->SetCurSel(0);
	}

	// инициализация параметров
	CButton     *checkRunStart  = (CButton*)GetDlgItem(IDC_CHECKRUNSTART);
	CButton     *checkRunMin    = (CButton*)GetDlgItem(IDC_CHECKRUNMIN);
	CButton     *checkInsSpace  = (CButton*)GetDlgItem(IDC_CHECKINSSPACE);
	CButton     *checkSpaceDown = (CButton*)GetDlgItem(IDC_CHECKSPACEDOWN);
	CButton     *checkAutoAdd   = (CButton*)GetDlgItem(IDC_CHECKAUTOADD);
	CComboBox   *cbMinLetter    = (CComboBox*)GetDlgItem(IDC_CBMINLETTER);
	CButton     *checkHotkeyAdd = (CButton*)GetDlgItem(IDC_CHECKHOTKEYADD);
	CHotKeyCtrl *hkAdd          = (CHotKeyCtrl*)GetDlgItem(IDC_HOTKEYADD);
	CComboBox   *cbLettersShow  = (CComboBox*)GetDlgItem(IDC_CBLETTERSSHOW);

	// запускать при старте Windows
	if (pDlgQI->GetRunStart())
	{
		checkRunStart->SetCheck(TRUE);
	}
	else
	{
		checkRunStart->SetCheck(FALSE);
	}

	// минимизировать при старте
	if (pDlgQI->GetRunMin())
	{
		checkRunMin->SetCheck(TRUE);
	}
	else
	{
		checkRunMin->SetCheck(FALSE);
	}

	// ставить завершающий пробел
	if (pDlgQI->GetInsSpace())
	{
		checkInsSpace->SetCheck(TRUE);
	}
	else
	{
		checkInsSpace->SetCheck(FALSE);
	}

	// перемещаться по списку слов с помощью пробела
	if (pDlgQI->GetSpaceDown())
	{
		checkSpaceDown->SetCheck(TRUE);
	}
	else
	{
		checkSpaceDown->SetCheck(FALSE);
	}

	// добавлять автоматом слово в словарь
	if (pDlgQI->GetAutoAdd())
	{
		checkAutoAdd->SetCheck(TRUE);
		cbMinLetter->EnableWindow(TRUE);
	}
	else
	{
		checkAutoAdd->SetCheck(FALSE);
		cbMinLetter->EnableWindow(FALSE);
	}

	// минимальное количество символов в слове
	cbMinLetter->ResetContent();

	for (UINT i = MINLETTERNUMBER; i <= MAXLETTERNUMBER; i++)
	{
		CString strTmp;

		strTmp.Format(_T("%d"), i);
		cbMinLetter->AddString(strTmp);
	}

	cbMinLetter->SetCurSel(pDlgQI->GetMinLetter() - MINLETTERNUMBER);

	// добавлять выделенное слово в словарь по нажатию hotkey
	if (pDlgQI->GetHotkeyAdd())
	{
		checkHotkeyAdd->SetCheck(TRUE);
		hkAdd->EnableWindow(TRUE);
	}
	else
	{
		checkHotkeyAdd->SetCheck(FALSE);
		hkAdd->EnableWindow(FALSE);
	}

	hkAdd->SetHotKey(pDlgQI->GetHKAddVK(), pDlgQI->GetHKAddMod());

	// минимальное количество букв для вывода списка
	cbLettersShow->ResetContent();

	for (UINT i = MINLETTERNUMBER; i <= MAXLETTERNUMBER; i++)
	{
		CString strTmp;

		strTmp.Format(_T("%d"), i);
		cbLettersShow->AddString(strTmp);
	}

	cbLettersShow->SetCurSel(pDlgQI->GetLettersShow() - MINLETTERNUMBER);

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
// смена языка  — перезагрузка программы
//
void CDlgOptions::ChangeLanguage()
{
	CQuickInputDlg *dlgQI = ((CQuickInputDlg*)(GetParent()->GetParent()));
	CComboBox      *cbLanguages = (CComboBox*)GetDlgItem(IDC_CBLANGUAGES);
	
	dlgQI->SetLanguage(iLanguageID.GetAt(iLanguageID.FindIndex(cbLanguages->GetCurSel())));
	dlgQI->Restart();
}

//////////////////////////////////////////////////////////////////////////
// обработка IDOK
//
VOID CDlgOptions::OnOK()
{
	CQuickInputDlg *pDlgQI = ((CQuickInputDlg*)(GetParent()->GetParent()));
	CHotKeyCtrl    *hkAdd  = (CHotKeyCtrl*)GetDlgItem(IDC_HOTKEYADD);

	WORD wVK  = 0;
	WORD wMod = 0;

	hkAdd->GetHotKey(wVK, wMod);
	pDlgQI->SetHKAddVK(wVK);
	pDlgQI->SetHKAddMod(wMod);

	return;
}

//////////////////////////////////////////////////////////////////////////
// обработка IDCANCEL
//
VOID CDlgOptions::OnCancel()
{
	return;
}

//////////////////////////////////////////////////////////////////////////
// обработка переключения чекбокса — запускать программу при старте ОС
//
VOID CDlgOptions::CheckRunStart()
{
	CQuickInputDlg *dlgQI         = ((CQuickInputDlg*)(GetParent()->GetParent()));
	CButton        *checkRunStart = (CButton*)GetDlgItem(IDC_CHECKRUNSTART);

	dlgQI->SetRunStart(checkRunStart->GetCheck());
}


//////////////////////////////////////////////////////////////////////////
// обработка переключения чекбокса — запускать свернутой в трей
//
VOID CDlgOptions::CheckRunMin()
{
	CQuickInputDlg *dlgQI       = ((CQuickInputDlg*)(GetParent()->GetParent()));
	CButton        *checkRunMin = (CButton*)GetDlgItem(IDC_CHECKRUNMIN);

	dlgQI->SetRunMin(checkRunMin->GetCheck());
}


//////////////////////////////////////////////////////////////////////////
// обработка изменения минимального количества букв в слове
//
VOID CDlgOptions::ChangeMinLetter()
{
	CQuickInputDlg *dlgQI       = ((CQuickInputDlg*)(GetParent()->GetParent()));
	CComboBox      *cbMinLetter = (CComboBox*)GetDlgItem(IDC_CBMINLETTER);

	dlgQI->SetMinLetter(MINLETTERNUMBER + cbMinLetter->GetCurSel());
}


//////////////////////////////////////////////////////////////////////////
// обработка изменения вставки завершающего пробела
//
VOID CDlgOptions::CheckInsSpace()
{
	CQuickInputDlg *dlgQI         = ((CQuickInputDlg*)(GetParent()->GetParent()));
	CButton        *checkInsSpace = (CButton*)GetDlgItem(IDC_CHECKINSSPACE);

	dlgQI->SetInsSpace(checkInsSpace->GetCheck());
}


//////////////////////////////////////////////////////////////////////////
// обработка изменения автоматического дополнения слова в словаре
//
VOID CDlgOptions::CheckAutoAdd()
{
	CQuickInputDlg *dlgQI        = ((CQuickInputDlg*)(GetParent()->GetParent()));
	CButton        *checkAutoAdd = (CButton*)GetDlgItem(IDC_CHECKAUTOADD);
	CComboBox      *cbMinLetter  = (CComboBox*)GetDlgItem(IDC_CBMINLETTER);

	if (checkAutoAdd->GetCheck())
	{
		cbMinLetter->EnableWindow(TRUE);
	}
	else
	{
		cbMinLetter->EnableWindow(FALSE);
	}

	dlgQI->SetAutoAdd(checkAutoAdd->GetCheck());
}


//////////////////////////////////////////////////////////////////////////
// обработка изменения перемещения по списку с помощью пробела
//
VOID CDlgOptions::CheckSpaceDown()
{
	CQuickInputDlg *dlgQI          = ((CQuickInputDlg*)(GetParent()->GetParent()));
	CButton        *checkSpaceDown = (CButton*)GetDlgItem(IDC_CHECKSPACEDOWN);

	dlgQI->SetSpaceDown(checkSpaceDown->GetCheck());
}


//////////////////////////////////////////////////////////////////////////
// обработка изменения количества символов для вывода списка
//
VOID CDlgOptions::ChangeLettersShow()
{
	CQuickInputDlg *dlgQI         = ((CQuickInputDlg*)(GetParent()->GetParent()));
	CComboBox      *cbLettersShow = (CComboBox*)GetDlgItem(IDC_CBLETTERSSHOW);

	dlgQI->SetLettersShow(MINLETTERNUMBER + cbLettersShow->GetCurSel());
}


//////////////////////////////////////////////////////////////////////////
// обработка изменения сохранения выделенного слова в словаре по нажатию hotkey
//
VOID CDlgOptions::CheckHotkeyAdd()
{
	CQuickInputDlg *dlgQI          = ((CQuickInputDlg*)(GetParent()->GetParent()));
	CButton        *checkHotkeyAdd = (CButton*)GetDlgItem(IDC_CHECKHOTKEYADD);
	CHotKeyCtrl    *hkAdd          = (CHotKeyCtrl*)GetDlgItem(IDC_HOTKEYADD);

	if (checkHotkeyAdd->GetCheck())
	{
		hkAdd->EnableWindow(TRUE);
	}
	else
	{
		hkAdd->EnableWindow(FALSE);
	}

	dlgQI->SetHotkeyAdd(checkHotkeyAdd->GetCheck());
}
