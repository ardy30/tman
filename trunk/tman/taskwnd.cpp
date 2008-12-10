/**
 *  taskwnd.cpp
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
#include "taskwnd.h"
#include "../share/Config.h"
#include "visual.h"
#include "hw.h"

BOOL RegisterTaskWnd();

static BOOL Registred = RegisterTaskWnd();
static int HotKeyID = 0;

HWND HTaskWnd = 0;				// handle to the AltTab task window
UINT AltTabTimer = 7;
BOOL AltTabPrgIdx = 0;

extern HINSTANCE HInstance;
extern HWND HMain;
extern int TaskCount;
extern CProgramItem *TaskList[MAX_TASK_COUNT];
HWND GetTopParent(HWND hwnd);
void GetProcessPathFileName(CProgramItem *pi, LPTSTR pathFileName);


static void OnCreate(HWND hWnd) {
	UnregisterHotKey(HMain, HotKeyID);

	UnregisterHotKey(hWnd, HotKeyID);
	RegisterHotKey(hWnd, HotKeyID, MOD_WIN, Config.TaskListHWKey);

	SetTimer(hWnd, AltTabTimer, Config.AltTabTimeout, NULL);

	SetCapture(hWnd);

	AltTabPrgIdx = 1;
}

static void OnPaint(HWND hWnd) {
	PAINTSTRUCT ps;
	HDC hDC = BeginPaint(hWnd, &ps);

	RECT rc;
	GetClientRect(hWnd, &rc);

	HBRUSH brHilight = CreateSolidBrush(GetSysColor(COLOR_HIGHLIGHT));
	HBRUSH brBkgnd = CreateSolidBrush(GetSysColor(COLOR_WINDOW));
	HPEN penBkgnd = CreatePen(PS_NULL, 0, 0);

	int maxTasks = (TaskCount > MAX_TASKS) ? MAX_TASKS : TaskCount;
	int half = maxTasks / 2;
	int firstTask = (AltTabPrgIdx + TaskCount - half) % TaskCount;

	for (int i = 0; i < maxTasks; i++) {
		int act = (firstTask + i) % TaskCount;
		CProgramItem *pi = TaskList[act];

		RECT rcItem;
		rcItem.left = rc.left;
		rcItem.top = rc.top + (i * SCALEY(20));
		rcItem.right = rc.right;
		rcItem.bottom = rc.top + ((i + 1) * SCALEY(20));

		HGDIOBJ hOrigBrush;
		if (act == AltTabPrgIdx) hOrigBrush = SelectObject(hDC, brHilight);
		else hOrigBrush = SelectObject(hDC, brBkgnd);
		HGDIOBJ hOrigPen = SelectObject(hDC, penBkgnd);

		Rectangle(hDC, rcItem.left, rcItem.top, rcItem.right, rcItem.bottom);

		SelectObject(hDC, hOrigPen);
		SelectObject(hDC, hOrigBrush);

		// icon for progs
		TCHAR pathFileName[MAX_PATH];
		GetProcessPathFileName(pi, pathFileName);

		SHFILEINFO sfi = { 0 };
		DWORD imageList;
		imageList = SHGetFileInfo(pathFileName, 0, &sfi, sizeof(sfi), SHGFI_ICON | SHGFI_SYSICONINDEX | SHGFI_SMALLICON);

		if (imageList != 0) {
			ImageList_DrawEx((HIMAGELIST) imageList, sfi.iIcon, hDC,
				rcItem.left + SCALEX(7), rcItem.top + SCALEY(1), SCALEX(16), SCALEY(16), CLR_NONE, CLR_NONE, ILD_NORMAL);
		}

		// font
		HGDIOBJ hOrigFont = SelectObject(hDC, HBoldFont);
		RECT rcText = rcItem;
		rcText.left += SCALEX(25);
		rcText.right -= SCALEX(25);
		if (act == AltTabPrgIdx) SetTextColor(hDC, GetSysColor(COLOR_HIGHLIGHTTEXT));
		else SetTextColor(hDC, GetSysColor(COLOR_WINDOWTEXT));
		int oldMode = SetBkMode(hDC, TRANSPARENT);

		UINT uFmt = DT_LEFT | DT_VCENTER | DT_SINGLELINE;
		if (pi->ProcessId != 0) uFmt |= DT_NOPREFIX;		// we don not want to interpret & as mark of hot key

		DrawTextEndEllipsis(hDC, pi->Name, lstrlen(pi->Name), &rcText, uFmt);
		SetBkMode(hDC, oldMode);
		SelectObject(hDC, hOrigFont);

	}

	DeleteObject(brBkgnd);
	DeleteObject(penBkgnd);

	EndPaint(hWnd, &ps);
}

static void OnTimer(UINT eventID) {
	if (eventID == AltTabTimer) {
		// activate task
		KillTimer(HTaskWnd, AltTabTimer);
		DestroyWindow(HTaskWnd);
	}
}

static LRESULT OnHotKey(HWND hWnd, WPARAM wParam, LPARAM lParam) {
	KillTimer(HTaskWnd, AltTabTimer);

	AltTabPrgIdx++;
	if (AltTabPrgIdx >= TaskCount) AltTabPrgIdx = 0;

	InvalidateRect(HTaskWnd, NULL, FALSE);
	UpdateWindow(HTaskWnd);

	SetTimer(HTaskWnd, AltTabTimer, Config.AltTabTimeout, NULL);

	return 0;
}

static void OnDestroy() {
	ReleaseCapture();

	UnregisterHotKey(HTaskWnd, HotKeyID);
	HTaskWnd = NULL;

	RegisterHotKey(HMain, HotKeyID, MOD_WIN | MOD_KEYUP, Config.TaskListHWKey);

	// activate the selected window
	if (AltTabPrgIdx >= 0 && AltTabPrgIdx < TaskCount && TaskList[AltTabPrgIdx] != NULL) {
		CProgramItem *pi = TaskList[AltTabPrgIdx];
		SetForegroundWindow(pi->HWnd);
	}
}

static LRESULT CALLBACK TaskWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
		case WM_CREATE:
			OnCreate(hWnd);
			return 0;

		case WM_ERASEBKGND:
			return 0;

		case WM_PAINT:
			OnPaint(hWnd);
			return 0;

		case WM_TIMER:
			OnTimer(wParam);
			return 0;

		case WM_HOTKEY:
			return OnHotKey(HTaskWnd, wParam, lParam);

		case WM_DESTROY:
			OnDestroy();
			return 0;

		default:
			return DefWindowProc(hWnd, msg, wParam, lParam);
	}
}

static BOOL RegisterTaskWnd() {
	WNDCLASS wc = { 0 };
	wc.style = CS_PARENTDC;
	wc.lpszClassName = TASKWND_CLASS;
	wc.hbrBackground = (HBRUSH) GetStockObject(COLOR_WINDOW);
	wc.hInstance = HInstance;
	wc.lpfnWndProc = TaskWndProc;

	return RegisterClass(&wc) != 0;
}

