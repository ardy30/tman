/**
 *  tManTaskListPg.h
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

#if !defined(AFX_TMANTASKLISTPG_H__9F607DAE_21CC_4A68_96E1_59075DCDC386__INCLUDED_)
#define AFX_TMANTASKLISTPG_H__9F607DAE_21CC_4A68_96E1_59075DCDC386__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Caption.h"
#include "CePropertyPage.h"

/////////////////////////////////////////////////////////////////////////////
// CListCtrlNoDots

class CListCtrlNoDots : public CListCtrl
{
// Construction
public:
	CListCtrlNoDots();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CListCtrlNoDots)
	//}}AFX_VIRTUAL
//protected:
//	CPoint m_pt;

// Implementation
public:
	virtual ~CListCtrlNoDots();

	// Generated message map functions
protected:
	//{{AFX_MSG(CListCtrlNoDots)
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRecognizeGesture(NMHDR *pNMHDR, LRESULT *pResult);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// CtManTaskListPg dialog

class CtManTaskListPg : public CCePropertyPage
{
	DECLARE_DYNCREATE(CtManTaskListPg)

// Construction
public:
	CtManTaskListPg();
	~CtManTaskListPg();

// Dialog Data
	//{{AFX_DATA(CtManTaskListPg)
	enum { IDD = IDD_TASKLIST };
	CListCtrlNoDots	m_ctlTaskListItems;
	CCaption	m_ctlItemsLbl;
	BOOL	m_bCompactModel;
	//}}AFX_DATA

// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CtManTaskListPg)
	public:
	virtual void OnOK();
	virtual BOOL OnSetActive();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	void UpdateControls();

	// Generated message map functions
	//{{AFX_MSG(CtManTaskListPg)
	virtual BOOL OnInitDialog();
	afx_msg void OnIgnoreList();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TMANTASKLISTPG_H__9F607DAE_21CC_4A68_96E1_59075DCDC386__INCLUDED_)
