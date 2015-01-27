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
#include "DialogList.h"
#include "DialogName.h"

#ifdef _DEBUG
	#define new DEBUG_NEW
#undef THIS_FILE
	static char THIS_FILE[] = __FILE__;
#endif

BEGIN_MESSAGE_MAP(CDialogList, CListBox)
	ON_WM_CTLCOLOR_REFLECT()
	ON_CONTROL_REFLECT(LBN_SELCHANGE, &CDialogList::SelectDialog)
END_MESSAGE_MAP()

//////////////////////////////////////////////////////////////////////////
// конструктор
//
CDialogList::CDialogList()
{
	m_uiItemHeight   = 48;
	m_dwBmpHeight    = 48;
	m_dwBmpWidth     = 48;
	m_dwBackground   = GetSysColor(COLOR_WINDOW);
	m_dwColorItem    = GetSysColor(COLOR_WINDOWTEXT);
	m_dwColorItemHlt = GetSysColor(COLOR_HIGHLIGHTTEXT);
	m_dwSelected     = GetSysColor(COLOR_HIGHLIGHT);
	m_wndCaption     = NULL;

	m_cBackground.DeleteObject();
	m_cBackground.CreateSolidBrush(m_dwBackground);
}

//////////////////////////////////////////////////////////////////////////
// деструктор
//
CDialogList::~CDialogList()
{
	m_listImg.DeleteImageList();
	m_listDlg.RemoveAll();
	m_listCaption.RemoveAll();
}

//////////////////////////////////////////////////////////////////////////
// инициализация
//
VOID CDialogList::PreSubclassWindow() 
{
	m_listImg.Create(m_dwBmpWidth, m_dwBmpHeight, ILC_COLOR32 | ILC_MASK, 0, 4);

	CListBox::PreSubclassWindow();
}

//////////////////////////////////////////////////////////////////////////
// отрисовка элемента
//
VOID CDialogList::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct) 
{
	CDC*     pDC = CDC::FromHandle(lpDrawItemStruct->hDC);
	CRect    rectItem = lpDrawItemStruct->rcItem;
	CMyMemDC memDC(pDC, &rectItem);
	CFont*   pFont = GetParent()->GetFont();
	CString  strItem;
	CSize    szItemText;
	CPoint   pointBmp;

	// установка шрифта
	if (pFont)
	{
		memDC.SelectObject(pFont);
	}

	// закраска фона
	if (lpDrawItemStruct->itemState & ODS_SELECTED)
	{
		memDC.FillSolidRect(rectItem, m_dwSelected);
		memDC.SetTextColor(m_dwColorItemHlt);
	}
	else
	{
		memDC.FillSolidRect(rectItem, m_dwBackground);
		memDC.SetTextColor(m_dwColorItem);
	}
	memDC.SetBkMode(TRANSPARENT);

	// вывод текста
	GetText(lpDrawItemStruct->itemID, strItem);
	szItemText = memDC->GetTextExtent(strItem);
	memDC.TextOut(rectItem.left + (rectItem.Width() >> 1) - (szItemText.cx >> 1), 
		rectItem.bottom - szItemText.cy, strItem); 

	// вывод картинки
	pointBmp.x = rectItem.left + (rectItem.Width() >> 1) - (m_dwBmpWidth >> 1);
	pointBmp.y = rectItem.top + ((rectItem.Height() - szItemText.cy) >> 1) - (m_dwBmpHeight >> 1);
	m_listImg.Draw(memDC, lpDrawItemStruct->itemID, pointBmp, ILD_TRANSPARENT);
}

//////////////////////////////////////////////////////////////////////////
// установка параметров
//
VOID CDialogList::MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct)
{
	lpMeasureItemStruct->itemHeight = m_uiItemHeight;
}

//////////////////////////////////////////////////////////////////////////
// Контроль за цветами
//
HBRUSH CDialogList::CtlColor(CDC* pDC, UINT nCtlColor)
{
	return m_cBackground;
}


//////////////////////////////////////////////////////////////////////////
// добавление элементов в список
//
VOID CDialogList::AddItem(TCHAR *tcItem, CDialog *cDlg, TCHAR *tcCaption, DWORD dwResID, UINT uiType)
{
	// добавляем строку в список
	AddString(tcItem);

	// добавляем диалог в список
	m_listDlg.Add(cDlg);

	// добавляем строку в список
	m_listCaption.Add(CString(tcCaption));

	// добавляем битмап в список
	CBitmap bmpBlank;

	// создание пустого битмапа
	bmpBlank.CreateBitmap(48, 48, 1, 32, NULL);

	// если битмап не задан, пихаем пустой
	if (!dwResID)
	{
		m_listImg.Add(&bmpBlank, TRANSPARENT_RGB);

		return;
	}

	// загрузка иконки из ресурсов
	if (IMAGE_ICON == uiType)
	{
		if (-1 == m_listImg.Add(AfxGetApp()->LoadIcon(dwResID)))
		{
			m_listImg.Add(&bmpBlank, TRANSPARENT_RGB);
		}
	}

	// загрузка битмапа из ресурсов
	else
	{
		CBitmap bmpTmp;

		bmpTmp.LoadBitmap(dwResID);

		if (-1 == m_listImg.Add(&bmpTmp, TRANSPARENT_RGB))
		{
			m_listImg.Add(&bmpBlank, TRANSPARENT_RGB);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
// обработка выбора диалога из списка
//
VOID CDialogList::SelectDialog()
{
	// установка выбранного диалога
	CDialog *dlgShow = m_listDlg.GetAt(GetCurSel());

	if (NULL != dlgShow)
	{
		// скрываем все окна
		for(INT i = 0; i < m_listDlg.GetCount(); i++)
		{
			CDialog *dlgHide;

			dlgHide = (CDialog*)(m_listDlg.GetAt(i));
			if ((NULL != dlgHide) && (dlgHide != dlgShow))
			{
				dlgHide->ShowWindow(SW_HIDE);
			}
		}

		dlgShow->ShowWindow(SW_SHOW);
		dlgShow->RedrawWindow();
	}

	// установка заголовка выбранного диалога
	CString     strTmp = m_listCaption.GetAt(GetCurSel());
	CDialogName *wnd   = (CDialogName*)m_wndCaption;
	wnd->SetWindowText(strTmp);
	wnd->RedrawWindow();

	return;
}
