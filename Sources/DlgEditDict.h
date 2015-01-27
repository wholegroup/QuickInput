#pragma once

//
class CDlgEditDict : public CDialog
{
	DECLARE_DYNAMIC(CDlgEditDict)
	DECLARE_MESSAGE_MAP()

	//
	enum {IDD = IDD_EDITDICT};

	public:

		//
		CDlgEditDict(CWnd* pParent = NULL);

		//
		virtual ~CDlgEditDict();

	protected:

		// инициализация диалога
		virtual BOOL OnInitDialog();

		// поиск слова в словаре
		afx_msg VOID SearchWord();

		// добавление слова в словарь
		afx_msg VOID AddWord();

		// редактирование слова в словаре
		afx_msg VOID EditWord();

		// удаление слова из словаря
		afx_msg VOID DeleteWord();

		// импорт словаря
		afx_msg VOID ImportDictionary();

		// экспорт словаря
		afx_msg VOID ExportDictionary();

		// обработка IDOK
		virtual VOID OnOK();

		// обработка IDCANCEL
		virtual VOID OnCancel();

		// поиск слова в БД
		BOOL SearchWordInDB(CString strWord);

		// вставка слова в БД
		BOOL InsertWordInDB(CString strWord);

		// удаление слова из БД
		BOOL DeleteWordFromDB(CString strWord);

		// обработка смены слова
		afx_msg VOID ChangeSearchWord();

		// обработка WM_SHOWWINDOW
		afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);

		// формирование статистики по словарю
		BOOL SetInfoDictionary();
};
