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
#include "HyperLink.h"
#include "DialogList.h"
#include "QuickInput.h"

IMPLEMENT_DYNAMIC(CHyperLink, CButton)

BEGIN_MESSAGE_MAP(CHyperLink, CButton)
	ON_WM_SETCURSOR()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSELEAVE()
	ON_WM_LBUTTONDOWN()
END_MESSAGE_MAP()

//////////////////////////////////////////////////////////////////////////
// конструктор
//
CHyperLink::CHyperLink()
	:CButton()
{
	m_bMouseInside = FALSE;

	m_hLinkCursor = ::LoadCursor(NULL, IDC_HAND);

	if (!m_hLinkCursor)
	{
		m_hLinkCursor = ::LoadCursor(NULL, IDC_ARROW);
	}

	m_clrLink = RGB(0, 0, 0xFF);
	m_strURL  = _T("about:blank");
}

//////////////////////////////////////////////////////////////////////////
// деструктор
//
CHyperLink::~CHyperLink()
{
}

//////////////////////////////////////////////////////////////////////////
//
//
VOID CHyperLink::PreSubclassWindow()
{
	LOGFONT lf;

	GetFont()->GetLogFont(&lf);
	m_fontLink.CreateFontIndirect(&lf);

	lf.lfUnderline = lf.lfUnderline ? FALSE : TRUE;
	m_fontUnderline.CreateFontIndirect(&lf);
}

//////////////////////////////////////////////////////////////////////////
// отрисовка ссылки
//
VOID CHyperLink::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct) 
{
	CDC*     pDC = CDC::FromHandle(lpDrawItemStruct->hDC);
	CRect    rectItem = lpDrawItemStruct->rcItem;
	CMyMemDC memDC(pDC, &rectItem);
	CString  strText;

	// установка шрифта
	if (m_bMouseInside)
	{
		memDC.SelectObject(&m_fontUnderline);
	}
	else
	{
		memDC.SelectObject(m_fontLink);
	}

	// закраска фона
	memDC.FillSolidRect(rectItem, GetSysColor(COLOR_BTNFACE));

	// цвет текста
	memDC.SetTextColor(m_clrLink);

	// вывод текста по центру
	GetWindowText(strText);

	CSize sizeText = memDC.GetTextExtent(strText);
	rectItem.top  += (rectItem.Height() - sizeText.cy) / 2;
	rectItem.left += (rectItem.Width()  - sizeText.cx) / 2;;

	memDC.DrawText(strText, rectItem, 0);
};

//////////////////////////////////////////////////////////////////////////
// обработка движения мыши
//
afx_msg VOID CHyperLink::OnMouseMove(UINT nFlags, CPoint point)
{
	if (FALSE == m_bMouseInside)
	{
		TRACKMOUSEEVENT TrackMEvent;

		TrackMEvent.cbSize    = sizeof(TrackMEvent);
		TrackMEvent.hwndTrack = m_hWnd;
		TrackMEvent.dwFlags   = TME_LEAVE;

		if (TrackMouseEvent(&TrackMEvent))
		{
			m_bMouseInside = TRUE;

			Invalidate(FALSE);
		}
	}

	CButton::OnMouseMove(nFlags, point);
}

//////////////////////////////////////////////////////////////////////////
// обработка WM_MOUSELEAVE
//
afx_msg VOID CHyperLink::OnMouseLeave()
{
	m_bMouseInside = FALSE;

	Invalidate(FALSE);
}

//////////////////////////////////////////////////////////////////////////
// установка курсора
//
BOOL CHyperLink::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	if (m_bMouseInside)
	{
		::SetCursor(m_hLinkCursor);
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
// обработка нажатия - переход по ссылке
//
VOID CHyperLink::OnLButtonDown(UINT nFlags, CPoint point)
{
	SHELLEXECUTEINFO sei;

	::ZeroMemory(&sei, sizeof(SHELLEXECUTEINFO));

	sei.cbSize = sizeof( SHELLEXECUTEINFO );
	sei.lpVerb = TEXT("open");
	sei.lpFile = m_strURL;
	sei.nShow  = SW_SHOWNORMAL;

	ShellExecuteEx(&sei);
}

//////////////////////////////////////////////////////////////////////////
// установка значений гиперссылки
//
VOID CHyperLink::SetURL(CString strUrl, CString strFontName, UINT uiSize)
{
	m_strURL = strUrl;

	LOGFONT lf;

	GetFont()->GetLogFont(&lf);

	if (strFontName)
	{
		_tcscpy(lf.lfFaceName, strFontName);
	}

	if (uiSize)
	{
		lf.lfHeight = -MulDiv(uiSize, GetDeviceCaps(GetDC()->m_hDC, LOGPIXELSY), 72);;
	}

	m_fontLink.DeleteObject();
	m_fontLink.CreateFontIndirect(&lf);

	lf.lfUnderline = lf.lfUnderline ? FALSE : TRUE;
	m_fontUnderline.DeleteObject();
	m_fontUnderline.CreateFontIndirect(&lf);
}