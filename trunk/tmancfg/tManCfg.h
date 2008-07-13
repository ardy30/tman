/**
 *  tManCfg.h : main header file for the TMANCFG application
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

#if !defined(AFX_TMANCFG_H__B29AB9B5_7E4C_4A34_8F98_113EF6195570__INCLUDED_)
#define AFX_TMANCFG_H__B29AB9B5_7E4C_4A34_8F98_113EF6195570__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CtManCfgApp:
// See batticfg.cpp for the implementation of this class
//

class CtManCfgApp : public CWinApp
{
public:
	CtManCfgApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CtManCfgApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CtManCfgApp)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#include "..\share\Localization.h"

class CLocalizedString : public CString {
public:
	BOOL LoadString(UINT nID) {
		if (Localization != NULL) {
			TCHAR *str = Localization->GetStr(nID);
			if (str != NULL) {
				Format(_T("%s"), str);
				return TRUE;
			}
			else
				return CString::LoadString(nID);
		}
		else {
			return CString::LoadString(nID);
		}
	}
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft eMbedded Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BATTICFG_H__B29AB9B5_7E4C_4A34_8F98_113EF6195570__INCLUDED_)
