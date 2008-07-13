/**
 *  defs.h: application-wide definitions
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

#ifndef _DEFS_H_
#define _DEFS_H_

// window
#define WINDOW_CLASS						L"tManClass"
#define WINDOW_NAME							L"tManWindow"

#define BUTTON_ACTION_CLOSE					0
#define BUTTON_ACTION_MINIMIZE				1
#define BUTTON_ACTION_SHOW_MENU				2

#define ORIENTATION_LANDSCAPE_RIGHT			0
#define ORIENTATION_LANDSCAPE_LEFT			1

#define INSTALL_DIR							_T("InstallDir")
#define ROOT								_T("Software\\DaProfik\\tMan")

//
#define READ_CONFIG_MESSAGE					_T("DaProfik::tMan::Messages::ReadConfig")

#endif
