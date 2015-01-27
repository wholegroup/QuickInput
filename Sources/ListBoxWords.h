#pragma once

//////////////////////////////////////////////////////////////////////////
// список слов для быстрого ввода
//
class CListBoxWords : public CListBox
{
	DECLARE_MESSAGE_MAP()

	public:

		//
		CListBoxWords();

		//
		virtual ~CListBoxWords();

	protected:	

		CBrush   m_cBackground;    // кисть для закрашивания общего фона
		COLORREF m_clrBGSelection; // цвет фона выделенной строки
		COLORREF m_clrText;        // цвет текста
		CFont    *m_fontText;      // шрифт

	protected:

		//
		virtual VOID DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);

		//
		virtual VOID MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct);

		//
		afx_msg HBRUSH CtlColor(CDC* pDC, UINT nCtlColor);

	public:

		// установка цвета фона
		VOID SetBackground(COLORREF clrBackground);

		// установка цвета фона выделенной строки
		VOID SetBGSelection(COLORREF clrBGSelection);

		// установка цвета текста
		VOID SetTextColor(COLORREF clrTextB);

		// установка шрифта
		VOID SetFont(CFont *fontText = NULL);

		// возвращает текущий шрифт
		CFont* GetFont() {return m_fontText;}
};


