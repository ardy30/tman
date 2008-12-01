/**
 *  ExceptionAppsDlg.h
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

#if !defined(AFX_EXCEPTIONAPPSDLG_H__EBF9F00C_5D09_4B27_8F8D_FAC63F9BE7A7__INCLUDED_)
#define AFX_EXCEPTIONAPPSDLG_H__EBF9F00C_5D09_4B27_8F8D_FAC63F9BE7A7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../share/Config.h"
#include "CeListCtrl.h"
#include "CeDialog.h"

/////////////////////////////////////////////////////////////////////////////
// CExceptionAppsDlg dialog

class CExceptionAppsDlg : public CCeDialog
{
// Construction
public:
	CExceptionAppsDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CExceptionAppsDlg)
	enum { IDD = IDD_EXCEPTIONS };
	CCeListCtrl	m_ctlPrograms;
	CComboBox	m_ctlTapAction;
	CComboBox	m_ctlGestureAction;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CExceptionAppsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	void UpdateControls();
	CImageList m_ilApps;

	// Generated message map functions
	//{{AFX_MSG(CExceptionAppsDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnAdd();
	afx_msg void OnItemchangedPrograms(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSelendokGestureAction();
	afx_msg void OnSelendokTapAction();
	//}}AFX_MSG
	void OnContextMenu(NMHDR *pNMHDR, LRESULT *pResult);
	void OnRemove();

	DECLARE_MESSAGE_MAP()

	int AddApplication(const CConfig::CAppException *ex);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EXCEPTIONAPPSDLG_H__EBF9F00C_5D09_4B27_8F8D_FAC63F9BE7A7__INCLUDED_)
