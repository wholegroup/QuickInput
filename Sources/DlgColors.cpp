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

IMPLEMENT_DYNAMIC(CDlgColors, CDialog)

BEGIN_MESSAGE_MAP(CDlgColors, CDialog)
	ON_BN_CLICKED(IDB_CHANGEFONT, &CDlgColors::ChangeFont)
	ON_WM_WINDOWPOSCHANGED()
	ON_WM_SHOWWINDOW()
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDB_CHANGEBACKGROUND, &CDlgColors::ChangeBackground)
	ON_BN_CLICKED(IDB_CHANGECOLORSELECT, &CDlgColors::ChangeSelectColor)
	ON_WM_PAINT()
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()

//////////////////////////////////////////////////////////////////////////
//
//
CDlgColors::CDlgColors(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgColors::IDD, pParent)
{
	m_wndListWords = NULL;
	m_bNoShowList  = FALSE;
}

//////////////////////////////////////////////////////////////////////////
//
//
CDlgColors::~CDlgColors()
{
	if (NULL != m_wndListWords)
	{
		delete m_wndListWords;
	}
}

//////////////////////////////////////////////////////////////////////////
// инициализации диалога
//
BOOL CDlgColors::OnInitDialog()
{
	SetFontList();

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
// Диалог выбора шрифта окна быстрого ввода
//
VOID CDlgColors::ChangeFont()
{
	CQuickInputDlg *dlqQI = ((CQuickInputDlg*)(GetParent()->GetParent()));
	CHARFORMAT     charFormat = dlqQI->GetCharFormat();
	CFontDialog    dlgFont(charFormat);
	
	m_bNoShowList = TRUE;
	DestroyList();

	if (IDOK == dlgFont.DoModal())
	{
		dlgFont.GetCharFormat(charFormat);
		dlqQI->SetCharFormat(charFormat);

		SetFontList();
	}

	m_bNoShowList = FALSE;
	CreateList();
}

//////////////////////////////////////////////////////////////////////////
// обработка изменения позиции диалога
//
VOID CDlgColors::OnWindowPosChanged(WINDOWPOS* lpwndpos)
{
	if (NULL != lpwndpos)
	{
		CDialog::OnWindowPosChanged(lpwndpos);
	}

	if (NULL != m_wndListWords)
	{
		CRect rcTestList;
		GetDlgItem(IDC_TESTLIST)->GetWindowRect(&rcTestList);

		m_wndListWords->SetWindowPos(&wndTopMost, rcTestList.left, rcTestList.top, rcTestList.Width(), rcTestList.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE);
		m_wndListWords->ShowWindow(SW_SHOW);
	}
}

//////////////////////////////////////////////////////////////////////////
// Обработка WM_SHOW
//
VOID CDlgColors::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialog::OnShowWindow(bShow, nStatus);

	if (bShow)
	{
		CreateList();
	}
	else
	{
		DestroyList();
	}
}

//////////////////////////////////////////////////////////////////////////
// Обработка WM_ACTIVATE приложения
//
VOID CDlgColors::ActivateList(BOOL bActivate, CWnd* pWndOther, BOOL bMinimized)
{
	if (!IsWindowVisible())
	{
		DestroyList();

		return;
	}

	if ((NULL != pWndOther) && (pWndOther == m_wndListWords))
	{
		::PostMessage(m_hWnd, WM_SETFOCUS, NULL, NULL);

		return;
	}
	
	if ((NULL != pWndOther) && (this == pWndOther->GetParent()))
	{
		return;
	}

	if ((TRUE == bActivate))
	{
		CreateList();
	}
	else
	{
		DestroyList();
	}
}

//////////////////////////////////////////////////////////////////////////
// обработка нажатия ENTER
//
VOID CDlgColors::OnOK()
{
	return;

	CDialog::OnOK();
}

//////////////////////////////////////////////////////////////////////////
// обработка нажатия ESC
//
VOID CDlgColors::OnCancel()
{
	return;

	CDialog::OnCancel();
}

//////////////////////////////////////////////////////////////////////////
// создание тестового списка слов
//
VOID CDlgColors::CreateList()
{
	if (m_bNoShowList)
	{
		return;
	}
	
	if (NULL == m_wndListWords)
	{
		CSliderCtrl *slider = (CSliderCtrl*)GetDlgItem(IDC_SLIDER_TRANSPARENT);
		slider->SetRange(0, 100);
		slider->SetPos(((CQuickInputDlg*)(GetParent()->GetParent()))->GetTransparent());

		CString strTransparent;
		strTransparent.Format(_T("%d %%"), slider->GetPos());
		GetDlgItem(IDC_STATIC_TRANSPARENT)->SetWindowText(strTransparent);

		m_wndListWords = new CListWords();
		m_wndListWords->CreateEx(WS_EX_TOOLWINDOW, NULL, NULL, WS_POPUP, CRect(0, 0, 200, 100), GetDesktopWindow(), 0);

		CQuickInputDlg *dlgQI = (CQuickInputDlg*)(GetParent()->GetParent());

		m_wndListWords->AddString(dlgQI->LoadStringLang(IDS_TESTLIST_1).GetBuffer(0));
		m_wndListWords->AddString(dlgQI->LoadStringLang(IDS_TESTLIST_2).GetBuffer(0));
		m_wndListWords->AddString(dlgQI->LoadStringLang(IDS_TESTLIST_3).GetBuffer(0));
		m_wndListWords->AddString(dlgQI->LoadStringLang(IDS_TESTLIST_4).GetBuffer(0));
		m_wndListWords->AddString(dlgQI->LoadStringLang(IDS_TESTLIST_5).GetBuffer(0));
		m_wndListWords->AddString(dlgQI->LoadStringLang(IDS_TESTLIST_6).GetBuffer(0));
		m_wndListWords->AddString(dlgQI->LoadStringLang(IDS_TESTLIST_7).GetBuffer(0));
		m_wndListWords->AddString(dlgQI->LoadStringLang(IDS_TESTLIST_8).GetBuffer(0));
		m_wndListWords->AddString(dlgQI->LoadStringLang(IDS_TESTLIST_9).GetBuffer(0));
		m_wndListWords->AddString(dlgQI->LoadStringLang(IDS_TESTLIST_10).GetBuffer(0));
		m_wndListWords->AddString(dlgQI->LoadStringLang(IDS_TESTLIST_11).GetBuffer(0));
		m_wndListWords->AddString(dlgQI->LoadStringLang(IDS_TESTLIST_12).GetBuffer(0));

		m_wndListWords->SetFont(&m_fontList);
		m_wndListWords->SetTextColor(dlgQI->GetCharFormat().crTextColor);
		m_wndListWords->SetBackground(dlgQI->GetBGColor());
		m_wndListWords->SetBGSelection(dlgQI->GetSelectColor());

		::SetWindowLong(m_wndListWords->m_hWnd, GWL_EXSTYLE, GetWindowLong(m_wndListWords->m_hWnd, GWL_EXSTYLE) | WS_EX_LAYERED);
		::SetLayeredWindowAttributes(m_wndListWords->m_hWnd, 0, (255 * slider->GetPos()) / 100, LWA_ALPHA);

		OnWindowPosChanged(NULL);
	}
}

//////////////////////////////////////////////////////////////////////////
// удаление тестового списка слов
//
VOID CDlgColors::DestroyList()
{
	if (NULL != m_wndListWords)
	{
		m_wndListWords->DestroyWindow();

		m_wndListWords = NULL;
	}
}

//////////////////////////////////////////////////////////////////////////
// обработка сдвига слайдера установки прозрачности
//
void CDlgColors::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	if (pScrollBar == GetDlgItem(IDC_SLIDER_TRANSPARENT))
	{
		CSliderCtrl *slider = (CSliderCtrl*)pScrollBar;
		INT          iPos   = slider->GetPos();

		if (NULL != m_wndListWords)
		{
			::SetLayeredWindowAttributes(m_wndListWords->m_hWnd, 0, (255 * iPos) / 100, LWA_ALPHA);

			CString strTransparent;
			strTransparent.Format(_T("%d %%"), iPos);
			GetDlgItem(IDC_STATIC_TRANSPARENT)->SetWindowText(strTransparent);

			((CQuickInputDlg*)(GetParent()->GetParent()))->SetTransparent(iPos);
		}
	}

	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}

//////////////////////////////////////////////////////////////////////////
// изменение цвета фона списка быстрого ввода
//
VOID CDlgColors::ChangeBackground()
{
	CQuickInputDlg *dlgQI = ((CQuickInputDlg*)(GetParent()->GetParent()));
	COLORREF       colorBG = dlgQI->GetBGColor();
	CColorDialog   dlgColor;

	m_bNoShowList = TRUE;
	DestroyList();

	dlgColor.m_cc.Flags |= CC_FULLOPEN | CC_RGBINIT;
	dlgColor.m_cc.rgbResult = colorBG;

	if (IDOK == dlgColor.DoModal())
	{
		colorBG = dlgColor.GetColor();
		dlgQI->SetBGColor(colorBG);
	}

	m_bNoShowList = FALSE;
	CreateList();

	Invalidate();
}

//////////////////////////////////////////////////////////////////////////
// смена цвета выделения
// 
void CDlgColors::ChangeSelectColor()
{
	CQuickInputDlg *dlgQI = ((CQuickInputDlg*)(GetParent()->GetParent()));
	COLORREF       colorSelect = dlgQI->GetSelectColor();
	CColorDialog   dlgColor;

	m_bNoShowList = TRUE;
	DestroyList();

	dlgColor.m_cc.Flags |= CC_FULLOPEN | CC_RGBINIT;
	dlgColor.m_cc.rgbResult = colorSelect;

	if (IDOK == dlgColor.DoModal())
	{
		colorSelect = dlgColor.GetColor();
		dlgQI->SetSelectColor(colorSelect);
	}

	m_bNoShowList = FALSE;
	CreateList();

	Invalidate();
}

//////////////////////////////////////////////////////////////////////////
// обработка WM_PAINT
//
void CDlgColors::OnPaint()
{
	CPaintDC       dc(this);
	CQuickInputDlg *dlgQI = ((CQuickInputDlg*)(GetParent()->GetParent()));
	CWnd           *wndTmp;
	CRect          rect;

	wndTmp = GetDlgItem(IDC_STATICBGCOLOR);
	wndTmp->GetWindowRect(rect);
	ScreenToClient(rect);
	dc.FillSolidRect(rect.left, rect.top, rect.Width(), rect.Height(), dlgQI->GetBGColor());

	wndTmp = GetDlgItem(IDC_STATICSELECTCOLOR);
	wndTmp->GetWindowRect(rect);
	ScreenToClient(rect);
	dc.FillSolidRect(rect.left, rect.top, rect.Width(), rect.Height(), dlgQI->GetSelectColor());
}

//////////////////////////////////////////////////////////////////////////
// установка шрифта в тестовом окне
//
VOID CDlgColors::SetFontList()
{
	CWnd           *wndTmp;
	CQuickInputDlg *dlqQI = ((CQuickInputDlg*)(GetParent()->GetParent()));
	CHARFORMAT     charFormat = dlqQI->GetCharFormat();
	LOGFONT        lf;
	CClientDC      dc(this);

	ZeroMemory(&lf, sizeof(LOGFONT));

	_tcscpy(lf.lfFaceName, charFormat.szFaceName);
	lf.lfHeight         = -MulDiv(charFormat.yHeight / 20, dc.GetDeviceCaps(LOGPIXELSY), 72);
	lf.lfPitchAndFamily = charFormat.bPitchAndFamily;
	lf.lfUnderline      = charFormat.dwEffects & CFM_UNDERLINE ? TRUE : FALSE;
	lf.lfStrikeOut      = charFormat.dwEffects & CFM_STRIKEOUT ? TRUE : FALSE;
	lf.lfItalic         = charFormat.dwEffects & CFM_ITALIC ? TRUE : FALSE;
	lf.lfWeight         = charFormat.dwEffects & CFM_BOLD ? 700 : 400;
	lf.lfCharSet        = charFormat.bCharSet;

	m_fontList.DeleteObject();
	m_fontList.CreateFontIndirect(&lf);

	wndTmp = GetDlgItem(IDC_STATICFONT);
	wndTmp->SetFont(&m_fontList);
}

//////////////////////////////////////////////////////////////////////////
// WM_CTLCOLOR
//
HBRUSH CDlgColors::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	if (pWnd->GetDlgCtrlID() == IDC_STATICFONT)
	{
		CQuickInputDlg *dlqQI = ((CQuickInputDlg*)(GetParent()->GetParent()));
		CHARFORMAT     charFormat = dlqQI->GetCharFormat();

		pDC->SetTextColor(charFormat.crTextColor);
	}

	return hbr;
}
