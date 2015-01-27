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
#include "DlgEditWord.h"

IMPLEMENT_DYNAMIC(CDlgEditWord, CDialog)

BEGIN_MESSAGE_MAP(CDlgEditWord, CDialog)
	ON_EN_CHANGE(IDC_EDITWORD_WORD, &CDlgEditWord::ChangeWord)
END_MESSAGE_MAP()

//////////////////////////////////////////////////////////////////////////
// конструктор
//
CDlgEditWord::CDlgEditWord(CWnd* pParent /*=NULL*/, CQuickInputDlg* pDlgQI/* = NULL*/)
	: CDialog()
{
	m_pParentWnd = pParent;
	m_pDlgQI     = pDlgQI;
	m_strWord    = _T("");
}

//////////////////////////////////////////////////////////////////////////
// деструктор
//
CDlgEditWord::~CDlgEditWord()
{
}

//////////////////////////////////////////////////////////////////////////
// создание модального диалога
//
INT_PTR CDlgEditWord::DoModal()
{
	// создание локализованного диалога
	if (!InitModalIndirect((LPDLGTEMPLATE)m_pDlgQI->LoadResourceLang(RT_DIALOG, this->IDD), m_pParentWnd))
		return IDCANCEL;

	return CDialog::DoModal();
}

//////////////////////////////////////////////////////////////////////////
// обработка изменения слова
//
VOID CDlgEditWord::ChangeWord()
{
	((CEdit*)GetDlgItem(IDC_EDITWORD_WORD))->GetWindowText(m_strWord);
}

//////////////////////////////////////////////////////////////////////////
// инициализация диалога
//
BOOL CDlgEditWord::OnInitDialog()
{
	((CEdit*)GetDlgItem(IDC_EDITWORD_WORD))->SetWindowText(m_strWord);

	return TRUE;
}
