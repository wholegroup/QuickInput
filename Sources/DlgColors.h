#pragma once

//
class CDlgColors : public CDialog
{
	DECLARE_DYNAMIC(CDlgColors)
	DECLARE_MESSAGE_MAP()

	//
	enum { IDD = IDD_COLORS };

	public:

		//
		CDlgColors(CWnd* pParent = NULL);

		//
		virtual ~CDlgColors();

	protected:

		CListWords *m_wndListWords; //
		BOOL       m_bNoShowList;   //
		CFont      m_fontList;      //
		COLORREF   m_clrBackground; //

	public:

		//
		afx_msg VOID OnWindowPosChanged(WINDOWPOS* lpwndpos);

		//
		VOID ActivateList(BOOL bActivate, CWnd* pWndOther, BOOL bMinimized);

	protected:

		//
		afx_msg VOID ChangeFont();

		//
		afx_msg VOID ChangeBackground();

		//
		afx_msg VOID ChangeSelectColor();

		//
		afx_msg VOID OnPaint();

		//
		afx_msg VOID OnShowWindow(BOOL bShow, UINT nStatus);

		//
		afx_msg VOID OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);

		//
		afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);

		//
		virtual BOOL OnInitDialog();

		//
		virtual VOID OnOK();

		//
		virtual VOID OnCancel();

		//
		VOID CreateList();

		//
		VOID DestroyList();

		//
		VOID SetFontList();
};
