/**
 *  tManIgnoreListPg.h
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

#if !defined(AFX_TMANIGNORELISTPG_H__1E622142_6FD1_42DF_A9A6_ED806FDC92FA__INCLUDED_)
#define AFX_TMANIGNORELISTPG_H__1E622142_6FD1_42DF_A9A6_ED806FDC92FA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "CeListCtrl.h"
#include "CeDialog.h"

/////////////////////////////////////////////////////////////////////////////
// CIgnoreListDlg dialog

class CIgnoreListDlg : public CCeDialog
{
// Construction
public:
	CIgnoreListDlg();
	~CIgnoreListDlg();

// Dialog Data
	//{{AFX_DATA(CIgnoreListDlg)
	enum { IDD = IDD_IGNORE_LIST };
	CCeListCtrl	m_ctlAppList;
	//}}AFX_DATA

protected:
	CImageList m_ilApps;

// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CIgnoreListDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	void AddApplication(const CString &strFileName);

	// Generated message map functions
	//{{AFX_MSG(CIgnoreListDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnAdd();
	virtual void OnOK();
	//}}AFX_MSG
	afx_msg void OnContextMenu(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnRemove();

	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TMANIGNORELISTPG_H__1E622142_6FD1_42DF_A9A6_ED806FDC92FA__INCLUDED_)
