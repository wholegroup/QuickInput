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

#ifdef _DEBUG
	#define new DEBUG_NEW
#undef THIS_FILE
	static char THIS_FILE[] = __FILE__;
#endif
	
BEGIN_MESSAGE_MAP(CListWords, CFrameWnd)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_WM_WINDOWPOSCHANGED()
END_MESSAGE_MAP()

//////////////////////////////////////////////////////////////////////////
// конструктор
//
CListWords::CListWords(CWnd *pWnd/* = NULL*/)
{
	m_wndQI = pWnd;

	m_iMinHeight = LWMINHEIGHT;
	m_iMaxHeight = LWMAXHEIGHT;

	m_iMinWidth  = LWMINWIDTH;
	m_iMaxWidth  = LWMAXWIDTH;
}

//////////////////////////////////////////////////////////////////////////
// деструктор
//
CListWords::~CListWords()
{
}

//////////////////////////////////////////////////////////////////////////
// инициализация свойств окна перед созданием
//
BOOL CListWords::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWnd::PreCreateWindow(cs))
	{
		return FALSE;
	}

	cs.style     |= WS_BORDER;
	cs.dwExStyle &= ~WS_EX_CLIENTEDGE;
	cs.dwExStyle &= ~WS_EX_TOOLWINDOW;
	cs.lpszClass = AfxRegisterWndClass(0);

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
// WM_CREATE
//
int CListWords::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	m_hwndLB = ::CreateWindowEx(WS_EX_LEFT | WS_EX_LTRREADING | WS_EX_RIGHTSCROLLBAR | 
		WS_EX_NOPARENTNOTIFY, _T("LISTBOX"), _T("MyLB"),
		WS_CHILDWINDOW | WS_VISIBLE | WS_VSCROLL | WS_TABSTOP | LBS_NOTIFY | 
		LBS_OWNERDRAWVARIABLE | LBS_HASSTRINGS | LBS_USETABSTOPS | LBS_NOINTEGRALHEIGHT/* | WS_BORDER*/, 
		0, 0, 50, 50, this->GetSafeHwnd(), NULL, 0, 0);
	
	m_lbText.SubclassWindow(m_hwndLB);

	return 0;
}

//////////////////////////////////////////////////////////////////////////
// WM_DESTROY
//
VOID CListWords::OnDestroy()
{
	CFrameWnd::OnDestroy();

	if (NULL != m_hwndLB)
	{
		::DestroyWindow(m_hwndLB);
	}
}

//////////////////////////////////////////////////////////////////////////
// сдвиг выделения вниз
//
VOID CListWords::SelectedDown()
{
	if (NULL == m_hwndLB)
	{
		return;
	}

	m_lbText.SetCurSel(m_lbText.GetCurSel() + 1);
}

//////////////////////////////////////////////////////////////////////////
// циклический двиг выделения вниз
//
VOID CListWords::SelectedDownCycle()
{
	if (NULL == m_hwndLB)
	{
		return;
	}

	if (m_lbText.GetCurSel() == (m_lbText.GetCount() - 1))
	{
		m_lbText.SetCurSel(0);
	}
	else
	{
		m_lbText.SetCurSel(m_lbText.GetCurSel() + 1);
	}
}

//////////////////////////////////////////////////////////////////////////
// сдвиг выделения вверх
//
VOID CListWords::SelectedUp()
{
	if (NULL == m_hwndLB)
	{
		return;
	}

	if (m_lbText.GetCurSel())
	{
		m_lbText.SetCurSel(m_lbText.GetCurSel() - 1);
	}
}

//////////////////////////////////////////////////////////////////////////
// возвращает текущую строку
//
VOID CListWords::GetSelected(CString &strReturn)
{
	INT i = m_lbText.GetCurSel();

	if (LB_ERR != i)
	{
		m_lbText.GetText(i, strReturn);
	}
	else
	{
		strReturn = _T("");
	}
}

//////////////////////////////////////////////////////////////////////////
// обновляет содержимое списка
//
VOID CListWords::RefreshList(CStringList* listAllWords, CString* strWord)
{
	POSITION pos;
	CString  strTmp;
	CString  strFindWord = *strWord;

	strFindWord.MakeLower();

	m_lbText.ResetContent();

	for (pos = listAllWords->GetHeadPosition(); NULL != pos; )
	{
		strTmp = listAllWords->GetAt(pos);

		if (0 == strTmp.Find(strFindWord))
		{
			m_lbText.AddString(strTmp);
		}

		listAllWords->GetNext(pos);
	}

	m_lbText.SetCurSel(0);
	m_lbText.SetFont();

	SetSize();
}

//////////////////////////////////////////////////////////////////////////
// обработка изменения размеров фрейма
//
void CListWords::OnSize(UINT nType, int cx, int cy)
{
	CFrameWnd::OnSize(nType, cx, cy);

	CRect rect;
	GetClientRect(&rect);

	if (NULL != m_hwndLB)
	{
		m_lbText.SetWindowPos(NULL, rect.left, rect.top, rect.Width(), rect.Height(), 0);
	}
}

//////////////////////////////////////////////////////////////////////////
// добавляет строку в список
//
VOID CListWords::AddString(TCHAR *tcAdd)
{
	if (NULL != m_hwndLB)
	{
		m_lbText.AddString(tcAdd);

		m_lbText.SetCurSel(0);
	}
}

//////////////////////////////////////////////////////////////////////////
// установка цвета фона
//
VOID CListWords::SetBackground(COLORREF clrBackground)
{
	m_lbText.SetBackground(clrBackground);
}

//////////////////////////////////////////////////////////////////////////
// установка цвета фона выделенной строки
//
VOID CListWords::SetBGSelection(COLORREF clrBGSelection)
{
	m_lbText.SetBGSelection(clrBGSelection);
}

//////////////////////////////////////////////////////////////////////////
// установка цвета текста
//
VOID CListWords::SetTextColor(COLORREF clrText)
{
	m_lbText.SetTextColor(clrText);
}

//////////////////////////////////////////////////////////////////////////
// установка шрифта
//
VOID CListWords::SetFont(CFont *fontText)
{
	m_lbText.SetFont(fontText);
}

//////////////////////////////////////////////////////////////////////////
// автоматическая установка размеров окна быстрого ввода
//
VOID CListWords::SetSize()
{
	LOGFONT lf;
	CFont   *fontText = m_lbText.GetFont();
	INT     iHeight = 0, iWidth = 0;
	CRect   rectDesktop;

	// получение параметров шрифта
	ZeroMemory(&lf, sizeof(LOGFONT));
	fontText->GetLogFont(&lf);

	// получение размеров десктопа
	ZeroMemory(&rectDesktop, sizeof(rectDesktop));
	GetDesktopWindow()->GetClientRect(rectDesktop);

	// получение стилей окна
	LONG lStyle = GetWindowLong(m_hWnd, GWL_STYLE);

	// вычисление высоты списка
	if (lf.lfHeight > 0)
	{
		iHeight = lf.lfHeight + 6;
	}
	else
	{
		iHeight = lf.lfHeight * -1 + 6;
	}

	iHeight *= m_lbText.GetCount();
	iHeight += 2;

	if ((lStyle & WS_CAPTION) & ~WS_BORDER)
	{
		TITLEBARINFO tbInfo;

		ZeroMemory(&tbInfo, sizeof(tbInfo));
		tbInfo.cbSize = sizeof(TITLEBARINFO);

		GetTitleBarInfo(&tbInfo);
		iHeight += (tbInfo.rcTitleBar.bottom - tbInfo.rcTitleBar.top) + 6;
	}

	if (iHeight < m_iMinHeight)
	{
		iHeight = m_iMinHeight;
	}

	if (iHeight > m_iMaxHeight)
	{
		iHeight = m_iMaxHeight;
	}

	// вычисление ширины списка
	CClientDC dc(this);
	CString   strCur;
	INT       iCurWidth;

	dc.SelectObject(fontText);

	for (INT i = 0; i < m_lbText.GetCount(); ++i)
	{
		m_lbText.GetText(i, strCur);

		iCurWidth = (dc.GetTextExtent(strCur)).cx;
		if (iCurWidth > iWidth)
		{
			iWidth = iCurWidth;
		}
	}

	iWidth += 3 * 2 + 1;

	if (iWidth < m_iMinWidth)
	{
		iWidth = m_iMinWidth;
	}

	if (iWidth > m_iMaxWidth)
	{
		iWidth = m_iMaxWidth;
	}

	// установка позиции списка
	CPoint ptPosition = m_ptPosition;

	if ((lStyle & WS_CAPTION) & ~WS_BORDER)
	{
		if ((ptPosition.y + iHeight) > rectDesktop.Height())
		{
			iHeight = rectDesktop.Height() - ptPosition.y;
		}
	}
	else
	{
		if ((ptPosition.y + m_iHeightLetter + m_iMinHeight) > rectDesktop.Height())
		{
			iHeight       = m_iMinHeight;
			ptPosition.y -= m_iMinHeight;
		}
		else
		{
			ptPosition.y += m_iHeightLetter;

			if ((ptPosition.y + iHeight) > rectDesktop.Height())
			{
				iHeight = rectDesktop.Height() - ptPosition.y;
			}
		}
	}

	if ((ptPosition.x + m_iMinWidth) > rectDesktop.Width())
	{
		iWidth        = m_iMinWidth;
		ptPosition.x -= m_iMinWidth;
	}
	else
	{
		if ((ptPosition.x + iWidth) > rectDesktop.Width())
		{
			iWidth = m_iMinWidth;
		}
	}

	SetWindowPos(&wndTopMost, ptPosition.x, ptPosition.y, iWidth, iHeight, 
		/*SWP_NOZORDER | */SWP_NOACTIVATE);
}

//////////////////////////////////////////////////////////////////////////
// показывает/скрывает title bar
//
VOID CListWords::ShowTitleBar(BOOL bShow/* = TRUE*/)
{
	LONG lStyle;

	lStyle = GetWindowLong(m_hWnd, GWL_STYLE);

	if (TRUE == bShow)
	{
		lStyle |= WS_CAPTION;
	}
	else
	{
		lStyle &= ~WS_CAPTION;
		lStyle |= WS_BORDER;
	}

	SetWindowLong(m_hWnd, GWL_STYLE, lStyle);
}

//////////////////////////////////////////////////////////////////////////
// обработка изменения позиции окна
//
VOID CListWords::OnWindowPosChanged(WINDOWPOS* lpwndpos)
{
	CFrameWnd::OnWindowPosChanged(lpwndpos);

	LONG lStyle;

	lStyle = GetWindowLong(m_hWnd, GWL_STYLE);
	if ((lStyle & WS_CAPTION) & ~WS_BORDER)
	{
		if (NULL != m_wndQI)
		{
			CQuickInputDlg *pDlgQI = (CQuickInputDlg*)m_wndQI;

			pDlgQI->SetListWordsX(lpwndpos->x);
			pDlgQI->SetListWordsY(lpwndpos->y);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// устанавливает позицию списка
//
VOID CListWords::SetPositionList(CPoint ptPosition, INT iHeightLetter/* = 0*/)
{
	m_ptPosition    = ptPosition;
	m_iHeightLetter = iHeightLetter;

	SetWindowPos(&wndTopMost, ptPosition.x, ptPosition.y, 0, 0, 
		SWP_NOSIZE | SWP_NOACTIVATE);
}
