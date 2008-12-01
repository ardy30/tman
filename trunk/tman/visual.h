/**
 *  visual.h
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

#ifndef VISUAL_H_
#define VISUAL_H_

#define TMAN_WND_WD							28
#define TMAN_WND_TOP						2

#define TMAN_DOT_SIZE						8
#define CLOSE_BUTTON_SIZE					18

// type of displayed button (Ok/Close/None) on the task bar
enum BtnStyle {
	BTN_NONE,
	BTN_CLOSE,
	BTN_OK
};


void LoadFonts();
void Line(HDC hDC, int x1, int y1, int x2, int y2);
int DrawTextEndEllipsis(HDC hDC, LPCTSTR strText, int nLen, RECT *rc, UINT uFormat);

void DrawOK(HDC hDC, RECT rc, COLORREF clr);
void DrawX(HDC hDC, RECT rc, COLORREF clr);

// WM2002 + WM2003
void DrawBkgndWM2002(HDC hDC, RECT rc);
void DrawBkgndWM2003(HDC hDC, RECT rc);
void DrawButtonWM2003(int type, HDC hDC, RECT rc, BOOL bLoDPI);

// WM5
void DrawBkgndWM5(HDC hDC, RECT rc);
void DrawButtonWM5(int type, HDC hDC, RECT rc, BOOL bLoDPI);

// WM6
void DrawBkgndWM6(HDC hDC, RECT rc);
void DrawButtonWM6(int type, HDC hDC, RECT rc, BOOL bLoDPI);

extern HFONT HButtonFont;
extern HFONT HBoldFont;			// bold font for menu drawing

#endif /* VISUAL_H_ */
