#pragma once


class CDlgEditWord : public CDialog
{
	DECLARE_DYNAMIC(CDlgEditWord)
	DECLARE_MESSAGE_MAP()

	enum { IDD = IDD_EDITWORD };

	public:
		CDlgEditWord(CWnd* pParent = NULL, CQuickInputDlg* pDlgQI = NULL);
		virtual ~CDlgEditWord();

	protected:
		CQuickInputDlg *m_pDlgQI;
		CString        m_strWord;

	protected:
		// обработка изменения слова
		afx_msg VOID ChangeWord();

		// инициализация диалога
		virtual BOOL OnInitDialog();

	public:
		// создание модального диалога
		virtual INT_PTR DoModal();

		// возвращает введенную строку
		inline CString GetWord() {return m_strWord;}

		// устанавливает текущую строку
		inline VOID SetWord(CString &strWord) {m_strWord = strWord;}
};
