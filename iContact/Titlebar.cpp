/*******************************************************************
This file is part of iContact.

iContact is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

iContact is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with iContact.  If not, see <http://www.gnu.org/licenses/>.
*******************************************************************/

#include "StdAfx.h"
#include "windows.h"
#include "regext.h"
#include "snapi.h"

#include "Titlebar.h"

int nBattery; //0-34
int nBars; //0-5
TCHAR szCarrier[50];
TCHAR szTime[12];

bool bCharging;
bool bBluetooth;
bool bSpeakerOn;
bool bVibrate;
bool bWifi;
bool bLocked;
bool bAlarm;
bool bA2dp;
bool bConnection;

#define HR_TITLEBAR_NOTIFY_COUNT 7
HREGNOTIFY hrTitlebarNotify[HR_TITLEBAR_NOTIFY_COUNT] = {0};

void InitTitlebar(HWND hWnd) {
    // Signal Strength
    HRESULT hr = RegistryNotifyWindow(
        SN_PHONESIGNALSTRENGTH_ROOT,
        SN_PHONESIGNALSTRENGTH_PATH,
        SN_PHONESIGNALSTRENGTH_VALUE, 
        hWnd, WM_TITLEBAR, TB_SIGNAL_MASK,
        NULL, &hrTitlebarNotify[0]);

    // Time
    hr = RegistryNotifyWindow(
        SN_TIME_ROOT,
        SN_TIME_PATH,
        SN_TIME_VALUE,
        hWnd, WM_TITLEBAR, TB_TIME_MASK,
        NULL, &hrTitlebarNotify[1]);

    // Battery
    hr = RegistryNotifyWindow(
        SN_POWERBATTERYSTRENGTH_ROOT, 
        SN_POWERBATTERYSTRENGTH_PATH, 
        SN_POWERBATTERYSTRENGTH_VALUE,
        hWnd, WM_TITLEBAR, TB_BATTERY_MASK,
		NULL, &hrTitlebarNotify[2]);

    // Bluetooth
    hr = RegistryNotifyWindow(
        SN_BLUETOOTHSTATEPOWERON_ROOT,
        SN_BLUETOOTHSTATEPOWERON_PATH,
        SN_BLUETOOTHSTATEPOWERON_VALUE,
        hWnd, WM_TITLEBAR, TB_BLUETOOTH_MASK,
		NULL, &hrTitlebarNotify[3]);

    // Speaker / Volume
    hr = RegistryNotifyWindow(
	    SN_RINGMODE_ROOT,
	    SN_RINGMODE_PATH,	
	    SN_RINGMODE_VALUE,
        hWnd, WM_TITLEBAR, TB_VOLUME_MASK,
		NULL, &hrTitlebarNotify[4]);

    // WiFi
    hr = RegistryNotifyWindow(
        SN_WIFISTATEPOWERON_ROOT,
        SN_WIFISTATEPOWERON_PATH,
        SN_WIFISTATEPOWERON_VALUE,
        hWnd, WM_TITLEBAR, TB_CONNECTIONS_MASK,
		NULL, &hrTitlebarNotify[5]);

    // Connections
    hr = RegistryNotifyWindow(
        SN_CONNECTIONSCOUNT_ROOT,
        SN_CONNECTIONSCOUNT_PATH,
        SN_CONNECTIONSCOUNT_VALUE,
        hWnd, WM_TITLEBAR, TB_CONNECTIONS_MASK,
		NULL, &hrTitlebarNotify[6]);

    RefreshTitlebar(0xFF);   
}

void DestroyTitlebar() {
    // Clean up HREGNOTIFY's
    for (int i = 0; i < HR_TITLEBAR_NOTIFY_COUNT; i++) {
        if (NULL != hrTitlebarNotify[i]) {
            RegistryCloseNotification(hrTitlebarNotify[i]);
            hrTitlebarNotify[i] = 0;
        }
    }
}

void RefreshTitlebar(UINT uWhich) {
    DWORD dw;
    HRESULT hr;

#ifdef DEBUG_SCREENSHOTS
    StringCchPrintf(szTime, 12, TEXT("12:38 PM"));
    StringCchPrintf(szCarrier, 50, TEXT("AT&T"));
    nBars = 3;
    nBattery = 19;
    bVibrate = false;
    bSpeakerOn = true;
    bBluetooth = true;
    bA2dp = false;
    bWifi = false;
    bConnection = true;
    return;
#endif

    // Signal Strength
    if (uWhich & TB_SIGNAL_MASK) {
        hr = RegistryGetDWORD(
            SN_PHONESIGNALSTRENGTH_ROOT, 
            SN_PHONESIGNALSTRENGTH_PATH, 
            SN_PHONESIGNALSTRENGTH_VALUE,
            &dw);
        if (SUCCEEDED(hr))
            nBars = (int)((double)dw / 20.0);
    }

    // Operator name
    if (uWhich & TB_OPERATOR_MASK) {
        hr = RegistryGetString(
            SN_PHONEOPERATORNAME_ROOT, 
            SN_PHONEOPERATORNAME_PATH, 
            SN_PHONEOPERATORNAME_VALUE, 
            szCarrier, sizeof(szCarrier));
    }

    // Time
    if (uWhich & TB_TIME_MASK) {
        ::GetTimeFormat(LOCALE_USER_DEFAULT,
            TIME_NOSECONDS, NULL, NULL, szTime, 12);
    }

    // Battery level & charging state
    if (uWhich & TB_BATTERY_MASK) {
        SYSTEM_POWER_STATUS_EX2 sps = {0};
        DWORD result = GetSystemPowerStatusEx2(&sps,
            sizeof(SYSTEM_POWER_STATUS_EX2), false);

        if (result > 0) {
            //nBattery = 0-34
            nBattery = min(34, MulDiv(sps.BatteryLifePercent, 34, 100));
            bCharging = (sps.BatteryFlag & BATTERY_FLAG_CHARGING) > 0;
        }
    }

    // Bluetooth on/off
    if (uWhich & TB_BLUETOOTH_MASK) {
        hr = RegistryGetDWORD(
            SN_BLUETOOTHSTATEPOWERON_ROOT,
            SN_BLUETOOTHSTATEPOWERON_PATH,
            SN_BLUETOOTHSTATEPOWERON_VALUE,
            &dw);
        if (SUCCEEDED(hr)) {
            bBluetooth = (dw & SN_BLUETOOTHSTATEPOWERON_BITMASK) != 0;

            // this is not needed, since this is the same RegKey as SN_BLUETOOTH*
            //RegistryGetDWORD(
            //    SN_BLUETOOTHSTATEA2DPCONNECTED_ROOT,
            //    SN_BLUETOOTHSTATEA2DPCONNECTED_PATH,
            //    SN_BLUETOOTHSTATEA2DPCONNECTED_VALUE,
            //    &dw);
            bA2dp = (dw & SN_BLUETOOTHSTATEA2DPCONNECTED_BITMASK) != 0;
        }
    }

    // Speaker / volume
    if (uWhich & TB_VOLUME_MASK) {
        hr = RegistryGetDWORD(
	    SN_RINGMODE_ROOT,
	    SN_RINGMODE_PATH,	
	    SN_RINGMODE_VALUE,
            &dw);

        if (SUCCEEDED(hr)) {
            bSpeakerOn = dw == SN_RINGMODE_SOUND;
            bVibrate = dw == SN_RINGMODE_VIBRATE;
        }
    }

    // Connections & Wifi
    if (uWhich & TB_CONNECTIONS_MASK) {
        hr = RegistryGetDWORD(
            SN_WIFISTATEPOWERON_ROOT,
            SN_WIFISTATEPOWERON_PATH,
            SN_WIFISTATEPOWERON_VALUE,
            &dw);

        if (SUCCEEDED(hr))
            bWifi = (dw & SN_WIFISTATEPOWERON_BITMASK) != 0;

        hr = RegistryGetDWORD(
            SN_CONNECTIONSCOUNT_ROOT,
            SN_CONNECTIONSCOUNT_PATH,
            SN_CONNECTIONSCOUNT_VALUE,
            &dw);

        if (SUCCEEDED(hr))
            bConnection = dw != 0;
    }

    if (uWhich & TB_OTHER_MASK) {
        bLocked = false; // TODO?
        bAlarm = false; // TODO?
    }
}

void DrawTitlebarOn(HDC hdc, RECT rTitlebar, HDC hdcSkin, 
                    HFONT hfTitleFont, const TCHAR * tszTitle) {

    RECT rc = {0};
    HGDIOBJ hOld;

    int rTitlebarHeight = rTitlebar.bottom - rTitlebar.top;
    int scale = rTitlebarHeight / TITLE_BAR_HEIGHT;

	SetTextColor(hdc, GetPixel(hdcSkin, FOREGROUND_X_OFFSET * scale, 0));
    SetBkMode(hdc, TRANSPARENT);

    hOld = SelectObject(hdc, hfTitleFont);

    // Fill in background, stretch the last column across
    StretchBlt(hdc, rTitlebar.left, rTitlebar.top, 
        rTitlebar.right - rTitlebar.left, rTitlebarHeight,
        hdcSkin, BACKGROUND_X_OFFSET * scale, 0, 
		BACKGROUND_WIDTH * scale, TITLE_BAR_HEIGHT * scale,
        SRCCOPY);

    // LEFT SIDE
    // "Filled" Bars
    int w = nBars * SIGNAL_WIDTH / 5;
    BitBlt(hdc, rTitlebar.left, rTitlebar.top,
        w * scale, rTitlebarHeight, hdcSkin,
		SIGNAL_X_OFFSET * scale, 0, SRCCOPY);

    // "Empty" Bars
    BitBlt(hdc, rTitlebar.left + w * scale, rTitlebar.top,
        (SIGNAL_WIDTH - w) * scale, rTitlebarHeight,
        hdcSkin, (SIGNAL_OFF_X_OFFSET + w) * scale, 0, SRCCOPY);

    // Carrier (or tszTitle if it exists)
    rc.top = (rTitlebar.top + 1) * scale;
    rc.left = (SIGNAL_WIDTH + 2) * scale;
	SetTextAlign(hdc, TA_LEFT);
    
    DrawText(hdc, tszTitle && _tcslen(tszTitle) > 0 
        ? tszTitle : szCarrier, -1, &rc, 
        DT_LEFT | DT_TOP | DT_NOCLIP | DT_NOPREFIX);

    // Time
    rc.left = rTitlebar.left;
    rc.right = rTitlebar.right;
    DrawText(hdc, szTime, -1, &rc, DT_CENTER | DT_TOP | DT_NOCLIP);

    // Battery
    int x = rTitlebar.right - BATTERY_WIDTH * scale;
    int y = rTitlebar.top;

    // copy the whole battery section of the skin
    BitBlt(hdc, x, y, BATTERY_WIDTH * scale, rTitlebarHeight, 
        hdcSkin, BATTERY_X_OFFSET * scale, 0, SRCCOPY);

    // copy the "empty" section of the battery
    StretchBlt(hdc, x + scale, y, 
		(BATTERY_WIDTH - 4) * scale, rTitlebarHeight,
        hdcSkin, (BATTERY_X_OFFSET + 17) * scale, 0,
		1, TITLE_BAR_HEIGHT * scale, SRCCOPY);

    // copy the "full" section of the battery
    if (nBattery) {
        StretchBlt(hdc, rTitlebar.right - (BATTERY_WIDTH - 1) * scale, y,
            nBattery * scale / 2, rTitlebarHeight,
            hdcSkin, (BATTERY_X_OFFSET + 1) * scale, 0,
			1, TITLE_BAR_HEIGHT * scale, SRCCOPY);
    }

    // Speaker
    if (bVibrate) {
        x -= (VIBRATE_WIDTH + TITLE_BAR_ICON_SPACING) * scale;
        BitBlt(hdc, x, y, VIBRATE_WIDTH * scale, rTitlebarHeight, 
            hdcSkin, VIBRATE_X_OFFSET * scale, 0, SRCCOPY);
    }
    else if (bSpeakerOn) {
        x -= (SPEAKER_ON_WIDTH + TITLE_BAR_ICON_SPACING) * scale;
        BitBlt(hdc, x, y, SPEAKER_ON_WIDTH * scale, rTitlebarHeight, 
            hdcSkin, SPEAKER_ON_X_OFFSET * scale, 0, SRCCOPY);
    }
    else {
        x -= (SPEAKER_OFF_WIDTH + TITLE_BAR_ICON_SPACING) * scale;
        BitBlt(hdc, x, y, SPEAKER_OFF_WIDTH * scale, rTitlebarHeight, 
            hdcSkin, SPEAKER_OFF_X_OFFSET * scale, 0, SRCCOPY);
    }

    // Bluetooth
    if (bBluetooth) {
        x -= (BLUETOOTH_WIDTH + TITLE_BAR_ICON_SPACING) * scale;
        BitBlt(hdc, x, y, BLUETOOTH_WIDTH * scale, rTitlebarHeight, 
            hdcSkin, BLUETOOTH_X_OFFSET * scale, 0, SRCCOPY);
    }
    if (bA2dp) {
        x -= (A2DP_WIDTH + TITLE_BAR_ICON_SPACING) * scale;
        BitBlt(hdc, x, y, A2DP_WIDTH * scale, rTitlebarHeight, 
            hdcSkin, A2DP_X_OFFSET * scale, 0, SRCCOPY);
    }

    // WiFi Connection
    if (bWifi) {
        x -= (WIFI_WIDTH + TITLE_BAR_ICON_SPACING) * scale;
        BitBlt(hdc, x, y, WIFI_WIDTH * scale, rTitlebarHeight, 
            hdcSkin, WIFI_X_OFFSET * scale, 0, SRCCOPY);
    }

    // Data Connection
    if (bConnection) {
        x -= (CONNECTION_WIDTH + TITLE_BAR_ICON_SPACING) * scale;
        BitBlt(hdc, x, y, CONNECTION_WIDTH * scale, rTitlebarHeight, 
            hdcSkin, CONNECTION_X_OFFSET * scale, 0, SRCCOPY);
    }

    //cleanup
    SelectObject(hdc, hOld);
}