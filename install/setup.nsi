; Script generated with the Venis Install Wizard

!define CABFILE "tMan.arm.cab"

; Define your application name
!define APPNAME "tMan"
!define APPNAMEANDVERSION "tMan ${VERSION}"

; Main Install settings
Name "${APPNAMEANDVERSION}"
InstallDir "$PROGRAMFILES\tMan"
InstallDirRegKey HKLM "Software\${APPNAME}" ""
OutFile "tMan-${VERSION}.exe"

; do not touch anything beyond this point unless you KNOW what you are doing
;
; some declarations
;

; Modern interface settings
!include "MUI.nsh"

!define MUI_ABORTWARNING

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "..\COPYING"
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

; Set languages (first is default language)
!insertmacro MUI_LANGUAGE "${LANGUAGE}"
!insertmacro MUI_RESERVEFILE_LANGDLL


;
; MAIN
;

Section "tman" Section1

	; Set Section properties
	SetOverwrite on

	; Set Section Files and Shortcuts
	SetOutPath "$INSTDIR\"
	File "${CABFILE}"
	File "tMan.ini"
	File "..\COPYING"
SectionEnd

Section -FinishSection

	WriteRegStr HKLM "Software\${APPNAME}" "" "$INSTDIR"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "DisplayName" "${APPNAME}"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "UninstallString" "$INSTDIR\uninstall.exe"
	WriteUninstaller "$INSTDIR\uninstall.exe"

SectionEnd

; Modern install component descriptions
!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
!insertmacro MUI_DESCRIPTION_TEXT ${Section1} ""
!insertmacro MUI_FUNCTION_DESCRIPTION_END

;Uninstall section
Section Uninstall

	;Remove from registry...
	DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}"
	DeleteRegKey HKLM "SOFTWARE\${APPNAME}"

	; Delete self
	Delete "$INSTDIR\uninstall.exe"

	; Clean up Birthdays
	Delete "$INSTDIR\tMan.arm.cab"
	Delete "$INSTDIR\tMan.ini"
	Delete "$INSTDIR\COPYING"

	; Remove remaining directories
	RMDir "$INSTDIR\"

SectionEnd

; --- Pocket PC install ---

;Var to hold the location of CeAppMgr.exe
var CeAppMgrLocation

Section PathObtain
	;It can be obtained from the registry at this location always
	ReadRegStr $CeAppMgrLocation HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\App Paths\CEAPPMGR.EXE" ""
SectionEnd
 
Section CEInstallAndDelete
	;This calls the program and passes to it the program's ini file which tells it which CAB file(s) it needs and sets it up to install
	ExecWait '"$CeAppMgrLocation" "$INSTDIR\tMan.ini"' $0
	;Changes the output path so the temporary folder can be deleted
;	SetOutPath $TEMP
	;Remove the directory and all the files put into it
;	RMDir /r /REBOOTOK $INSTDIR
SectionEnd

; eof

