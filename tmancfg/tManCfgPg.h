/**
 *  tManCfgPg.h
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

#if !defined(AFX_TMANCFGDLG_H__D5D3790D_D3E9_4674_A3BD_A29F33E619D3__INCLUDED_)
#define AFX_TMANCFGDLG_H__D5D3790D_D3E9_4674_A3BD_A29F33E619D3__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "Caption.h"
#include "CePropertyPage.h"

/////////////////////////////////////////////////////////////////////////////
// CtManCfgPg dialog

class CtManCfgPg : public CCePropertyPage
{
// Construction
public:
	CtManCfgPg();	// standard constructor
	~CtManCfgPg();

// Dialog Data
	//{{AFX_DATA(CtManCfgPg)
	enum { IDD = IDD_CONFIG };
	CComboBox	m_ctlTapAndHoldAction;
	CComboBox	m_ctlTapAction;
	CComboBox	m_ctlGestureAction;
	CCaption	m_ctlBehaviorLbl;
	CCaption	m_ctlGeneralLbl;
	BOOL	m_bRunOnStartup;
	BOOL	m_bTapAndHoldTaskMenu;
	int		m_nGestureAction;
	int		m_nTapAction;
	int		m_nTapAndHoldAction;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CtManCfgPg)
	public:
	virtual BOOL OnSetActive();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	BOOL m_bShortcutExists;

	// Generated message map functions
	//{{AFX_MSG(CtManCfgPg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnExceptions();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft eMbedded Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BATTICFGDLG_H__D5D3790D_D3E9_4674_A3BD_A29F33E619D3__INCLUDED_)
