#if !defined(AFX_STDAFX_H__02A117D1_B6A1_4B13_A07E_FBFB9370A433__INCLUDED_)
#define AFX_STDAFX_H__02A117D1_B6A1_4B13_A07E_FBFB9370A433__INCLUDED_

#pragma once

#define _CRT_SECURE_NO_WARNINGS

#define WINVER         0x0501 // Change this to the appropriate value to target other versions of Windows.
#define _WIN32_WINNT   0x0501 // Change this to the appropriate value to target other versions of Windows.
#define _WIN32_WINDOWS 0x0410 // Change this to the appropriate value to target Windows Me or later.
#define _WIN32_IE      0x0600 // Change this to the appropriate value to target other versions of IE.

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxdisp.h>        // MFC Automation classes
#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#include "sqlite/sqlite3.h"
#include <shlobj.h>

#define IDRESTART 45753

// манифест
#if defined _M_IX86
  #pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
  #pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
  #pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
  #pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif

#endif // !defined(AFX_STDAFX_H__02A117D1_B6A1_4B13_A07E_FBFB9370A433__INCLUDED_)
