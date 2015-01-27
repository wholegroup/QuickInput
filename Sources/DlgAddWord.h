#pragma once


class CDlgAddWord : public CDialog
{
	DECLARE_DYNAMIC(CDlgAddWord)
	DECLARE_MESSAGE_MAP()

	enum { IDD = IDD_ADDWORD };

	public:
		CDlgAddWord(CWnd* pParent = NULL, CQuickInputDlg* pDlgQI = NULL);
		virtual ~CDlgAddWord();

	protected:
		CQuickInputDlg *m_pDlgQI;
		CString        m_strWord;

	protected:
		// обработка изменения слова
		afx_msg VOID ChangeWord();

	public:
		// создание модального диалога
		virtual INT_PTR DoModal();

		// возвращает введенную строку
		inline CString GetWord() {return m_strWord;}
};
