#pragma once

class CHyperLink : public CButton
{
	DECLARE_DYNAMIC(CHyperLink)
	DECLARE_MESSAGE_MAP()

	public:
		CHyperLink();
		virtual ~CHyperLink();

	protected:
		BOOL     m_bMouseInside;
		HCURSOR  m_hLinkCursor;
		COLORREF m_clrLink;
		CString  m_strURL;
		CFont    m_fontLink;
		CFont    m_fontUnderline;

	protected:
		virtual VOID PreSubclassWindow();
		virtual VOID DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
		afx_msg VOID OnMouseMove(UINT nFlags, CPoint point);
		afx_msg VOID OnMouseLeave();
		afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
		afx_msg VOID OnLButtonDown(UINT nFlags, CPoint point);

	public:
		VOID SetURL(CString strUrl, CString strFontName = _T(""), UINT uiSize = 0);
};


