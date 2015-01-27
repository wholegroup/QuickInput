#pragma once

//
class CListWords : public CFrameWnd
{
	DECLARE_MESSAGE_MAP()

	public:

		//
		CListWords(CWnd *pWnd = NULL);

		//
		virtual ~CListWords();

	protected:

		CWnd         *m_wndQI;
		HWND          m_hwndLB;
		CListBoxWords m_lbText;
		INT           m_iMaxHeight, m_iMinHeight; // минимум и максимум по высоте
		INT           m_iMaxWidth, m_iMinWidth;   // минимум и максимум по ширине
		CPoint        m_ptPosition;               // позиция списка
		INT           m_iHeightLetter;            // высота шрифта

	protected:

		//
		afx_msg INT  OnCreate(LPCREATESTRUCT lpCreateStruct);

		//
		afx_msg VOID OnDestroy();

		//
		afx_msg void OnSize(UINT nType, int cx, int cy);

		//
		virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

		// обработка изменения позиции окна
		afx_msg VOID OnWindowPosChanged(WINDOWPOS* lpwndpos);

	public:

		// сдвиг выделения вниз
		VOID SelectedDown();

		// циклический двиг выделения вниз
		VOID SelectedDownCycle();

		// сдвиг выделение вверх
		VOID SelectedUp();

		// возвращает количество строк
		INT GetCount() {return m_lbText.GetCount();}

		// возвращает текущую строку
		VOID GetSelected(CString &strReturn);

		// обновляет содержимое списка
		VOID RefreshList(CStringList* listAllWords, CString* strWord);

		// добавляет строку в список
		VOID AddString(TCHAR *tcAdd);

		// установка цвета фона
		VOID SetBackground(COLORREF clrBackground);

		// установка цвета фона выделенной строки
		VOID SetBGSelection(COLORREF clrBGSelection);

		// установка цвета текста
		VOID SetTextColor(COLORREF clrTextB);

		// установка шрифта
		VOID SetFont(CFont *fontText);

		// автоматическая установка размеров окна быстрого ввода
		VOID SetSize();

		// показывает/скрывает title bar
		VOID ShowTitleBar(BOOL bShow = TRUE);

		// устанавливает позицию списка
		VOID SetPositionList(CPoint ptPosition, INT iHeightLetter = 0);
};
