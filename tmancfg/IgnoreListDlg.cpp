/**
 *  tManIgnoreListPg.cpp
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
#include "IgnoreListDlg.h"
#include "BrowseDlg.h"
#include "../share/Config.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CIgnoreListDlg property page

CIgnoreListDlg::CIgnoreListDlg() : CCeDialog(CIgnoreListDlg::IDD) {
	//{{AFX_DATA_INIT(CIgnoreListDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_ilApps.Create(SCALEX(16), SCALEY(16), ILC_MASK | ILC_COLOR, 10, 10);
	SetCaption(IDS_IGNORE_LIST);
	m_nMenuID = IDR_EXCEPTIONS;
}

CIgnoreListDlg::~CIgnoreListDlg() {
}

void CIgnoreListDlg::DoDataExchange(CDataExchange* pDX) {
	CCeDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CIgnoreListDlg)
	DDX_Control(pDX, IDC_APP_LIST, m_ctlAppList);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CIgnoreListDlg, CCeDialog)
	//{{AFX_MSG_MAP(CIgnoreListDlg)
	//}}AFX_MSG_MAP
	ON_NOTIFY(GN_CONTEXTMENU, IDC_APP_LIST, OnContextMenu)
	ON_COMMAND(ID_ADD, OnAdd)
	ON_COMMAND(ID_REMOVE, OnRemove)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CIgnoreListDlg message handlers

void CIgnoreListDlg::AddApplication(const CString &strFileName) {
	// icon for progs
	int im = 0;

	SHFILEINFO sfi = { 0 };
	DWORD imageList;
	imageList = SHGetFileInfo(strFileName, 0, &sfi, sizeof(sfi), SHGFI_ICON | SHGFI_SYSICONINDEX | SHGFI_SMALLICON);

	if (imageList != 0) {
		HICON hIcon = ImageList_GetIcon((HIMAGELIST) imageList, sfi.iIcon, ILD_NORMAL);
		im = m_ilApps.Add(hIcon);
	}

	int nItem = m_ctlAppList.InsertItem(-1, strFileName, im);
}

BOOL CIgnoreListDlg::OnInitDialog() {
	CCeDialog::OnInitDialog();
	LocalizeDialog(GetSafeHwnd(), IDD);

	//
	m_ctlAppList.SetExtendedStyle(LVS_EX_FULLROWSELECT);
	m_ctlAppList.SetImageList(&m_ilApps, LVSIL_SMALL);
	CRect rc;
	m_ctlAppList.GetClientRect(&rc);
	m_ctlAppList.InsertColumn(0, _T(""), LVCFMT_LEFT, rc.Width());
	for (int i = 0; i < Config.IgnoredAppsCount; i++) {
		AddApplication(Config.IgnoredApps[i]);
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CIgnoreListDlg::OnAdd() {
	CBrowseDlg dlg(TRUE);
	dlg.SetCaption(IDS_SELECT_PROGRAM);
	dlg.m_strExts = _T("exe");
	dlg.m_strPath = _T("\\");

	if (dlg.DoModal() == IDOK) {
		// do not add already added application
		BOOL found = FALSE;
		for (int i = 0; i < m_ctlAppList.GetItemCount(); i++) {
			CString sApp = m_ctlAppList.GetItemText(i, 0);
			if (sApp.CompareNoCase(dlg.m_strPath) == 0) {
				found = TRUE;
				break;
			}
		}

		if (!found)
			AddApplication(dlg.m_strPath);
	}
}

void CIgnoreListDlg::OnOK() {
	int i;
	// delete the old list
	for (i = 0; i < Config.IgnoredAppsCount; i++)
		delete [] Config.IgnoredApps[i];
	delete [] Config.IgnoredApps;

	// create a new one
	int nItemCount = m_ctlAppList.GetItemCount();
	Config.IgnoredApps = new TCHAR *[nItemCount];
	for (i = 0; i < nItemCount; i++) {
		CString sApp = m_ctlAppList.GetItemText(i, 0);
		
		Config.IgnoredApps[i] = new TCHAR [sApp.GetLength() + 1];
		swprintf(Config.IgnoredApps[i], _T("%s"), sApp);
	}
	Config.IgnoredAppsCount = nItemCount;

	CCeDialog::OnOK();
}

void CIgnoreListDlg::OnContextMenu(NMHDR *pNMHDR, LRESULT *pResult) {
	PNMRGINFO pInfo = (PNMRGINFO) pNMHDR;

	CMenu mnu;
	mnu.LoadMenu(IDR_REMOVE);

	CMenu *popup = mnu.GetSubMenu(0);
	LocalizeMenu(popup->GetSafeHmenu());
	popup->TrackPopupMenu(TPM_LEFTALIGN | TPM_TOPALIGN, pInfo->ptAction.x, pInfo->ptAction.y, this);

	*pResult = TRUE;
}

void CIgnoreListDlg::OnRemove() {
	int nSelItem = m_ctlAppList.GetNextItem(-1, LVNI_SELECTED);
	if (nSelItem != -1) {
		m_ctlAppList.DeleteItem(nSelItem);
	}
}
