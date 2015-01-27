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
#include "ListBoxWords.h"
#include "DialogList.h"

BEGIN_MESSAGE_MAP(CListBoxWords, CListBox)
	ON_WM_CTLCOLOR_REFLECT()
END_MESSAGE_MAP()

//////////////////////////////////////////////////////////////////////////
//
//
CListBoxWords::CListBoxWords()
	:CListBox()
{
	m_fontText = NULL;
}

//////////////////////////////////////////////////////////////////////////
//
//
CListBoxWords::~CListBoxWords()
{
}

//////////////////////////////////////////////////////////////////////////
//
//
VOID CListBoxWords::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	CDC*     pDC = CDC::FromHandle(lpDrawItemStruct->hDC);
	CRect    rectItem = lpDrawItemStruct->rcItem;
	CMyMemDC memDC(pDC, &rectItem);
	CString  strItem;

	// установка шрифта
	if (m_fontText)
	{
		memDC.SelectObject(m_fontText);
	}

	// закраска фона
	if (lpDrawItemStruct->itemState & ODS_SELECTED)
	{
		memDC.FillSolidRect(rectItem, m_clrBGSelection);
	}
	else
	{
		memDC.FillRect(rectItem, &m_cBackground);
	}

	memDC.SetBkMode(TRANSPARENT);
	memDC.SetTextColor(m_clrText);

	GetText(lpDrawItemStruct->itemID, strItem);

	CSize sizeText = memDC.GetTextExtent(strItem);
	rectItem.top += (rectItem.Height() - sizeText.cy) / 2;
	rectItem.left += 3;

	memDC.DrawText(strItem, rectItem, 0);
}

//////////////////////////////////////////////////////////////////////////
//
//
VOID CListBoxWords::MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct)
{
}

//////////////////////////////////////////////////////////////////////////
// контроль за цветами
//
HBRUSH CListBoxWords::CtlColor(CDC* pDC, UINT nCtlColor)
{
	return m_cBackground;
}

//////////////////////////////////////////////////////////////////////////
// установка цвета фона
//
VOID CListBoxWords::SetBackground(COLORREF clrBackground)
{
	m_cBackground.DeleteObject();
	m_cBackground.CreateSolidBrush(clrBackground);
}

//////////////////////////////////////////////////////////////////////////
// установка цвета фона выделенной строки
//
VOID CListBoxWords::SetBGSelection(COLORREF clrBGSelection)
{
	m_clrBGSelection = clrBGSelection;
}

//////////////////////////////////////////////////////////////////////////
// установка цвета текста
//
VOID CListBoxWords::SetTextColor(COLORREF clrText)
{
	m_clrText = clrText;
}

//////////////////////////////////////////////////////////////////////////
// установка шрифта
//
VOID CListBoxWords::SetFont(CFont *fontText)
{
	LOGFONT lf;

	if (NULL != fontText)
	{
		fontText->GetLogFont(&lf);
	}
	else
	{
		m_fontText->GetLogFont(&lf);
	}

	for (INT i = 0; i < GetCount(); i++)
	{
		if (lf.lfHeight > 0)
		{
			SetItemHeight(i, lf.lfHeight * -1 + 6);
		}
		else

		if (lf.lfHeight < 0)
		{
			SetItemHeight(i, (lf.lfHeight * -1) + 6);
		}
	}

	if (NULL != fontText)
	{
		m_fontText = fontText;
	}
}

