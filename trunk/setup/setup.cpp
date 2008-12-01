/**
 *  setup.cpp : Defines the entry point for the DLL application.
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

#include "StdAfx.h"
#include <ce_setup.h>
#include "../share/defs.h"
#include "../share/regs.h"
#include "../share/helpers.h"

HWND FindtManWindow() {
	HWND hTaskBar = FindWindow(_T("HHTaskBar"), NULL);
	if (hTaskBar == NULL) return NULL;

	if (hTaskBar != 0) {
		HWND hChild = GetWindow(hTaskBar, GW_CHILD);
		while (hChild != NULL) {
			TCHAR className[16];
			GetClassName(hChild, className, 16);
			if (wcscmp(className, WINDOW_CLASS) == 0) {
				return hChild;
			}

			hChild = GetWindow(hChild, GW_HWNDNEXT);
		}
	}

	return NULL;
}

void TerminateTMan() {
	// find tMan window
	HWND htMan = FindtManWindow();
	if (htMan != NULL) {
		// end the app
		SendMessage(htMan, WM_CLOSE, 0, 0);

		// wait for the end of tMan
		DWORD processID;
		DWORD threadID = GetWindowThreadProcessId(htMan, &processID);
		HANDLE hProcess = OpenProcess(0, FALSE, processID);
		DWORD dwResult = WaitForSingleObject(hProcess, 8000);		// 4 sec
		if (dwResult == WAIT_TIMEOUT) {
			TerminateProcess(hProcess, 0);
		}
		CloseHandle(hProcess);
	}
}

BOOL APIENTRY DllMain(HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    return TRUE;
}

codeINSTALL_INIT
Install_Init(HWND hwndParent, BOOL fFirstCall, BOOL fPreviouslyInstalled, LPCTSTR pszInstallDir) {
	TerminateTMan();
	return codeINSTALL_INIT_CONTINUE;
}

codeINSTALL_EXIT
Install_Exit(HWND hwndParent, LPCTSTR pszInstallDir, WORD cFailedDirs, WORD cFailedFiles,
	WORD cFailedRegKeys, WORD cFailedRegVals, WORD cFailedShortcuts) {

	// write install path
	HKEY hRoot;
	DWORD dwDisposition;
	if (RegCreateKeyEx(HKEY_CURRENT_USER, ROOT, 0, NULL, 0, KEY_ALL_ACCESS, NULL, &hRoot, &dwDisposition) != ERROR_SUCCESS)
		return codeINSTALL_EXIT_UNINSTALL;

	if (RegSetValueEx(hRoot, INSTALL_DIR, 0, REG_SZ, (LPBYTE) pszInstallDir, sizeof(TCHAR) * (lstrlen(pszInstallDir) + 1)) != ERROR_SUCCESS) {
		RegCloseKey(hRoot);
		return codeINSTALL_EXIT_UNINSTALL;
	}
	RegCloseKey(hRoot);


	// run application on startup
	CreateStartupShortcut(pszInstallDir);

	// run tMan
	TCHAR *exePathName = GetExePathName(pszInstallDir);
	PROCESS_INFORMATION pi;
	CreateProcess(exePathName, _T(""), NULL, NULL, FALSE, 0, NULL, NULL, NULL, &pi);
	delete [] exePathName;

	return codeINSTALL_EXIT_DONE;
}

codeUNINSTALL_INIT
Uninstall_Init(HWND hwndParent, LPCTSTR pszInstallDir) {
	RegDeleteKey(HKEY_CURRENT_USER, ROOT);

	TerminateTMan();

	// remove the link
	codeUNINSTALL_INIT ret;
	if (RemoveStartupShortcut())
		ret = codeUNINSTALL_INIT_CONTINUE;
	else
		ret = codeUNINSTALL_INIT_CANCEL;

	return ret;
}

codeUNINSTALL_EXIT
Uninstall_Exit(HWND hwndParent) {
	return codeUNINSTALL_EXIT_DONE;
}
