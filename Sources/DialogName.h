#pragma once

//
class CDialogName : public CStatic
{
	DECLARE_MESSAGE_MAP()

	public:

		//
		CDialogName();

		//
		virtual ~CDialogName();

	protected:

		CString m_strCaption;  //
		DWORD   m_dwColorText; //
		CFont   m_cFontText;   //

	public:

		//
		VOID SetWindowText(LPCTSTR lpszString);

	protected:

		//
		afx_msg VOID OnPaint();
};


