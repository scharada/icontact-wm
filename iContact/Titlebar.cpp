#include "StdAfx.h"
#include "windows.h"
#include "regext.h"
#include "snapi.h"

#include "Titlebar.h"

int nBattery; //0-14
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

void InitTitlebar(HWND hWnd) {
    HREGNOTIFY hNotify = NULL;

    // Signal Strength
    HRESULT hr = RegistryNotifyWindow(
        SN_PHONESIGNALSTRENGTH_ROOT,
        SN_PHONESIGNALSTRENGTH_PATH,
        SN_PHONESIGNALSTRENGTH_VALUE, 
        hWnd, WM_TITLEBAR, TB_SIGNAL_MASK,
        NULL, &hNotify);

    // Time
    hr = RegistryNotifyWindow(
        SN_TIME_ROOT,
        SN_TIME_PATH,
        SN_TIME_VALUE,
        hWnd, WM_TITLEBAR, TB_TIME_MASK,
        NULL, &hNotify);

    // Battery
    hr = RegistryNotifyWindow(
        SN_POWERBATTERYSTRENGTH_ROOT, 
        SN_POWERBATTERYSTRENGTH_PATH, 
        SN_POWERBATTERYSTRENGTH_VALUE,
        hWnd, WM_TITLEBAR, TB_BATTERY_MASK,
		NULL, &hNotify);

    // Bluetooth
    hr = RegistryNotifyWindow(
        SN_BLUETOOTHSTATEPOWERON_ROOT,
        SN_BLUETOOTHSTATEPOWERON_PATH,
        SN_BLUETOOTHSTATEPOWERON_VALUE,
        hWnd, WM_TITLEBAR, TB_BLUETOOTH_MASK,
		NULL, &hNotify);

    // Speaker / Volume
    hr = RegistryNotifyWindow(
	    SN_RINGMODE_ROOT,
	    SN_RINGMODE_PATH,	
	    SN_RINGMODE_VALUE,
        hWnd, WM_TITLEBAR, TB_VOLUME_MASK,
		NULL, &hNotify);

    // WiFi
    hr = RegistryNotifyWindow(
        SN_WIFISTATEPOWERON_ROOT,
        SN_WIFISTATEPOWERON_PATH,
        SN_WIFISTATEPOWERON_VALUE,
        hWnd, WM_TITLEBAR, TB_CONNECTIONS_MASK,
		NULL, &hNotify);

    // Connections
    hr = RegistryNotifyWindow(
        SN_CONNECTIONSCOUNT_ROOT,
        SN_CONNECTIONSCOUNT_PATH,
        SN_CONNECTIONSCOUNT_VALUE,
        hWnd, WM_TITLEBAR, TB_CONNECTIONS_MASK,
		NULL, &hNotify);

    RefreshTitlebar(0xFF);   
}

void RefreshTitlebar(UINT uWhich) {
    DWORD dw;
    HRESULT hr;

#ifdef DEBUG_SCREENSHOTS
    StringCchPrintf(szTime, 12, TEXT("12:38 PM"));
    StringCchPrintf(szCarrier, 50, TEXT("AT&T"));
    nBars = 3;
    nBattery = 13;
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
    // http://msdn.microsoft.com/en-us/library/aa456240.aspx
    if (uWhich & TB_BATTERY_MASK) {
        hr = RegistryGetDWORD(
            SN_POWERBATTERYSTRENGTH_ROOT, 
            SN_POWERBATTERYSTRENGTH_PATH, 
            SN_POWERBATTERYSTRENGTH_VALUE,
            &dw);
        if (SUCCEEDED(hr)) {
            nBattery = (dw & SN_POWERBATTERYSTRENGTH_BITMASK) >> 16;
            nBattery = (int)(nBattery * 0.172834 + 0.5);
            bCharging = (dw & BATTERY_STATE_CHARGING) != 0;
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

void DrawTitlebarOn(HDC hdc, RECT rTitlebar, HDC hdcSkin, HFONT hfTitleFont,
    COLORREF rgbBackground, COLORREF rgbForeground, 
    COLORREF rgbSignal, COLORREF rgbBattery, 
    COLORREF rgbBatteryCharge) {

    RECT rc;
    HGDIOBJ hOld;

    HBRUSH hbrForeground = CreateSolidBrush(rgbForeground);
    HBRUSH hbrBackground = CreateSolidBrush(rgbBackground);
    HBRUSH hbrSignal = CreateSolidBrush(rgbSignal);
    HBRUSH hbrBattery = bCharging ? CreateSolidBrush(rgbBatteryCharge)
        : CreateSolidBrush(rgbBattery);

	SetTextColor(hdc, rgbForeground);
	SetBkColor(hdc, rgbBackground);
    SetBkMode(hdc, TRANSPARENT);

    hOld = SelectObject(hdc, hfTitleFont);

    // Fill in background
	FillRect(hdc, &rTitlebar, hbrBackground);
    int rTitlebarHeight = rTitlebar.bottom - rTitlebar.top;
    int scale = rTitlebarHeight / TITLE_BAR_HEIGHT;

    // LEFT SIDE
    // Bars
    rc.left = rTitlebar.left;
    rc.bottom = rTitlebar.bottom - 3 * scale;
    for (int i = 0; i < 5; i++) {
        rc.right = rc.left + 3 * scale;
        rc.top = nBars > i ? rc.bottom - (2 * i + 3) * scale : rc.bottom - 1 * scale;
        FillRect(hdc, &rc, hbrSignal);
        rc.left = rc.right + 1 * scale;
    }

    // Carrier
    rc.top = rTitlebar.top + 1 * scale;
    rc.left = rc.right + 2 * scale;
	SetTextAlign(hdc, TA_LEFT);
    
    DrawText(hdc, szCarrier, -1, &rc, 
        DT_LEFT | DT_TOP | DT_NOCLIP | DT_NOPREFIX);

    // Time
    rc.left = rTitlebar.left;
    rc.right = rTitlebar.right;
    DrawText(hdc, szTime, -1, &rc, DT_CENTER | DT_TOP | DT_NOCLIP);

    // Battery
    int x = rTitlebar.right - BATTERY_WIDTH * scale;
    int y = rTitlebar.top;
    StretchBlt(hdc, x, y, BATTERY_WIDTH * scale, rTitlebarHeight, 
        hdcSkin, BATTERY_X_OFFSET, 0, BATTERY_WIDTH, TITLE_BAR_HEIGHT, SRCCOPY);

    if (nBattery) {
        rc.top = 5 * scale;
        rc.bottom = rc.top + 6 * scale;
        rc.left = x + 3 * scale;
        rc.right = rc.left + nBattery * scale;
        FillRect(hdc, &rc, hbrBattery);
    }

    // Speaker
    if (bVibrate) {
        x -= (VIBRATE_WIDTH + TITLE_BAR_ICON_SPACING) * scale;
        StretchBlt(hdc, x, y, VIBRATE_WIDTH * scale, rTitlebarHeight, 
            hdcSkin, VIBRATE_X_OFFSET, 0, VIBRATE_WIDTH, TITLE_BAR_HEIGHT, SRCCOPY);
    }
    else if (bSpeakerOn) {
        x -= (SPEAKER_ON_WIDTH + TITLE_BAR_ICON_SPACING) * scale;
        StretchBlt(hdc, x, y, SPEAKER_ON_WIDTH * scale, rTitlebarHeight, 
            hdcSkin, SPEAKER_ON_X_OFFSET, 0, SPEAKER_ON_WIDTH, TITLE_BAR_HEIGHT, SRCCOPY);
    }
    else {
        x -= (SPEAKER_OFF_WIDTH + TITLE_BAR_ICON_SPACING) * scale;
        StretchBlt(hdc, x, y, SPEAKER_OFF_WIDTH * scale, rTitlebarHeight, 
            hdcSkin, SPEAKER_OFF_X_OFFSET, 0, SPEAKER_OFF_WIDTH, TITLE_BAR_HEIGHT, SRCCOPY);
    }

    // Bluetooth
    if (bBluetooth) {
        x -= (BLUETOOTH_WIDTH + TITLE_BAR_ICON_SPACING) * scale;
        StretchBlt(hdc, x, y, BLUETOOTH_WIDTH * scale, rTitlebarHeight, 
            hdcSkin, BLUETOOTH_X_OFFSET, 0, BLUETOOTH_WIDTH, TITLE_BAR_HEIGHT, SRCCOPY);
    }
    if (bA2dp) {
        x -= (A2DP_WIDTH + TITLE_BAR_ICON_SPACING) * scale;
        StretchBlt(hdc, x, y, A2DP_WIDTH * scale, rTitlebarHeight, 
            hdcSkin, A2DP_X_OFFSET, 0, A2DP_WIDTH, TITLE_BAR_HEIGHT, SRCCOPY);
    }

    // WiFi Connection
    if (bWifi) {
        x -= (WIFI_WIDTH + TITLE_BAR_ICON_SPACING) * scale;
        StretchBlt(hdc, x, y, WIFI_WIDTH * scale, rTitlebarHeight, 
            hdcSkin, WIFI_X_OFFSET, 0, WIFI_WIDTH, TITLE_BAR_HEIGHT, SRCCOPY);
    }

    // Data Connection
    if (bConnection) {
        x -= (CONNECTION_WIDTH + TITLE_BAR_ICON_SPACING) * scale;
        StretchBlt(hdc, x, y, CONNECTION_WIDTH * scale, rTitlebarHeight, 
            hdcSkin, CONNECTION_X_OFFSET, 0, CONNECTION_WIDTH, TITLE_BAR_HEIGHT, SRCCOPY);
    }

    //cleanup
    SelectObject(hdc, hOld);
    DeleteObject(hbrForeground);
    DeleteObject(hbrBackground);
    DeleteObject(hbrSignal);
    DeleteObject(hbrBattery);
}