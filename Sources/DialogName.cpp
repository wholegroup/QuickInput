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
#include "QuickInput.h"
#include "DialogName.h"
#include "DialogList.h"

#ifdef _DEBUG
	#define new DEBUG_NEW
#undef THIS_FILE
	static char THIS_FILE[] = __FILE__;
#endif

BEGIN_MESSAGE_MAP(CDialogName, CStatic)
	ON_WM_PAINT()
END_MESSAGE_MAP()

//
CDialogName::CDialogName()
{
	m_dwColorText = GetSysColor(COLOR_WINDOWTEXT);

	m_cFontText.CreateFont(
		32,                        // nHeight
		0,                         // nWidth
		0,                         // nEscapement
		0,                         // nOrientation
		FW_NORMAL,                   // nWeight
		FALSE,                     // bItalic
		FALSE,                     // bUnderline
		0,                         // cStrikeOut
		ANSI_CHARSET,              // nCharSet
		OUT_DEFAULT_PRECIS,        // nOutPrecision
		CLIP_DEFAULT_PRECIS,       // nClipPrecision
		DEFAULT_QUALITY,           // nQuality
		DEFAULT_PITCH | FF_SWISS,  // nPitchAndFamily
		_T("Tahoma"));              // lpszFacename
}

//
CDialogName::~CDialogName()
{
}

//
void CDialogName::OnPaint() 
{
	CPaintDC dc(this);
	CRect    rectClient;
	CFont*   pFont = GetParent()->GetFont();
	CSize    szText;

	// получение координат клиентской области
	GetClientRect(&rectClient);

	// создание контекста в памяти
	CMyMemDC memDC(&dc, &rectClient);

	// установка шрифта
	if (pFont)
	{
		memDC.SelectObject(&m_cFontText);
	}

	// закраска фона
	memDC.FillSolidRect(rectClient, GetSysColor(COLOR_BTNFACE));

	// вывод текста
	memDC.SetTextColor(m_dwColorText);
	memDC.SetBkMode(TRANSPARENT);

	szText = memDC.GetTextExtent(m_strCaption);
	memDC.TextOut(0, rectClient.top + (rectClient.Height() >> 1) - (szText.cy >> 1), m_strCaption); 
}

//
VOID CDialogName::SetWindowText(LPCTSTR lpszString)
{
	m_strCaption = lpszString;
}