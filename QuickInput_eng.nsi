; QuickInput.nsi
;
; инстал€ционный скрипт дл€ программы "Quick Input"
; 17.01.2007

; описание продукта
!define PRODUCT "Quick Input"
!define VERSION "v0.7"

!include "MUI.nsh"

; включение проверки CRC
CRCCheck on

; установка компрессии
SetCompress   force
SetCompressor lzma

; включение стил€ XP
XPStyle on

; определение параметров установки
Name                   "${PRODUCT}"
OutFile                "QuickInput_${VERSION}_install.exe"
InstallDir             "$PROGRAMFILES\${PRODUCT}"
BrandingText /TRIMLEFT "${PRODUCT} ${VERSION}"

VAR MUI_TEMP
VAR STARTMENU_FOLDER

; настройки страниц
!define MUI_ABORTWARNING
!define MUI_HEADERIMAGE
!define MUI_HEADERIMAGE_BITMAP         "Documents\header.bmp"
!define MUI_ICON                       "${NSISDIR}\Contrib\Graphics\Icons\win-install.ico"
!define MUI_UNICON                     "${NSISDIR}\Contrib\Graphics\Icons\win-uninstall.ico"
!define MUI_FINISHPAGE_NOREBOOTSUPPORT
!define MUI_FINISHPAGE_RUN             "$INSTDIR\Quick Input.exe"
!define MUI_FINISHPAGE_LINK            "${PRODUCT} ${VERSION} (www.qinput.com)" 
!define MUI_FINISHPAGE_LINK_LOCATION   "http://www.qinput.com"
!define MUI_COMPONENTSPAGE_NODESC

; install страницы 
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "Documents\License_eng.rtf"
!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_DIRECTORY
!define MUI_STARTMENUPAGE_REGISTRY_ROOT      "HKCU"
!define MUI_STARTMENUPAGE_REGISTRY_KEY       "Software\Whole Group (www.wholegroup.com)\Quick Input"
!define MUI_STARTMENUPAGE_REGISTRY_VALUENAME "LabelNSIS" 
!insertmacro MUI_PAGE_STARTMENU Application $STARTMENU_FOLDER
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

; uninstall страницы  
!insertmacro MUI_UNPAGE_WELCOME
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_UNPAGE_FINISH

; настройки €зыка
!insertmacro MUI_LANGUAGE "English"

; функци€ вызывает перед запуском install
Function .onInit

	;; проверка версии Windows
   Call TestWindowsVersion
   Pop $R0

	strcmp $R0 "NO" 0 +3
	MessageBox MB_OK|MB_ICONSTOP "For install need Windows 2000 or later."
	Quit

	FindWindow $0 "#32770" "Quick Input"
	IntCmp $0 0 notwindow
		MessageBox MB_OK "Quick Input is running. To run Quick Input install you need to first close Quick Input."
	notwindow:

	FindWindow $0 "#32770" "Quick Input"
	IntCmp $0 0 notwindow2
		Quit
	notwindow2:
FunctionEnd

; функци€ вызываетс€ перед запуском uninstall
Function un.onInit
	FindWindow $0 "#32770" "Quick Input"
	IntCmp $0 0 notwindow
		MessageBox MB_OK "Quick Input is running. To run Quick Input uninstall you need to first close Quick Input."
	notwindow:

	FindWindow $0 "#32770" "Quick Input"
	IntCmp $0 0 notwindow2
		Quit
	notwindow2:
FunctionEnd

; секци€ установки программы
Section "${PRODUCT} ${VERSION}" SecGUI
	SectionIn RO ; не измен€ет по умолчанию

	SetOutPath "$INSTDIR"                                     ; пусть установки
	File       "Sources\vs2010\Release\Quick Input.exe"     ; программа
	File       "Sources\vs2010\Release\HookDll.dll"           ; DLL-hook
	File       "Sources\sqlite\sqlite3.dll" ; sqlite
	File       "Documents\License_eng.rtf"                 ; лицензи€
  
	WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT}" "DisplayName" "${PRODUCT} (remove only)"
	WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT}" "UninstallString" '"$INSTDIR\Uninstall.exe"'
    
	!insertmacro MUI_STARTMENU_WRITE_BEGIN Application
		CreateDirectory "$SMPROGRAMS\$STARTMENU_FOLDER"
		CreateShortCut  "$SMPROGRAMS\$STARTMENU_FOLDER\${PRODUCT}.lnk" "$INSTDIR\Quick Input.exe"
		CreateShortCut  "$SMPROGRAMS\$STARTMENU_FOLDER\${PRODUCT} Uninstall.lnk" "$INSTDIR\Uninstall.exe"
		CreateShortCut  "$SMPROGRAMS\$STARTMENU_FOLDER\${PRODUCT} License.lnk" "$INSTDIR\License_eng.rtf"
	!insertmacro MUI_STARTMENU_WRITE_END

	WriteUninstaller "$INSTDIR\Uninstall.exe"
SectionEnd

; секци€ создани€ иконок на рабочем столе
Section "Create Shortcut on Desktop" SecICON
    CreateShortCut  "$DESKTOP\${PRODUCT}.lnk" "$INSTDIR\Quick Input.exe"
SectionEnd

Section "Uninstall"
	; удаление файлов
	Delete "$INSTDIR\Quick Input.exe"
	Delete "$INSTDIR\HookDll.dll"
	Delete "$INSTDIR\sqlite3.dll"
	Delete "$INSTDIR\Uninstall.exe"
	Delete "$INSTDIR\License_eng.rtf"

	; удаление €рлыков из пуска  
	!insertmacro MUI_STARTMENU_GETFOLDER Application $MUI_TEMP
	Delete "$SMPROGRAMS\$STARTMENU_FOLDER$MUI_TEMP\${PRODUCT}.lnk"
	Delete "$SMPROGRAMS\$STARTMENU_FOLDER$MUI_TEMP\${PRODUCT} uninstall.lnk"
	Delete "$SMPROGRAMS\$STARTMENU_FOLDER$MUI_TEMP\${PRODUCT} License.lnk"
	RMDir "$SMPROGRAMS\$STARTMENU_FOLDER$MUI_TEMP"

	; удаление €рлыков с рабочего стола
	Delete "$DESKTOP\${PRODUCT}.lnk"

	RMDir "$INSTDIR"

	DeleteRegKey HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT}"
SectionEnd


;;
;; функци€ возвращает YES, если верси€ Windows не младше 2000
;; иначе функци€ возвращает NO
;;
Function TestWindowsVersion
	Push $R0
	Push $R1

	ClearErrors

	ReadRegStr $R0 HKLM \
	"SOFTWARE\Microsoft\Windows NT\CurrentVersion" CurrentVersion

	IfErrors 0 lbl_winnt
  
	StrCpy $R0 'NO'
	Goto lbl_done

lbl_winnt:

	StrCpy $R1 $R0 1

	StrCmp $R1 '3' lbl_winnt_x
	StrCmp $R1 '4' lbl_winnt_x

	StrCpy $R1 $R0 3

	StrCpy $R0 'YES'
	Goto lbl_done

lbl_winnt_x:
	StrCpy $R0 'NO'
	Goto lbl_done

lbl_done:
	Pop $R1
	Exch $R0

FunctionEnd
