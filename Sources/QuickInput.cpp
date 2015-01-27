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

#ifdef _DEBUG
	#define new DEBUG_NEW
#undef THIS_FILE
	static char THIS_FILE[] = __FILE__;
#endif

BEGIN_MESSAGE_MAP(CQuickInputApp, CWinApp)
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

//
CQuickInputApp theApp;

//
CQuickInputApp::CQuickInputApp()
{
}

//
BOOL CQuickInputApp::InitInstance()
{
	AfxEnableControlContainer();

	SetRegistryKey(_T(COMPANY_NAME));

	free((void*)m_pszProfileName);
	m_pszProfileName=_tcsdup(_T(PROGRAM_NAME));

	#ifdef _DEBUG
	CMemoryState msStart, msEnd, msDiff;
	msStart.Checkpoint();
	#endif

	BOOL fStart = TRUE;

	while (fStart)
	{
		fStart = FALSE;
			
		CQuickInputDlg dlg;

		INT nResponse = dlg.DoModal();

		if (IDOK == nResponse)
		{
		}
		else 

		if (IDCANCEL == nResponse)
		{
		}
		else 

		if (IDRESTART == nResponse)
		{
			fStart = TRUE;
		}
	}

	#ifdef _DEBUG
	msEnd.Checkpoint();

	if (msDiff.Difference(msStart,msEnd))
	{
		TRACE0("Memory leaked!\n");

		// в Output-окне отладчика выведется
		msDiff.DumpAllObjectsSince(); 

		// информация о созданных объектах		
		// и о динамической памяти вообще
		msDiff.DumpStatistics();      
	}
	#endif

	return FALSE;
}
