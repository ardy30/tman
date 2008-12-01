/**
 *  colors.h
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

#ifndef COLORS_H_
#define COLORS_H_

COLORREF hsl2rgb(double h, double sl, double l);
void rgb2hsl(COLORREF rgb, double &h, double &s, double &l);

void LoadThemeColors();

extern COLORREF ClrBg;					// color of background
extern COLORREF ClrBg2;					// color of background
extern COLORREF ClrFg;					// color of foreground
extern COLORREF ClrBtnBg;				// color of background
extern double BaseHue;					// base hue
extern double BaseSat;
extern double BaseLit;
extern COLORREF LastColorHilight;		// for detecting theme change

#endif /* COLORS_H_ */
