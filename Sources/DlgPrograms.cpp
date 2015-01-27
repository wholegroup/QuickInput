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
#include "DlgPrograms.h"
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
#include <psapi.h>

IMPLEMENT_DYNAMIC(CDlgPrograms, CDialog)

BEGIN_MESSAGE_MAP(CDlgPrograms, CDialog)
	ON_WM_SHOWWINDOW()
	ON_BN_CLICKED(IDB_PROGRAMS_ADD, &CDlgPrograms::Add)
	ON_BN_CLICKED(IDB_PROGRAMS_ADDALL, &CDlgPrograms::AddAll)
	ON_BN_CLICKED(IDB_PROGRAMS_REMOVE, &CDlgPrograms::Remove)
	ON_BN_CLICKED(IDB_PROGRAMS_REMOVEALL, &CDlgPrograms::RemoveAll)
	ON_BN_CLICKED(IDC_CHOSENNOTWORK, &CDlgPrograms::OnChangeChosenWork)
	ON_BN_CLICKED(IDC_CHOSENWORK, &CDlgPrograms::OnChangeChosenWork)
END_MESSAGE_MAP()

//////////////////////////////////////////////////////////////////////////
// конструктор
//
CDlgPrograms::CDlgPrograms(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgPrograms::IDD, pParent)
{
}

//////////////////////////////////////////////////////////////////////////
// деструктор
//
CDlgPrograms::~CDlgPrograms()
{
}

//////////////////////////////////////////////////////////////////////////
// инициализация
//
BOOL CDlgPrograms::OnInitDialog()
{
	CDialog::OnInitDialog();

	CQuickInputDlg *dlgQI = ((CQuickInputDlg*)(GetParent()->GetParent()));

	if (dlgQI->GetChosenWork())
	{
		((CButton*)GetDlgItem(IDC_CHOSENWORK))->SetCheck(TRUE);
		((CButton*)GetDlgItem(IDC_CHOSENNOTWORK))->SetCheck(FALSE);
	}
	else
	{
		((CButton*)GetDlgItem(IDC_CHOSENWORK))->SetCheck(FALSE);
		((CButton*)GetDlgItem(IDC_CHOSENNOTWORK))->SetCheck(TRUE);
	}

	// создание списка иконок программ
	m_listIcons.Create(16, 16, ILC_COLOR32 | ILC_MASK, 0, 3);

	// привязка списка иконок к списку программ
	CTreeCtrl *treeRunningPrograms = (CTreeCtrl*)GetDlgItem(IDC_TREERUNNING);
	treeRunningPrograms->SetImageList(&m_listIcons, LVSIL_NORMAL);

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
// WM_SHOWWINDOW
//
VOID CDlgPrograms::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialog::OnShowWindow(bShow, nStatus);

	if (bShow == TRUE)
	{
		FillLists();
	}
}

//////////////////////////////////////////////////////////////////////////
// заполнение списков
//
VOID CDlgPrograms::FillLists()
{
	CTreeCtrl *treeRunningPrograms;

	// очистка списка программ
	strListRunning.RemoveAll();

	// очистка списка иконок к программам
	for (INT i = 0; i < m_listIcons.GetImageCount(); i++)
	{
		m_listIcons.Remove(i);
	}

	// очистка окна со списком программ
	treeRunningPrograms = (CTreeCtrl*)GetDlgItem(IDC_TREERUNNING);
	treeRunningPrograms->DeleteAllItems();

	// заполнение списка
	EnumWindows((WNDENUMPROC)EnumWindowsProc, LPARAM(this));
}

//////////////////////////////////////////////////////////////////////////
// обработка IDOK
//
VOID CDlgPrograms::OnOK()
{
	return;

	CDialog::OnOK();
}

//////////////////////////////////////////////////////////////////////////
// обработка IDCANCEL
//
VOID CDlgPrograms::OnCancel()
{
	return;

	CDialog::OnCancel();
}

//////////////////////////////////////////////////////////////////////////
// добавление выбранной программы в список
//
VOID CDlgPrograms::Add()
{
	CTreeCtrl *treeRunningPrograms;
	CListBox  *listChosenPrograms;
	INT       iCurSel = 0;
	CString   strCurSel;
	CString   strFullPath;
 
	treeRunningPrograms = (CTreeCtrl*)GetDlgItem(IDC_TREERUNNING);
	listChosenPrograms  = (CListBox*)GetDlgItem(IDC_CHOSENPROGRAMS);

	// получаем номер выделенного элемента
	HTREEITEM hItem    = treeRunningPrograms->GetRootItem();
	HTREEITEM hSelItem = treeRunningPrograms->GetSelectedItem();

	if (NULL == hSelItem)
	{
		return;
	}

	while ((hItem != hSelItem) && (hItem = treeRunningPrograms->GetNextItem(hItem, TVGN_NEXT)))
	{
		iCurSel++;
	}

	strFullPath = strCurSel = strListRunning.GetAt(strListRunning.FindIndex(iCurSel));

	strCurSel.Delete(0, strCurSel.ReverseFind(_T('\\')) + 1);

	if (AddString(listChosenPrograms, &strCurSel))
	{
		strListChosen.AddTail(strFullPath);
	}
}

//////////////////////////////////////////////////////////////////////////
// добавление всех программ в список
//
VOID CDlgPrograms::AddAll()
{
	CTreeCtrl *treeRunningPrograms = (CTreeCtrl*)GetDlgItem(IDC_TREERUNNING);
	HTREEITEM hItem                = treeRunningPrograms->GetRootItem();

	if (NULL == hItem)
	{
		return;
	}

	do
	{
		treeRunningPrograms->SelectItem(hItem);

		Add();
	} while ((hItem = treeRunningPrograms->GetNextItem(hItem, TVGN_NEXT)));
}

//////////////////////////////////////////////////////////////////////////()
// удаление выбранной программы из списка
//
VOID CDlgPrograms::Remove()
{
	CListBox *listChosenPrograms;
	INT      iCurSel;

	listChosenPrograms = (CListBox*)GetDlgItem(IDC_CHOSENPROGRAMS);

	iCurSel = listChosenPrograms->GetCurSel();
	if (LB_ERR != iCurSel)
	{
		listChosenPrograms->DeleteString(iCurSel);
		
		strListChosen.RemoveAt(strListChosen.FindIndex(iCurSel));

		// вычисление размера по горизонтали
		CClientDC dcClient(this);
		CSize     sizeCurrent; 
		CString   strCurrent;
		INT       iWidth;

		dcClient.SelectObject(listChosenPrograms->GetFont());

		iWidth = 0;
		for (INT i = 0; i < listChosenPrograms->GetCount(); ++i)
		{
			listChosenPrograms->GetText(i, strCurrent);
			sizeCurrent = dcClient.GetTextExtent(strCurrent);
			if (iWidth < sizeCurrent.cx)
			{
				iWidth = sizeCurrent.cx;
			}
		}
		listChosenPrograms->SetHorizontalExtent(iWidth);
	}
}


//////////////////////////////////////////////////////////////////////////
// удаление всех программ из списка
//
VOID CDlgPrograms::RemoveAll()
{
	CListBox *listChosenPrograms;

	listChosenPrograms = (CListBox*)GetDlgItem(IDC_CHOSENPROGRAMS);
	listChosenPrograms->ResetContent();
	listChosenPrograms->SetHorizontalExtent(0);

	strListChosen.RemoveAll();
}


//////////////////////////////////////////////////////////////////////////
// добавляет указанную строку в указанный ListBox
// 
BOOL CDlgPrograms::AddString(CListBox *listBox, CString *strAdd, CString *strPath)
{
	CString strTest;

	// проверка существования
	for (INT i = 0; i < listBox->GetCount(); ++i)
	{
		listBox->GetText(i, strTest);

		if (strTest == *strAdd)
		{
			return FALSE;
		}
	}

	listBox->AddString(*strAdd);

	if (NULL != strPath)
	{
		strListChosen.AddTail(*strPath);
	}

	// вычисление размера по горизонтали
	CClientDC dcClient(this);
	CSize     sizeAdd; 
	
	dcClient.SelectObject(listBox->GetFont());

	sizeAdd     = dcClient.GetTextExtent(*strAdd);
	sizeAdd.cx += 3 * ::GetSystemMetrics(SM_CXBORDER);

	if (sizeAdd.cx > listBox->GetHorizontalExtent())
	{
		listBox->SetHorizontalExtent(sizeAdd.cx);
	}

	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
// Обработка радиобатонов
//
VOID CDlgPrograms::OnChangeChosenWork()
{
	CQuickInputDlg *dlgQI = ((CQuickInputDlg*)(GetParent()->GetParent()));

	if (((CButton*)GetDlgItem(IDC_CHOSENWORK))->GetCheck())
	{
		dlgQI->SetChosenWork(1);
	}
	else
	{
		dlgQI->SetChosenWork(0);
	}
}


//////////////////////////////////////////////////////////////////////////
// callback функция для перебора активных приложения
//
BOOL CALLBACK CDlgPrograms::EnumWindowsProc(HWND hWnd, LPARAM lParam)
{
	if (0 == GetWindowLong(hWnd, GWL_HWNDPARENT))
	{
		CDlgPrograms *dlgPrograms = (CDlgPrograms*)lParam;
		CString      strName;

		if (CWnd::FromHandle(hWnd)->IsWindowVisible())
		{
			CWnd::FromHandle(hWnd)->GetWindowText(strName);
			if (strName.GetLength() && (strName != _T("Program Manager")) && (strName != _T("Quick Input")))
			{
				DWORD   dwProcessID;
				HANDLE  hProcess;
				TCHAR   tcPath[MAX_PATH];

				GetWindowThreadProcessId(hWnd, &dwProcessID);
				hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwProcessID);
				if (hProcess)
				{
					ZeroMemory(tcPath, MAX_PATH);
					GetModuleFileNameEx(hProcess, NULL, tcPath, MAX_PATH);
					CloseHandle(hProcess);

					dlgPrograms->strListRunning.AddTail(tcPath);
				}
				else
				{
					dlgPrograms->strListRunning.AddTail(_T(""));
				}

				// получение иконки приложения
				HICON hIcon = NULL;

				if (SendMessageTimeout(hWnd, WM_GETICON, ICON_SMALL, 0, SMTO_ABORTIFHUNG | SMTO_BLOCK, 1000, (DWORD_PTR *)&hIcon))
				{
					if (NULL == hIcon)
					{
						if (!SendMessageTimeout(hWnd, WM_GETICON, ICON_BIG, 0, SMTO_ABORTIFHUNG|SMTO_BLOCK, 1000, (DWORD_PTR *)&hIcon))
						{
							hIcon = NULL;
						}
					}
				}
				else
				{
					hIcon = NULL;
				}

				if (NULL == hIcon)
				{
					hIcon = (HICON)GetClassLongPtr(hWnd, GCLP_HICONSM);

					if (NULL == hIcon)
					{
						hIcon = (HICON)GetClassLongPtr(hWnd, GCLP_HICON);

						if (NULL == hIcon)
						{
							hIcon = LoadIcon(NULL, IDI_APPLICATION);
						}
					}
				}

				dlgPrograms->m_listIcons.Add(hIcon);

				// занесение программы в список
				CTreeCtrl *treeView = (CTreeCtrl*)dlgPrograms->GetDlgItem(IDC_TREERUNNING);
				treeView->InsertItem(strName, dlgPrograms->m_listIcons.GetImageCount() - 1, dlgPrograms->m_listIcons.GetImageCount() - 1);
			}
		}
	}

	return TRUE;
}

