/**
 *  AboutDlg.h
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

#if !defined(AFX_TMANABOUTPG_H__09295F7D_AC63_4C8C_B9EC_82EE35D48DB1__INCLUDED_)
#define AFX_TMANABOUTPG_H__09295F7D_AC63_4C8C_B9EC_82EE35D48DB1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// tManAboutPg.h : header file
//

#include "LinkCtrl.h"
#include "CeDialog.h"

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog

class CAboutDlg : public CCeDialog
{
//	DECLARE_DYNCREATE(CAboutDlg)

// Construction
public:
	CAboutDlg();
	~CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_TMANCFG_ABOUT };
	CLinkCtrl	m_ctlHomepageLink;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	public:
	virtual void OnOK();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CAboutDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnHomepageLink();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_tManAboutPg_H__09295F7D_AC63_4C8C_B9EC_82EE35D48DB1__INCLUDED_)
