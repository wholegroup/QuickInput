#pragma once

// имя компании
#define COMPANY_NAME "Whole Group (www.wholegroup.com)"

// имя программы (при смене не забыть поменять эту директиву у ХУКА)
#define PROGRAM_NAME "Quick Input"

// homepage url
#define HOMEPAGE_URL "http://www.qinput.com"

//
class CQuickInputApp : public CWinApp
{
	DECLARE_MESSAGE_MAP()

	public:

		//
		CQuickInputApp();

	public:

		//
		virtual BOOL InitInstance();
};

//
extern CQuickInputApp theApp;