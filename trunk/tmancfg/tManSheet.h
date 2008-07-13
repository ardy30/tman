/**
 *  tManSheet.h
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

#if !defined(AFX_TMANSHEET_H__CFD7B2A8_55B2_4ABF_A08D_28C20C90722B__INCLUDED_)
#define AFX_TMANSHEET_H__CFD7B2A8_55B2_4ABF_A08D_28C20C90722B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "CePropertySheet.h"
#include "tManCfgPg.h"
#include "tManTaskListPg.h"
#include "tManButtonsPg.h"

/////////////////////////////////////////////////////////////////////////////
// CtManSheet

class CtManSheet : public CCePropertySheet {
	DECLARE_DYNAMIC(CtManSheet)

// Construction
public:
	CtManSheet(CWnd *pParentWnd = NULL, UINT iSelectPage = 0);

// Attributes
protected:
	CtManCfgPg m_pgConfig;
	CtManTaskListPg m_pgTaskList;
	CtManButtonsPg m_pgButtons;

	HWND m_hwndCmdBar;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CtManSheet)
	public:
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CtManSheet();

	// Generated message map functions
protected:
	void Init();
	void OnAbout();
	//{{AFX_MSG(CtManSheet)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BATTISHEET_H__CFD7B2A8_55B2_4ABF_A08D_28C20C90722B__INCLUDED_)
