/**
 *  Logger.cpp: implementation of the CLogger class.
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

#include <afx.h>
#include "Logger.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#ifdef LOGGING

CLogger *Logger = NULL;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CLogger::CLogger(LPCTSTR fileName, int level/* = 1*/) {
	InitializeCriticalSection(&CS);

	Level = level;
	if (Level != 0) {
		// CreateFile do not delete existing file -> when the file is overwritten, it may contain parts of previous content
		DeleteFile(fileName);
		File = CreateFile(fileName, GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	}
	else
		File = NULL;	// no logging
}

CLogger::~CLogger() {
	if (File != NULL)
		CloseHandle(File);

	DeleteCriticalSection(&CS);
}

void CLogger::Log(int level, char *str, ...) {
	// if the level of a message to log if bigger than watched level => exit (do not log)
	if (level > Level)
		return;

	EnterCriticalSection(&CS);

	char msg[1024], txt[2048];
	TCHAR time[256];

	va_list arglist;
	va_start(arglist, str);
	vsprintf(msg, str, arglist);
	va_end(arglist);

	SYSTEMTIME now;
	GetLocalTime(&now);
	int l = GetTimeFormat(LOCALE_USER_DEFAULT, 0, &now, _T("HH:mm:ss"), time, 255);
	time[l] = '\0';
	char mbTime[256];
	WideCharToMultiByte(CP_OEMCP, 0, time, -1, mbTime, 255, NULL, NULL);

	DWORD written;
	sprintf(txt, "%s - %s\n", mbTime, msg);
	WriteFile(File, txt, strlen(txt), &written, NULL);

	LeaveCriticalSection(&CS);
}

#endif
