/**
 *  colors.cpp
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
#include "colors.h"
#include "../share/Logger.h"
#include "../share/Config.h"


COLORREF ClrBg;					// color of background
COLORREF ClrBg2;				// color of background
COLORREF ClrFg;					// color of foreground
COLORREF ClrBtnBg;				// color of background
double BaseHue;					// base hue
double BaseSat;
double BaseLit;
COLORREF LastColorHilight;		// for detecting theme change


COLORREF hsl2rgb(double h, double sl, double l) {
	double v;
	double r, g, b;

	r = l;   // default to gray
	g = l;
	b = l;
	v = (l <= 0.5) ? (l * (1.0 + sl)) : (l + sl - l * sl);

	if (v > 0) {
		double m;
		double sv;
		int sextant;
		double fract, vsf, mid1, mid2;

		m = l + l - v;
		sv = (v - m ) / v;
		h *= 6.0;
		sextant = ((int) h);
		fract = h - sextant;
		vsf = v * sv * fract;
		mid1 = m + vsf;
		mid2 = v - vsf;
		switch (sextant) {
			case 0:
				r = v;
				g = mid1;
				b = m;
				break;
			case 1:
				r = mid2;
				g = v;
				b = m;
				break;
			case 2:
				r = m;
				g = v;
				b = mid1;
				break;
			case 3:
				r = m;
				g = mid2;
				b = v;
				break;
			case 4:
				r = mid1;
				g = m;
				b = v;
				break;
			case 5:
				r = v;
				g = m;
				b = mid2;
				break;
		}
	}

	return RGB((BYTE) (r * 255.0f), (BYTE) (g * 255.0f), (BYTE) (b * 255.0f));
}

void rgb2hsl(COLORREF rgb, double &h, double &s, double &l) {
	double r = GetRValue(rgb) / 255.0;
	double g = GetGValue(rgb) / 255.0;
	double b = GetBValue(rgb) / 255.0;
	double v;
	double m;
	double vm;
	double r2, g2, b2;

	h = 0; // default to black
	s = 0;
	l = 0;
	v = max(r, g);
	v = max(v, b);
	m = min(r, g);
	m = min(m, b);
	l = (m + v) / 2.0;
	if (l <= 0.0) {
		return;
	}
	vm = v - m;
	s = vm;
	if (s > 0.0) {
		s /= (l <= 0.5) ? (v + m ) : (2.0 - v - m);
	}
	else {
		return;
	}
	r2 = (v - r) / vm;
	g2 = (v - g) / vm;
	b2 = (v - b) / vm;
	if (r == v) {
		h = (g == m ? 5.0 + b2 : 1.0 - g2);
	}
	else if (g == v) {
		h = (b == m ? 1.0 + r2 : 3.0 - b2);
	}
	else {
		h = (r == m ? 3.0 + g2 : 5.0 - r2);
	}
	h /= 6.0;
}

void LoadThemeColors() {
	LOG0(5, "LoadThemeColors()");

	TCHAR *szSubKey = _T("Software\\Microsoft\\Color");
	HKEY hColor;
	DWORD dwDisposition;
	if (RegCreateKeyEx(HKEY_LOCAL_MACHINE, szSubKey, 0, NULL, 0, KEY_ALL_ACCESS, NULL, &hColor, &dwDisposition) == ERROR_SUCCESS) {
		DWORD dwType;
		DWORD sizeBg = sizeof(COLORREF);
		DWORD sizeFg = sizeof(COLORREF);

		COLORREF shColors[41] = { 0 };
		DWORD sizeSHColors = sizeof(shColors);

		if (RegQueryValueEx(hColor, _T("6"), NULL, &dwType, (LPBYTE) &(shColors[6]), &sizeBg) == ERROR_SUCCESS &&
			RegQueryValueEx(hColor, _T("8"), NULL, &dwType, (LPBYTE) &(shColors[8]), &sizeBg) == ERROR_SUCCESS &&
			RegQueryValueEx(hColor, _T("9"), NULL, &dwType, (LPBYTE) &(shColors[9]), &sizeFg) == ERROR_SUCCESS &&
			RegQueryValueEx(hColor, _T("13"), NULL, &dwType, (LPBYTE) &(shColors[13]), &sizeFg) == ERROR_SUCCESS)
		{
			// user defined theme
			ClrBg2 = shColors[6];
			ClrBg = shColors[8];
			ClrFg = shColors[9];
			ClrBtnBg = shColors[13];
		}
		else if (RegQueryValueEx(hColor, _T("SHColor"), NULL, &dwType, (LPBYTE) &shColors, &sizeSHColors) == ERROR_SUCCESS) {
			// default device theme
			ClrBg2 = shColors[6];
			ClrBg = shColors[8];
			ClrFg = shColors[9];
			ClrBtnBg = shColors[13];
		}
		else {
			// no theme use some system colors
			ClrBg = GetSysColor(COLOR_HIGHLIGHT);
			ClrFg = GetSysColor(COLOR_HIGHLIGHTTEXT);
		}

//			ClrBg2 = RGB(0X00, 0X11, 0X29);
//		RegQueryValueEx(hColor, _T("25"), NULL, &dwType, (LPBYTE) &(shColors[25]), &sizeBg);
//		ClrBtnBg = shColors[25];

//		RegQueryValueEx(hColor, _T("26"), NULL, &dwType, (LPBYTE) &(shColors[26]), &sizeBg);
//		ClrBtnFrame = shColors[26];
//		RegQueryValueEx(hColor, _T("40"), NULL, &dwType, (LPBYTE) &(ClrBtnFrame), &sizeBg);

//		RegQueryValueEx(hColor, _T("13"), NULL, &dwType, (LPBYTE) &(ClrBtnBg), &sizeBg);

//		RegQueryValueEx(hColor, _T("6"), NULL, &dwType, (LPBYTE) &(shColors[6]), &sizeBg);
//		ClrBtnBg = shColors[6];

		rgb2hsl(ClrBg, BaseHue, BaseSat, BaseLit);

		RegCloseKey(hColor);
	}
	else {
		ClrBg = GetSysColor(COLOR_HIGHLIGHT);
		ClrFg = GetSysColor(COLOR_HIGHLIGHTTEXT);
	}

	Config.ClrMenuHiBg = GetSysColor(COLOR_HIGHLIGHT);
	Config.ClrMenuHiFg = GetSysColor(COLOR_HIGHLIGHTTEXT);
	Config.ClrMenuBg = GetSysColor(COLOR_MENU);
	Config.ClrMenuFg = GetSysColor(COLOR_MENUTEXT);
}
