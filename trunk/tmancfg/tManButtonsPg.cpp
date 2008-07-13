/**
 *  tManButtonsPg.cpp
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
#include "tmancfg.h"
#include "tManButtonsPg.h"
#include "GrabKeyDlg.h"

#include "../share/Config.h"
#include "../share/helpers.h"
#include "../share/defs.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

void CHwKeyEdit::SetKey(int key) {
	switch (key) {
		case 000: m_sKey.LoadString(IDS_NONE); break;
		case 193: m_sKey.LoadString(IDS_BUTTON1); break;
		case 194: m_sKey.LoadString(IDS_BUTTON2); break;
		case 195: m_sKey.LoadString(IDS_BUTTON3); break;
		case 196: m_sKey.LoadString(IDS_BUTTON4); break;
		case 197: m_sKey.LoadString(IDS_SPEED); break;
		case 198: m_sKey.LoadString(IDS_RECORD); break;
		default:  m_sKey.Format(_T("%d"), key); break;
	}

	SetWindowText(m_sKey);
}

void CHwKeyEdit::ClearKey() {
	SetKey(0);
}


/////////////////////////////////////////////////////////////////////////////
// CtManButtonsPg property page

IMPLEMENT_DYNCREATE(CtManButtonsPg, CCePropertyPage)

CtManButtonsPg::CtManButtonsPg() : CCePropertyPage(CtManButtonsPg::IDD) {
	//{{AFX_DATA_INIT(CtManButtonsPg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	HwKey = Config.TaskListHWKey;
}

CtManButtonsPg::~CtManButtonsPg() {
}

void CtManButtonsPg::DoDataExchange(CDataExchange* pDX) {
	CCePropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CtManButtonsPg)
	DDX_Control(pDX, IDC_HWKEY, m_ctlHWKey);
	DDX_Control(pDX, IDC_GRAB, m_ctlGrab);
	DDX_Control(pDX, IDC_CLEAR, m_ctlClear);
	DDX_Control(pDX, IDC_TASKLIST_LBL, m_ctlTaskListLbl);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CtManButtonsPg, CCePropertyPage)
	//{{AFX_MSG_MAP(CtManButtonsPg)
	ON_BN_CLICKED(IDC_GRAB, OnGrab)
	ON_BN_CLICKED(IDC_CLEAR, OnClear)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CtManButtonsPg message handlers

BOOL CtManButtonsPg::OnInitDialog() {
	CCePropertyPage::OnInitDialog();
	LocalizeDialog(GetSafeHwnd(), IDD);

	m_ctlHWKey.SetKey(HwKey);

	m_ctlGrab.SetFocus();
	
	return TRUE;
}

void CtManButtonsPg::OnGrab() {
	CGrabKeyDlg dlg;
	if (dlg.DoModal() == IDOK) {
		HwKey = dlg.VKey;
		m_ctlHWKey.SetKey(HwKey);
	}

	// during the key grabbing, hw keys are unmapped -> map it back
	RegisterTMansHotKey(Config.TaskListHWKey);
}

void CtManButtonsPg::OnClear() {
	m_ctlHWKey.ClearKey();
	HwKey = 0;
	RegisterTMansHotKey(Config.TaskListHWKey);
}


void CtManButtonsPg::OnOK() {
	if (UpdateData()) {
		Config.TaskListHWKey = HwKey;

		// change tMan's hot key
		RegisterTMansHotKey(Config.TaskListHWKey);
	}
	
	CCePropertyPage::OnOK();
}

BOOL CtManButtonsPg::OnSetActive() {
	Config.Page = 2;
	
	return CCePropertyPage::OnSetActive();
}
