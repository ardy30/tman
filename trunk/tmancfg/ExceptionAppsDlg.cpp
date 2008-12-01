/**
 *  ExceptionAppsDlg.cpp
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
#include "ExceptionAppsDlg.h"
#include "BrowseDlg.h"
#include "../share/defs.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CExceptionAppsDlg dialog


CExceptionAppsDlg::CExceptionAppsDlg(CWnd* pParent /*=NULL*/)
	: CCeDialog(CExceptionAppsDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CExceptionAppsDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_ilApps.Create(SCALEX(16), SCALEY(16), ILC_MASK | ILC_COLOR, 10, 10);
	SetMenu(IDR_EXCEPTIONS);
	SetCaption(IDS_EXCEPTIONS);
}


void CExceptionAppsDlg::DoDataExchange(CDataExchange* pDX) {
	CCeDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CExceptionAppsDlg)
	DDX_Control(pDX, IDC_PROGRAMS, m_ctlPrograms);
	DDX_Control(pDX, IDC_TAP_ACTION, m_ctlTapAction);
	DDX_Control(pDX, IDC_GESTURE_ACTION, m_ctlGestureAction);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CExceptionAppsDlg, CCeDialog)
	//{{AFX_MSG_MAP(CExceptionAppsDlg)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_PROGRAMS, OnItemchangedPrograms)
	ON_CBN_SELENDOK(IDC_GESTURE_ACTION, OnSelendokGestureAction)
	ON_CBN_SELENDOK(IDC_TAP_ACTION, OnSelendokTapAction)
	//}}AFX_MSG_MAP
	ON_NOTIFY(GN_CONTEXTMENU, IDC_PROGRAMS, OnContextMenu)
	ON_COMMAND(ID_ADD, OnAdd)
	ON_COMMAND(ID_REMOVE, OnRemove)
END_MESSAGE_MAP()

struct CCBItem {
	UINT Id;
	DWORD Data;

	CCBItem(int id, DWORD data) {
		Id = id;
		Data = data;
	}
}
Actions[] = {
	CCBItem(IDS_CLOSE, BUTTON_ACTION_CLOSE),
	CCBItem(IDS_MINIMIZE, BUTTON_ACTION_MINIMIZE),
//	CCBItem(IDS_SHOW_MENU, BUTTON_ACTION_SHOW_MENU)
};

/////////////////////////////////////////////////////////////////////////////
// CExceptionAppsDlg message handlers

int CExceptionAppsDlg::AddApplication(const CConfig::CAppException *ex) {
	// icon for progs
	int im = 0;

	SHFILEINFO sfi = { 0 };
	DWORD imageList;
	imageList = SHGetFileInfo(ex->ExeName, 0, &sfi, sizeof(sfi), SHGFI_ICON | SHGFI_SYSICONINDEX | SHGFI_SMALLICON);

	if (imageList != 0) {
		HICON hIcon = ImageList_GetIcon((HIMAGELIST) imageList, sfi.iIcon, ILD_NORMAL);
		im = m_ilApps.Add(hIcon);
	}

	int nItem = m_ctlPrograms.InsertItem(-1, ex->ExeName, im);
	m_ctlPrograms.SetItemData(nItem, (DWORD) ex);

	return nItem;
}

BOOL CExceptionAppsDlg::OnInitDialog() {
	CCeDialog::OnInitDialog();
	LocalizeDialog(GetSafeHwnd(), IDD);

	int i;

	//
	m_ctlPrograms.SetExtendedStyle(LVS_EX_FULLROWSELECT);
	m_ctlPrograms.SetImageList(&m_ilApps, LVSIL_SMALL);
	CRect rc;
	m_ctlPrograms.GetClientRect(&rc);
	m_ctlPrograms.InsertColumn(0, _T(""), LVCFMT_LEFT, rc.Width());
	for (i = 0; i < Config.ExceptionsCount; i++) {
		CConfig::CAppException *ex = Config.Exceptions[i];
		AddApplication(new CConfig::CAppException(ex->ExeName, ex->TapAction, ex->GestureAction));
	}

	//
	for (i = 0; i < sizeof(Actions) / sizeof(Actions[0]); i++) {
		CLocalizedString sText;
		sText.LoadString(Actions[i].Id);

		int item;
		item = m_ctlTapAction.AddString(sText);
		m_ctlTapAction.SetItemData(item, Actions[i].Data);

		item = m_ctlGestureAction.AddString(sText);
		m_ctlGestureAction.SetItemData(item, Actions[i].Data);
	}

	UpdateControls();

	return TRUE;
}

void CExceptionAppsDlg::OnAdd() {
	CBrowseDlg dlg(TRUE);
	dlg.SetCaption(IDS_SELECT_PROGRAM);
	dlg.m_strExts = _T("exe");
	dlg.m_strPath = _T("\\");

	if (dlg.DoModal() == IDOK) {
		// do not add already added application
		BOOL found = FALSE;
		for (int i = 0; i < m_ctlPrograms.GetItemCount(); i++) {
			CString sApp = m_ctlPrograms.GetItemText(i, 0);
			if (sApp.CompareNoCase(dlg.m_strPath) == 0) {
				found = TRUE;
				break;
			}
		}

		if (!found) {
			LPTSTR lpValue = dlg.m_strPath.GetBuffer(dlg.m_strPath.GetLength());

			CConfig::CAppException *ex =
				new CConfig::CAppException(lpValue, BUTTON_ACTION_CLOSE, BUTTON_ACTION_MINIMIZE);
			int nItem = AddApplication(ex);
			m_ctlPrograms.SetItemState(nItem, LVIS_SELECTED, LVIS_SELECTED);

			dlg.m_strPath.ReleaseBuffer();
		}
	}
}

void CExceptionAppsDlg::OnOK() {
	int i;
	// dispose old exception list
	for (i = 0; i < Config.ExceptionsCount; i++)
		delete Config.Exceptions[i];
	delete [] Config.Exceptions;

	// create a new one
	int nItemCount = m_ctlPrograms.GetItemCount();
	Config.Exceptions = new CConfig::CAppException *[nItemCount];
	for (i = 0; i < nItemCount; i++) {
		CConfig::CAppException *ex = (CConfig::CAppException *) m_ctlPrograms.GetItemData(i);
		Config.Exceptions[i] = ex;
	}
	Config.ExceptionsCount = nItemCount;

	CCeDialog::OnOK();
}

void CExceptionAppsDlg::OnCancel() {
	int nItemCount = m_ctlPrograms.GetItemCount();
	for (int i = 0; i < nItemCount; i++) {
		CConfig::CAppException *ex = (CConfig::CAppException *) m_ctlPrograms.GetItemData(i);
		delete ex;
	}

	CCeDialog::OnCancel();
}

void CExceptionAppsDlg::OnItemchangedPrograms(NMHDR *pNMHDR, LRESULT *pResult) {
	NM_LISTVIEW *pNMListView = (NM_LISTVIEW *) pNMHDR;

	// set the appropriate values
	if (pNMListView->iItem != -1 && pNMListView->uNewState & LVIS_SELECTED) {
		CConfig::CAppException *ex = (CConfig::CAppException *) pNMListView->lParam;

		if (ex != NULL) {
			m_ctlTapAction.SetCurSel(ex->TapAction);
			m_ctlGestureAction.SetCurSel(ex->GestureAction);
		}
	}

	UpdateControls();

	*pResult = 0;
}

void CExceptionAppsDlg::UpdateControls() {
	int nSelItem = m_ctlPrograms.GetNextItem(-1, LVNI_SELECTED);
	if (nSelItem != -1) {
		m_ctlGestureAction.EnableWindow();
		m_ctlTapAction.EnableWindow();
	}
	else {
		m_ctlGestureAction.EnableWindow(FALSE);
		m_ctlTapAction.EnableWindow(FALSE);
	}
}

void CExceptionAppsDlg::OnSelendokTapAction() {
	int nSelItem = m_ctlPrograms.GetNextItem(-1, LVNI_SELECTED);
	if (nSelItem != -1) {
		CConfig::CAppException *ex = (CConfig::CAppException *) m_ctlPrograms.GetItemData(nSelItem);
		if (ex != NULL) {
			int nCurSel = m_ctlTapAction.GetCurSel();
			int action = m_ctlTapAction.GetItemData(nCurSel);
			ex->TapAction = action;
		}
	}
}

void CExceptionAppsDlg::OnSelendokGestureAction() {
	int nSelItem = m_ctlPrograms.GetNextItem(-1, LVNI_SELECTED);
	if (nSelItem != -1) {
		CConfig::CAppException *ex = (CConfig::CAppException *) m_ctlPrograms.GetItemData(nSelItem);
		if (ex != NULL) {
			int nCurSel = m_ctlGestureAction.GetCurSel();
			int action = m_ctlGestureAction.GetItemData(nCurSel);
			ex->GestureAction = action;
		}
	}
}

void CExceptionAppsDlg::OnContextMenu(NMHDR *pNMHDR, LRESULT *pResult) {
	PNMRGINFO pInfo = (PNMRGINFO) pNMHDR;

	CMenu mnu;
	mnu.LoadMenu(IDR_REMOVE);

	CMenu *popup = mnu.GetSubMenu(0);
	LocalizeMenu(popup->GetSafeHmenu());
	popup->TrackPopupMenu(TPM_LEFTALIGN | TPM_TOPALIGN, pInfo->ptAction.x, pInfo->ptAction.y, this);

	*pResult = TRUE;
}

void CExceptionAppsDlg::OnRemove() {
	int nSelItem = m_ctlPrograms.GetNextItem(-1, LVNI_SELECTED);
	if (nSelItem != -1) {
		CConfig::CAppException *ex = (CConfig::CAppException *) m_ctlPrograms.GetItemData(nSelItem);
		delete ex;

		m_ctlPrograms.DeleteItem(nSelItem);
	}
}
