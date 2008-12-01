/**
 *  hw.h
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

#ifndef HW_H_
#define HW_H_


#define PLATFORM_WM2002					0
#define PLATFORM_WM2003					1
#define PLATFORM_WM5					2
#define PLATFORM_WM6					3

int GetPlatform();

BOOL ResetPocketPC();
void DisplayOff();
void PowerOff();

void SetScreenOrientation(int orientation);
int GetScreenOrientation();

void DetectWiFiCard();
BOOL IsWiFiOn();
void SetWiFiState(BOOL enable);

BOOL IsBluetoothOn();
void SetBluetoothState(BOOL enable);


extern TCHAR *WiFiDeviceName;

#endif /* HW_H_ */
