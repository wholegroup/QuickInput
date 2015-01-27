#pragma once

//
class CDlgAbout : public CDialog
{
	DECLARE_DYNAMIC(CDlgAbout)
	DECLARE_MESSAGE_MAP()

	//
	enum { IDD = IDD_ABOUT };

	public:

		//
		CDlgAbout(CWnd* pParent = NULL);

		//
		virtual ~CDlgAbout();

	protected:

		CHyperLink m_hlHomePage;  //
		CFont      m_fontRegText; //

	protected:

		// инициализация диалога
		virtual BOOL OnInitDialog();
		
		// обработка IDOK
		virtual VOID OnOK();
		
		// обработка IDCANCEL
		virtual VOID OnCancel();

		// окраска контролов
		afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
};
