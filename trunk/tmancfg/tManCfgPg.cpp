/**
 *  tManCfgPg.cpp
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
#include "tManCfgPg.h"
#include "../share/Config.h"
#include "../share/helpers.h"

#include "ExceptionAppsDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CtManCfgPg dialog

CtManCfgPg::CtManCfgPg() : CCePropertyPage(CtManCfgPg::IDD) {
	TCHAR target[MAX_PATH + 1];
	TCHAR *shortcut = GetLinkPathName();
	m_bShortcutExists = SHGetShortcutTarget(shortcut, target, MAX_PATH);
	delete [] shortcut;

	//{{AFX_DATA_INIT(CtManCfgPg)
	m_bRunOnStartup = m_bShortcutExists;
//	m_bTapAndHoldTaskMenu = Config.TapAndHoldTaskMenu;
	m_nGestureAction = Config.GestureAction;
	m_nTapAction = Config.TapAction;
	m_nTapAndHoldAction = Config.TapAndHoldAction;
	//}}AFX_DATA_INIT
}

CtManCfgPg::~CtManCfgPg() {
}

void CtManCfgPg::DoDataExchange(CDataExchange* pDX) {
	CCePropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CtManCfgPg)
	DDX_Control(pDX, IDC_TAP_AND_HOLD_ACTION, m_ctlTapAndHoldAction);
	DDX_Control(pDX, IDC_TAP_ACTION, m_ctlTapAction);
	DDX_Control(pDX, IDC_GESTURE_ACTION, m_ctlGestureAction);
	DDX_Control(pDX, IDC_BEHAVIOR_LBL, m_ctlBehaviorLbl);
	DDX_Control(pDX, IDC_GENERAL_LBL, m_ctlGeneralLbl);
	DDX_Check(pDX, IDC_RUN_ON_STARTUP, m_bRunOnStartup);
//	DDX_Check(pDX, IDC_TAP_HOLD_TASK_MENU, m_bTapAndHoldTaskMenu);
	DDX_CBIndex(pDX, IDC_GESTURE_ACTION, m_nGestureAction);
	DDX_CBIndex(pDX, IDC_TAP_ACTION, m_nTapAction);
	DDX_CBIndex(pDX, IDC_TAP_AND_HOLD_ACTION, m_nTapAndHoldAction);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CtManCfgPg, CCePropertyPage)
	//{{AFX_MSG_MAP(CtManCfgPg)
	ON_BN_CLICKED(IDC_EXCEPTIONS, OnExceptions)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CtManCfgPg message handlers

BOOL CtManCfgPg::OnInitDialog() {
	CCePropertyPage::OnInitDialog();
	LocalizeDialog(GetSafeHwnd(), IDD);

//	SetIcon(m_hIcon, TRUE);			// Set big icon
//	SetIcon(m_hIcon, FALSE);		// Set small icon

	CenterWindow(GetDesktopWindow());	// center to the hpc screen

//	m_ctlSecondsSpin.SendMessage(CCM_SETVERSION, COMCTL32_VERSION, 0);

	UINT nID[] = {
		IDS_CLOSE,
		IDS_MINIMIZE,
		IDS_SHOW_MENU,
	};

	for (int i = 0; i < sizeof(nID) / sizeof(nID[0]); i++) {
		CLocalizedString sText;
		sText.LoadString(nID[i]);
		m_ctlTapAction.AddString(sText);
		m_ctlTapAndHoldAction.AddString(sText);
		m_ctlGestureAction.AddString(sText);
	}
	m_ctlTapAction.SetCurSel(Config.TapAction);
	m_ctlTapAndHoldAction.SetCurSel(Config.TapAndHoldAction);
	m_ctlGestureAction.SetCurSel(Config.GestureAction);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CtManCfgPg::OnOK() {
	if (UpdateData()) {
		// handle short cut
		if (m_bShortcutExists != m_bRunOnStartup) {
			if (m_bRunOnStartup)
				CreateStartupShortcut(Config.InstallDir);
			else
				RemoveStartupShortcut();
		}

		// save values to the config
		Config.GestureAction = m_nGestureAction;
		Config.TapAction = m_nTapAction;
		Config.TapAndHoldAction = m_nTapAndHoldAction;
	}

	CCePropertyPage::OnOK();
}

BOOL CtManCfgPg::OnSetActive() {
	Config.Page = 0;

	return CCePropertyPage::OnSetActive();
}

void CtManCfgPg::OnExceptions() {
	CExceptionAppsDlg dlg;

	if (dlg.DoModal() == IDOK) {
	}
}
