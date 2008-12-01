/**
 *  hw.cpp
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
#include "hw.h"
#include "../share/Logger.h"
#include "../share/Config.h"
#include "../share/regs.h"

#include <winioctl.h>
#include <bthutil.h>
#include <pm.h>
#include <ndispwr.h>
#include <ntddndis.h>

// ----------------------------------------------------------------------------
// reset

#define IOCTL_HAL_REBOOT CTL_CODE(FILE_DEVICE_HAL, 15, METHOD_BUFFERED, FILE_ANY_ACCESS)

extern "C" __declspec(dllimport) BOOL KernelIoControl(
	DWORD dwIoControlCode,
	LPVOID lpInBuf,
	DWORD nInBufSize,
	LPVOID lpOutBuf,
	DWORD nOutBufSize,
	LPDWORD lpBytesReturned);

// ----------------------------------------------------------------------------
// display

// GDI Escapes for ExtEscape()
#define QUERYESCSUPPORT    8

// The following are unique to CE
#define GETVFRAMEPHYSICAL			6144
#define GETVFRAMELEN				6145
#define DBGDRIVERSTAT				6146
#define SETPOWERMANAGEMENT			6147
#define GETPOWERMANAGEMENT			6148


typedef enum _VIDEO_POWER_STATE {
    VideoPowerOn = 1,
    VideoPowerStandBy,
    VideoPowerSuspend,
    VideoPowerOff
} VIDEO_POWER_STATE, *PVIDEO_POWER_STATE;


typedef struct _VIDEO_POWER_MANAGEMENT {
    ULONG Length;
    ULONG DPMSVersion;
    ULONG PowerState;
} VIDEO_POWER_MANAGEMENT, *PVIDEO_POWER_MANAGEMENT;


// ----------------------------------------------------------------------------

int GetPlatform() {
	static OSVERSIONINFO OSVI;
	static int platformID = -1;

	if (platformID == -1) {
		OSVI.dwOSVersionInfoSize = sizeof(OSVI);
		GetVersionEx(&OSVI);

		switch (OSVI.dwMajorVersion) {
			case 3: platformID = PLATFORM_WM2002; break;
			case 4: platformID = PLATFORM_WM2003; break;
			case 5:
				if (OSVI.dwMinorVersion < 2) platformID = PLATFORM_WM5;
				else if (OSVI.dwMinorVersion == 2) platformID = PLATFORM_WM6;
				else platformID = PLATFORM_WM6;
				break;
			default: platformID = PLATFORM_WM2002; break;
		}
	}

	return platformID;
}

//
// HW
//

BOOL ResetPocketPC() {
	LOG0(3, "ResetPocketPC()");

	return KernelIoControl(IOCTL_HAL_REBOOT, NULL, 0, NULL, 0, NULL);
}

void DisplayOff() {
	LOG0(3, "DisplayOff()");

	// display off
	HDC gdc;
	int iESC = SETPOWERMANAGEMENT;

	gdc = ::GetDC(NULL);
	if (ExtEscape(gdc, QUERYESCSUPPORT, sizeof(int), (LPCSTR) &iESC, 0, NULL) == 0) {
		// display off feature not
	}
	else {
		VIDEO_POWER_MANAGEMENT vpm;
		vpm.Length = sizeof(VIDEO_POWER_MANAGEMENT);
		vpm.DPMSVersion = 0x0001;
		vpm.PowerState = VideoPowerOff;
		// Power off the display
		ExtEscape(gdc, SETPOWERMANAGEMENT, vpm.Length, (LPCSTR) &vpm, 0, NULL);
	}

	::ReleaseDC(NULL, gdc);
}

void PowerOff() {
	LOG0(3, "PowerOff()");

	// Send keypresses that mean power off
	keybd_event(VK_OFF, 0, KEYEVENTF_SILENT, 0);
	keybd_event(VK_OFF, 0, (KEYEVENTF_KEYUP | KEYEVENTF_SILENT), 0);
}

void SetScreenOrientation(int orientation) {
	LOG1(3, "SetScreenOrientation(%d)", orientation);

	DEVMODE devmode = { 0 };
	devmode.dmSize = sizeof(DEVMODE);
	devmode.dmDisplayOrientation = orientation;
	// landscape mode
	devmode.dmFields = DM_DISPLAYORIENTATION;
	ChangeDisplaySettingsEx(NULL, &devmode, NULL, 0, NULL);

	// make the change pernament
	HKEY hKey;
	DWORD dwDisposition;
	if (RegCreateKeyEx(HKEY_LOCAL_MACHINE, _T("System\\GDI\\Rotation"), 0, NULL, 0, KEY_ALL_ACCESS, NULL, &hKey, &dwDisposition) == ERROR_SUCCESS) {
		DWORD angle = 0;
		switch (orientation) {
			case DMDO_90:		// right-handed landscape mode
				angle = 90;
				break;
			case DMDO_180:		// rotated upside-down
				angle = 180;
				break;
			case DMDO_270:		// left-handed landscape mode
				angle = 270;
				break;
		}
		RegSetValueEx(hKey, _T("Angle"), 0, REG_DWORD, (BYTE *) &angle, sizeof(angle));

		RegCloseKey(hKey);
	}
}

int GetScreenOrientation() {
	LOG0(3, "GetScreenOrientation()");

	DEVMODE devmode = { 0 };
	devmode.dmSize = sizeof(DEVMODE);
	devmode.dmFields = DM_DISPLAYORIENTATION;
	ChangeDisplaySettingsEx(NULL, &devmode, 0, CDS_TEST, NULL);

	return devmode.dmDisplayOrientation;
}

// WiFi

TCHAR *WiFiDeviceName = NULL;

void DetectWiFiCard() {
	// TODO: detect the wifi device name

	if (wcslen(Config.WiFiDevice) > 0) {
		WiFiDeviceName = new TCHAR[wcslen(PMCLASS_NDIS_MINIPORT) + 1 + wcslen(Config.WiFiDevice)];
		swprintf(WiFiDeviceName, _T("%s\\%s"), PMCLASS_NDIS_MINIPORT, Config.WiFiDevice);
	}
}

BOOL IsWiFiOn() {
	CEDEVICE_POWER_STATE state;
	GetDevicePower(WiFiDeviceName, POWER_NAME, &state);
	return state == D0;
}

void SetWiFiState(BOOL enable) {
	if (enable) {
/*		// THIS DOES NOT SEEM TO WORK :(
		HANDLE hNdisPwr = CreateFile(NDISPWR_DEVICE_NAME, 0x00, 0x00, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, NULL);
		if (hNdisPwr != INVALID_HANDLE_VALUE) {
			NDISPWR_SAVEPOWERSTATE sps = { 0 };
			sps.pwcAdapterName = Config.WiFiDevice;
			sps.CePowerState = PwrDeviceUnspecified;

			BOOL ret = DeviceIoControl(hNdisPwr, IOCTL_NPW_SAVE_POWER_STATE, &sps, sizeof(NDISPWR_SAVEPOWERSTATE), NULL, 0x00, NULL, NULL);
			CloseHandle(hNdisPwr);

			SetDevicePower(WiFiDeviceName, POWER_NAME, D0);

			// bind the adapter
	//		HANDLE	hNdis = CreateFile(NDISUIO_DEVICE_NAME, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, 0, NULL); //DD_NDIS_DEVICE_NAME
			HANDLE	hNdis = CreateFile(DD_NDIS_DEVICE_NAME, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, 0, NULL);
			if (hNdis != INVALID_HANDLE_VALUE) {
				TCHAR devName[256];
				swprintf(devName, _T("TIACXWLN1\0\0"), Config.WiFiDevice);
				if (DeviceIoControl(hNdis, IOCTL_NDIS_BIND_ADAPTER, devName, (wcslen(devName) + 2) * sizeof(TCHAR), NULL, NULL, NULL, NULL) == 0) {
					TCHAR a[256];
					swprintf(a, _T("K: %x"), GetLastError());
					MessageBox(NULL, a, _T("DBG"), MB_OK | MB_SETFOREGROUND);
				}
				CloseHandle(hNdis);
			}
			else {
				TCHAR a[256];
				swprintf(a, _T("NOpe: %d"), GetLastError());
				MessageBox(NULL, a, _T("DBG"), MB_OK | MB_SETFOREGROUND);
			}
		}
		else {
			TCHAR a[256];
			swprintf(a, _T("Ne-e: %d"), GetLastError());
			MessageBox(NULL, a, _T("DBG"), MB_OK | MB_SETFOREGROUND);
		}
*/
		SetDevicePower(WiFiDeviceName, POWER_NAME, D0);
	}
	else {
		SetDevicePower(WiFiDeviceName, POWER_NAME, D4);
	}
}

// Bluetooth

#define REG_WIDCOMM_BRANCH						L"SOFTWARE\\WIDCOMM\\BtConfig\\General"

BOOL IsBluetoothOn() {
	BOOL state = FALSE;
	HKEY hKey;
	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, REG_WIDCOMM_BRANCH, 0, NULL, &hKey) == ERROR_SUCCESS) {
		// we are on Widcomm BTH stack
		state = (RegReadDword(hKey, L"StackMode", -1) == 1);
		RegCloseKey(hKey);
	}
	else {
		// we are on MS BTH stack
		DWORD bth;
		BthGetMode(&bth);
		state = (bth != BTH_POWER_OFF);
	}

	return state;
}

static const int CMD_BT_OFF = 0x1001;
static const int CMD_BT_ON = 0x1002;

void SetBluetoothState(BOOL enable) {
	HKEY hKey;
	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, REG_WIDCOMM_BRANCH, 0, NULL, &hKey) == ERROR_SUCCESS) {
		// we are on Widcomm BTH stack

		// use <ctacke/> hack
		HWND hBtWnd = FindWindow(_T("WCE_BTTRAY"), _T("Bluetooth Console"));
		if (hBtWnd != NULL) {
/*			// This is the whole CTACKE hack, but only WM_COMMAND works for me...
			// tap
			SendMessage(hBtWnd, WM_USER + 1, 0x1267, 0x201);
			SendMessage(hBtWnd, WM_USER + 1, 0x1267, 0x202);
			SendMessage(hBtWnd, WM_USER + 1, 0x1267, 0x200);
			Sleep(100);		// give it a time to create the menu
			HWND hWndMenu = FindWindow(_T("MNU"), _T(""));
			// enter menu loop
			SendMessage(hBtWnd, WM_ENTERMENULOOP, 0x01, 0x00);
			//
//			SendMessage(hBtWnd, WM_COMMAND, enable ? CMD_BT_OFF : CMD_BT_ON, 0x00);
			SendMessage(hBtWnd, WM_COMMAND, enable ? CMD_BT_ON : CMD_BT_OFF, 0x00);
			// end menu loop
			SendMessage(hBtWnd, WM_EXITMENULOOP, 0x01, 0x00);
			// some magic
			SendMessage(hBtWnd, WM_USER + ((enable) ? (UINT) 0xC00D : 0xC00C), 0x01, 0x00);
			// hide menu
			SendMessage(hWndMenu, WM_DESTROY, 0x00, 0x00);
			SendMessage(hWndMenu, WM_CANCELMODE, 0x00, 0x00);
*/
			PostMessage(hBtWnd, WM_COMMAND, enable ? CMD_BT_ON : CMD_BT_OFF, 0x00);
		}

		RegCloseKey(hKey);
	}
	else {
		// we are on MS BTH stack
		DWORD bth;
		if (enable)
			bth = BTH_DISCOVERABLE;
		else
			bth = BTH_POWER_OFF;
		BthSetMode(bth);
	}
}
