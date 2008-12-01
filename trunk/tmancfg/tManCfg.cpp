/**
 *  tManCfg.cpp : Defines the class behaviors for the application.
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

#include "../share/defs.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

UINT ReadConfigMessage;

/////////////////////////////////////////////////////////////////////////////
// CtManCfgApp

BEGIN_MESSAGE_MAP(CtManCfgApp, CWinApp)
	//{{AFX_MSG_MAP(CtManCfgApp)
	//}}AFX_MSG
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CtManCfgApp construction

CtManCfgApp::CtManCfgApp()
	: CWinApp()
{
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CtManCfgApp object

CtManCfgApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CtManCfgApp initialization

BOOL CtManCfgApp::InitInstance()
{
	// Standard initialization
	HIDPI_InitScaling();

	ReadConfigMessage = RegisterWindowMessage(READ_CONFIG_MESSAGE);

	Config.Load();

	Localization = new CLocalization();
	Localization->Init();

	CtManSheet sheet;
	sheet.SetActivePage(Config.Page);
	if (sheet.DoModal()) {
		Config.Save();

		// send message to the tman window
		HWND hWnd = FindWindow(_T("HHTaskBar"), NULL);
		if (hWnd != NULL) {
			HWND hChild = GetWindow(hWnd, GW_CHILD);
			while (hChild != NULL) {
				TCHAR className[10];
				GetClassName(hChild, className, 10);
				if (wcscmp(className, _T("tManClass")) == 0) {
					SendMessage(hChild, ReadConfigMessage, 0, 0);
					break;
				}

				hChild = GetWindow(hChild, GW_HWNDNEXT);
			}
		}
	}

	delete Localization;

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}
