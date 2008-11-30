/**
 *  tman.cpp : Defines the entry point for the application.
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

#include "stdafx.h"
#include "resource.h"

#include "../share/defs.h"
#include "../share/regs.h"
#include "../share/Config.h"
#include "../share/Localization.h"
#include "../share/Logger.h"

#include <winioctl.h>
#include <bthutil.h>
#include <pm.h>
#include <ndispwr.h>
#include <ntddndis.h>

#define TMAN_WND_WD							28
#define TMAN_WND_TOP						2

#define TMAN_DOT_SIZE						8
#define CLOSE_BUTTON_SIZE					18

HINSTANCE HInstance = 0;
HWND HTaskBar = 0;
HWND HMain = 0;
HWND DesktopHWND = 0;

HANDLE CloseThread;

HICON HCloseIcon = 0;			// handle to the close icon
HFONT HBoldFont = 0;			// bold font for menu drawing
HFONT HButtonFont = 0;			// font for ok button

COLORREF ClrBg;					// color of background
COLORREF ClrBg2;				// color of background
COLORREF ClrFg;					// color of foreground
COLORREF ClrBtnBg;				// color of background
//COLORREF ClrBtnFrame;			// color of background
double BaseHue;					// base hue
double BaseSat;
double BaseLit;
COLORREF LastColorHilight;		// for detecting theme change

HICON HSwResetIcon = 0;			// handle to the sw. reset icon
HICON HDisplayOffIcon = 0;		// handle to the display off icon
HICON HPowerOffIcon = 0;		// handle to the power off icon
HICON HRotateLeftIcon = 0;		// handle to the rotate left icon
HICON HRotateRightIcon = 0;		// handle to the rotate right icon
HICON HSettingsIcon = 0;		// handle to the settings icon
HICON HWiFiOnIcon = 0;			// handle to the WiFi On icon
HICON HWiFiOffIcon = 0;			// handle to the WiFi Off icon
HICON HBluetoothOnIcon = 0;		// handle to the Bluetooth On icon
HICON HBluetoothOffIcon = 0;	// handle to the Bluetooth Off icon

int ScreenOrientation = -1;

BOOL OKPressed = FALSE;			// state of the OK/Close button
BOOL BTaskButt = FALSE;			// click starts in task button

UINT MinimizeTimer = 1;
UINT TapAndHoldTimer = 2;
UINT RedrawTimer = 3;

int BtnType;

POINT LastCursorPos;			// last position of "cursor"
DWORD PosBefore;
BOOL InvokedByHotKey = FALSE;

// type of displayed button (Ok/Close/None) on the task bar
enum BtnStyle {
	BTN_NONE,
	BTN_CLOSE,
	BTN_OK
};

// actions
enum ActionType {
	ACTION_TAP = 0,
	ACTION_TAP_AND_HOLD = 1,
	ACTION_GESTURE = 2
};

#define ID_PROGRAM_FIRST           0x8000
#define ID_PROGRAM_LAST            0x8FFF

//
BOOL CloseTask = FALSE;
BOOL TaskMenuOpened = FALSE;
BOOL ClosingTaskMenu = FALSE;
int HotKeyID = 0;

UINT ReadConfigMessage;

HDC MemDC;

// list of task that are being closed
#define MAX_TASKS_TO_CLOSE			128
HWND TasksToClose[MAX_TASKS_TO_CLOSE] = { 0 };
CRITICAL_SECTION CSTasksToClose;
//

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

#define MAX_TASK_COUNT						128
CProgramItem *TaskList[MAX_TASK_COUNT] = { 0 };
int TaskCount = 0;

// compact row
struct CCompactItem {
	UINT ID;					// menu ID
	HICON HIcon;				// handle to icon
};

#define MAX_COMPACT_ITEM					8
CCompactItem CompactRow[MAX_COMPACT_ITEM];
int CompactRowSize;				// number of items in compact row
int MenuWidth;					// width of the menu (necessary for compact row)

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

#define PLATFORM_WM2002					0
#define PLATFORM_WM2003					1
#define PLATFORM_WM5					2
#define PLATFORM_WM6					3

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


// ----------------------------------------------------------------------------
// appearance


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

//

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

// ----------------------------------------------------------------------------
// helpers

#define STRING_BUFFER_SIZE				1024

TCHAR *StringBuffer = NULL;   // buffer pro mnoho stringu
TCHAR *StrAct = StringBuffer;

TCHAR *LoadStr(int resID) {
	TCHAR *ret;

	if (Localization != NULL) {
		// first check for localized string
		ret = Localization->GetStr(resID);
		if (ret != NULL) return ret;
	}

	// load from resources
	if (!StringBuffer) ret = _T("ERROR LOADING STRING - INSUFFICIENT MEMORY");
	else {
		if (STRING_BUFFER_SIZE - (StrAct - StringBuffer) < 200) StrAct = StringBuffer;

	RELOAD:
		int size = LoadString(HInstance, resID, StrAct, STRING_BUFFER_SIZE - (StrAct - StringBuffer));
		// size obsahuje pocet nakopirovanych znaku bez terminatoru

		// error je NO_ERROR, i kdyz string neexistuje - nepouzitelne
		if (size != 0/* || error == NO_ERROR*/) {
			if (STRING_BUFFER_SIZE - (StrAct - StringBuffer) == size + 1 && StrAct > StringBuffer) {
				// pokud byl retezec presne na konci bufferu, mohlo
				// jit o oriznuti retezce -- pokud muzeme posunout okno
				// na zacatek bufferu, nacteme string jeste jednou
				StrAct = StringBuffer;
				goto RELOAD;
			}
			else {
				ret = StrAct;
				StrAct += size + 1;
			}
		}
		else {
			ret = _T("ERROR LOADING STRING");
		}
	}

	return ret;
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

void Line(HDC hDC, int x1, int y1, int x2, int y2) {
	POINT pts[2];
	pts[0].x = x1; pts[0].y = y1;
	pts[1].x = x2; pts[1].y = y2;
	Polyline(hDC, pts, 2);
}

//

BOOL AddTaskToCloseList(HWND hWnd) {
	EnterCriticalSection(&CSTasksToClose);
	BOOL found = FALSE;
	int firstFreeIdx = -1;
	for (int i = 0; i < MAX_TASKS_TO_CLOSE; i++) {
		if (TasksToClose[i] == NULL) {
			if (firstFreeIdx == -1)
				firstFreeIdx = i;
		}
		else {
			if (TasksToClose[i] == hWnd) {
				found = TRUE;
				break;
			}
		}
	}

	BOOL ret = FALSE;
	if (!found && firstFreeIdx != -1) {
		TasksToClose[firstFreeIdx] = hWnd;
		ret = TRUE;
	}
	LeaveCriticalSection(&CSTasksToClose);

	return ret;
}

void RemoveTaskFromCloseList(HWND hWnd) {
	EnterCriticalSection(&CSTasksToClose);
	for (int i = 0; i < MAX_TASKS_TO_CLOSE; i++) {
		if (TasksToClose[i] == hWnd)
			TasksToClose[i] = NULL;
	}
	LeaveCriticalSection(&CSTasksToClose);
}

//
//
//

BOOL InsertMenuTask(HMENU hMenu, int taskCount, LPCTSTR name, DWORD processID, HWND hWnd) {
	LOG0(5, "InsertMenuItem()");

	BOOL ignore = FALSE;

	HANDLE hProcess = OpenProcess(0, FALSE, processID);
	if (hProcess != NULL) {
		// Get program file path
		TCHAR lpFileName[MAX_PATH + 1];
		GetModuleFileName((HMODULE) hProcess, lpFileName, MAX_PATH);
		CloseHandle(hProcess);

		// look for an entry in ignore list
		for (int i = 0; i < Config.IgnoredAppsCount; i++) {
			if (wcsicmp(lpFileName, Config.IgnoredApps[i]) == 0) {
				ignore = TRUE;
				break;
			}
		}

		if (!ignore) {
			CProgramItem *pi = new CProgramItem(name, processID, hWnd);
				InsertMenu(hMenu, taskCount, MF_BYPOSITION | MF_ENABLED | MF_OWNERDRAW, ID_PROGRAM_FIRST + taskCount, (LPCTSTR) pi);
		}
	}

	return !ignore;
}

BOOL InsertMenuItem(HMENU hMenu, UINT nID, LPCTSTR name, HWND hWnd) {
	LOG0(5, "InsertMenuItem()");

	CProgramItem *pi = new CProgramItem(name, 0, hWnd);
	return InsertMenu(hMenu, 0xFFFFFFFF, MF_BYPOSITION | MF_ENABLED | MF_OWNERDRAW, nID, (LPCTSTR) pi);
}

BOOL InsertMenuSeparator(HMENU hMenu) {
	return InsertMenu(hMenu, 0xFFFFFFFF, MF_BYPOSITION | MF_GRAYED | MF_OWNERDRAW, -1, NULL);
}

BOOL InsertMenuItem(HMENU hMenu, UINT nID, LPCTSTR name, HICON hIcon = 0) {
	LOG0(5, "InsertMenuItem()");

	CProgramItem *pi = new CProgramItem(name, 0, 0, hIcon);
	return InsertMenu(hMenu, 0xFFFFFFFF, MF_BYPOSITION | MF_ENABLED | MF_OWNERDRAW, nID, (LPCTSTR) pi);
}

BOOL AddToCompactRow(int idx, UINT nID, HICON hIcon) {
	if (idx >= 0 && idx < MAX_COMPACT_ITEM) {
		CompactRow[idx].ID = nID;
		CompactRow[idx].HIcon = hIcon;
		return TRUE;
	}
	else
		return FALSE;
}

HWND GetTopParent(HWND hwnd) {
	HWND hParent = hwnd;
	HWND hTmp;
	while ((hTmp = GetParent(hParent)) != NULL)
		hParent = hTmp;

	return hParent;
}

BOOL IsTaskWindow(HWND hWnd) {
	TCHAR className[232] = { 0 };
	::GetClassName(hWnd, className, 231);

	// filter out some windows
	if (wcscmp(className, _T("DesktopExplorerWindow")) == 0 ||
		wcscmp(className, _T("MS_SIPBUTTON")) == 0 ||
		wcscmp(className, _T("FAKEIMEUI")) == 0 ||
		wcscmp(className, _T("static")) == 0 ||
		wcscmp(className, _T("MS_SOFTKEY_CE_1.0")) == 0 ||
		wcscmp(className, _T("tooltips_class32")) == 0 ||
		wcscmp(className, _T("OLEAUT32")) == 0 ||
		wcscmp(className, _T("Ime")) == 0 ||
		wcscmp(className, _T("Default Ime")) == 0 ||
		wcscmp(className, _T("menu_worker")) == 0)
		return FALSE;
	else
		return TRUE;
}

BOOL IsNotificationBubble(HWND hWnd) {
	DWORD dwStyle = GetWindowLong(hWnd, GWL_STYLE);
	DWORD dwExStyle = GetWindowLong(hWnd, GWL_EXSTYLE);
//	LONG wndProc = GetWindowLong(hWnd, GWL_WNDPROC);
//	LONG id = GetWindowLong(hWnd, GWL_ID);

	TCHAR lpWindowText[128] = { 0 };
	::GetWindowText(hWnd, lpWindowText, 127);

//	LOG4(1, "NB (%S): hWnd = 0x%X, style = 0x%X, proc = 0x%X", lpWindowText, hWnd, dwStyle, wndProc);
//	LOG6(1, "NB (%S): hWnd = 0x%X, style = 0x%X, styleEx = 0x%X, proc = 0x%X, id = 0x%X", lpWindowText, hWnd, dwStyle, dwExStyle, wndProc, id);

	if ((((dwStyle & 0x50000404) == 0x50000404) && ((dwExStyle & 0x88) == 0x88)) ||
		(((dwStyle & 0x80000000) == 0x80000000) && ((dwExStyle & 0x80) == 0x80)))
		return TRUE;
	else
		return FALSE;

}


void CloseNotificationBubble(HWND hWnd) {
	// TODO: close notification bubble
	// virtual click works, but where to click?
/*	mouse_event(MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_LEFTDOWN, 0xffff, 0, 0, 0);
	Sleep(250);
	mouse_event(MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_LEFTUP, 0xffff, 0, 0, 0);
*/
}

HWND GetUnderlyingWindow(HWND hWnd) {
	HWND hNext = ::GetWindow(hWnd, GW_HWNDNEXT);
	while (hNext != NULL) {
		if (hNext == DesktopHWND)
			return hNext;

		if (::IsWindow(hNext) && ::IsWindowVisible(hNext) && IsTaskWindow(hNext))
			return hNext;

		hNext = ::GetWindow(hNext, GW_HWNDNEXT);
	}

	return NULL;
}

BOOL CALLBACK TmanEnumWindowsProc(HWND hWnd, LPARAM lParam) {
	LOG2(5, "TmanEnumWindowsProc(0x%X, 0x%X)", hWnd, lParam);

	if (!::IsWindow(hWnd))
		return TRUE;

	if (!IsWindowVisible(hWnd))
		return TRUE;

	if (!IsTaskWindow(hWnd))
		return TRUE;

	// do not show notification bubbles in task menu
	if (IsNotificationBubble(hWnd))
		return TRUE;

	TCHAR lpWindowText[128] = { 0 };
	int len = ::GetWindowText(hWnd, lpWindowText, 127);
	if (len > 0) {
		HMENU hMenu = (HMENU) lParam;

		// search task list for duplicates
		BOOL exists = FALSE;

		int idx = 0;
		MENUITEMINFO mii = { 0 };
		mii.cbSize = sizeof(mii);
		mii.fMask = MIIM_DATA | MIIM_TYPE;
		GetMenuItemInfo(hMenu, ID_PROGRAM_FIRST + idx, FALSE, &mii);
		while (idx < TaskCount) {
			CProgramItem *pii = (CProgramItem *) mii.dwItemData;

			TCHAR taskName[128] = { 0 };
			::GetWindowText(GetTopParent(pii->HWnd), taskName, 127);

			if (wcscmp(taskName, lpWindowText) == 0) {
				exists = TRUE;
				break;
			}

			idx++;
			GetMenuItemInfo(hMenu, ID_PROGRAM_FIRST + idx, FALSE, &mii);
		}

		if (!exists) {
			HWND hParent = GetTopParent(hWnd);

			DWORD processID = 0;
			DWORD threadID = GetWindowThreadProcessId(hWnd, &processID);

			// use the window text of the parent window
			if (GetWindowText(hParent, lpWindowText, 127) > 0) {
				if (InsertMenuTask(hMenu, TaskCount, lpWindowText, processID, hWnd))
					TaskCount++;
			}
		}
	}

	return TRUE;
}

//
// @return		TRUE if the calling thread should wait for close, otherwise FALSE
//
BOOL Close(HWND hWnd) {
	LOG1(2, "Close(%x)", hWnd);

	BOOL ret = FALSE;

	DWORD processID;
	GetWindowThreadProcessId(hWnd, &processID);

	HANDLE hProcess = OpenProcess(0, FALSE, processID);
	if (hProcess != NULL) {
		TCHAR fileName[MAX_PATH];
		GetModuleFileName((HMODULE) hProcess, fileName, MAX_PATH);

		LONG wndProc = GetWindowLong(hWnd, GWL_WNDPROC);

		DWORD dwStyle = GetWindowLong(hWnd, GWL_STYLE);
		DWORD dwExStyle = GetWindowLong(hWnd, GWL_EXSTYLE);

		if (IsWindow(hWnd)) {
			TCHAR className[64];
			GetClassName(hWnd, className, 64);

			if (dwExStyle & WS_EX_CAPTIONOKBTN) {
				if ((DWORD) (wndProc & 0x00FFFFFF) == 0x3173B0) {
					// to close .NET dialogs with #NET_AGL_BASE_ class
//					LOG0(1, "WM_CLOSE - #NET_AGL_BASE_");
					LRESULT r = SendMessage(hWnd, WM_CLOSE, 0, 0);
					LOG1(1, "WM_CLOSE - %d", r);
				}
				else {
					if (SendMessage(hWnd, PSM_GETCURRENTPAGEHWND, 0, 0) != NULL)
						PropSheet_PressButton(hWnd, PSBTN_OK);

					SendMessage(hWnd, WM_COMMAND, 0x10000001, (LPARAM) 0);
				}

				if (wcsicmp(fileName, _T("\\windows\\cprog.exe")) == 0)
					SetWindowPos(hWnd, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_HIDEWINDOW);

				ret = TRUE;
			}
			else {
				if (wcsicmp(fileName, _T("\\windows\\shell32.exe")) == 0 ||
					wcsicmp(fileName, _T("\\windows\\cprog.exe")) == 0)
				{
					// do not kill the shell (shell32.exe) or phone (cprog.exe)
					SetWindowPos(hWnd, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_HIDEWINDOW);
					ret = FALSE;
				}
				else {
					if (wcscmp(className, _T("WMP for Mobile Devices")) == 0) {
//						LOG0(1, "WM_QUIT");
						LRESULT r = PostMessage(hWnd, WM_QUIT, 0, 0);		// windows media player needs WM_QUIT
						LOG1(1, "WM_QUIT - %d", r);
					}
					else {
//						LOG0(1, "WM_CLOSE");
						LRESULT r = SendMessage(hWnd, WM_CLOSE, 0, 0);
						LOG1(1, "WM_CLOSE2 - %d", r);
					}

					if (GetParent(hWnd) == NULL)
						ret = TRUE;
				}
			}
		}

		CloseHandle(hProcess);
	}

	return ret;
}

//
// Command handlers
//

DWORD WINAPI CloseThreadProc(LPVOID lpParam) {
	LOG0(2, "CloseThreadProc - start");

	HWND hWnd = (HWND) lpParam;

	DWORD processID;
	GetWindowThreadProcessId(hWnd, &processID);

	HANDLE hProcess = OpenProcess(0, FALSE, processID);
	TCHAR fileName[MAX_PATH] = { 0 };
	if (hProcess != NULL) {
		GetModuleFileName((HMODULE) hProcess, fileName, MAX_PATH);
		CloseHandle(hProcess);
	}

	if (wcsicmp(fileName, _T("\\windows\\shell32.exe")) == 0 ||
		wcsicmp(fileName, _T("\\windows\\cprog.exe")) == 0)
	{
		// special treatment with shell32.exe and cprog.exe
		Close(hWnd);
		RemoveTaskFromCloseList(hWnd);
		LOG0(2, "CloseThreadProc - end");
		return 0;
	}

	TCHAR lpWindowText[128] = { 0 };
	int len = GetWindowText(hWnd, lpWindowText, 127);

	::EnableWindow(hWnd, TRUE);
	if (Close(hWnd) && Config.ConfirmTerminate) {
/*		do {
			Sleep(Config.KillTimeout * 1000);
			if (IsWindow(hWnd) && IsWindowVisible(hWnd)) {
				TCHAR text[2048] = { 0 };
				swprintf(text, LoadStr(IDS_KILL_FORCLY), lpWindowText);
				TCHAR *caption = LoadStr(IDS_TMAN);

				int res = MessageBox(NULL, text, caption, MB_YESNOCANCEL | MB_SETFOREGROUND | MB_TOPMOST);
				if (res == IDYES) {
					DWORD pid;
					DWORD tid = GetWindowThreadProcessId(hWnd, &pid);
					if (pid != 0) {
						HANDLE process = OpenProcess(0, FALSE, pid);
						if (process != NULL) {
							TerminateProcess(process, 0);
							CloseHandle(process);
						}
					}
					break;
				}
				else if (res == IDCANCEL) {
					break;
				}
			}
			else
				break;
		} while (TRUE);
*/
	}

	RemoveTaskFromCloseList(hWnd);

	LOG0(2, "CloseThreadProc - end");

	return 0;
}

BOOL CALLBACK KillWindowsProc(HWND hWnd, LPARAM lParam) {
	LPCTSTR lpCloseWindowText = (LPCTSTR) lParam;

	TCHAR lpWindowText[128] = { 0 };
	GetWindowText(hWnd, lpWindowText, 127);

	if (wcscmp(lpWindowText, lpCloseWindowText) == 0) {
		Close(hWnd);
	}

	return TRUE;
}

DWORD WINAPI TerminateThreadProc(LPVOID lpParam) {
	LOG0(2, "TerminateThreadProc - start");

	HWND hWnd = (HWND) lpParam;

	DWORD processID;
	GetWindowThreadProcessId(hWnd, &processID);

	HANDLE hProcess = OpenProcess(0, FALSE, processID);
	TCHAR fileName[MAX_PATH] = { 0 };
	if (hProcess != NULL) {
		GetModuleFileName((HMODULE) hProcess, fileName, MAX_PATH);
		CloseHandle(hProcess);
	}

	if (wcsicmp(fileName, _T("\\windows\\shell32.exe")) == 0) {
		// special treatment with shell32.exe
		Close(hWnd);
		RemoveTaskFromCloseList(hWnd);
		LOG0(2, "TerminateThreadProc - end");
		return 0;
	}

	if (wcsicmp(fileName, _T("\\windows\\cprog.exe")) == 0) {
		// special treatment with cprog.exe
		Close(hWnd);
		RemoveTaskFromCloseList(hWnd);
		LOG0(2, "TerminateThreadProc - end");
		return 0;
	}

	//
	HWND hParent = GetTopParent(hWnd);
	BOOL bWaitClose = FALSE;

	TCHAR lpWindowText[128] = { 0 };
	GetWindowText(hParent, lpWindowText, 127);

	// kill all from a window to its parent
	HWND hTmp = hWnd;
	while (hTmp != NULL) {
		bWaitClose |= Close(hTmp);
		hTmp = GetParent(hTmp);
	}
	// kill all windows with the same caption
	EnumWindows(KillWindowsProc, (LPARAM) lpWindowText);

	if (bWaitClose && Config.ConfirmTerminate) {
/*		do {
			Sleep(Config.KillTimeout * 1000);
			if (IsWindow(hParent) && IsWindowVisible(hParent)) {
				TCHAR text[2048] = { 0 };
				swprintf(text, LoadStr(IDS_KILL_FORCLY), lpWindowText);
				TCHAR *caption = LoadStr(IDS_TMAN);

				int res = MessageBox(NULL, text, caption, MB_YESNOCANCEL | MB_SETFOREGROUND | MB_TOPMOST);
				if (res == IDYES) {
					DWORD pid;
					DWORD tid = GetWindowThreadProcessId(hWnd, &pid);
					if (pid != 0) {
						HANDLE process = OpenProcess(0, FALSE, pid);
						if (process != NULL) {
							TerminateProcess(process, 0);
							CloseHandle(process);
						}
					}
					break;
				}
				else if (res == IDCANCEL) {
					break;
				}
			}
			else
				break;
		} while (TRUE);
*/
	}

	RemoveTaskFromCloseList(hWnd);

	LOG0(2, "TerminateThreadProc - end");

	return 0;
}

void OnClose(HWND hWnd, BOOL terminate = FALSE) {
	LOG1(2, "OnClose(%x)", hWnd);

	if (IsNotificationBubble(hWnd)) {
		HWND hBubble = hWnd;
		hWnd = GetUnderlyingWindow(hWnd);
		CloseNotificationBubble(hBubble);
	}

	if (hWnd == DesktopHWND)
		return;

	if (AddTaskToCloseList(hWnd)) {
		HANDLE hThread;
		if (terminate)
			hThread = CreateThread(NULL, 0, TerminateThreadProc, hWnd, 0, NULL);
		else
			hThread = CreateThread(NULL, 0, CloseThreadProc, hWnd, 0, NULL);
		CloseHandle(hThread);
	}
}

void OnCloseAll() {
	LOG0(2, "OnCloseAll()");

	int firstIdx = Config.ShowDesktop ? 1 : 0;
	for (int i = firstIdx; i < TaskCount; i++) {
//		if (TaskList != NULL && TaskList[i] != NULL)
		if (TaskList[i] != NULL)
			Close(TaskList[i]->HWnd);
	}
}

void OnCloseAllButCurrent() {
	LOG0(2, "OnCloseAllButCurrent()");

	HWND hwndFore = GetForegroundWindow();
	int firstIdx = Config.ShowDesktop ? 1 : 0;
	for (int i = firstIdx; i < TaskCount; i++) {
//		if (TaskList != NULL && TaskList[i] != NULL)
		if (TaskList[i] != NULL)
			if (TaskList[i]->HWnd != hwndFore)
				Close(TaskList[i]->HWnd);
	}
}

void OnSettings() {
	LOG0(2, "OnSettings()");

	TCHAR cfg[MAX_PATH];
	swprintf(cfg, _T("%s\\tManCfg.exe"), Config.InstallDir);
	PROCESS_INFORMATION pi;
	CreateProcess(cfg, _T(""), NULL, NULL, FALSE, 0, NULL, NULL, NULL, &pi);
}

void OnMinimize(HWND hWnd) {
	LOG1(2, "OnMinimize(%d)", hWnd);

	if (IsNotificationBubble(hWnd)) {
		HWND hBubble = hWnd;
		hWnd = GetUnderlyingWindow(hWnd);
		CloseNotificationBubble(hBubble);
	}

	if (hWnd == DesktopHWND)
		return;

	ShowWindow(hWnd, SW_MINIMIZE);
}

void OnShowDesktop() {
	LOG0(2, "OnShowDesktop()");

	// minimize all windows until the desktop is on the top
	while (GetForegroundWindow() != DesktopHWND) {
		OnMinimize(GetForegroundWindow());
	}
}

void OnRotateScreenLeft() {
	LOG0(2, "OnRotateScreenLeft()");

	int orientation = GetScreenOrientation();
	if (orientation == DMDO_270) {
		// rotate the screen back to portrait mode
		SetScreenOrientation(DMDO_0);
	}
	else {
		// rotate the screen according to the config
		SetScreenOrientation(DMDO_270);
	}
}

void OnRotateScreenRight() {
	LOG0(2, "OnRotateScreenRight()");

	int orientation = GetScreenOrientation();
	if (orientation == DMDO_90) {
		// rotate the screen back to portrait mode
		SetScreenOrientation(DMDO_0);
	}
	else {
		// rotate the screen according to the config
		SetScreenOrientation(DMDO_90);
	}
}

void OnWiFi() {
	if (WiFiDeviceName != NULL) {
		if (IsWiFiOn())
			SetWiFiState(FALSE);
		else
			SetWiFiState(TRUE);
	}
}

void OnBluetooth() {
	if (IsBluetoothOn())
		SetBluetoothState(FALSE);
	else
		SetBluetoothState(TRUE);
}

// ShowTaskMenu
void ShowTaskMenu() {
	LOG0(2, "ShowTaskMenu()");

	int i;

	RECT rc;
	GetWindowRect(HTaskBar, &rc);

	HMENU hPopupMenu = CreatePopupMenu();

	// free previously allocated program data
	for (i = 0; i < TaskCount; i++)
		delete TaskList[i];
//	delete [] TaskList;

//	TaskList = NULL;
	TaskCount = 0;

	//
	if (Config.ShowDesktop) {
		InsertMenuItem(hPopupMenu, ID_PROGRAM_FIRST, LoadStr(IDS_DESKTOP), DesktopHWND);
		TaskCount = 1;
	}

	EnumWindows(TmanEnumWindowsProc, (LPARAM) hPopupMenu);
	// separator
	InsertMenuSeparator(hPopupMenu);

	if (Config.ShowTaskOperations) {
		InsertMenuItem(hPopupMenu, ID_MINIMIZE, LoadStr(IDS_MINIMIZE));
		InsertMenuItem(hPopupMenu, ID_CLOSE_ALL_BUT_CURRENT, LoadStr(IDS_CLOSE_BKGND_TASKS));
		InsertMenuItem(hPopupMenu, ID_CLOSE_ALL, LoadStr(IDS_CLOSE_ALL_TASKS));
		TaskCount += 3;
		InsertMenuSeparator(hPopupMenu);
	}

	int items = 0;
	if (Config.ShowDisplayOff) {
		if (Config.CompactMode)
			AddToCompactRow(items, ID_DISPLAY_OFF, HDisplayOffIcon);
		else
			InsertMenuItem(hPopupMenu, ID_DISPLAY_OFF, LoadStr(IDS_DISPLAY_OFF), HDisplayOffIcon);
		TaskCount++;
		items++;
	}
	if (Config.ShowSoftReset) {
		if (Config.CompactMode)
			AddToCompactRow(items, ID_SOFT_RESET, HSwResetIcon);
		else
			InsertMenuItem(hPopupMenu, ID_SOFT_RESET, LoadStr(IDS_SOFTWARE_RESET), HSwResetIcon);
		TaskCount++;
		items++;
	}

	int scrOri = GetScreenOrientation();
	if (Config.ShowLandscapeLeft) {
		if (Config.CompactMode)
			AddToCompactRow(items, ID_ROTATE_SCREEN_LEFT, HRotateLeftIcon);
		else {
			InsertMenuItem(hPopupMenu, ID_ROTATE_SCREEN_LEFT, LoadStr(IDS_LANDSCAPE_LEFT), HRotateLeftIcon);
			UINT uCheck = scrOri == DMDO_270 ? MF_CHECKED : MF_UNCHECKED;
			CheckMenuItem(hPopupMenu, ID_ROTATE_SCREEN_LEFT, MF_BYCOMMAND | uCheck);
		}
		TaskCount++;
		items++;
	}
	if (Config.ShowLandscapeRight) {
		if (Config.CompactMode)
			AddToCompactRow(items, ID_ROTATE_SCREEN_RIGHT, HRotateRightIcon);
		else {
			InsertMenuItem(hPopupMenu, ID_ROTATE_SCREEN_RIGHT, LoadStr(IDS_LANDSCAPE_RIGHT), HRotateRightIcon);
			UINT uCheck = scrOri == DMDO_90 ? MF_CHECKED : MF_UNCHECKED;
			CheckMenuItem(hPopupMenu, ID_ROTATE_SCREEN_RIGHT, MF_BYCOMMAND | uCheck);
		}
		TaskCount++;
		items++;
	}

	if (WiFiDeviceName != NULL && Config.ShowWiFi) {
//	if (TRUE) {
		if (Config.CompactMode)
			if (IsWiFiOn())
				AddToCompactRow(items, ID_WIFI, HWiFiOffIcon);
			else
				AddToCompactRow(items, ID_WIFI, HWiFiOnIcon);
		else {
			if (IsWiFiOn())
				InsertMenuItem(hPopupMenu, ID_WIFI, LoadStr(IDS_WIFI_OFF), HWiFiOffIcon);
			else
				InsertMenuItem(hPopupMenu, ID_WIFI, LoadStr(IDS_WIFI_ON), HWiFiOnIcon);
		}
		TaskCount++;
		items++;
	}

	if (Config.ShowBth) {
		if (Config.CompactMode)
			if (IsBluetoothOn())
				AddToCompactRow(items, ID_BLUETOOTH, HBluetoothOffIcon);
			else
				AddToCompactRow(items, ID_BLUETOOTH, HBluetoothOnIcon);
		else {
			if (IsBluetoothOn())
				InsertMenuItem(hPopupMenu, ID_BLUETOOTH, LoadStr(IDS_BLUETOOTH_OFF), HBluetoothOffIcon);
			else
				InsertMenuItem(hPopupMenu, ID_BLUETOOTH, LoadStr(IDS_BLUETOOTH_ON), HBluetoothOnIcon);
		}
		TaskCount++;
		items++;
	}

	if (Config.ShowPowerOff) {
		if (Config.CompactMode)
			AddToCompactRow(items, ID_POWER_OFF, HPowerOffIcon);
		else
			InsertMenuItem(hPopupMenu, ID_POWER_OFF, LoadStr(IDS_POWER_OFF), HPowerOffIcon);
		TaskCount++;
		items++;
	}


	if (items > 0) {
		if (Config.CompactMode) {
			// insert compact row
			InsertMenuItem(hPopupMenu, ID_COMPACT_ROW, _T(""));
			CompactRowSize = items;
		}

		// separator
		InsertMenuSeparator(hPopupMenu);
	}

	InsertMenuItem(hPopupMenu, ID_SETTINGS, LoadStr(IDS_SETTINGS), HSettingsIcon);
	InsertMenuItem(hPopupMenu, ID_EXIT, LoadStr(IDS_EXIT));
	TaskCount += 2;

	// create program list (for further usage)
//	TaskList = new CProgramItem *[TaskCount];
	i = 0;
	int menuItemIdx = 0;
	MENUITEMINFO mii = { 0 };
	mii.cbSize = sizeof(mii);
	mii.fMask = MIIM_DATA | MIIM_TYPE;
	while (GetMenuItemInfo(hPopupMenu, menuItemIdx, TRUE, &mii)) {
		if (mii.dwItemData != NULL) {
			TaskList[i] = (CProgramItem *) mii.dwItemData;
			i++;
		}
		menuItemIdx++;
	}

	PosBefore = ::GetMessagePos();

	// set menu position
	WPARAM nID = TrackPopupMenu(hPopupMenu, TPM_RETURNCMD | TPM_RIGHTALIGN | TPM_TOPALIGN, rc.right, rc.bottom, 0, HMain, NULL);
	if (nID == 0) {
		TaskMenuOpened = FALSE;
		BTaskButt = FALSE;
		ClosingTaskMenu = TRUE;
	}
	else {
		// decide if the task will be closed or not
		DWORD pos = ::GetMessagePos();

		RECT rc;
		GetWindowRect(HTaskBar, &rc);

		POINT ptMouse;
		ptMouse.x = LOWORD(pos);
		ptMouse.y = HIWORD(pos);

		if (nID == ID_COMPACT_ROW) {
			if (ptMouse.x == LOWORD(PosBefore) && ptMouse.y == HIWORD(PosBefore)) {
				// activated by key
			}
			else if (ptMouse.x >= rc.right - MenuWidth) {
				int idx = (ptMouse.x - rc.right + MenuWidth) / SCALEX(20);
				// change nID to the correct command associated to the item of compact row
				if (idx >= 0 && idx < CompactRowSize)
					PostMessage(HMain, WM_COMMAND, CompactRow[idx].ID, 0);
			}
		}
		else {
			if (ptMouse.x == LOWORD(PosBefore) && ptMouse.y == HIWORD(PosBefore))
				// activated by key
				CloseTask = FALSE;
			else {
				if (ptMouse.x >= rc.right - SCALEX(CLOSE_BUTTON_SIZE) && ptMouse.x <= rc.right)
					CloseTask = TRUE;
				else
					CloseTask = FALSE;
			}
			PostMessage(HMain, WM_COMMAND, nID, 0);
		}
	}

	DestroyMenu(hPopupMenu);
}

void OnShowTaskMenu() {
	LOG0(2, "OnShowTaskMenu()");

	TaskMenuOpened = TRUE;
	ShowTaskMenu();
}


int GetCloseButtonType(HWND hWnd) {
	LOG0(3, "GetCloseButtonType()");

//	HWND hFore = GetForegroundWindow();
	DWORD dwStyle = GetWindowLong(hWnd, GWL_STYLE);
//	LONG wndProc = GetWindowLong(hFore, GWL_WNDPROC);

//	LOG2(1, "BS: hWnd: %X, WndProc: %lX", hFore, wndProc);

	if (IsNotificationBubble(hWnd))
		hWnd = GetUnderlyingWindow(hWnd);

	if (hWnd == DesktopHWND)
		return BTN_NONE;

	if (hWnd != NULL) {
		DWORD dwExStyle = GetWindowLong(hWnd, GWL_EXSTYLE);
		if ((dwExStyle & WS_EX_CAPTIONOKBTN))
			return BTN_OK;									// draw 'ok'
		else if ((dwStyle & WS_MINIMIZEBOX) || (dwStyle & WS_CAPTION))
			return BTN_NONE;
		else
			return BTN_CLOSE;								// draw closing 'x'
	}
	else
		return BTN_NONE;
}

void DoAction(ActionType actionType, HWND hWnd) {
	LOG2(3, "DoAction(%d, %d)", actionType, hWnd);

	int tapAction = Config.TapAction;
	int gestureAction = Config.GestureAction;
	int tapAndHoldAction = Config.TapAndHoldAction;

	// check for custom behavior (user defined exceptions)
	DWORD processID;
	GetWindowThreadProcessId(hWnd, &processID);

	HANDLE hProcess = OpenProcess(0, FALSE, processID);
	if (hProcess != NULL) {
		// Get program file path
		TCHAR lpFileName[MAX_PATH + 1];
		GetModuleFileName((HMODULE) hProcess, lpFileName, MAX_PATH);
		CloseHandle(hProcess);

		// look for an entry in exception list
		for (int i = 0; i < Config.ExceptionsCount; i++) {
			CConfig::CAppException *ex = Config.Exceptions[i];
			if (wcsicmp(lpFileName, ex->ExeName) == 0) {
				tapAction = ex->TapAction;
				gestureAction = ex->GestureAction;
				break;
			}
		}
	}

	int action = tapAction;
	switch (actionType) {
		case ACTION_TAP:          action = tapAction; break;
		case ACTION_TAP_AND_HOLD: action = tapAndHoldAction; break;
		case ACTION_GESTURE:      action = gestureAction; break;
	}

	int btnType = GetCloseButtonType(hWnd);
	switch (action) {
		case BUTTON_ACTION_CLOSE:
			OnClose(hWnd);
			break;

		case BUTTON_ACTION_SHOW_MENU:
			OnShowTaskMenu();
			break;

		case BUTTON_ACTION_MINIMIZE:
			if (btnType == BTN_OK)
				OnClose(hWnd);
			else
				OnMinimize(hWnd);
			break;
	}
}

void OnSoftReset() {
	TCHAR *txt = LoadStr(IDS_SOFT_RESET);
	TCHAR *title = LoadStr(IDS_TMAN);

	if (!Config.ConfirmSoftReset || MessageBox(NULL, txt, title, MB_YESNO | MB_ICONEXCLAMATION | MB_SETFOREGROUND) == IDYES) {
		ResetPocketPC();
	}
}

// ----------------------------------------------------------------------------
// window message handlers

void OnCreate() {
	LOG0(2, "OnCreate()");

	// close icon
	HCloseIcon = (HICON) LoadImage(HInstance, MAKEINTRESOURCE(IDI_CLOSE), IMAGE_ICON, SCALEX(16), SCALEY(16), 0);
	HDisplayOffIcon = (HICON) LoadImage(HInstance, MAKEINTRESOURCE(IDI_SCREENOFF), IMAGE_ICON, SCALEX(16), SCALEY(16), 0);
	HSwResetIcon = (HICON) LoadImage(HInstance, MAKEINTRESOURCE(IDI_SWRESET), IMAGE_ICON, SCALEX(16), SCALEY(16), 0);
	HPowerOffIcon = (HICON) LoadImage(HInstance, MAKEINTRESOURCE(IDI_POWEROFF), IMAGE_ICON, SCALEX(16), SCALEY(16), 0);
	HRotateLeftIcon = (HICON) LoadImage(HInstance, MAKEINTRESOURCE(IDI_ROT_LEFT), IMAGE_ICON, SCALEX(16), SCALEY(16), 0);
	HRotateRightIcon = (HICON) LoadImage(HInstance, MAKEINTRESOURCE(IDI_ROT_RIGHT), IMAGE_ICON, SCALEX(16), SCALEY(16), 0);
	HSettingsIcon = (HICON) LoadImage(HInstance, MAKEINTRESOURCE(IDI_SETTINGS), IMAGE_ICON, SCALEX(16), SCALEY(16), 0);

	HWiFiOnIcon = (HICON) LoadImage(HInstance, MAKEINTRESOURCE(IDI_WIFI_ON), IMAGE_ICON, SCALEX(16), SCALEY(16), 0);
	HWiFiOffIcon = (HICON) LoadImage(HInstance, MAKEINTRESOURCE(IDI_WIFI_OFF), IMAGE_ICON, SCALEX(16), SCALEY(16), 0);
	HBluetoothOnIcon = (HICON) LoadImage(HInstance, MAKEINTRESOURCE(IDI_BLUETOOTH_ON), IMAGE_ICON, SCALEX(16), SCALEY(16), 0);
	HBluetoothOffIcon = (HICON) LoadImage(HInstance, MAKEINTRESOURCE(IDI_BLUETOOTH_OFF), IMAGE_ICON, SCALEX(16), SCALEY(16), 0);

	LoadFonts();
	LoadThemeColors();
	LastColorHilight = GetSysColor(COLOR_HIGHLIGHT);
}

void OnDestroy() {
	LOG0(2, "OnDestroy()");

	DestroyIcon(HCloseIcon);
	DestroyIcon(HDisplayOffIcon);
	DestroyIcon(HSwResetIcon);
	DestroyIcon(HPowerOffIcon);
	DestroyIcon(HRotateLeftIcon);
	DestroyIcon(HRotateRightIcon);
	DestroyIcon(HSettingsIcon);

	DeleteObject(HBoldFont);
	DeleteObject(HButtonFont);
}

void OnMeasureItem(UINT idCtl, LPMEASUREITEMSTRUCT lpmis) {
	LOG0(5, "OnMeasureItem()");

	CProgramItem *pi = (CProgramItem *) lpmis->itemData;
	if (pi != NULL) {
		HDC hDC = GetDC(NULL);
		RECT rcText = { 0 };
		DrawText(hDC, pi->Name, lstrlen(pi->Name), &rcText, DT_LEFT | DT_TOP | DT_CALCRECT);
		ReleaseDC(NULL, hDC);

		lpmis->itemWidth = rcText.right - rcText.left + SCALEX(16) + SCALEX(30);
		lpmis->itemHeight = SCALEY(18);
	}
	else {
		// separator
		lpmis->itemWidth = SCALEX(0);
		lpmis->itemHeight = SCALEY(8);
	}

	if (lpmis->itemWidth < (UINT) SCALEX(160))
		lpmis->itemWidth = SCALEX(160);
}

void OnDrawItem(UINT idCtl, LPDRAWITEMSTRUCT lpdis) {
	LOG0(5, "OnDrawItem()");

	//
	HDC hDC = lpdis->hDC;
	int saveDC = SaveDC(lpdis->hDC);

	COLORREF clrFg, clrBg;

	if (lpdis->itemState == ODS_SELECTED) {
		clrBg = Config.ClrMenuHiBg;
		clrFg = Config.ClrMenuHiFg;
	}
	else {
		clrBg = Config.ClrMenuBg;
		clrFg = Config.ClrMenuFg;
	}

	// background
	RECT rcItem = lpdis->rcItem;
	HBRUSH brBkgnd = CreateSolidBrush(clrBg);
	HGDIOBJ hOrigBrush = SelectObject(hDC, brBkgnd);
//	SelectObject(hDC, brBkgnd);

	MenuWidth = rcItem.right - rcItem.left;

	HPEN penBkgnd = CreatePen(PS_NULL, 0, clrBg);
	HGDIOBJ hOrigPen = SelectObject(hDC, penBkgnd);
//	SelectObject(hDC, penBkgnd);
	Rectangle(hDC, rcItem.left, rcItem.top, rcItem.right, rcItem.bottom);

	SelectObject(hDC, hOrigPen);
	SelectObject(hDC, hOrigBrush);

	if (lpdis->itemID == ID_COMPACT_ROW) {
		// compact row
		int left = rcItem.left + SCALEX(2);
		for (int i = 0; i < CompactRowSize; i++) {
			DrawIconEx(hDC, left, rcItem.top + SCALEY(1), CompactRow[i].HIcon, SCALEX(16), SCALEY(16), 0, NULL, DI_NORMAL);
			left += SCALEX(20);
		}
	}
	else {
		CProgramItem *pi = (CProgramItem *) lpdis->itemData;

		if (pi == NULL) {
			// separator
			HPEN penSep = CreatePen(PS_SOLID, SCALEX(1), clrFg);
			HGDIOBJ hOrigPen = SelectObject(hDC, penSep);

			int middle = (rcItem.top + rcItem.bottom) / 2;
			POINT pts[] = {
				{ rcItem.left + SCALEX(6), middle },
				{ rcItem.right - SCALEX(6), middle }
			};
			Polyline(hDC, pts, 2);

			SelectObject(hDC, hOrigPen);
			DeleteObject(penSep);
		}
		else {
			if (pi->ProcessId == 0) {
				if (lpdis->itemState & ODS_CHECKED) {
					HPEN penSep = CreatePen(PS_SOLID, 1, clrFg);
					HGDIOBJ hOrigPen = SelectObject(hDC, penSep);

					if (GetDeviceCaps(hDC, LOGPIXELSX) < 130) {
						for (int i = 0; i < SCALEY(3); i++) {
							POINT pts[] = {
								{ rcItem.left + SCALEX( 8), rcItem.top + SCALEY( 8) + i },
								{ rcItem.left + SCALEX(10), rcItem.top + SCALEY(10) + i },
								{ rcItem.left + SCALEX(15), rcItem.top + SCALEY( 5) + i }
							};
							Polyline(hDC, pts, 3);
						}
					}
					else {
						for (int i = 0; i < SCALEY(3) + 1; i++) {
							POINT pts[] = {
								{ rcItem.left + SCALEX( 8),     rcItem.top + SCALEY( 8) + i },
								{ rcItem.left + SCALEX(10) + 1, rcItem.top + SCALEY(10) + 1 + i },
								{ rcItem.left + SCALEX(15) + 1, rcItem.top + SCALEY( 5) + 1 + i }
							};
							Polyline(hDC, pts, 3);
						}
					}

					SelectObject(hDC, hOrigPen);
					DeleteObject(penSep);
				}
			}
			else {
				// icon for progs
				HANDLE hProcess = OpenProcess(0, FALSE, pi->ProcessId);
				if (hProcess != NULL) {
					TCHAR pathFileName[MAX_PATH];

					HWND hParent = GetTopParent(pi->HWnd);
					TCHAR className[64];
					GetClassName(hParent, className, 64);
					if (wcscmp(className, _T("Calendar")) == 0) {
						wcscpy(pathFileName, _T("\\windows\\calendar.lnk"));
					}
					else if (wcscmp(className, _T("Tasks Application")) == 0) {
						wcscpy(pathFileName, _T("\\windows\\tasks.lnk"));
					}
					else if (wcscmp(className, _T("Contacts")) == 0 ||
						wcscmp(className, _T("MSContactsUI")) == 0)			// WM5
					{
						wcscpy(pathFileName, _T("\\windows\\addrbook.lnk"));
					}
					else {
						GetModuleFileName((HMODULE) hProcess, pathFileName, MAX_PATH);
						// find application
						if (wcsicmp(pathFileName, _T("\\windows\\shfind.exe")) == 0) {
							if (GetPlatform() == PLATFORM_WM5 || GetPlatform() == PLATFORM_WM6)
								wcscpy(pathFileName, _T("\\windows\\search.lnk"));
							else
								wcscpy(pathFileName, _T("\\windows\\find.lnk"));
						}
						// pictures & images
						else if (wcsicmp(pathFileName, _T("\\windows\\pimg.exe")) == 0) {
							if (GetPlatform() == PLATFORM_WM6)
								wcscpy(pathFileName, _T("\\windows\\pimg.lnk"));
						}
						// internet sharing
						else if (wcsicmp(pathFileName, _T("\\windows\\intshrui.exe")) == 0) {
							if (GetPlatform() == PLATFORM_WM6)
								wcscpy(pathFileName, _T("\\windows\\intshrui.lnk"));
						}
						// Phone
						else if (wcsicmp(pathFileName, _T("\\windows\\cprog.exe")) == 0) {
							if (GetPlatform() == PLATFORM_WM5 || GetPlatform() == PLATFORM_WM6)
								wcscpy(pathFileName, _T("\\windows\\cprog.lnk"));
						}
						// modem link
						else if (wcsicmp(pathFileName, _T("\\windows\\atciui.exe")) == 0) {
							if (GetPlatform() == PLATFORM_WM5 || GetPlatform() == PLATFORM_WM6)
								wcscpy(pathFileName, _T("\\windows\\atciui.lnk"));
						}
					}

					SHFILEINFO sfi = { 0 };
					DWORD imageList;
					imageList = SHGetFileInfo(pathFileName, 0, &sfi, sizeof(sfi), SHGFI_ICON | SHGFI_SYSICONINDEX | SHGFI_SMALLICON);

					if (imageList != 0) {
						ImageList_DrawEx((HIMAGELIST) imageList, sfi.iIcon, hDC,
							rcItem.left + SCALEX(2), rcItem.top + SCALEY(1), SCALEX(16), SCALEY(16), CLR_NONE, CLR_NONE, ILD_NORMAL);
					}

					CloseHandle(hProcess);
				}
			}

			// font
			HGDIOBJ hOrigFont = SelectObject(hDC, HBoldFont);
		//	SelectObject(hDC, HBoldFont);
			RECT rcText = rcItem;
			rcText.left += SCALEX(20);
			rcText.right -= SCALEX(20);
			SetTextColor(hDC, clrFg);
			int oldMode = SetBkMode(hDC, TRANSPARENT);

			UINT uFmt = DT_LEFT | DT_VCENTER | DT_SINGLELINE;
			if (pi->ProcessId != 0)
				uFmt |= DT_NOPREFIX;		// we don not want to interpret & as mark of hot key

			DrawTextEndEllipsis(hDC, pi->Name, lstrlen(pi->Name), &rcText, uFmt);
			SetBkMode(hDC, oldMode);
			SelectObject(hDC, hOrigFont);

			// close button for all windows except for the desktop
			if (pi->ProcessId != 0) {
				// close button
				DrawIconEx(hDC, rcItem.right - SCALEX(CLOSE_BUTTON_SIZE), rcItem.top + SCALEY(1), HCloseIcon, SCALEX(16), SCALEY(16), 0, NULL, DI_NORMAL);
			}

			if (pi->HIcon != NULL) {
				// icon of the item
				DrawIconEx(hDC, rcItem.left + SCALEX(2), rcItem.top + SCALEY(1), pi->HIcon, SCALEX(16), SCALEY(16), 0, NULL, DI_NORMAL);
			}
		}
	}

	DeleteObject(brBkgnd);
	DeleteObject(penBkgnd);

	RestoreDC(hDC, saveDC);
}

/////

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

//
// OnPaint
//
void OnPaint(HWND hWnd) {
	PAINTSTRUCT ps;
	HDC hDC = BeginPaint(hWnd, &ps);

	if (GetSysColor(COLOR_HIGHLIGHT) != LastColorHilight) {
		LoadThemeColors();
		LastColorHilight = GetSysColor(COLOR_HIGHLIGHT);
	}

	BOOL bLoDPI = GetDeviceCaps(hDC, LOGPIXELSX) < 130;

	int saveDC = SaveDC(MemDC);

	RECT rc;
	GetClientRect(hWnd, &rc);

	// background
	HPEN pen = CreatePen(PS_NULL, 0, RGB(0, 0, 0));
	HGDIOBJ hOrigPen = SelectObject(MemDC, pen);

	if (Config.OldStyleBkgnd) {
		DrawBkgndWM2003(MemDC, rc);
	}
	else {
		switch (GetPlatform()) {
			case PLATFORM_WM6: DrawBkgndWM6(MemDC, rc); break;
			case PLATFORM_WM5: DrawBkgndWM5(MemDC, rc); break;
			case PLATFORM_WM2003: DrawBkgndWM2003(MemDC, rc); break;
			default: DrawBkgndWM2002(MemDC, rc); break;
		}
	}


	RECT rcBtn = rc;
	// draw point for task menu
	if (Config.ShowDot) {
		RECT rcCirc;
		rcCirc.top = rc.top + SCALEY(7);
		rcCirc.left = rc.left;
		if (bLoDPI) {
			rcCirc.right = rcCirc.left + SCALEX(TMAN_DOT_SIZE);
			rcCirc.bottom = rcCirc.top + SCALEX(TMAN_DOT_SIZE);
		}
		else {
			rcCirc.left += SCALEX(1);
			rcCirc.right = rcCirc.left + SCALEX(TMAN_DOT_SIZE - 1);
			rcCirc.bottom = rcCirc.top + SCALEX(TMAN_DOT_SIZE - 1);
		}

		HBRUSH brFg = CreateSolidBrush(ClrFg);
		HGDIOBJ hOrigBrush = SelectObject(MemDC, brFg);
		Ellipse(MemDC, rcCirc.left, rcCirc.top, rcCirc.right, rcCirc.bottom);
		SelectObject(MemDC, hOrigBrush);
		DeleteObject(brFg);
	}

	SelectObject(MemDC, hOrigPen);
	DeleteObject(pen);

	// Ok/Close button
	int type = GetCloseButtonType(GetForegroundWindow());
	if (type != BTN_NONE) {
		if (Config.OldStyleButton)
			DrawButtonWM2003(type, MemDC, rcBtn, bLoDPI);
		else {
			switch (GetPlatform()) {
				case PLATFORM_WM6: DrawButtonWM6(type, MemDC, rcBtn, bLoDPI); break;
				case PLATFORM_WM5: DrawButtonWM5(type, MemDC, rcBtn, bLoDPI); break;
				default: DrawButtonWM2003(type, MemDC, rcBtn, bLoDPI); break;
			}
		}
	}

	RestoreDC(MemDC, saveDC);

	// copy to screen
	BitBlt(hDC, 0, 0, rc.right - rc.left, rc.bottom - rc.top, MemDC, 0, 0, SRCCOPY);

	EndPaint(hWnd, &ps);
}

LRESULT OnLButtonDown(HWND hWnd, WORD xPos, WORD yPos, WORD fwKeys) {
	LOG4(5, "OnLButtonDown(%d, %d, %d, %d)", hWnd, xPos, yPos, fwKeys);

	if (TaskMenuOpened) {
//		ReleaseCapture();
		SendMessage(HMain, WM_CANCELMODE, 0, 0);
//		TaskMenuOpened = FALSE;
		BTaskButt = FALSE;
		return 0;
	}

	ClosingTaskMenu = FALSE;

	RECT rc;
	GetClientRect(hWnd, &rc);
	rc.left = rc.right - SCALEX(TMAN_WND_WD);

	int left = rc.left;
	if (Config.ShowDot) left += SCALEX(TMAN_DOT_SIZE);

	SetCapture(HMain);

	if (xPos > left  && xPos <= rc.right && yPos >= rc.top && yPos <= rc.bottom) {
		KillTimer(HMain, TapAndHoldTimer);	// for sure
		SetTimer(HMain, TapAndHoldTimer, 100, NULL);
	}

	if (Config.ShowDot && xPos >= rc.left && xPos <= left && yPos >= rc.top && yPos <= rc.bottom) {
		BTaskButt = TRUE;
	}
	else {
		// redraw button
		OKPressed = TRUE;
		InvalidateRect(HMain, NULL, FALSE);
		BTaskButt = FALSE;
		KillTimer(HMain, MinimizeTimer);
		SetTimer(HMain, MinimizeTimer, 200, NULL);
	}

	LastCursorPos.x = xPos;
	LastCursorPos.y = yPos;

	return 0;
}

LRESULT OnMouseMove(HWND hWnd, WORD xPos, WORD yPos, WORD fwKeys) {
	LOG4(5, "OnMouseMove(%d, %d, %d, %d)", hWnd, xPos, yPos, fwKeys);

	RECT rc;
	GetClientRect(hWnd, &rc);
	rc.left = rc.right - SCALEX(TMAN_WND_WD);

	if (!BTaskButt &&
		xPos > rc.left + SCALEX(TMAN_DOT_SIZE) && xPos <= rc.right)
	{
	}
	else {
		ReleaseCapture();
		OKPressed = FALSE;
		InvalidateRect(HMain, NULL, FALSE);
		KillTimer(HMain, MinimizeTimer);
	}

	if (abs(LastCursorPos.x - xPos) > SCALEX(3) ||
		abs(LastCursorPos.y - yPos) > SCALEX(3))
	{
		KillTimer(HMain, TapAndHoldTimer);
	}

	return 0;
}

LRESULT OnLButtonUp(HWND hWnd, WORD xPos, WORD yPos, WORD fwKeys) {
	LOG4(5, "OnLButtonUp(%d, %d, %d, %d)", hWnd, xPos, yPos, fwKeys);

	OKPressed = FALSE;
	InvalidateRect(HMain, NULL, FALSE);

	KillTimer(HMain, MinimizeTimer);
	KillTimer(HMain, TapAndHoldTimer);

	ReleaseCapture();

	if (TaskMenuOpened) {
		SendMessage(HMain, WM_CANCELMODE, 0, 0);
		TaskMenuOpened = FALSE;
		BTaskButt = FALSE;
		return 0;
	}

	if (ClosingTaskMenu) {
		ClosingTaskMenu = FALSE;
		return 0;
	}

	POINT pt = { xPos, yPos };

	RECT rc;
	GetClientRect(hWnd, &rc);
	rc.left = rc.right - SCALEX(TMAN_WND_WD);

	// task button
	if (BTaskButt) {
		if (xPos >= rc.left && xPos <= rc.left + SCALEX(TMAN_DOT_SIZE) &&
			yPos >= rc.top && yPos <= rc.bottom)
		{
			// show tasks
			OnShowTaskMenu();
			return 0;
		}
	}
	else {
		int left = rc.left;
		if (Config.ShowDot)
			left += SCALEX(TMAN_DOT_SIZE);

		// the mouse action ends in the ok/close button area
		if (xPos > left && xPos <= rc.right && yPos >= rc.top && yPos <= rc.bottom) {
			DoAction(ACTION_TAP, GetForegroundWindow());
			return 0;
		}
		// recognize gesture (the cursor must ends minimaly 5 pixels under the bottom of top bar)
		else if (xPos > left && xPos <= rc.right && yPos >= rc.bottom + SCALEY(5)) {
			// tap and hold
			DoAction(ACTION_GESTURE, GetForegroundWindow());
			return 0;
		}
	}

	return 0;
}

void OnTimer(UINT eventID) {
	LOG1(5, "OnTimer(%d)", eventID);

//	static internalTick = 0;

	if (eventID == MinimizeTimer) {
		OKPressed = FALSE;
		KillTimer(HMain, MinimizeTimer);

		InvalidateRect(HMain, NULL, FALSE);
	}
	else if (eventID == TapAndHoldTimer) {
		KillTimer(HMain, TapAndHoldTimer);

		ReleaseCapture();

		// tap and hold
		SHRGINFO shrg;
		shrg.cbSize = sizeof(shrg);
		shrg.hwndClient = HMain;
		shrg.ptDown.x = LastCursorPos.x;
		shrg.ptDown.y = LastCursorPos.y;
		shrg.dwFlags = SHRG_RETURNCMD;

		if (::SHRecognizeGesture(&shrg) == GN_CONTEXTMENU) {
			InvalidateRect(HMain, NULL, FALSE);
			UpdateWindow(HMain);
			DoAction(ACTION_TAP_AND_HOLD, GetForegroundWindow());
		}
	}
	else if (eventID == RedrawTimer) {
		// it can happen that a window may change the style
		HWND hForeWnd = GetForegroundWindow();
		int btnType = GetCloseButtonType(hForeWnd);
		if (btnType != BtnType) {
			BtnType = btnType;
			InvalidateRect(HMain, NULL, FALSE);
		}

		// get taskbar size and reposition the window
		if (IsWindow(HMain)) {
			// update window size (to reflect landscape/portrait mode)
			HWND hParent = GetParent(HMain);
			if (hParent != NULL) {
				RECT rcTaskbar = { 0 };
				GetClientRect(hParent, &rcTaskbar);

				RECT rcMain = { 0 };
				GetWindowRect(HMain, &rcMain);

				if (rcMain.left != rcTaskbar.right - SCALEX(TMAN_WND_WD) ||
					rcMain.top != rcTaskbar.top + SCALEY(TMAN_WND_TOP))
				{
					SetWindowPos(HMain, NULL,
						rcTaskbar.right - SCALEX(TMAN_WND_WD), rcTaskbar.top + SCALEY(TMAN_WND_TOP),
						SCALEX(TMAN_WND_WD), rcTaskbar.bottom - rcTaskbar.top - (2 * SCALEX(TMAN_WND_TOP)),
						SWP_NOZORDER);
					InvalidateRect(HMain, NULL, FALSE);
				}
			}
		}
	}
}

void OnReadConfig() {
	LOG0(2, "OnReadConfig()");

	int prevHWKey = Config.TaskListHWKey;

	Config.Load();

	if (prevHWKey != Config.TaskListHWKey) {
		// change hot key registration
		if (prevHWKey != 0)
			UnregisterHotKey(HMain, HotKeyID);

		if (Config.TaskListHWKey != 0)
			RegisterHotKey(HMain, HotKeyID, MOD_WIN, Config.TaskListHWKey);
	}
}

LRESULT OnHotKey(HWND hWnd, WPARAM wParam, LPARAM lParam) {
	LOG3(3, "OnHotKey(%d, %d, %d)", hWnd, wParam, lParam);

	int idHotKey = (int) wParam;
	UINT fuModifiers = (UINT) LOWORD(lParam);
	UINT uVirtKey = (UINT) HIWORD(lParam);

	if (TaskMenuOpened) {
		ReleaseCapture();
		SendMessage(HMain, WM_CANCELMODE, 0, 0);

		// close Opened Task Menu
		TaskMenuOpened = FALSE;
		BTaskButt = FALSE;
	}
	else {
		OnShowTaskMenu();
	}

	return 0;
}


void OnCommand(UINT nID) {
	LOG0(2, "OnCommand()");
	// command received - this means user has selected a menu item
	UINT first;
	if (Config.ShowDesktop)
		first = ID_PROGRAM_FIRST + 1;
	else
		first = ID_PROGRAM_FIRST;

	if (nID >= first && nID <= ID_PROGRAM_LAST) {
		int taskIdx = nID - ID_PROGRAM_FIRST;

//		if (TaskList != NULL && TaskList[taskIdx] != NULL) {
		if (TaskList[taskIdx] != NULL) {
			CProgramItem *pi = TaskList[taskIdx];

			if (CloseTask)
				// terminate application
				OnClose(pi->HWnd, TRUE);
			else {
				SetForegroundWindow(pi->HWnd); // switch to process
			}
		}
	}
	else {
		switch (nID) {
			case ID_PROGRAM_FIRST:			OnShowDesktop(); break;
			case ID_MINIMIZE:				OnMinimize(GetForegroundWindow()); break;
			case ID_CLOSE_ALL_BUT_CURRENT:	OnCloseAllButCurrent(); break;
			case ID_CLOSE_ALL:				OnCloseAll(); break;
			case ID_DISPLAY_OFF:			DisplayOff(); break;
			case ID_SOFT_RESET:				OnSoftReset(); break;
			case ID_ROTATE_SCREEN_LEFT:		OnRotateScreenLeft(); break;
			case ID_ROTATE_SCREEN_RIGHT:	OnRotateScreenRight(); break;
			case ID_POWER_OFF:				PowerOff(); break;
			case ID_WIFI:					OnWiFi(); break;
			case ID_BLUETOOTH:				OnBluetooth(); break;
			case ID_SETTINGS:				OnSettings(); break;
			case ID_EXIT:					PostQuitMessage(0); break;
		}
	}

	BTaskButt = FALSE;
	TaskMenuOpened = FALSE;
}

//
// MainWndProc
//
LRESULT CALLBACK MainWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	if (msg == ReadConfigMessage) {
		OnReadConfig();
		return 0;
	}
	else {
		switch (msg) {
			case WM_CREATE:
				OnCreate();
				break;

			case WM_DESTROY:
				OnDestroy();
				break;

			case WM_ERASEBKGND:
				return 0;

			case WM_TIMER:
				OnTimer(wParam);
				return 0;

			case WM_PAINT:
				OnPaint(hWnd);
				return 0;

			case WM_LBUTTONDOWN:
				return OnLButtonDown(hWnd, LOWORD(lParam), HIWORD(lParam), wParam);

			case WM_MOUSEMOVE:
				return OnMouseMove(hWnd, LOWORD(lParam), HIWORD(lParam), wParam);

			case WM_LBUTTONUP:
				return OnLButtonUp(hWnd, LOWORD(lParam), HIWORD(lParam), wParam);

			case WM_MEASUREITEM:
				OnMeasureItem((UINT) wParam, (LPMEASUREITEMSTRUCT) lParam);
				return TRUE;

			case WM_DRAWITEM:
				OnDrawItem((UINT) wParam, (LPDRAWITEMSTRUCT) lParam);
				return TRUE;

			case WM_CLOSE:
				PostQuitMessage(0);
				break;

			case WM_COMMAND:
				OnCommand(LOWORD(wParam));
				return 0;

			case WM_HOTKEY:
				return OnHotKey(HMain, wParam, lParam);
		}


		return DefWindowProc(hWnd, msg, wParam, lParam);
	}
}

//
// WinMain
//
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow) {
	HInstance = hInstance;

	HANDLE hMutex = CreateMutex(NULL, TRUE, WINDOW_CLASS WINDOW_NAME);
	if (hMutex == NULL)
		return 0;	// something is wrong
	if (GetLastError() == ERROR_ALREADY_EXISTS)
		return 0;	// there is already one instance of the tMan

	if (!StringBuffer) StrAct = StringBuffer = new TCHAR [STRING_BUFFER_SIZE];
	if (!StringBuffer)
		return FALSE;

	Config.Load();

	HIDPI_InitScaling();

	DetectWiFiCard();

#ifdef LOGGING
	Logger = new CLogger(Config.LogFileName, Config.LogLevel);
#endif

	// localization
	if (!Localization) {
		Localization = new CLocalization();
		Localization->Init();
	}

	WNDCLASS wc;
	ZeroMemory(&wc, sizeof(wc));
	wc.style = CS_PARENTDC;
	wc.lpfnWndProc = MainWndProc;
	wc.hInstance = hInstance;
//	wc.hbrBackground = GetSysColorBrush(ClrBg);
	wc.hbrBackground = (HBRUSH) GetStockObject(NULL_BRUSH);
	wc.lpszClassName = WINDOW_CLASS;
	if (RegisterClass(&wc) == 0)
		return 0;

	InitializeCriticalSection(&CSTasksToClose);

	ReadConfigMessage = RegisterWindowMessage(READ_CONFIG_MESSAGE);

	// desktop window
	DesktopHWND = FindWindow(_T("DesktopExplorerWindow"), _T("Desktop"));

	// task bar
	HTaskBar = FindWindow(_T("HHTaskBar"), NULL);
	RECT rcTaskbar = { 0 };
	GetWindowRect(HTaskBar, &rcTaskbar);

//	LastWnd = GetForegroundWindow();

	//
	ScreenOrientation = GetScreenOrientation();

	HMain = CreateWindow(WINDOW_CLASS, WINDOW_NAME, WS_CHILD | WS_VISIBLE,
		rcTaskbar.right - SCALEX(TMAN_WND_WD), rcTaskbar.top + SCALEY(TMAN_WND_TOP),
		SCALEX(TMAN_WND_WD), rcTaskbar.bottom - rcTaskbar.top - (2 * SCALEY(TMAN_WND_TOP)),
		HTaskBar, NULL, hInstance, NULL);
	SetWindowPos(HMain, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

	if (lpCmdLine != 0 && wcslen(lpCmdLine) != 0) {
		COPYDATASTRUCT cs;
		cs.dwData = 0;
		cs.cbData = 2 + 2 * wcslen(lpCmdLine);
		cs.lpData = lpCmdLine;
		SendMessage(HMain, WM_COPYDATA, NULL, (LPARAM) &cs);
	}

	// set a little higher priority for better UI reaction
	if (Config.TaskListHWKey != 0) {
		UnregisterHotKey(HMain, HotKeyID);
		RegisterHotKey(HMain, HotKeyID, MOD_WIN, Config.TaskListHWKey);
	}

	// create off-screen buffer
	int wd = SCALEX(TMAN_WND_WD);
	int ht = rcTaskbar.bottom - rcTaskbar.top - (2 * SCALEY(TMAN_WND_TOP));

	HDC scrDC = ::GetDC(NULL);
	HBITMAP hBmp = CreateCompatibleBitmap(scrDC, wd, ht);

	MemDC = CreateCompatibleDC(scrDC);
	HGDIOBJ saveBmp = SelectObject(MemDC, hBmp);
	::ReleaseDC(NULL, scrDC);

	BtnType = GetCloseButtonType(GetForegroundWindow());

	// set timer
	SetTimer(HMain, RedrawTimer, 500, NULL);

	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	// HW keys
	if (Config.TaskListHWKey != 0)
		UnregisterHotKey(HMain, HotKeyID);

	KillTimer(HMain, RedrawTimer);
	KillTimer(HMain, MinimizeTimer);
	KillTimer(HMain, TapAndHoldTimer);

	delete Localization;

	// free memory
	delete [] StringBuffer;
	StringBuffer = NULL;

	//
	SelectObject(MemDC, saveBmp);
	DeleteObject(hBmp);
	DeleteDC(MemDC);

	delete [] WiFiDeviceName;

#ifdef LOGGING
	delete Logger;
#endif

	DeleteCriticalSection(&CSTasksToClose);

	return msg.wParam;
}
