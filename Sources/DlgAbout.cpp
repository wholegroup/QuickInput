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
#include "ListBoxWords.h"
#include "ListWords.h"
#include "clipboardbackup.h"
#include "DialogName.h"
#include "DialogList.h"
#include "DlgAbout.h"
#include "DlgPrograms.h"
#include "DlgEditDict.h"
#include "DlgOptions.h"
#include "DlgColors.h"
#include "QuickInputDlg.h"
#include "DlgAbout.h"

IMPLEMENT_DYNAMIC(CDlgAbout, CDialog)

BEGIN_MESSAGE_MAP(CDlgAbout, CDialog)
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()

//////////////////////////////////////////////////////////////////////////
// конструктор
//
CDlgAbout::CDlgAbout(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgAbout::IDD, pParent)
{
}

//////////////////////////////////////////////////////////////////////////
// деструктор
//
CDlgAbout::~CDlgAbout()
{
}

//////////////////////////////////////////////////////////////////////////
// инициализация диалога
//
BOOL CDlgAbout::OnInitDialog()
{
	CDialog::OnInitDialog();

	CQuickInputDlg *pDlgQI = ((CQuickInputDlg*)(GetParent()->GetParent()));

	m_hlHomePage.SubclassDlgItem(IDC_HL_HOMEPAGE, this);
	m_hlHomePage.SetURL(pDlgQI->LoadStringLang(IDS_DLGABOUT_HOMEPAGE), _T("Tahoma"), 10);

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
// обработка IDOK
//
VOID CDlgAbout::OnOK()
{
	return;
}

//////////////////////////////////////////////////////////////////////////
// обработка IDCANCEL
//
VOID CDlgAbout::OnCancel()
{
	return;
}

//////////////////////////////////////////////////////////////////////////
// окраска контролов
//
HBRUSH CDlgAbout::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	return hbr;
}
