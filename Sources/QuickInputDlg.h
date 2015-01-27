#pragma once

#define WM_TRAY        WM_USER + 11 // сообщение из трея
#define WM_STARTHIDE   WM_USER + 12 // сообщение на скрытие окна при старте

#define SHMEMSIZE      2048         // размер разделяемой памяти

#define DB_NAME        "setting.db" // имя файла с БД
#define TABLE_OPTIONS  "options"    // имя таблицы с настройками
#define TABLE_WORDS    "words"      // имя таблицы словаря
#define TABLE_PROGRAMS "programs"   // имя таблицы с выбранными программами
#define SQL_MAXSIZE    2048         // максимальный размер SQL запроса
#define MAX_ROWS       512          // максимальное количество записей в выборке

#define WAITCLIPBOARD  1000         // время ожидания изменения clipboard (ms)

#define BALLOONTIPSIZE 128          // максимальная длинна текста в balloon tip
#define BALLOONTIPTIME 2500         // время показа ballon tip

#define LWMINHEIGHT    150          // минимальная высота списка слов 
#define LWMAXHEIGHT    500          // максимальная высота списка слов
#define LWMINWIDTH     200          // минимальная ширина списка слов
#define LWMAXWIDTH     500          // максимальная ширина списка слов

// шаблон файлов словарей
#define DBFILE_TPL     "Quick Input Dictionary (*.txt)\0*.txt\0\0"

// переопределение sqlite функций на UTF-16 при необходимости
#pragma warning(disable:4005)
	#ifdef _UNICODE
		#define sqlite3_open(a, b)             sqlite3_open16(a, b)
		#define sqlite3_prepare(a, b, c, d, e) sqlite3_prepare16(a, b, c, d, e)
		#define sqlite3_column_text(a, b)      sqlite3_column_text16(a, b)
	#endif
#pragma warning(default:4005)

#define SQLITE3      sqlite3
#define SQLITE3_STMT sqlite3_stmt

enum EWindowsVersion {WV_NONE = 0, WV_2000, WV_XP, WV_2003, WV_VISTA};

//////////////////////////////////////////////////////////////////////////
// определение класса главного диалога
//
class CQuickInputDlg : public CDialog
{
	DECLARE_MESSAGE_MAP()

	//
	enum { IDD = IDD_QUICKINPUT_DIALOG };

	public:

		//
		CQuickInputDlg(CWnd* pParent = NULL);

	protected:

		HANDLE      m_hSMemory;         // хендл созданной шаредмемори
		PVOID       m_pSMemory;         // указатель на созданную шаредмемори
		SQLITE3     *m_pDB;             // хендл открытой БД
		BOOL        m_bKeyDown;         //
		CString     m_strWord;          // текущее набранное слово
		CStringList *m_listAllWords;    // список слов
		CListWords  *m_wndListWords;    // окно со списком слов
		BOOL        m_bWork;            // флаг работы программы
		BOOL        m_bUserWork;        // флаг выключения работы программы пользователем 
		HANDLE      m_hEventClipboard;  // хендл event для работы с clipboard
		HWND        m_hNextViewer;      // текущий наблюдатель за clipboard
		CToolTipCtrl m_cToolTip;
		
		CClipboardBackup *m_pcbBackup;     // backup clipboard

		// параметры активного окна
		CPoint     m_ptCaret;           // позиция каретки
		HWND       m_hActiveWnd;        // хендл активного окна
		LONG       m_lActiveWndID;      // ИД элемента
		CString    m_strActiveWndClass; // имя класса
		CString    m_strActiveWndPath;  // полный путь к процессу которому принадлежит окно
		
		// диалог настроек
		CDialogName  m_cDlgName;        // имя диалога
		CDialogList  m_cDlgList;        // список диалогов
		CDlgAbout    *m_dlgAbout;       // диалог о программе
		CDlgOptions  *m_dlgOptions;     // диалог настроек
		CDlgColors   *m_dlgColors;      // настройка цветов списка быстрого ввода
		CDlgPrograms *m_dlgPrograms;    // указание программ
		CDlgEditDict *m_dlgEditDict;    // редактирования словаря

		// настройки программы
		DWORD      m_dwLanguage;        // идентификатор установленного языка
		UINT       m_uiTransparent;     // прозрачность окна быстрого ввода (от 0 до 100)
		CHARFORMAT m_charFormat;        // формат шрифта в окне быстрого ввода
		CFont      m_fontList;          // шрифт окна быстрого ввода
		COLORREF   m_colorBG;           // цвет фона окна быстрого ввода
		COLORREF   m_colorSelect;       // цвет выделения в окне быстрого ввода
		CHAR       m_cChosenWork;       // работать в выбранных программах (по умолчанию 0 - в НЕ выбранных)
		CHAR       m_cRunStart;         // запускать при старте ОС
		CHAR       m_cRunMin;           // запускать минимизированной
		UINT       m_uiMinLetter;       // минимальное количество букв в слове
		CHAR       m_cInsSpace;         // вставлять завершающий пробел при вставке слова
		CHAR       m_cAutoAdd;          // автоматически вносить вводимые слова в словарь
		CHAR       m_cSpaceDown;        // перемещаться по списку с помощью пробела
		CHAR       m_cHotkeyAdd;        // добавлять выделенное слово в словарь по нажатию hotkey
		UINT       m_uiHKAddVK;         // код виртуальной клавиши для m_cHotkeyAdd
		UINT       m_uiHKAddMod;        // модификатор для m_cHotkeyAdd
		UINT       m_uiLettersShow;     // минимальное количество букв для вывода списка слов
		UINT       m_uiListWordsX;      // координаты выпадающего списка слов
		UINT       m_uiListWordsY;      // 

	public:

		//
		virtual VOID DoDataExchange(CDataExchange* pDX);

		//
		virtual VOID OnOK();

		//
		virtual VOID OnCancel();

		//
		virtual INT  DoModal();

	protected:

		// инициализация диалога
		virtual BOOL OnInitDialog();

		// обработка изменения положения окна
		VOID OnWindowPosChanged(WINDOWPOS* lpwndpos);

		// обработка смены активного окна
		VOID OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);

		// обработка WM_DESTROY
		VOID OnDestroy();

		// выход из программы
		VOID Close();

		// кнопка "тест" - тестирование какой-либо фичи
		VOID TestFeatures();

		// показывает диалог "настройки"
		VOID ShowOptionsDlg();

		// показывает диалог "О Quick Input"
		VOID ShowAboutDlg();

		// показывает диалог "Внешний вид"
		VOID ShowColorsDlg();

		// показывает диалог "Программы"
		VOID ShowProgramsDlg();

		// показывает диалог "Редактирование словаря"
		VOID ShowEditDictDlg();

		// переключение режима работы программы
		VOID ChangeUserWorking();

		// для скрытия диалога при старте
		LRESULT HideDialog(WPARAM wParam, LPARAM lParam);

		// минимизирование окна
		VOID SetMinimized(BOOL bMinimized = TRUE);

		// установка/удаление иконки в трее
		VOID ShowTrayIcon(BOOL bShow = TRUE, BOOL bModify = FALSE, CString strText = L"");

		// обработка сообщений из трея
		LRESULT NotifyTrayIcon(WPARAM wParam, LPARAM lParam); 

		// загрузка настроек
		VOID LoadSettings();

		// сохранение настроек
		VOID SaveSettings();

		// создание таблиц в БД
		BOOL CreateTables();

		// обработка сообщения WM_CHAR от хука
		LRESULT ProcessChar(WPARAM wParam, LPARAM lParam);

		// обработка сообщения WM_KEYDOWN от хука
		LRESULT ProcessKeyDown(WPARAM wParam, LPARAM lParam);

		// обработка сообщения WM_KEYUP от хука
		LRESULT ProcessKeyUp(WPARAM wParam, LPARAM lParam);

		// обработка сообщения WM_SETFOCUS от хука
		LRESULT ProcessSetFocus(WPARAM wParam, LPARAM lParam);

		// обработка сообщения WM_KILLFOCUS от хука
		LRESULT ProcessKillFocus(WPARAM wParam, LPARAM lParam);

		// обработка сообщения WM_TASKBARCREATED
		LRESULT TaskbarCreated(WPARAM wParam, LPARAM lParam);

		// функция возвращает координаты каретки
		VOID GetQICaretPos(CPoint &ptCaret, INT &iHeightLetter);

		// поиск слова в БД
		BOOL SearchWordInDB(CString strWord);

		// функция считывает список слов из базы данных
		VOID ReadWordsFromDB();

		// функция увеличивается счетчик использования слова
		VOID AddCountWordsInDB(CString strWord);

		// функция скрывает окно со списком слов
		VOID ListWordsShow(BOOL bShow = TRUE, BOOL bErase = TRUE);

		// эмулирует нажатие кнопки на клаве
		VOID EmulatePressButton(WORD wCode, BOOL bUnicode = TRUE);

		// ставит обработку в хуке на паузу
		VOID PauseHook(BOOL bPause = TRUE);

		// возвращает версию продукта
		CString GetProductVersion();

		// возвращает версию Windows
		EWindowsVersion GetWindowsVersion();

		// обработка хоткеев
		LRESULT ProcessHotKey(WPARAM wParam, LPARAM lParam);

		// обработка WM_DRAWCLIPBOARD - изменение clipboard
		VOID OnDrawClipboard();

		// обработка WM_CHANGECBCHAIN - удаление наблюдателя clipboard
		VOID OnChangeCbChain(HWND hWndRemove, HWND hWndAfter);

		// регистрация хоткея для добавления слова в словарь
		BOOL RegisterHotkeyAdd(BOOL bRegister = TRUE);

	public:

		// рестарт программы - для смены языка
		VOID Restart();

		// функция записывает слово в базу данных
		VOID SaveWordInDB(CString strSaveWord, BOOL bForceAdd = FALSE);

		// загружает указанный ресурс
		HGLOBAL LoadResourceLang(LPCTSTR lpResType, DWORD dwResID);

		// загружает строковой ресурс
		CString LoadStringLang(DWORD dwResID);

		// прозрачность окна быстрого ввода
		VOID SetTransparent(UINT uiTransparent);
		UINT GetTransparent() {return m_uiTransparent;}

		// формат шрифта окна быстрого ввода
		VOID SetCharFormat(CHARFORMAT &charFormat);
		CHARFORMAT GetCharFormat() {return m_charFormat;}

		// цвета фона окна быстрого ввода
		VOID SetBGColor(COLORREF &colorBG) {m_colorBG = colorBG; m_wndListWords->SetBackground(colorBG);}
		COLORREF GetBGColor() {return m_colorBG;}

		// цвет выделения в окне быстрого ввода
		VOID SetSelectColor(COLORREF &colorSelect) { m_colorSelect = colorSelect; 
			m_wndListWords->SetBGSelection(colorSelect);}
		COLORREF GetSelectColor() {return m_colorSelect;}

		// загрузить список программ из БД
		VOID LoadChosenPrograms();

		// параметр - работать в выбранных программах
		CHAR GetChosenWork() {return m_cChosenWork;}
		VOID SetChosenWork(CHAR cChosenWork) {m_cChosenWork = cChosenWork;}

		// язык программы
		DWORD GetLanguage() {return m_dwLanguage;}
		VOID SetLanguage(DWORD dwLanguage) {m_dwLanguage = dwLanguage;}

		// запуск при старте винды
		CHAR GetRunStart() {return m_cRunStart;}
		VOID SetRunStart(CHAR cRunStart);

		// минимизация при запуске программы
		CHAR GetRunMin() {return m_cRunMin;}
		VOID SetRunMin(CHAR cRunMin) {m_cRunMin = cRunMin;}

		// минимальное количество символов в слове
		UINT GetMinLetter() {return m_uiMinLetter;}
		VOID SetMinLetter(UINT uiMinLetter) {m_uiMinLetter = uiMinLetter;}

		// установка завершающего пробела
		CHAR GetInsSpace() {return m_cInsSpace;}
		VOID SetInsSpace(CHAR cInsSpace) {m_cInsSpace = cInsSpace;}

		// автоматически вносить вводимые слова в словарь
		CHAR GetAutoAdd() {return m_cAutoAdd;}
		VOID SetAutoAdd(CHAR cAutoAdd) {m_cAutoAdd = cAutoAdd;}

		// перемещаться по списку слов с помощью пробела
		CHAR GetSpaceDown() {return m_cSpaceDown;}
		VOID SetSpaceDown(CHAR cSpaceDown) {m_cSpaceDown = cSpaceDown;}

		// добавлять выделенное слово в словарь по нажатию hotkey
		CHAR GetHotkeyAdd() {return m_cHotkeyAdd;}
		VOID SetHotkeyAdd(CHAR cHotkeyAdd) {m_cHotkeyAdd = cHotkeyAdd;}
		UINT GetHKAddVK() {return m_uiHKAddVK;}
		VOID SetHKAddVK(UINT uiHKAddVK) {m_uiHKAddVK = uiHKAddVK;}
		UINT GetHKAddMod() {return m_uiHKAddMod;}
		VOID SetHKAddMod(UINT uiHKAddMod) {m_uiHKAddMod = uiHKAddMod;}

		// минимальное количество символов для вывода списка слов
		UINT GetLettersShow() {return m_uiLettersShow;}
		VOID SetLettersShow(UINT uiLettersShow) {m_uiLettersShow = uiLettersShow;}

		// возвращает указатель на дескриптор БД
		SQLITE3* GetSQLiteDB() {return m_pDB;}

		// координаты выпадающего списка
		VOID SetListWordsX(UINT uiListWordsX) {m_uiListWordsX = uiListWordsX;}
		VOID SetListWordsY(UINT uiListWordsY) {m_uiListWordsY = uiListWordsY;}

		// возвращает хендл event для работы с clipboard
		HANDLE GetEventClipboard() {return m_hEventClipboard;}

		// возвращает указатель на backup clipboard
		CClipboardBackup* GetCLipboardBackup() {return m_pcbBackup;}

		// показывает balloon tip
		VOID ShowBalloonTip(CString strText) {ShowTrayIcon(TRUE, TRUE, strText);}
}; 
