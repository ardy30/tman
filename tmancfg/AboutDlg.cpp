/**
 *  AboutDlg.cpp
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

#include "stdafx.h"
#include "tManCfg.h"
#include "AboutDlg.h"
#include "../share/Config.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg property page

//IMPLEMENT_DYNCREATE(CAboutDlg, CCeDialog)

CAboutDlg::CAboutDlg() : CCeDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

CAboutDlg::~CAboutDlg()
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CCeDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	DDX_Control(pDX, IDC_HOMEPAGE_LINK, m_ctlHomepageLink);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAboutDlg, CCeDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
	ON_BN_CLICKED(IDC_HOMEPAGE_LINK, OnHomepageLink)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg message handlers

void CAboutDlg::OnOK() {
	CCeDialog::OnOK();
}

BOOL CAboutDlg::OnInitDialog() {
	CCeDialog::OnInitDialog();
	LocalizeDialog(GetSafeHwnd(), IDD);

	return TRUE;
}

void CAboutDlg::OnHomepageLink() {
	SHELLEXECUTEINFO ei = { 0 };
	ei.cbSize = sizeof(ei);
	ei.fMask = SEE_MASK_NOCLOSEPROCESS;
	ei.lpFile = _T("iexplore.exe");
	CString strLink;
	m_ctlHomepageLink.GetWindowText(strLink);
	ei.lpParameters = strLink;
	ShellExecuteEx(&ei);
}

