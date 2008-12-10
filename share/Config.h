/**
 *  Config.h: interface for the CConfig class.
 *
 *  Copyright (C) 2008  David Andrs <pda@jasnapaka.com>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#if !defined(AFX_CONFIG_H__3AC5942A_61A5_4469_ADFE_264704448F3C__INCLUDED_)
#define AFX_CONFIG_H__3AC5942A_61A5_4469_ADFE_264704448F3C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


// config
#define CONFIG_DEFAULT_PAGE						0

#define CONFIG_DEFAULT_COMPACT_MODE				FALSE
#define CONFIG_DEFAULT_SHOW_DESKTOP				TRUE
#define CONFIG_DEFAULT_SHOW_SOFT_RESET			FALSE
#define CONFIG_DEFAULT_SHOW_DISPLAY_OFF			FALSE
#define CONFIG_DEFAULT_SHOW_LANDSCAPE_RIGHT		FALSE
#define CONFIG_DEFAULT_SHOW_LANDSCAPE_LEFT		FALSE
#define CONFIG_DEFAULT_SHOW_POWER_OFF			FALSE
#define CONFIG_DEFAULT_SHOW_TASK_OPERATIONS		TRUE
#define CONFIG_DEFAULT_SHOW_WIFI				FALSE
#define CONFIG_DEFAULT_SHOW_BTH					FALSE

#define CONFIG_DEFAULT_ALT_TAB					TRUE

#define CONFIG_DEFAULT_COLOR_MENU_HI_BG			GetSysColor(COLOR_HIGHLIGHT)
#define CONFIG_DEFAULT_COLOR_MENU_HI_FG			GetSysColor(COLOR_HIGHLIGHTTEXT)
#define CONFIG_DEFAULT_COLOR_MENU_BG			GetSysColor(COLOR_MENU)
#define CONFIG_DEFAULT_COLOR_MENU_FG			GetSysColor(COLOR_MENUTEXT)

// TWEAKS
#define CONFIG_DEFAULT_OLD_STYLE_BUTTON			FALSE
#define CONFIG_DEFAULT_CONFIRM_SOFT_RESET		TRUE
#define CONFIG_DEFAULT_CONFIRM_TERMINATE		TRUE
#define CONFIG_DEFAULT_KILL_TIMEOUT				10
#define CONFIG_DEFAULT_SHOW_DOT					TRUE
#define CONFIG_DEFAULT_OLD_STYLE_BKGND			FALSE
#define CONFIG_DEFAULT_LONGPRESS_TIMEOUT		500

#define CONFIG_DEFAULT_WIFI_DEVICE_NAME			_T("")

#ifdef LOGGING
#define CONFIG_DEFAULT_LOG_LEVEL				0
#define CONFIG_DEFAULT_LOG_FILE_NAME			_T("\\tman.log")
#endif

///////////////////////////////////////////////////////////////////////////////

class CConfig {
public:
	// defines custom behaviour of Close button for specified application
	class CAppException {
	public:
		TCHAR *ExeName;			// full path to the exe file
		int TapAction;				// what to do on tap
		int GestureAction;			// what to do on gesture

		CAppException();
		CAppException(HKEY hKey);
		CAppException(TCHAR *exeName, int tapAction, int gestureAction);
		~CAppException();

		void Load(HKEY hKey);
		void Save(HKEY hKey);
	};

	TCHAR *InstallDir;

//	BOOL TapAndHoldTaskMenu;
	int TapAction;
	int TapAndHoldAction;
	int GestureAction;
	BOOL AltTab;

	//
	TCHAR **IgnoredApps;		// do not show these apps in task list
	int IgnoredAppsCount;

	//
	CAppException **Exceptions;
	int ExceptionsCount;

	// task list
	int TaskListHWKey;
	BOOL CompactMode;
	BOOL ShowDesktop;
	BOOL ShowSoftReset;
	BOOL ShowDisplayOff;
	BOOL ShowRotateScreen;
	BOOL ShowLandscapeRight;
	BOOL ShowLandscapeLeft;
	BOOL ShowPowerOff;
	BOOL ShowWiFi;
	BOOL ShowBth;

	//
	BOOL ShowTaskOperations;

	TCHAR *WiFiDevice;

	// colors
	COLORREF ClrMenuHiFg, ClrMenuHiBg;
	COLORREF ClrMenuFg, ClrMenuBg;

	// tweaks
	BOOL ConfirmSoftReset;
	BOOL ConfirmTerminate;
	int KillTimeout;
	BOOL OldStyleButton;
	BOOL ShowDot;
	BOOL OldStyleBkgnd;
	int LongPressTimeout;

	// other
	int Page;

#ifdef LOGGING
	DWORD LogLevel;
	TCHAR *LogFileName;
#endif

	CConfig();
	virtual ~CConfig();

	void Save();
	void Load();
	void Free();
};

extern CConfig Config;
extern LPCTSTR szConfig;

extern LPCTSTR szRoot;
extern LPCTSTR szInstallDir;

#endif // !defined(AFX_CONFIG_H__3AC5942A_61A5_4469_ADFE_264704448F3C__INCLUDED_)
