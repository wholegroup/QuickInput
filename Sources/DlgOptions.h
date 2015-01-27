#pragma once

#define MINLETTERNUMBER 1
#define MAXLETTERNUMBER 9 

//
class CDlgOptions : public CDialog
{
	DECLARE_DYNAMIC(CDlgOptions)
	DECLARE_MESSAGE_MAP()

	//
	enum { IDD = IDD_OPTIONS };

	public:

		//
		CDlgOptions(CWnd* pParent = NULL);

		//
		virtual ~CDlgOptions();

	protected:

		CStringList     strLocale;   // лист с именами локалей
		CList<INT, INT> iLanguageID; // лист с доступными в программе LCID

	protected:

		// инициализация диалога
		virtual BOOL OnInitDialog();

		// обработка IDCANCEL
		virtual VOID OnCancel();

		// смена языка  — перезагрузка программы
		VOID ChangeLanguage();

		// обработка переключения чекбокса — запускать программу при старте ОС
		VOID CheckRunStart();

		// обработка переключения чекбокса — запускать свернутой в трей
		VOID CheckRunMin();

		// обработка изменения минимального количества букв в слове
		VOID ChangeMinLetter();

		// обработка изменения вставки завершающего пробела
		VOID CheckInsSpace();

		// обработка изменения автоматического дополнения слова в словаре
		VOID CheckAutoAdd();

		// обработка изменения перемещения по списку с помощью пробела
		VOID CheckSpaceDown();

		// обработка изменения количества символов для вывода списка
		VOID ChangeLettersShow();

		// перебор всех доступных локалей
		static BOOL CALLBACK EnumLocalesProc(LPTSTR lpLocaleString);

		// обработка изменения сохранения выделенного слова в словаре по нажатию hotkey
		VOID CheckHotkeyAdd();

	public:

		// обработка IDOK
		virtual VOID OnOK();
};
