/**
 *  CePropertyPage.h
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


#if !defined(AFX_CELISTCTRL_H__0E73C775_E66E_446D_94D2_3121224575CC__INCLUDED_)
#define AFX_CELISTCTRL_H__0E73C775_E66E_446D_94D2_3121224575CC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CeListCtrl.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CCeListCtrl window

class CCeListCtrl : public CListCtrl
{
// Construction
public:
	CCeListCtrl();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCeListCtrl)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CCeListCtrl();

	// Generated message map functions
protected:
	//{{AFX_MSG(CCeListCtrl)
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CELISTCTRL_H__0E73C775_E66E_446D_94D2_3121224575CC__INCLUDED_)
