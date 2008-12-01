/**
 *  Caption.cpp
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
#include "Caption.h"

#ifdef _DEBUG
#define new DEBUG_NEW
//#define new new(_T(__FILE__), __LINE__)
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCaption

CCaption::CCaption() {
	HGDIOBJ hObj = GetStockObject(SYSTEM_FONT);
	LOGFONT lf;
	::GetObject(hObj, sizeof(LOGFONT), (LPVOID) &lf);
	lf.lfHeight = SCALEY(12) + 1;
	lf.lfWeight = FW_BOLD;
	m_fntBold.CreateFontIndirect(&lf);

	m_rcPadding = CRect(0, 0, 0, 0);
	m_bBold = TRUE;
}

CCaption::~CCaption() {
	m_fntBold.DeleteObject();
}


BEGIN_MESSAGE_MAP(CCaption, CStatic)
	//{{AFX_MSG_MAP(CCaption)
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCaption message handlers

void CCaption::OnPaint() {
	CPaintDC dc(this); // device context for painting

	CRect rcClient;

	GetClientRect(&rcClient);
	dc.FillSolidRect(&rcClient, GetSysColor(COLOR_STATIC));

	// padding
	CRect rcText = rcClient;
	rcText.DeflateRect(m_rcPadding);

	// caption
	CString strText;
	if (m_strTitle.IsEmpty())
		GetWindowText(strText);
	else
		strText = m_strTitle;

	if (m_bBold) {
		CFont *oldFont = dc.SelectObject(&m_fntBold);
		dc.DrawText(strText, rcText, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
		dc.SelectObject(oldFont);
	}
	else {
		dc.DrawText(strText, rcText, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
	}

/*	// line
	CPen blackPen(PS_SOLID, SCALEY(1), GetSysColor(COLOR_3DSHADOW));

	CPen *oldPen  = dc.SelectObject(&blackPen);
	int nHalf = (rcClient.bottom + rcClient.top) / SCALEY(2);
	dc.MoveTo(rcClient.left, rcClient.bottom - SCALEY(2));
	dc.LineTo(rcClient.right, rcClient.bottom - SCALEY(2));

	dc.SelectObject(oldPen);
*/
	ValidateRect(NULL);

	// Do not call CStatic::OnPaint() for painting messages
}

void CCaption::SetTitle(LPCTSTR lpszString) {
	m_strTitle = lpszString;
}

void CCaption::GetTitle(CString &rString) const {
	rString = m_strTitle;
}
