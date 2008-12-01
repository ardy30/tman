/**
 *  tManButtonsPg.h
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

#if !defined(AFX_TMANBUTTONSPG_H__FCD93215_8949_44D9_93C9_F95166068F7F__INCLUDED_)
#define AFX_TMANBUTTONSPG_H__FCD93215_8949_44D9_93C9_F95166068F7F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Caption.h"
#include "CePropertyPage.h"

/////////////////////////////////////////////////////////////////////////////
// CHwKeyEdit class

class CHwKeyEdit : public CEdit {
public:
	void SetKey(int key);
	void ClearKey();

protected:
	CLocalizedString m_sKey;
};


/////////////////////////////////////////////////////////////////////////////
// CtManButtonsPg dialog

class CtManButtonsPg : public CCePropertyPage
{
	DECLARE_DYNCREATE(CtManButtonsPg)

// Construction
public:
	CtManButtonsPg();
	~CtManButtonsPg();

// Dialog Data
	//{{AFX_DATA(CtManButtonsPg)
	enum { IDD = IDD_BUTTONS };
	CHwKeyEdit	m_ctlHWKey;
	CButton	m_ctlGrab;
	CButton	m_ctlClear;
	CCaption	m_ctlTaskListLbl;
	BOOL	m_bAltTab;
	//}}AFX_DATA
	int HwKey;


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CtManButtonsPg)
	public:
	virtual void OnOK();
	virtual BOOL OnSetActive();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CtManButtonsPg)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG

	afx_msg void OnGrab();
	afx_msg void OnClear();

	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TMANBUTTONSPG_H__FCD93215_8949_44D9_93C9_F95166068F7F__INCLUDED_)
