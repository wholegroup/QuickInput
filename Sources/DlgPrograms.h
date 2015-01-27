#pragma once

//
class CDlgPrograms : public CDialog
{
	DECLARE_DYNAMIC(CDlgPrograms)
	DECLARE_MESSAGE_MAP()

	//
	enum { IDD = IDD_PROGRAMS };

	public:

		// standard constructor
		CDlgPrograms(CWnd* pParent = NULL);   

		//
		virtual ~CDlgPrograms();

	protected:

		CStringList strListRunning;  //
		CStringList strListChosen;   //
		CImageList  m_listIcons;     //

	protected:

		//
		virtual BOOL OnInitDialog();

		//
		virtual VOID OnOK();

		//
		virtual VOID OnCancel();

		//
		afx_msg VOID OnShowWindow(BOOL bShow, UINT nStatus);

		//
		afx_msg VOID Add();

		//
		afx_msg VOID AddAll();

		//
		afx_msg VOID Remove();

		//
		afx_msg VOID RemoveAll();

		//
		afx_msg VOID OnChangeChosenWork();

		// добавляет строку в листбокс
		BOOL AddString(CListBox *listBox, CString *strAdd, CString *strPath = NULL);

		// callback функция для перебора активных приложения
		static BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam);

	public:

		// заполняет список запущенных программ
		VOID FillLists();

		// возвращает количество в списке выбранных программ
		UINT GetCount()	{ return ((CListBox*)GetDlgItem(IDC_CHOSENPROGRAMS))->GetCount();	}

		// возвращает указанную строку
		VOID GetString(INT i, CString &strReturn, CString &strPath)
		{
			((CListBox*)GetDlgItem(IDC_CHOSENPROGRAMS))->GetText(i, strReturn); 
			strPath = strListChosen.GetAt(strListChosen.FindIndex(i));
		}

		// вставляет строку в список выбранных программ
		VOID AddChosenProgram(CString &strInsert, CString &strPath)
		{
			AddString((CListBox*)GetDlgItem(IDC_CHOSENPROGRAMS), &strInsert, &strPath);
		}
};
