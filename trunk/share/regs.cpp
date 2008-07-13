/**
 *  regs.cpp: Registry manipulating functions
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
#include "regs.h"

//////////////////////////////////////////////////////////////////////

#ifdef TMAN_CONFIG

void RegWriteDword(HKEY hKey, LPCTSTR valueName, DWORD value) {
	RegSetValueEx(hKey, valueName, 0, REG_DWORD, (BYTE *) &value, sizeof(value));
}

void RegWriteString(HKEY hKey, LPCTSTR valueName, LPCTSTR value) {
	if (value != NULL)
		RegSetValueEx(hKey, valueName, 0, REG_SZ, (BYTE *) value, sizeof(TCHAR) * (lstrlen(value) + 1));
	else {
		TCHAR v = 0;
		RegSetValueEx(hKey, valueName, 0, REG_SZ, (BYTE *) v, sizeof(TCHAR));
	}
}

#endif

DWORD RegReadDword(HKEY hKey, LPCTSTR valueName, DWORD defaultValue) {
	DWORD dwType, dwValue, dwSize = sizeof(DWORD);

	if (RegQueryValueEx(hKey, valueName, NULL, &dwType, (LPBYTE) &dwValue, &dwSize) == ERROR_SUCCESS)
		return dwValue;
	else
		return defaultValue;
}

TCHAR *RegReadString(HKEY hKey, LPCTSTR valueName, LPCTSTR defaultValue) {
	DWORD dwType, dwSize;
	LPTSTR lpValue = NULL;
	if (RegQueryValueEx(hKey, valueName, NULL, &dwType, NULL, &dwSize) == ERROR_SUCCESS) {
		lpValue = new TCHAR [dwSize + 1];
		memset(lpValue, 0, sizeof(TCHAR) * (dwSize + 1));
		if (RegQueryValueEx(hKey, valueName, NULL, &dwType, (BYTE *) lpValue, &dwSize) == ERROR_SUCCESS) {
			return lpValue;
		}
		else {
			delete [] lpValue;
		}
	}

	lpValue = new TCHAR[wcslen(defaultValue) + 1];
	wcscpy(lpValue, defaultValue);
	return lpValue;
}

