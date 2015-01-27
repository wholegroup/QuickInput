/*
 * Copyright (C) 2015 Andrey Rychkov <wholegroup@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "stdafx.h"
#include "resource.h"
#include "QuickInput.h"
#include "HyperLink.h"
#include "DialogName.h"
#include "DialogList.h"
#include "ListBoxWords.h"
#include "ListWords.h"
#include "DlgColors.h"
#include "DlgPrograms.h"
#include "DlgAbout.h"
#include "DlgOptions.h"
#include "DlgEditDict.h"
#include "ClipboardBackup.h"
#include "QuickInputDlg.h"
#include "DlgAddWord.h"
#include "DlgEditWord.h"

IMPLEMENT_DYNAMIC(CDlgEditDict, CDialog)

BEGIN_MESSAGE_MAP(CDlgEditDict, CDialog)
	ON_BN_CLICKED(IDB_SEARCHWORD, &CDlgEditDict::SearchWord)
	ON_BN_CLICKED(IDB_ADDWORD, &CDlgEditDict::AddWord)
	ON_BN_CLICKED(IDB_EDITWORD, &CDlgEditDict::EditWord)
	ON_BN_CLICKED(IDB_DELWORD, &CDlgEditDict::DeleteWord)
	ON_BN_CLICKED(IDB_IMPORTDICT, &CDlgEditDict::ImportDictionary)
	ON_BN_CLICKED(IDB_EXPORTDICT, &CDlgEditDict::ExportDictionary)
	ON_EN_CHANGE(IDC_EDITSEARCHWORD, &CDlgEditDict::ChangeSearchWord)
	ON_WM_SHOWWINDOW()
END_MESSAGE_MAP()

//////////////////////////////////////////////////////////////////////////
// конструктор
//
CDlgEditDict::CDlgEditDict(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgEditDict::IDD, pParent)
{

}

//////////////////////////////////////////////////////////////////////////
// деструктор
//
CDlgEditDict::~CDlgEditDict()
{
}

//////////////////////////////////////////////////////////////////////////
// поиск слова в словаре
//
VOID CDlgEditDict::SearchWord()
{
	CQuickInputDlg *dlgQI         = ((CQuickInputDlg*)(GetParent()->GetParent()));
	CListBox       *plbWords      = (CListBox*)GetDlgItem(IDC_LISTWORDS);
	CEdit          *peSearchWord  = (CEdit*)GetDlgItem(IDC_EDITSEARCHWORD);
	CString        strSearchWord;
	CString        strQuery;
	TCHAR          tcSQLQuery[SQL_MAXSIZE];
	INT            iReturn;
	SQLITE3_STMT   *pStmt;

	// инициализация
	plbWords->ResetContent();
	peSearchWord->GetWindowText(strSearchWord);
	strSearchWord.MakeLower();

	// формирование запроса к БД
	strSearchWord.Replace(_T("\'"),_T("\'\'"));
	strQuery.Format(_T("SELECT DISTINCT WORD FROM ")_T(TABLE_WORDS)
		_T(" WHERE WORD LIKE '%s%%' ORDER BY WORD;"), strSearchWord);

	ZeroMemory(tcSQLQuery, SQL_MAXSIZE);
	_tcscpy(tcSQLQuery, strQuery);

	// заполнение списка слов
	iReturn = sqlite3_prepare(dlgQI->GetSQLiteDB(), tcSQLQuery, -1, &pStmt, NULL);
	if (SQLITE_OK != iReturn)
	{
		MessageBox(_T("Edit Dictionary::sqlite3_prepare::select::")
			_T(TABLE_WORDS), _T("ERROR"), MB_OK | MB_ICONERROR);

		sqlite3_finalize(pStmt);

		return;
	}

	iReturn = sqlite3_step(pStmt);
	while (SQLITE_ROW == iReturn)
	{
		plbWords->AddString((TCHAR*)sqlite3_column_text(pStmt, 0));

		iReturn = sqlite3_step(pStmt);
	}

	sqlite3_finalize(pStmt);

	SetInfoDictionary();
}

//////////////////////////////////////////////////////////////////////////
// добавление слова в словарь
//
VOID CDlgEditDict::AddWord()
{
	CQuickInputDlg *pdlgQI   = ((CQuickInputDlg*)(GetParent()->GetParent()));
	CListBox       *plbWords = (CListBox*)GetDlgItem(IDC_LISTWORDS);
	CDlgAddWord    dlg(this, pdlgQI);

	if (IDOK == dlg.DoModal())
	{
		CString strAddWord = dlg.GetWord();

		if (0 >= strAddWord.GetLength())
		{
			return;
		}

		strAddWord.MakeLower();

		if (FALSE == SearchWordInDB(strAddWord))
		{
			if (TRUE == InsertWordInDB(strAddWord))
			{
				plbWords->InsertString(0, strAddWord);

				SetInfoDictionary();
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// редактирование слова в словаре
//
VOID CDlgEditDict::EditWord()
{
	CQuickInputDlg *pdlgQI   = ((CQuickInputDlg*)(GetParent()->GetParent()));
	CListBox       *plbWords = (CListBox*)GetDlgItem(IDC_LISTWORDS);
	CDlgEditWord    dlg(this, pdlgQI);
	CString         strSelWord;

	// инициализация
	if (LB_ERR == plbWords->GetCurSel())
	{
		return;
	}

	plbWords->GetText(plbWords->GetCurSel(), strSelWord);
	strSelWord.MakeLower();

	dlg.SetWord(strSelWord);

	// изменение
	if (IDOK == dlg.DoModal())
	{
		CString strAddWord = dlg.GetWord();

		if (0 >= strAddWord.GetLength())
		{
			return;
		}

		if (strAddWord == strSelWord)
		{
			return;
		}

		if (TRUE == DeleteWordFromDB(strSelWord))
		{
			plbWords->DeleteString(plbWords->GetCurSel());
		}

		strAddWord.MakeLower();

		if (FALSE == SearchWordInDB(strAddWord))
		{
			if (TRUE == InsertWordInDB(strAddWord))
			{
				plbWords->InsertString(0, strAddWord);

				SetInfoDictionary();
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// удаление слова из словаря
//
VOID CDlgEditDict::DeleteWord()
{
	CQuickInputDlg *pdlgQI   = ((CQuickInputDlg*)(GetParent()->GetParent()));
	CListBox       *plbWords = (CListBox*)GetDlgItem(IDC_LISTWORDS);
	CString        strDeleteWord;
	CString        strQuery;

	// инициализация
	if (LB_ERR == plbWords->GetCurSel())
	{
		return;
	}

	// запрос на удаление
	if (IDYES == MessageBox(pdlgQI->LoadStringLang(IDS_DLGEDITDICT_DELETEWORD), NULL, 
		MB_YESNO | MB_ICONQUESTION))
	{
		// удаление
		plbWords->GetText(plbWords->GetCurSel(), strDeleteWord);

		if (TRUE == DeleteWordFromDB(strDeleteWord))
		{
			plbWords->DeleteString(plbWords->GetCurSel());

			SetInfoDictionary();
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// импорт словаря
//
VOID CDlgEditDict::ImportDictionary()
{
	CQuickInputDlg *pdlgQI = ((CQuickInputDlg*)(GetParent()->GetParent()));
	CListBox       *plbWords = (CListBox*)GetDlgItem(IDC_LISTWORDS);
	CFileDialog    dlgFile(TRUE, _T(""), _T(PROGRAM_NAME));

	dlgFile.GetOFN().lpstrFilter = _T(DBFILE_TPL);

	if (IDOK == dlgFile.DoModal())
	{
		CFile fileImport;

		if (fileImport.Open(dlgFile.GetPathName(), CFile::modeRead))
		{
			// проверка префикса юникодного файла
			WCHAR wcPrefixUnicode = 0;

			fileImport.Read(&wcPrefixUnicode, sizeof(wcPrefixUnicode));
			if (0xFEFF != wcPrefixUnicode)
			{
				MessageBox(pdlgQI->LoadStringLang(IDS_DLGEDITDICT_ERRORREAD), NULL, MB_OK | MB_ICONERROR);

				fileImport.Close();

				return;
			}

			sqlite3_exec(pdlgQI->GetSQLiteDB(), "BEGIN; ", NULL, NULL, NULL);

			// считывание списка слов
			WCHAR   *pwcBuffer;
			CString strWord;
			CString strSearch;
			INT     iFind;

			pwcBuffer = new WCHAR[1001];
			ZeroMemory(pwcBuffer, sizeof(WCHAR) * 1001);

			strSearch = _T("");

			while (fileImport.Read(pwcBuffer, sizeof(WCHAR) * 1000))
			{
				strSearch += pwcBuffer;

				while (-1 != (iFind = strSearch.Find(_T("\r\n"))))
				{
					strWord = strSearch.Left(iFind);
					strSearch.Delete(0, iFind + 2);

					// вставляем слово в БД
					if ((strWord.GetLength() > 0) && ((strWord.GetLength() <= 100)))
					{
						strWord.MakeLower();

						if (FALSE == SearchWordInDB(strWord))
						{
							if (TRUE == InsertWordInDB(strWord))
							{
								plbWords->InsertString(0, strWord);
							}
						}

						strWord = _T("");
					}
				}

				ZeroMemory(pwcBuffer, sizeof(WCHAR) * 1001);

				// вставляем слово в БД
				if ((strSearch.GetLength() > 0) && ((strSearch.GetLength() <= 100)))
				{
					strSearch.MakeLower();

					if (FALSE == SearchWordInDB(strSearch))
					{
						if (TRUE == InsertWordInDB(strSearch))
						{
							plbWords->InsertString(0, strSearch);
						}
					}

					strSearch = _T("");
				}
			}

			delete [] pwcBuffer;

			fileImport.Close();

			sqlite3_exec(pdlgQI->GetSQLiteDB(), "END; ", NULL, NULL, NULL);

			SetInfoDictionary();
		}
		else
		{
			MessageBox(_T("ERROR"), _T("ERROR"), MB_OK | MB_ICONERROR);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// экспорт словаря
//
VOID CDlgEditDict::ExportDictionary()
{
	CQuickInputDlg *pdlgQI = ((CQuickInputDlg*)(GetParent()->GetParent()));
	CFileDialog    dlgFile(FALSE, L"", TEXT(PROGRAM_NAME));

	dlgFile.GetOFN().lpstrFilter = _T(DBFILE_TPL);

	if (IDOK == dlgFile.DoModal())
	{
		CFile fileExport;

		if (fileExport.Open(dlgFile.GetPathName(), CFile::modeWrite | CFile::modeCreate))
		{
			// записываем префикс юникодного текстового файла
			WCHAR wcUnicodePrefix;

			wcUnicodePrefix = 0xFEFF;
			fileExport.Write(&wcUnicodePrefix, sizeof(wcUnicodePrefix));

			// записываем словарь
			CString      strQuery;
			TCHAR        tcSQLQuery[SQL_MAXSIZE];
			INT          iReturn;
			SQLITE3_STMT *pStmt;
			CString      strSaveWord;

			// формирование запроса к БД
			strQuery.Format(_T("SELECT DISTINCT WORD FROM ")_T(TABLE_WORDS)_T(" ORDER BY WORD;"));

			ZeroMemory(tcSQLQuery, SQL_MAXSIZE);
			_tcscpy(tcSQLQuery, strQuery);

			// заполнение списка слов
			iReturn = sqlite3_prepare(pdlgQI->GetSQLiteDB(), tcSQLQuery, -1, &pStmt, NULL);
			if (SQLITE_OK != iReturn)
			{
				MessageBox(_T("Export Dictionary::sqlite3_prepare::select::")
					_T(TABLE_WORDS), _T("ERROR"), MB_OK | MB_ICONERROR);

				sqlite3_finalize(pStmt);
				fileExport.Close();

				return;
			}

			iReturn = sqlite3_step(pStmt);

			while (SQLITE_ROW == iReturn)
			{
				strSaveWord = (TCHAR*)sqlite3_column_text(pStmt, 0);
				strSaveWord += _T("\r\n");

				fileExport.Write(strSaveWord, strSaveWord.GetLength() * sizeof(WCHAR));

				iReturn = sqlite3_step(pStmt);
			}

			sqlite3_finalize(pStmt);
			fileExport.Close();
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// обработка IDOK
//
VOID CDlgEditDict::OnOK()
{
	SearchWord();
}

//////////////////////////////////////////////////////////////////////////
// обработка IDCANCEL
//
VOID CDlgEditDict::OnCancel()
{
	return;
}

//////////////////////////////////////////////////////////////////////////
// поиск слова в БД
//
BOOL CDlgEditDict::SearchWordInDB(CString strWord)
{
	CQuickInputDlg *dlgQI = ((CQuickInputDlg*)(GetParent()->GetParent()));
	CString        strQuery;
	TCHAR          tcSQLQuery[SQL_MAXSIZE];
	INT            iReturn;
	SQLITE3_STMT   *pStmt;

	// формирование запроса к БД
	strWord.Replace(_T("\'"),_T("\'\'"));
	strQuery.Format(_T("SELECT WORD FROM ")_T(TABLE_WORDS)_T(" WHERE WORD LIKE '%s';"), strWord);

	ZeroMemory(tcSQLQuery, SQL_MAXSIZE);
	_tcscpy(tcSQLQuery, strQuery);

	// заполнение списка слов
	iReturn = sqlite3_prepare(dlgQI->GetSQLiteDB(), tcSQLQuery, -1, &pStmt, NULL);
	if (SQLITE_OK != iReturn)
	{
		MessageBox(_T("Search word in Dictionary::sqlite3_prepare::select::")
			_T(TABLE_WORDS), _T("ERROR"), MB_OK | MB_ICONERROR);

		sqlite3_finalize(pStmt);

		return FALSE;
	}

	iReturn = sqlite3_step(pStmt);

	if (SQLITE_ROW == iReturn)
	{
		sqlite3_finalize(pStmt);

		return TRUE;
	}

	sqlite3_finalize(pStmt);

	return FALSE;
}

//////////////////////////////////////////////////////////////////////////
// вставка слова в БД
//
BOOL CDlgEditDict::InsertWordInDB(CString strWord)
{
	CQuickInputDlg *dlgQI = ((CQuickInputDlg*)(GetParent()->GetParent()));
	CString        strQuery;
	TCHAR          tcSQLQuery[SQL_MAXSIZE];
	INT            iReturn;
	SQLITE3_STMT   *pStmt;

	// формирование запроса к БД
	strWord.Replace(_T("\'"),_T("\'\'"));
	strQuery.Format(_T("INSERT INTO ")_T(TABLE_WORDS)_T("(WORD) VALUES(\'%s\');"), strWord);

	ZeroMemory(tcSQLQuery, SQL_MAXSIZE);
	_tcscpy(tcSQLQuery, strQuery);

	// заполнение списка слов
	iReturn = sqlite3_prepare(dlgQI->GetSQLiteDB(), tcSQLQuery, -1, &pStmt, NULL);
	if (SQLITE_OK != iReturn)
	{
		MessageBox(_T("Insert Dictionary::sqlite3_prepare::insert::")
			_T(TABLE_WORDS), _T("ERROR"), MB_OK | MB_ICONERROR);

		sqlite3_finalize(pStmt);

		return FALSE;
	}

	iReturn = sqlite3_step(pStmt);
	if (SQLITE_DONE != iReturn)
	{
		MessageBox(_T("Insert Dictionary::sqlite3_step::insert::")
			_T(TABLE_WORDS), _T("ERROR"), MB_OK | MB_ICONERROR);

		sqlite3_finalize(pStmt);

		return FALSE;
	}

	sqlite3_finalize(pStmt);

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
// удаление слова из БД
//
BOOL CDlgEditDict::DeleteWordFromDB(CString strWord)
{
	CQuickInputDlg *dlgQI = ((CQuickInputDlg*)(GetParent()->GetParent()));
	CString        strQuery;
	TCHAR          tcSQLQuery[SQL_MAXSIZE];
	INT            iReturn;
	SQLITE3_STMT   *pStmt;

	// формирование запроса к БД
	strWord.Replace(_T("\'"),_T("\'\'"));
	strQuery.Format(_T("DELETE FROM ")_T(TABLE_WORDS)_T(" WHERE WORD LIKE '%s';"), strWord);

	ZeroMemory(tcSQLQuery, SQL_MAXSIZE);
	_tcscpy(tcSQLQuery, strQuery);

	// заполнение списка слов
	iReturn = sqlite3_prepare(dlgQI->GetSQLiteDB(), tcSQLQuery, -1, &pStmt, NULL);
	if (SQLITE_OK != iReturn)
	{
		MessageBox(_T("Delete Dictionary::sqlite3_prepare::delete::")
			_T(TABLE_WORDS), _T("ERROR"), MB_OK | MB_ICONERROR);

		sqlite3_finalize(pStmt);

		return FALSE;
	}

	iReturn = sqlite3_step(pStmt);
	if (SQLITE_DONE != iReturn)
	{
		MessageBox(_T("Delete Dictionary::sqlite3_step::delete::")
			_T(TABLE_WORDS), _T("ERROR"), MB_OK | MB_ICONERROR);

		sqlite3_finalize(pStmt);

		return FALSE;
	}

	sqlite3_finalize(pStmt);

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
// инициализация диалога
//
BOOL CDlgEditDict::OnInitDialog()
{
	CDialog::OnInitDialog();

	SearchWord();

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
// обработка смены слова
//
VOID CDlgEditDict::ChangeSearchWord()
{
	SearchWord();
}

//////////////////////////////////////////////////////////////////////////
// обработка WM_SHOWWINDOW
// 
VOID CDlgEditDict::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialog::OnShowWindow(bShow, nStatus);

	if (TRUE == bShow)
	{
		SearchWord();
	}
}

//////////////////////////////////////////////////////////////////////////
// формирование статистики по словарю
//
BOOL CDlgEditDict::SetInfoDictionary()
{
	CQuickInputDlg *pdlgQI = ((CQuickInputDlg*)(GetParent()->GetParent()));
	CString        strQuery;
	TCHAR          tcSQLQuery[SQL_MAXSIZE];
	INT            iReturn;
	SQLITE3_STMT   *pStmt;
	INT            iVisibleWords, iCountWords;
	CEdit          *peSearchWord  = (CEdit*)GetDlgItem(IDC_EDITSEARCHWORD);
	CString        strSearchWord;
	CString        strInfoDict;

	// инициализация
	iVisibleWords = 0;
	iCountWords   = 0;

	peSearchWord->GetWindowText(strSearchWord);
	strSearchWord.MakeLower();

	// показано слов
	CListBox       *plbWords = (CListBox*)GetDlgItem(IDC_LISTWORDS);
	iVisibleWords = plbWords->GetCount();

	/*
	// формирование запроса к БД
	strSearchWord.Replace(_T("\'"),_T("\'\'"));
	strQuery.Format(_T("SELECT DISTINCT COUNT(WORD) AS VISIBLEWORDS FROM ")
		_T(TABLE_WORDS)_T(" WHERE WORD LIKE '%s%%';"), strSearchWord);

	ZeroMemory(tcSQLQuery, SQL_MAXSIZE);
	_tcscpy(tcSQLQuery, strQuery);

	// заполнение списка слов
	iReturn = sqlite3_prepare(pdlgQI->GetSQLiteDB(), tcSQLQuery, -1, &pStmt, NULL);
	if (SQLITE_OK != iReturn)
	{
		MessageBox(_T("Set Info about Dictionary(visible)::sqlite3_prepare::select::")
			_T(TABLE_WORDS), _T("ERROR"), MB_OK | MB_ICONERROR);

		sqlite3_finalize(pStmt);

		return FALSE;
	}

	iReturn = sqlite3_step(pStmt);
	if (SQLITE_ROW == iReturn)
	{
		iVisibleWords = sqlite3_column_int(pStmt, 0);
	}

	sqlite3_finalize(pStmt);
	*/

	// общее количество слов в базе
	strQuery.Format(_T("SELECT COUNT(WORD) AS COUNTWORDS FROM ")_T(TABLE_WORDS)_T(";"));

	ZeroMemory(tcSQLQuery, SQL_MAXSIZE);
	_tcscpy(tcSQLQuery, strQuery);

	// заполнение списка слов
	iReturn = sqlite3_prepare(pdlgQI->GetSQLiteDB(), tcSQLQuery, -1, &pStmt, NULL);

	if (SQLITE_OK != iReturn)
	{
		MessageBox(_T("Set Info about Dictionary(all)::sqlite3_prepare::select::")
			_T(TABLE_WORDS), _T("ERROR"), MB_OK | MB_ICONERROR);

		sqlite3_finalize(pStmt);

		return FALSE;
	}

	iReturn = sqlite3_step(pStmt);

	if (SQLITE_ROW == iReturn)
	{
		iCountWords = sqlite3_column_int(pStmt, 0);
	}

	sqlite3_finalize(pStmt);

	// вывод статистики
	strInfoDict.Format(pdlgQI->LoadStringLang(IDS_DLGEDITDICT_INFODICT), iVisibleWords, iCountWords);
	SetDlgItemText(IDC_STATICINFODICT, strInfoDict);

	return TRUE;
}
