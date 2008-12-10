/**
 *  taskwnd.h
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


#ifndef TASKWND_H_
#define TASKWND_H_

#define TASKWND_CLASS						_T("tManTaskWndClass")
#define TASKWND_WINDOW						_T("tManTaskWnd")

// maximum number of task displayed in task window
#define MAX_TASKS							7

// maximum number of tasks in the task list
#define MAX_TASK_COUNT						128

class CProgramItem {
public:
	TCHAR *Name;
	DWORD ProcessId;
	HWND HWnd;
	HWND HParent;
	HICON HIcon;

	CProgramItem(LPCTSTR name, DWORD processId, HWND hwnd, HICON hIcon = NULL) {
		Name = new TCHAR [wcslen(name) + 1];
		wcscpy(Name, name);
		ProcessId = processId;
		HWnd = hwnd;
		HIcon = hIcon;
	}

	virtual ~CProgramItem() {
		delete [] Name;
	}
};

void GetProcessPathFileName(CProgramItem *pi, LPTSTR pathFileName);

extern HWND HTaskWnd;

#endif /* TASKWND_H_ */
