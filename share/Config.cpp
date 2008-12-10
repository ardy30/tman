/**
 *  Config.cpp: implementation of the CConfig class.
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

#include <windows.h>
#include "Config.h"
#include "defs.h"
#include "regs.h"

LPCTSTR szTapAction = _T("TapAction");
LPCTSTR szTapAndHoldAction = _T("TapAndHoldAction");
LPCTSTR szGestureAction = _T("GestureAction");

LPCTSTR szPage = _T("Page");

LPCTSTR szTaskListHWKey = _T("TLHWKey");
LPCTSTR szAltTab = _T("AltTab");
LPCTSTR szCompactMode = _T("CompactMode");
LPCTSTR szShowDesktop = _T("ShowDesktop");
LPCTSTR szShowSoftReset = _T("ShowSoftReset");
LPCTSTR szShowDisplayOff = _T("ShowDisplayOff");
LPCTSTR szShowLandscapeLeft = _T("ShowLandscapeLeft");
LPCTSTR szShowLandscapeRight = _T("ShowLandscapeRight");
LPCTSTR szShowPowerOff = _T("ShowPowerOff");
LPCTSTR szShowTaskOperations = _T("ShowTaskOperations");
LPCTSTR szShowWiFi = _T("ShowWiFi");
LPCTSTR szShowBth = _T("ShowBth");

LPCTSTR szWiFiDevice = _T("WiFiDevice");

LPCTSTR szClrMenuHiFg = _T("ClrMenuHiFg");
LPCTSTR szClrMenuHiBg = _T("ClrMenuHiBg");
LPCTSTR szClrMenuFg = _T("ClrMenuFg");
LPCTSTR szClrMenuBg = _T("ClrMenuBg");

// TWEAKS
LPCTSTR szConfirmSoftReset = _T("ConfirmSoftReset");
LPCTSTR szConfirmTerminate = _T("ConfirmTerminate");
LPCTSTR szKillTimeout = _T("KillTimeout");
LPCTSTR szOldStyleButton = _T("OldStyleButton");
LPCTSTR szShowDot = _T("ShowDot");
LPCTSTR szOldStyleBkgnd = _T("OldStyleBkgnd");
LPCTSTR szLongPressTimeout = _T("LongPressTimeout");
LPCTSTR szAltTabTimeout = _T("AltTabTimeout");

// Exception List
LPCTSTR szExceptions = _T("Exceptions");

LPCTSTR szExeName = _T("ExeName");

// list of applications to ignore
LPCTSTR szIgnoreList = _T("IgnoredApps");
LPCTSTR szItem = _T("Item%d");

#ifdef LOGGING
LPCTSTR szLogLevel = _T("LogLevel");
LPCTSTR szLogFile = _T("LogFile");
#endif


CConfig Config;

// CAppException //////

CConfig::CAppException::CAppException() {
	ExeName = NULL;
	TapAction = BUTTON_ACTION_CLOSE;
	GestureAction = BUTTON_ACTION_MINIMIZE;
}

CConfig::CAppException::CAppException(HKEY hKey) {
	Load(hKey);
}

CConfig::CAppException::CAppException(TCHAR *exeName, int tapAction, int gestureAction) {
	TapAction = tapAction;
	GestureAction = gestureAction;

	ExeName = new TCHAR [wcslen(exeName) + 1];
	swprintf(ExeName, _T("%s"), exeName);
}

CConfig::CAppException::~CAppException() {
	delete [] ExeName;
}

void CConfig::CAppException::Load(HKEY hKey) {
	ExeName = RegReadString(hKey, szExeName, _T(""));
	TapAction = RegReadDword(hKey, szTapAction, BUTTON_ACTION_CLOSE);
	GestureAction = RegReadDword(hKey, szTapAction, BUTTON_ACTION_MINIMIZE);
}

#ifdef TMAN_CONFIG

void CConfig::CAppException::Save(HKEY hKey) {
	RegWriteString(hKey, szExeName, ExeName);
	RegWriteDword(hKey, szTapAction, TapAction);
	RegWriteDword(hKey, szGestureAction, GestureAction);
}

#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CConfig::CConfig() {
	Page = CONFIG_DEFAULT_PAGE;

	TapAction = BUTTON_ACTION_CLOSE;
	TapAndHoldAction = BUTTON_ACTION_SHOW_MENU;
	GestureAction = BUTTON_ACTION_MINIMIZE;

	TaskListHWKey = 0;
	AltTab = CONFIG_DEFAULT_ALT_TAB;
	CompactMode = CONFIG_DEFAULT_COMPACT_MODE;
	ShowDesktop = CONFIG_DEFAULT_SHOW_DESKTOP;
	ShowSoftReset = CONFIG_DEFAULT_SHOW_SOFT_RESET;
	ShowDisplayOff = CONFIG_DEFAULT_SHOW_DISPLAY_OFF;
	ShowLandscapeLeft = CONFIG_DEFAULT_SHOW_LANDSCAPE_LEFT;
	ShowLandscapeRight = CONFIG_DEFAULT_SHOW_LANDSCAPE_RIGHT;
	ShowPowerOff = CONFIG_DEFAULT_SHOW_POWER_OFF;
	ShowTaskOperations = CONFIG_DEFAULT_SHOW_TASK_OPERATIONS;
	ShowWiFi = CONFIG_DEFAULT_SHOW_WIFI;
	ShowBth = CONFIG_DEFAULT_SHOW_BTH;

	// TWEAKS
	OldStyleButton = CONFIG_DEFAULT_OLD_STYLE_BUTTON;
	ConfirmSoftReset = CONFIG_DEFAULT_CONFIRM_SOFT_RESET;
	ConfirmTerminate = CONFIG_DEFAULT_CONFIRM_TERMINATE;
	KillTimeout = CONFIG_DEFAULT_KILL_TIMEOUT;
	ShowDot = CONFIG_DEFAULT_SHOW_DOT;
	OldStyleBkgnd = CONFIG_DEFAULT_OLD_STYLE_BKGND;
	LongPressTimeout = CONFIG_DEFAULT_LONGPRESS_TIMEOUT;
	AltTabTimeout = CONFIG_DEFAULT_ALTTAB_TIMEOUT;

	InstallDir = NULL;

	// color
	ClrMenuHiBg = CONFIG_DEFAULT_COLOR_MENU_HI_BG;
	ClrMenuHiFg = CONFIG_DEFAULT_COLOR_MENU_HI_FG;
	ClrMenuBg = CONFIG_DEFAULT_COLOR_MENU_BG;
	ClrMenuFg = CONFIG_DEFAULT_COLOR_MENU_FG;

	// ignore list
	IgnoredApps = NULL;
	IgnoredAppsCount = 0;

	Exceptions = NULL;
	ExceptionsCount = 0;

	WiFiDevice = CONFIG_DEFAULT_WIFI_DEVICE_NAME;

#ifdef LOGGING
	LogFileName = CONFIG_DEFAULT_LOG_FILE_NAME;
	LogLevel = CONFIG_DEFAULT_LOG_LEVEL;
#endif
}

CConfig::~CConfig() {
	Free();
}

void CConfig::Free() {
	delete [] InstallDir;

	// dispose ignore list
	if (IgnoredApps != NULL) {
		for (int i = 0; i < IgnoredAppsCount; i++)
			delete [] IgnoredApps[i];
		delete [] IgnoredApps;
	}

	// dispose exception list
	if (Exceptions != NULL) {
		for (int i = 0; i < ExceptionsCount; i++)
			delete Exceptions[i];
		delete [] Exceptions;
	}

#ifdef LOGGING
	if (LogFileName != CONFIG_DEFAULT_LOG_FILE_NAME)
		delete [] LogFileName;
#endif

	if (WiFiDevice != CONFIG_DEFAULT_WIFI_DEVICE_NAME)
		delete [] WiFiDevice;
}

#ifdef TMAN_CONFIG

void CConfig::Save() {
	DWORD dwDisposition;
	HKEY hApp;

	if (RegCreateKeyEx(HKEY_CURRENT_USER, ROOT, 0, NULL, 0, KEY_ALL_ACCESS, NULL, &hApp, &dwDisposition) == ERROR_SUCCESS) {
		RegWriteString(hApp, INSTALL_DIR, InstallDir);
//		RegWriteDword(hApp, szTapAndHoldTaskMenu, TapAndHoldTaskMenu);
		RegWriteDword(hApp, szTapAction, TapAction);
		RegWriteDword(hApp, szTapAndHoldAction, TapAndHoldAction);
		RegWriteDword(hApp, szGestureAction, GestureAction);

		// task list
		RegWriteDword(hApp, szTaskListHWKey, TaskListHWKey);
		RegWriteDword(hApp, szAltTab, AltTab);
		RegWriteDword(hApp, szCompactMode, CompactMode);
		RegWriteDword(hApp, szShowDesktop, ShowDesktop);
		RegWriteDword(hApp, szShowSoftReset, ShowSoftReset);
		RegWriteDword(hApp, szShowDisplayOff, ShowDisplayOff);
		RegWriteDword(hApp, szShowLandscapeLeft, ShowLandscapeLeft);
		RegWriteDword(hApp, szShowLandscapeRight, ShowLandscapeRight);
		RegWriteDword(hApp, szShowPowerOff, ShowPowerOff);
		RegWriteDword(hApp, szShowTaskOperations, ShowTaskOperations);
		RegWriteDword(hApp, szShowWiFi, ShowWiFi);
		RegWriteDword(hApp, szShowBth, ShowBth);

/*		// colors
		RegWriteDword(hApp, szClrMenuHiBg, ClrMenuHiBg);
		RegWriteDword(hApp, szClrMenuHiFg, ClrMenuHiFg);
		RegWriteDword(hApp, szClrMenuBg, ClrMenuBg);
		RegWriteDword(hApp, szClrMenuFg, ClrMenuFg);
*/

		// exception list
		RegDeleteKey(hApp, szExceptions);
		HKEY hExceptions;
		if (RegCreateKeyEx(hApp, szExceptions, 0, NULL, 0, KEY_ALL_ACCESS, NULL, &hExceptions, &dwDisposition) == ERROR_SUCCESS) {
			int j = 0;
			for (int i = 0; i < ExceptionsCount; i++) {
				TCHAR strItem[64] = { 0 };
				swprintf(strItem, _T("%02d"), j);

				HKEY hEx;
				if (RegCreateKeyEx(hExceptions, strItem, 0, NULL, 0, KEY_ALL_ACCESS, NULL, &hEx, &dwDisposition) == ERROR_SUCCESS) {
					if (Exceptions[i] != NULL)
						Exceptions[i]->Save(hEx);
					j++;
					RegCloseKey(hEx);
				}
			}

			RegCloseKey(hExceptions);
		}

		// list of applications to ignore
		RegDeleteKey(hApp, szIgnoreList);
		HKEY hIgnoreList;
		if (RegCreateKeyEx(hApp, szIgnoreList, 0, NULL, 0, KEY_ALL_ACCESS, NULL, &hIgnoreList, &dwDisposition) == ERROR_SUCCESS) {
			int j = 0;
			for (int i = 0; i < IgnoredAppsCount; i++) {
				TCHAR strItem[64] = { 0 };
				swprintf(strItem, szItem, j);
				if (IgnoredApps[i] != NULL && wcslen(IgnoredApps[i]) > 0) {
					RegWriteString(hIgnoreList, strItem, IgnoredApps[i]);
					j++;
				}
			}

			RegCloseKey(hIgnoreList);
		}

		// GUI
		RegWriteDword(hApp, szPage, Page);

		RegCloseKey(hApp);
	}
}

#endif

void CConfig::Load() {
	Free();

	HKEY hApp;
	if (RegOpenKeyEx(HKEY_CURRENT_USER, ROOT, 0, 0, &hApp) == ERROR_SUCCESS) {
		InstallDir = RegReadString(hApp, INSTALL_DIR, _T(""));

//		TapAndHoldTaskMenu = RegReadDword(hApp, szTapAndHoldTaskMenu, FALSE);
//		ButtonAction = RegReadDword(hApp, szButtonAction, FALSE);
		TapAction = RegReadDword(hApp, szTapAction, BUTTON_ACTION_CLOSE);
		TapAndHoldAction = RegReadDword(hApp, szTapAndHoldAction, BUTTON_ACTION_SHOW_MENU);
		GestureAction = RegReadDword(hApp, szGestureAction, BUTTON_ACTION_MINIMIZE);

		// task list
		TaskListHWKey = RegReadDword(hApp, szTaskListHWKey, 0);
		AltTab = RegReadDword(hApp, szAltTab, CONFIG_DEFAULT_ALT_TAB);
		CompactMode = RegReadDword(hApp, szCompactMode, CONFIG_DEFAULT_COMPACT_MODE);
		ShowDesktop = RegReadDword(hApp, szShowDesktop, CONFIG_DEFAULT_SHOW_DESKTOP);
		ShowSoftReset = RegReadDword(hApp, szShowSoftReset, CONFIG_DEFAULT_SHOW_SOFT_RESET);
		ShowDisplayOff = RegReadDword(hApp, szShowDisplayOff, CONFIG_DEFAULT_SHOW_DISPLAY_OFF);
		ShowLandscapeLeft = RegReadDword(hApp, szShowLandscapeLeft, CONFIG_DEFAULT_SHOW_LANDSCAPE_LEFT);
		ShowLandscapeRight = RegReadDword(hApp, szShowLandscapeRight, CONFIG_DEFAULT_SHOW_LANDSCAPE_RIGHT);
		ShowPowerOff = RegReadDword(hApp, szShowPowerOff, CONFIG_DEFAULT_SHOW_POWER_OFF);
		ShowTaskOperations = RegReadDword(hApp, szShowTaskOperations, CONFIG_DEFAULT_SHOW_TASK_OPERATIONS);
		ShowWiFi = RegReadDword(hApp, szShowWiFi, CONFIG_DEFAULT_SHOW_WIFI);
		ShowBth = RegReadDword(hApp, szShowBth, CONFIG_DEFAULT_SHOW_BTH);

		WiFiDevice = RegReadString(hApp, szWiFiDevice, _T(""));

		// exception list
		HKEY hExceptions;
		if (RegOpenKeyEx(hApp, szExceptions, 0, 0, &hExceptions) == ERROR_SUCCESS) {
			DWORD cSubKeys, cMaxSubKeyLen;
			RegQueryInfoKey(hExceptions, NULL, NULL, NULL, &cSubKeys, &cMaxSubKeyLen, NULL, NULL, NULL, NULL, NULL, NULL);

			int i;
			Exceptions = new CAppException *[cSubKeys];
			for (i = 0; i < (int) cSubKeys; i++)
				Exceptions[i] = NULL;
			ExceptionsCount = cSubKeys;

			TCHAR *name = new TCHAR[cMaxSubKeyLen + 1];
			DWORD dwIndex = 0;
			i = 0;
			while (dwIndex < cSubKeys) {
				DWORD cbName = cMaxSubKeyLen + 1;
				if (RegEnumKeyEx(hExceptions, dwIndex, name, &cbName, NULL, NULL, NULL, NULL) == ERROR_SUCCESS) {
					HKEY hEx;
					if (RegOpenKeyEx(hExceptions, name, KEY_ALL_ACCESS, 0, &hEx) == ERROR_SUCCESS) {
						Exceptions[i++] = new CAppException(hEx);

						RegCloseKey(hEx);
					}
				}
				dwIndex++;
			}

			delete [] name;

			RegCloseKey(hExceptions);
		}

		// list of applications to ignore
		HKEY hIgnoreList;
		if (RegOpenKeyEx(hApp, szIgnoreList, 0, 0, &hIgnoreList) == ERROR_SUCCESS) {
			DWORD cValues;
			RegQueryInfoKey(hIgnoreList, NULL, NULL, NULL, NULL, NULL, NULL, &cValues, NULL, NULL, NULL, NULL);

			IgnoredApps = new TCHAR *[cValues];
			for (int i = 0; i < (int) cValues; i++) {
				TCHAR strItem[64] = { 0 };
				swprintf(strItem, szItem, i);

				TCHAR *strApp = RegReadString(hIgnoreList, strItem, _T(""));
				if (wcslen(strApp) > 0)
					IgnoredApps[i] = strApp;
				else
					IgnoredApps[i] = NULL;
			}
			IgnoredAppsCount = cValues;

			RegCloseKey(hIgnoreList);
		}

		// color
		ClrMenuHiBg = RegReadDword(hApp, szClrMenuHiBg, CONFIG_DEFAULT_COLOR_MENU_HI_BG);
		ClrMenuHiFg = RegReadDword(hApp, szClrMenuHiFg, CONFIG_DEFAULT_COLOR_MENU_HI_FG);
		ClrMenuBg = RegReadDword(hApp, szClrMenuBg, CONFIG_DEFAULT_COLOR_MENU_BG);
		ClrMenuFg = RegReadDword(hApp, szClrMenuFg, CONFIG_DEFAULT_COLOR_MENU_FG);

		Page = RegReadDword(hApp, szPage, CONFIG_DEFAULT_PAGE);

		// TWEAKS
		OldStyleButton = RegReadDword(hApp, szOldStyleButton, CONFIG_DEFAULT_OLD_STYLE_BUTTON);
		ConfirmSoftReset = RegReadDword(hApp, szConfirmSoftReset, CONFIG_DEFAULT_CONFIRM_SOFT_RESET);
		ConfirmTerminate = RegReadDword(hApp, szConfirmTerminate, CONFIG_DEFAULT_CONFIRM_TERMINATE);
		KillTimeout = RegReadDword(hApp, szKillTimeout, CONFIG_DEFAULT_KILL_TIMEOUT);
		ShowDot = RegReadDword(hApp, szShowDot, CONFIG_DEFAULT_SHOW_DOT);
		OldStyleBkgnd = RegReadDword(hApp, szOldStyleBkgnd, CONFIG_DEFAULT_OLD_STYLE_BKGND);
		LongPressTimeout = RegReadDword(hApp, szLongPressTimeout, CONFIG_DEFAULT_LONGPRESS_TIMEOUT);
		AltTabTimeout = RegReadDword(hApp, szAltTabTimeout, CONFIG_DEFAULT_ALTTAB_TIMEOUT);

#ifdef LOGGING
		LogFileName = RegReadString(hApp, szLogFile, CONFIG_DEFAULT_LOG_FILE_NAME);
		LogLevel = RegReadDword(hApp, szLogLevel, CONFIG_DEFAULT_LOG_LEVEL);
#endif

		RegCloseKey(hApp);
	}
}
