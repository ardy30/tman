/**
 *  Localization.h: interface for the CLocalization class.
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

#if !defined(AFX_LOCALIZATION_H__28614813_8ABC_4EA5_9EDC_D501EDFF19B3__INCLUDED_)
#define AFX_LOCALIZATION_H__28614813_8ABC_4EA5_9EDC_D501EDFF19B3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "arraylt.h"

struct CLocalizationStr {
	UINT ID;
	TCHAR *Text;

	CLocalizationStr(UINT id, TCHAR *text) {
		ID = id;
		Text = text;
	}

	~CLocalizationStr() {
		delete [] Text;
	}
};

class CLocalization {
public:
	CLocalization();
	virtual ~CLocalization();

	BOOL Init();
	void Destroy();
//	BOOL Update();
	BOOL Load(LPCTSTR fileName);
	TCHAR *GetStr(UINT id);
	TCHAR *GetFileName();

	FILETIME *GetWriteTime() { return &WriteTime; }
	void SetWriteTime(FILETIME *ft) { WriteTime = *ft; }

protected:

	FILETIME WriteTime;
	TIndirectArray<CLocalizationStr> Str;
};


extern CLocalization *Localization;

void LocalizeDialog(HWND hDlg, UINT nID);
void LocalizeMenubar(HWND hMenuBar);
void LocalizeMenu(HMENU hMenu);


#endif // !defined(AFX_LOCALIZATION_H__28614813_8ABC_4EA5_9EDC_D501EDFF19B3__INCLUDED_)
