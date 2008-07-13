/**
 *  Localization.cpp: implementation of the CLocalization class.
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

#include "defs.h"
#include "windows.h"
#include "commctrl.h"
#include "Localization.h"
#include "stdlib.h"
#include "config.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define ROW_LEN						1024

void LocalizeDialog(HWND hDlg, UINT nID) {
	if (Localization == NULL)
		return;

	TCHAR *str;
	str = Localization->GetStr(nID);
	if (str != NULL) SetWindowText(hDlg, str);

	HWND hChild = GetWindow(hDlg, GW_CHILD);
	while (hChild != NULL) {
		str = Localization->GetStr(GetDlgCtrlID(hChild));
		if (str != NULL) SetWindowText(hChild, str);
		hChild = GetWindow(hChild, GW_HWNDNEXT);
	}
}

void LocalizeMenubar(HWND hMenuBar) {
	if (Localization == NULL)
		return;

	int btnCnt = ::SendMessage(hMenuBar, TB_BUTTONCOUNT, 0, 0); 

	for (int i = 0; i < btnCnt; i++) {
		TBBUTTON btn;
		::SendMessage(hMenuBar, TB_GETBUTTON, (WPARAM) i, (LPARAM) &btn);

		TBBUTTONINFO tbButtonInfo = { 0 };
		tbButtonInfo.cbSize = sizeof(TBBUTTONINFO);
		tbButtonInfo.dwMask = TBIF_TEXT;

		TCHAR *str = Localization->GetStr(btn.idCommand);
		if (str != NULL) {
			tbButtonInfo.pszText = (LPTSTR) (LPCTSTR) str;
			tbButtonInfo.cchText = wcslen(str);
			::SendMessage(hMenuBar, TB_SETBUTTONINFO, btn.idCommand, (LPARAM) &tbButtonInfo); 
		}
	}
}

void LocalizeMenu(HMENU hMenu) {
	if (Localization == NULL)
		return;

	MENUITEMINFO mii;
	mii.cbSize = sizeof(mii);
	mii.fMask = MIIM_ID | MIIM_TYPE | MIIM_DATA;

	UINT idx = 0;
	while (GetMenuItemInfo(hMenu, idx, TRUE, &mii)) {
		if (mii.fType == MFT_STRING) {
			TCHAR *str = Localization->GetStr(mii.wID);
			if (str != NULL) {
				mii.cbSize = sizeof(mii);
				mii.fMask = MIIM_TYPE;
				mii.dwTypeData = (LPTSTR) str;
				mii.cch = wcslen(str);
				SetMenuItemInfo(hMenu, idx, TRUE, &mii);
			}
		}
		
		idx++;

		mii.cbSize = sizeof(mii);
		mii.fMask = MIIM_ID | MIIM_TYPE | MIIM_DATA;
	}
}

CLocalization *Localization = NULL;

/***
 *char *bsearch() - do a binary search on an array
 *
 *Purpose:
 *       Does a binary search of a sorted array for a key.
 *
 *Entry:
 *       const char *key    - key to search for
 *       const char *base   - base of sorted array to search
 *       unsigned int num   - number of elements in array
 *       unsigned int width - number of bytes per element
 *       int (*compare)()   - pointer to function that compares two array
 *               elements, returning neg when #1 < #2, pos when #1 > #2, and
 *               0 when they are equal. Function is passed pointers to two
 *               array elements.
 *
 *Exit:
 *       if key is found:
 *               returns pointer to occurrence of key in array
 *       if key is not found:
 *               returns NULL
 *
 *Exceptions:
 *
 *******************************************************************************/

void * __cdecl bsearch (
        const void *key,
        const void *base,
        size_t num,
        size_t width,
        int (__cdecl *compare)(const void *, const void *)
        )
{
        char *lo = (char *)base;
        char *hi = (char *)base + (num - 1) * width;
        char *mid;
        unsigned int half;
        int result;

        while (lo <= hi)
                if (half = num / 2)
                {
                        mid = lo + (num & 1 ? half : (half - 1)) * width;
                        if (!(result = (*compare)(key,mid)))
                                return(mid);
                        else if (result < 0)
                        {
                                hi = mid - width;
                                num = num & 1 ? half : half-1;
                        }
                        else    {
                                lo = mid + width;
                                num = half;
                        }
                }
                else if (num)
                        return((*compare)(key,lo) ? NULL : lo);
                else
                        break;

        return(NULL);
}

//
// Compare Proc for search and sort
//

int CompareLocStr(const void *arg1, const void *arg2) {
   CLocalizationStr *s1 = *(CLocalizationStr **) arg1;
   CLocalizationStr *s2 = *(CLocalizationStr **) arg2;
   return s1->ID - s2->ID;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

#define DEFAULT_LNG				L"default.lng"

CLocalization::CLocalization() :
	Str(35, 10, dtNoDelete)
{
}

CLocalization::~CLocalization() {
	Destroy();
}

void CLocalization::Destroy() {
	for (int i = 0; i < Str.Count; i++)
		delete Str.At(i);
	Str.Delete(0, Str.Count);
}

TCHAR *CLocalization::GetFileName() {
	TCHAR *ret = NULL;

	// get location of DLL form registry
	HKEY hRoot;
	if (RegOpenKeyEx(HKEY_CURRENT_USER, ROOT, 0, KEY_READ, &hRoot) == ERROR_SUCCESS) {
		DWORD dwType, dwSize = sizeof(DWORD);
		// INSTALL_DIR
		if (RegQueryValueEx(hRoot, INSTALL_DIR, NULL, &dwType, NULL, &dwSize) == ERROR_SUCCESS) {
			TCHAR *dll = (TCHAR *) new BYTE[dwSize];
			if (RegQueryValueEx(hRoot, INSTALL_DIR, NULL, &dwType, (BYTE *) dll, &dwSize) == ERROR_SUCCESS) {
				TCHAR *fileName = new TCHAR [wcslen(dll) + 1 + wcslen(DEFAULT_LNG) + 1];
				fileName[0] = '\0';
				wcscpy(fileName, dll);
				wcscat(fileName, _T("\\"));
				wcscat(fileName, DEFAULT_LNG);
				ret = fileName;
			}
			delete [] dll;
		}

		RegCloseKey(hRoot);
	}

	return ret;
}

BOOL CLocalization::Init() {
	BOOL ret = FALSE;
	
	TCHAR *fileName = GetFileName();
	if (fileName != NULL) {
		ret = Load(fileName);

		// save the file time
		HANDLE hFile = ::CreateFile(fileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
		if (hFile != INVALID_HANDLE_VALUE) {
			::GetFileTime(hFile, NULL, NULL, &WriteTime);
			::CloseHandle(hFile);
		}
		else {
			memset(&WriteTime, 0, sizeof(WriteTime));
		}


		delete [] fileName;
	}

	return ret;
}

BOOL CLocalization::Load(LPCTSTR fileName) {
	FILE *file = _wfopen(fileName, _T("rb"));
	if (file == NULL) return FALSE;

	// skip two first bytes
	TCHAR ch = fgetwc(file);
	TCHAR row[ROW_LEN];
	while (fgetws(row, ROW_LEN, file) != NULL) {
		if (row[0] == ';') continue;		// comment
		TCHAR *equal = wcschr(row, '=');
		if (equal != NULL) {
			*equal = '\0';					// split into number and string parts
			TCHAR *str = equal + 1;			// localized string

			// get ID
			UINT id;
			swscanf(row, _T("%d"), &id);
			// remove trailing "
			TCHAR *quote = wcsrchr(str, '"');
			if (quote != NULL) *quote = '\0';
			// skip leading "
			quote = wcschr(str, '"');
			str = quote + 1;
			Str.Add(new CLocalizationStr(id, wcsdup(str)));
		}
	}

	fclose(file);

	qsort(Str.GetData(), Str.Count, sizeof(CLocalizationStr *), CompareLocStr);

	return TRUE;
}

/*
BOOL CLocalization::Update() {
	BOOL ret = FALSE;
	
	TCHAR *fileName = GetFileName();
	if (fileName != NULL) {
		// get current file time
		HANDLE hFile = CreateFile(fileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
		if (hFile != INVALID_HANDLE_VALUE) {
			FILETIME writeTime;
			GetFileTime(hFile, NULL, NULL, &writeTime);
			CloseHandle(hFile);

			// reload localization file if file times differ
			if (CompareFileTime(&WriteTime, &writeTime) != 0) {
				ret = Load(fileName);
			}
		}
		delete [] fileName;
	}

	return ret;
}
*/

TCHAR *CLocalization::GetStr(UINT id) {
	CLocalizationStr *key = new CLocalizationStr(id, NULL);
	CLocalizationStr **str = (CLocalizationStr **) bsearch(&key, Str.GetData(), Str.Count, sizeof(CLocalizationStr *), CompareLocStr);
	delete key;
	if (str != NULL)
		return (*str)->Text;
	else
		return NULL;
}
