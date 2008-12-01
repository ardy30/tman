/**
 *  visual.cpp
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
#include "visual.h"
#include "colors.h"
#include "hw.h"
#include "../share/Logger.h"
#include "../share/Config.h"

HFONT HButtonFont = 0;			// font for ok button
HFONT HBoldFont = 0;			// bold font for menu drawing

extern BOOL OKPressed;

// ------------------------------------------------------------------------------------------------

void LoadFonts() {
	LOG0(7, "LoadFonts()");

	int dwFontSize;

	HFONT hFont = (HFONT) GetStockObject(SYSTEM_FONT);
	LOGFONT lf;
	GetObject(hFont, sizeof(LOGFONT), &lf);
	lf.lfWeight = FW_BOLD;

	// font for menu
	dwFontSize = SCALEY(11);
//	SHGetUIMetrics(SHUIM_FONTSIZE_PIXEL, &dwFontSize, sizeof(dwFontSize), NULL);
//	int dwFontSize = GetSystemMetrics(SM_CYMENU);
	lf.lfHeight = -dwFontSize;
	HBoldFont = CreateFontIndirect(&lf);

	// font for ok button
	dwFontSize = SCALEY(11);
	lf.lfHeight = -dwFontSize;
	HButtonFont = CreateFontIndirect(&lf);

	DeleteObject(hFont);
}

void Line(HDC hDC, int x1, int y1, int x2, int y2) {
	POINT pts[2];
	pts[0].x = x1; pts[0].y = y1;
	pts[1].x = x2; pts[1].y = y2;
	Polyline(hDC, pts, 2);
}

//
// Draw
//

int DrawTextEndEllipsis(HDC hDC, LPCTSTR strText, int nLen, RECT *rc, UINT uFormat) {
	LOG2(7, "DrawTextEndEllipsis(, '%S', %d, ,)", strText, nLen);

	int nWidth = rc->right - rc->left;

	RECT rcTemp = *rc;
	::DrawText(hDC, strText, nLen, &rcTemp, uFormat | DT_CALCRECT);
	if (rcTemp.right - rcTemp.left > nWidth) {
		// Text doesn't fit in rect. We have to truncate it and add ellipsis to the end.
		TCHAR *strTemp = new TCHAR[nLen + 4]; // sizeof("..." + '\0') == 4
		strTemp[0] = '\0';

		for (int i = nLen; i >= 0; i--) {
			wcsncpy(strTemp, strText, i);
			strTemp[i] = '\0';
			wcscat(strTemp, _T("..."));

			rcTemp = *rc;
			::DrawText(hDC, strTemp, wcslen(strTemp), &rcTemp, uFormat | DT_CALCRECT);
			if (rcTemp.right - rcTemp.left < nWidth) {
				// Gotcha!
				break;
			}
		}
		int ret = ::DrawText(hDC, strTemp, wcslen(strTemp), rc, uFormat);

		delete [] strTemp;

		return ret;
	}

	return ::DrawText(hDC, strText, nLen, rc, uFormat);
}

void DrawOK(HDC hDC, RECT rc, COLORREF clr) {
	HGDIOBJ hOrigFont = SelectObject(hDC, HButtonFont);
	int oldBkMode = SetBkMode(hDC, TRANSPARENT);
	SetTextColor(hDC, clr);
	DrawText(hDC, _T("ok"), 2, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	SetBkMode(hDC, oldBkMode);
	SelectObject(hDC, hOrigFont);
}

void DrawX(HDC hDC, RECT rc, COLORREF clr) {
	// draw 'x'
	HPEN penK = CreatePen(PS_SOLID, 1, clr);
	HGDIOBJ hOrigPenK = SelectObject(hDC, penK);

	Line(hDC, rc.left, rc.top, rc.right + 1, rc.bottom + 1);
	Line(hDC, rc.right, rc.top, rc.left - 1, rc.bottom + 1);

	for (int i = 1; i < SCALEX(2); i++) {
		Line(hDC, rc.left, rc.top + i, rc.right - i + 1, rc.bottom + 1);
		Line(hDC, rc.left + i, rc.top, rc.right + 1, rc.bottom - i + 1);

		Line(hDC, rc.right - i, rc.top, rc.left - 1, rc.bottom - i + 1);
		Line(hDC, rc.right, rc.top + i, rc.left + i - 1, rc.bottom + 1);
	}

	SelectObject(hDC, hOrigPenK);
	DeleteObject(penK);
}

// WM2002 + WM2003

void DrawBkgndWM2002(HDC hDC, RECT rc) {
	HBRUSH br = CreateSolidBrush(ClrBg2);
	HGDIOBJ hOrigBrush = SelectObject(hDC, br);
	Rectangle(hDC, rc.left, rc.top, rc.right, rc.bottom);
	SelectObject(hDC, hOrigBrush);
	DeleteObject(br);
}

void DrawBkgndWM2003(HDC hDC, RECT rc) {
	COLORREF bk;
	if (Config.OldStyleBkgnd)
		bk = ::GetSysColor(COLOR_BACKGROUND);
	else
		bk = ClrBg;

	HBRUSH br = CreateSolidBrush(bk);
	HGDIOBJ hOrigBrush = SelectObject(hDC, br);
	Rectangle(hDC, rc.left, rc.top, rc.right, rc.bottom);
	SelectObject(hDC, hOrigBrush);
	DeleteObject(br);
}

void DrawButtonWM2003(int type, HDC hDC, RECT rc, BOOL bLoDPI) {
	if (bLoDPI) {
		if (Config.ShowDot)
			rc.right = rc.right - SCALEX(2);
		else
			rc.right = rc.right - SCALEX(3);
		rc.top  = rc.top + SCALEY(2);
		rc.left = rc.right - SCALEX(17);
		rc.bottom = rc.top + SCALEY(17);
	}
	else {
		if (Config.ShowDot)
			rc.right = rc.right - SCALEX(1);
		else
			rc.right = rc.right - SCALEX(3);
		rc.top  = rc.top + SCALEY(2);
		rc.left = rc.right - SCALEX(18);
		rc.bottom = rc.top + SCALEY(18);
	}

	COLORREF clrFg;
	if (GetPlatform() == PLATFORM_WM5 || GetPlatform() == PLATFORM_WM6) {
		// old style button on WM5/6
		if (OKPressed)
			clrFg = ClrBg2;
		else
			clrFg = ClrFg;
	}
	else {
		if (OKPressed)
			clrFg = ClrBg;
		else
			clrFg = ClrFg;
	}

	// circle
	HPEN hPen;
	if (bLoDPI)
		hPen = CreatePen(PS_SOLID, 1, clrFg);
	else
		hPen = CreatePen(PS_NULL, 0, clrFg);
	HGDIOBJ hOrigPen = SelectObject(hDC, hPen);
	HBRUSH brFg = CreateSolidBrush(clrFg);
	HGDIOBJ hOrigBrush = SelectObject(hDC, brFg);

	Ellipse(hDC, rc.left, rc.top, rc.right, rc.bottom);

	SelectObject(hDC, hOrigBrush);
	DeleteObject(brFg);
	SelectObject(hDC, hOrigPen);
	DeleteObject(hPen);

	// text
	COLORREF clrFont;
	if (OKPressed)
		clrFont = ClrFg;
	else
		clrFont = ClrBg;

	if (type == BTN_OK) {
		rc.top    = rc.top - 1;
		rc.bottom = rc.bottom - 1;
		if (bLoDPI) {
			rc.left++;
			rc.right++;
		}
		else {
		}
		DrawOK(hDC, rc, clrFont);
	}
	else if (type == BTN_CLOSE) {
		rc.left   = rc.left + SCALEX(4);
		rc.top    = rc.top + SCALEY(4);
		rc.right  = rc.right - SCALEX(5);
		rc.bottom = rc.bottom - SCALEY(5);

		if (bLoDPI) {
		}
		else {
			rc.left++;
			rc.top++;
		}
		DrawX(hDC, rc, clrFont);
	}
}

// WM5

void DrawBkgndWM5(HDC hDC, RECT rc) {
	int r1 = GetRValue(ClrBg);
	int g1 = GetGValue(ClrBg);
	int b1 = GetBValue(ClrBg);

	int r2 = GetRValue(ClrBg2);
	int g2 = GetGValue(ClrBg2);
	int b2 = GetBValue(ClrBg2);

	int w = rc.right - rc.left;
	int wd = (GetDeviceCaps(hDC, HORZRES) - SCALEX(27)) / 2;

	for (int i = 0; i <= w; i++) {
		int j = wd - w + i;

		int r = r1 + (j * (r2-r1) / wd);
		int g = g1 + (j * (g2-g1) / wd);
		int b = b1 + (j * (b2-b1) / wd);
		COLORREF rgb = RGB(r, g, b);

		HPEN pen = CreatePen(PS_SOLID, 1, rgb);
		HGDIOBJ oldPenStep = SelectObject(hDC, pen);

		POINT pts[] = {
			{ rc.left + i, rc.top },
			{ rc.left + i, rc.bottom },
		};
		Polyline(hDC, pts, 2);

		SelectObject(hDC, oldPenStep);
		DeleteObject(pen);
	}
}

void DrawButtonWM5(int type, HDC hDC, RECT rc, BOOL bLoDPI) {
	// draw close/ok button
	if (bLoDPI) {
		if (Config.ShowDot)
			rc.right = rc.right - SCALEX(3);
		else
			rc.right = rc.right - SCALEX(5);

		rc.top  = rc.top + SCALEY(3);
		rc.left = rc.right - SCALEX(16);
		rc.bottom = rc.top + SCALEY(15);
	}
	else {
		if (Config.ShowDot)
			rc.right = rc.right - SCALEX(2);
		else
			rc.right = rc.right - SCALEX(4) - 1;

		rc.top  = rc.top + SCALEY(3);
		rc.left = rc.right - SCALEX(17);
		rc.bottom = rc.top + SCALEY(15);
	}


	int r1 = GetRValue(ClrBg);
	int g1 = GetGValue(ClrBg);
	int b1 = GetBValue(ClrBg);

	int r2 = GetRValue(ClrBg2);
	int g2 = GetGValue(ClrBg2);
	int b2 = GetBValue(ClrBg2);

	int wd = rc.right - rc.left;

	for (int i = 0; i <= wd; i++) {
		int r = r1 + ((i / 2) * (r2-r1) / wd);
		int g = g1 + ((i / 2) * (g2-g1) / wd);
		int b = b1 + ((i / 2) * (b2-b1) / wd);
		COLORREF rgb = RGB(r, g, b);


		HPEN pen = CreatePen(PS_SOLID, 1, rgb);
		HGDIOBJ oldPenStep = SelectObject(hDC, pen);

		POINT pts[] = {
			{ rc.left + i, rc.top },
			{ rc.left + i, rc.bottom },
		};
		Polyline(hDC, pts, 2);

		SelectObject(hDC, oldPenStep);
		DeleteObject(pen);
	}

	// colors
	COLORREF clrFont;
	if (OKPressed)
		clrFont = ::GetSysColor(COLOR_MENUTEXT);
	else
		clrFont = ClrFg;

	if (type == BTN_OK) {
		// Low DPI
		if (bLoDPI) {
			rc.left++;
			rc.right++;
		}

		DrawOK(hDC, rc, clrFont);
	}
	else if (type == BTN_CLOSE) {
		rc.left   = rc.left + SCALEX(4);
		rc.top    = rc.top + SCALEY(2) + 1;
		rc.right  = rc.right - SCALEX(4);
		rc.bottom = rc.bottom - SCALEY(4);

		if (bLoDPI) {
		}
		else {
			rc.left++;
			rc.right++;
			rc.top++;
			rc.bottom++;
		}

		DrawX(hDC, rc, clrFont);
	}
}

// WM6

void DrawBkgndWM6(HDC hDC, RECT rc) {
	COLORREF rgb1, rgb2, rgb3;
	int r1, g1, b1;
	int r2, g2, b2;
	int r3, g3, b3;

	int w = rc.right - rc.left;
	int wd = (GetDeviceCaps(hDC, HORZRES) - SCALEX(27)) / 2;

	int middle = ((rc.top + rc. bottom) / 2) - SCALEY(2);

	// top part
	rgb1 = hsl2rgb(BaseHue, 0.75 * BaseSat, 0.45);
	r1 = GetRValue(rgb1);
	g1 = GetGValue(rgb1);
	b1 = GetBValue(rgb1);

	rgb2 = hsl2rgb(BaseHue, 0.45 * BaseSat, 0.25);
	r2 = GetRValue(rgb2);
	g2 = GetGValue(rgb2);
	b2 = GetBValue(rgb2);

	rgb3 = hsl2rgb(BaseHue, 0.45 * BaseSat, 0.75);
	r3 = GetRValue(rgb3);
	g3 = GetGValue(rgb3);
	b3 = GetBValue(rgb3);

	for (int i = 0; i <= w; i++) {
		int j = wd - w + i;

		int r = r1 + (j * (r2-r1) / wd);
		int g = g1 + (j * (g2-g1) / wd);
		int b = b1 + (j * (b2-b1) / wd);

		for (int k = 0; k <= middle; k++) {
			int ra = r + (k * (r3-r) / (middle + SCALEY(TMAN_WND_TOP)));
			int ga = g + (k * (g3-g) / (middle + SCALEY(TMAN_WND_TOP)));
			int ba = b + (k * (b3-b) / (middle + SCALEY(TMAN_WND_TOP)));

			COLORREF a = RGB(ra, ga, ba);
			SetPixel(hDC, rc.left + i, rc.top + middle - k, a);
		}
	}

	// bottom part
	rgb1 = hsl2rgb(BaseHue, 1.0 * BaseSat, 0.60 * BaseLit);
	r1 = GetRValue(rgb1);
	g1 = GetGValue(rgb1);
	b1 = GetBValue(rgb1);

	rgb2 = hsl2rgb(BaseHue, 1.0 * BaseSat, 0.50 * BaseLit);
	r2 = GetRValue(rgb2);
	g2 = GetGValue(rgb2);
	b2 = GetBValue(rgb2);

	rgb3 = hsl2rgb(BaseHue, 1.0 * BaseSat, 0.58 * BaseLit);
	r3 = GetRValue(rgb3);
	g3 = GetGValue(rgb3);
	b3 = GetBValue(rgb3);

	int ht = rc.bottom - middle;

	for (i = 0; i <= ht; i++) {
		int j = i;

		int r = r1 + (j * (r3-r1) / ht);
		int g = g1 + (j * (g3-g1) / ht);
		int b = b1 + (j * (b3-b1) / ht);

		for (int k = 0; k <= w; k++) {
			int ra = r + (k * (r2-r) / (w));
			int ga = g + (k * (g2-g) / (w));
			int ba = b + (k * (b2-b) / (w));

			COLORREF a = RGB(ra, ga, ba);
			SetPixel(hDC, rc.left + k, rc.bottom - i, a);
		}
	}
}

void DrawButtonWM6(int type, HDC hDC, RECT rc, BOOL bLoDPI) {
	// draw close/ok button
	if (bLoDPI) {
		if (Config.ShowDot)
			rc.right = rc.right - SCALEX(1);
		else
			rc.right = rc.right - SCALEX(3);

		rc.top  = rc.top + SCALEY(2);
		rc.left = rc.right - SCALEX(19);
		rc.bottom = rc.top + SCALEY(17);
	}
	else {
		if (Config.ShowDot)
			rc.right = rc.right - SCALEX(1);
		else
			rc.right = rc.right - SCALEX(3) - 1;

		rc.top  = rc.top + SCALEY(2) + 1;
		rc.left = rc.right - SCALEX(18);
		rc.bottom = rc.top + SCALEY(16);
	}

	HGDIOBJ oldPen;

	COLORREF clrBtnFrame1, clrBtnFrame2, clrBtnBg;

	if (Config.OldStyleBkgnd) {
//		clrBtnFrame1 = ::GetSysColor(COLOR_WINDOWFRAME);

		clrBtnBg = clrBtnFrame2 = ::GetSysColor(COLOR_HIGHLIGHT);
		double h, s, l;
		rgb2hsl(clrBtnFrame2, h, s, l);
		clrBtnFrame1 = hsl2rgb(h, s, l * 0.45);
	}
	else {
		clrBtnFrame1 = ::GetSysColor(COLOR_WINDOWFRAME);
		clrBtnFrame2 = ::GetSysColor(COLOR_HIGHLIGHT);
		clrBtnBg = ClrBg;
	}

	HPEN penFrame = CreatePen(PS_SOLID, SCALEX(1), clrBtnFrame1);
	oldPen = SelectObject(hDC, penFrame);

	Rectangle(hDC, rc.left, rc.top, rc.right, rc.bottom);

	SelectObject(hDC, oldPen);
	DeleteObject(penFrame);

	//
	HPEN pen = CreatePen(PS_SOLID, SCALEX(1), clrBtnFrame2);
	oldPen = SelectObject(hDC, pen);

	HBRUSH brush = CreateSolidBrush(clrBtnBg);
	HGDIOBJ oldBrush = SelectObject(hDC, brush);

	InflateRect(&rc, -SCALEX(1), -SCALEY(1));
	Rectangle(hDC, rc.left, rc.top, rc.right, rc.bottom);

	SelectObject(hDC, oldBrush);
	DeleteObject(brush);

	SelectObject(hDC, oldPen);
	DeleteObject(pen);

	// text
	COLORREF clrFont;
	if (OKPressed)
		clrFont = ::GetSysColor(COLOR_MENUTEXT);
	else
		clrFont = ClrFg;

	if (type == BTN_OK) {
		if (bLoDPI) {
			rc.left++;
			rc.right++;
		}
		else {
			rc.top++;
			rc.bottom++;
		}
		DrawOK(hDC, rc, clrFont);
	}
	else if (type == BTN_CLOSE) {
		rc.left   = rc.left + SCALEX(4);
		rc.top    = rc.top + SCALEY(2) + 1;
		rc.right  = rc.right - SCALEX(4);
		rc.bottom = rc.bottom - SCALEY(3);

		if (bLoDPI) {
			rc.bottom--;
			rc.right--;
		}
		else {
			rc.right++;
		}

		DrawX(hDC, rc, clrFont);
	}
}
