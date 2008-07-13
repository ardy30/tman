/**
 *  helpers.cpp: miscelaneous functions
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
#include "helpers.h"
#include "defs.h"

TCHAR ExeName[] = L"tman.exe";
TCHAR Link[] = L"\\tMan.lnk";

TCHAR *GetExePathName(LPCTSTR installDir) {
	TCHAR *exePathName = new TCHAR[lstrlen(installDir) + 1 + lstrlen(ExeName) + 1];
	if (exePathName == NULL) return NULL;

	wcscpy(exePathName, installDir);
	wcscat(exePathName, L"\\");
	wcscat(exePathName, ExeName);

	return exePathName;
}

TCHAR *GetLinkPathName() {
	TCHAR szStartupFolderPath[MAX_PATH];
	::SHGetSpecialFolderPath(NULL, szStartupFolderPath, CSIDL_STARTUP, FALSE);
	
	TCHAR *linkPathName = new TCHAR[lstrlen(szStartupFolderPath) + lstrlen(Link) + 1];
	if (linkPathName == NULL) return NULL;

	wcscpy(linkPathName, szStartupFolderPath);
	wcscat(linkPathName, Link);

	return linkPathName;
}

BOOL CreateStartupShortcut(LPCTSTR installDir) {
	BOOL ret = FALSE;
	// run application on startup
	TCHAR *exePathName = GetExePathName(installDir);
	if (exePathName != NULL) {
		TCHAR *target = new TCHAR [lstrlen(exePathName) + 1];
		if (target != NULL) {
			swprintf(target, _T("\"%s\""), exePathName);

			TCHAR *linkPathName = GetLinkPathName();
			if (linkPathName != NULL) {
				ret = ::SHCreateShortcut(linkPathName, target);
				delete [] linkPathName;
			}
			delete [] target;
		}
		delete [] exePathName;
	}

	return ret;
}

BOOL RemoveStartupShortcut() {
	// remove the link
	TCHAR szStartupFolderPath[MAX_PATH];
	::SHGetSpecialFolderPath(NULL, szStartupFolderPath, CSIDL_STARTUP, FALSE);

	TCHAR *linkPathName = GetLinkPathName();
	BOOL ret = DeleteFile(linkPathName);
	delete [] linkPathName;

	return ret;
}

HWND FindTManWindow() {
//	HWND hTaskBar = FindTaskBar();
	HWND hTaskBar = ::FindWindow(_T("HHTaskBar"), NULL);
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

void RegisterTMansHotKey(int key) {
	HWND tManWnd = FindTManWindow();
	// remove all key mapping
	for (int i = 0xc1; i <= 0xcf; i++)
		UnregisterHotKey(tManWnd, i);

	if (key != 0)
		RegisterHotKey(tManWnd, key, MOD_WIN, key);
}

