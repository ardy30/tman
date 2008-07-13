/**
 *  tManTaskListPg.cpp
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
#include "tmantasklistpg.h"
#include "IgnoreListDlg.h"

#include "../share/Config.h"
#include "../share/helpers.h"
#include "../share/defs.h"
#include "../share/Localization.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

struct CItem {
	UINT Id;			// id of the resouce string
	BOOL *Value;		// pointer to the value

	CItem(UINT id, BOOL *value) {
		Id = id;
		Value = value;
	}
}
TaskListItems[] = {
	CItem(IDS_DESKTOP, &Config.ShowDesktop),
	CItem(IDS_TASK_OPERATIONS, &Config.ShowTaskOperations),
	CItem(IDS_SOFT_RESET, &Config.ShowSoftReset),
	CItem(IDS_DISPLAY_OFF, &Config.ShowDisplayOff),
	CItem(IDS_POWER_OFF, &Config.ShowPowerOff),
	CItem(IDS_LANDSCAPE_RIGHT, &Config.ShowLandscapeRight),
	CItem(IDS_LANDSCAPE_LEFT, &Config.ShowLandscapeLeft),
	CItem(IDS_WIFI, &Config.ShowWiFi),
	CItem(IDS_BLUETOOTH, &Config.ShowBth),
};


/////////////////////////////////////////////////////////////////////////////
// CListCtrlNoDots

CListCtrlNoDots::CListCtrlNoDots() {
}

CListCtrlNoDots::~CListCtrlNoDots() {
}


BEGIN_MESSAGE_MAP(CListCtrlNoDots, CListCtrl)
	//{{AFX_MSG_MAP(CListCtrlNoDots)
	ON_WM_LBUTTONDOWN()
	ON_NOTIFY_REFLECT(NM_RECOGNIZEGESTURE, OnRecognizeGesture)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CListCtrlNoDots message handlers

void CListCtrlNoDots::OnLButtonDown(UINT nFlags, CPoint point) {
	Default();
}

void CListCtrlNoDots::OnRecognizeGesture(NMHDR* pNMHDR, LRESULT* pResult) {
	*pResult = TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CtManTaskListPg property page

IMPLEMENT_DYNCREATE(CtManTaskListPg, CCePropertyPage)

CtManTaskListPg::CtManTaskListPg() : CCePropertyPage(CtManTaskListPg::IDD) {
	//{{AFX_DATA_INIT(CtManTaskListPg)
	m_bCompactModel = Config.CompactMode;
	//}}AFX_DATA_INIT
}

CtManTaskListPg::~CtManTaskListPg() {
}

void CtManTaskListPg::DoDataExchange(CDataExchange* pDX)
{
	CCePropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CtManTaskListPg)
	DDX_Control(pDX, IDC_TASKLIST_ITEMS, m_ctlTaskListItems);
	DDX_Control(pDX, IDC_ITEMS_LBL, m_ctlItemsLbl);
	DDX_Check(pDX, IDC_COMPACT_MODE, m_bCompactModel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CtManTaskListPg, CCePropertyPage)
	//{{AFX_MSG_MAP(CtManTaskListPg)
	ON_BN_CLICKED(IDC_IGNORE_LIST, OnIgnoreList)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CtManTaskListPg message handlers

BOOL CtManTaskListPg::OnInitDialog() {
	CCePropertyPage::OnInitDialog();
	LocalizeDialog(GetSafeHwnd(), IDD);
	
	m_ctlTaskListItems.SetExtendedStyle(LVS_EX_CHECKBOXES);

	CRect rcTaskListItems;
	m_ctlTaskListItems.GetClientRect(&rcTaskListItems);

	m_ctlTaskListItems.InsertColumn(0, _T(""), LVCFMT_LEFT, rcTaskListItems.Width());
	for (int i = 0; i < countof(TaskListItems); i++) {
		// no wifi device specified, skip this option
		if (wcslen(Config.WiFiDevice) == 0 && TaskListItems[i].Id == IDS_WIFI) continue;

		CLocalizedString strItem;
		strItem.LoadString(TaskListItems[i].Id);
		int nItem = m_ctlTaskListItems.InsertItem(i, strItem);
		m_ctlTaskListItems.SetItemData(nItem, (DWORD) (TaskListItems + i));
		m_ctlTaskListItems.SetCheck(nItem, *(TaskListItems[i].Value));
	}

	UpdateControls();

	return TRUE;
}

void CtManTaskListPg::UpdateControls() {
}

void CtManTaskListPg::OnOK() {
	if (UpdateData()) {
		for (int i = 0; i < m_ctlTaskListItems.GetItemCount(); i++) {
			CItem *pItem = (CItem *) m_ctlTaskListItems.GetItemData(i);
			BOOL bCheck = m_ctlTaskListItems.GetCheck(i);

			if (pItem != NULL && pItem->Value != NULL)
				*(pItem->Value) = bCheck;
		}
	}

	Config.CompactMode = m_bCompactModel;

	CCePropertyPage::OnOK();
}

BOOL CtManTaskListPg::OnSetActive() {
	Config.Page = 1;
	
	return CCePropertyPage::OnSetActive();
}

void CtManTaskListPg::OnIgnoreList() {
	CIgnoreListDlg dlg;

	if (dlg.DoModal() == IDOK) {
	}
}
