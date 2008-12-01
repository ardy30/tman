/**
 *  tManSheet.cpp
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

#include "StdAfx.h"
#include "tManCfg.h"
#include "tManSheet.h"
#include "../share/Config.h"
#include "AboutPg.h"
#include "LicensePg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CtManSheet

IMPLEMENT_DYNAMIC(CtManSheet, CCePropertySheet)

CtManSheet::CtManSheet(CWnd* pParentWnd, UINT iSelectPage)
	: CCePropertySheet(IDS_TMAN_CONFIG, pParentWnd, iSelectPage)
{
	Init();
	CLocalizedString s;
	s.LoadString(IDS_OPTIONS);
	m_strTitle = s;
}

CtManSheet::~CtManSheet() {
}


BEGIN_MESSAGE_MAP(CtManSheet, CCePropertySheet)
	//{{AFX_MSG_MAP(CtManSheet)
	ON_COMMAND(ID_ABOUT, OnAbout)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CtManSheet message handlers

void CtManSheet::Init() {
	AddPage(&m_pgConfig);
	AddPage(&m_pgTaskList);
	AddPage(&m_pgButtons);
}

BOOL CtManSheet::OnInitDialog() {
	CCePropertySheet::OnInitDialog();

	SHINITDLGINFO sid;
	sid.dwMask   = SHIDIM_FLAGS;
	sid.dwFlags  = SHIDIF_SIZEDLGFULLSCREEN;
	sid.hDlg     = GetSafeHwnd();
	SHInitDialog(&sid);

	BOOL fSuccess;
	SHMENUBARINFO mbi = { 0 };

	mbi.cbSize = sizeof(mbi);
	mbi.dwFlags = SHCMBF_HMENU;
	mbi.hInstRes = AfxGetInstanceHandle();
	mbi.hwndParent = GetSafeHwnd();
	mbi.nToolBarId = IDR_MAIN;

	fSuccess = SHCreateMenuBar(&mbi);
	m_hwndCmdBar = mbi.hwndMB;

	LocalizeMenubar(m_hwndCmdBar);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CtManSheet::OnAbout() {
	CAboutPg pgAbout;
	CLicensePg pgLicense;

	CCePropertySheet sheet(IDS_ABOUT);
	sheet.AddPage(&pgAbout);
	sheet.AddPage(&pgLicense);

	sheet.SetMenu(IDR_DONE);
	sheet.DoModal();
}
