#include "StdAfx.h"
#include "windows.h"
#include "regext.h"
#include "snapi.h"

#include "Titlebar.h"
#include "GraphicFunctions.h"

int nBattery; //0-14
int nBars; //0-5
wchar_t szCarrier[50];
wchar_t szTime[12];

bool bCharging;
bool bBluetooth;
bool bSpeakerOn;
bool bVibrate;
bool bWifi;
bool bLocked;
bool bAlarm;
bool bA2dp;
bool bConnection;

HFONT hfTitleFont;

void InitTitlebar(HWND hWnd) {
    HREGNOTIFY hNotify = NULL;

    // Signal Strength
    HRESULT hr = RegistryNotifyWindow(
        SN_PHONESIGNALSTRENGTH_ROOT,
        SN_PHONESIGNALSTRENGTH_PATH,
        SN_PHONESIGNALSTRENGTH_VALUE, 
        hWnd, WM_TITLEBAR, TB_SIGNAL_MASK,
        NULL, &hNotify);
    //TODO: don't lose track of hNotify? 

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
    hfTitleFont = BuildFont(13, false, false);
}

void RefreshTitlebar(UINT uWhich) {
    DWORD dw;

    // Signal Strength
    if (uWhich & TB_SIGNAL_MASK) {
        RegistryGetDWORD(
            SN_PHONESIGNALSTRENGTH_ROOT, 
            SN_PHONESIGNALSTRENGTH_PATH, 
            SN_PHONESIGNALSTRENGTH_VALUE,
            &dw);
        nBars = (int)((double)dw / 20.0);
    }

    // Operator name
    if (uWhich & TB_OPERATOR_MASK) {
        RegistryGetString(
            SN_PHONEOPERATORNAME_ROOT, 
            SN_PHONEOPERATORNAME_PATH, 
            SN_PHONEOPERATORNAME_VALUE, 
            szCarrier, sizeof(szCarrier));
    }

    // Time
    if (uWhich & TB_TIME_MASK) {
   	    SYSTEMTIME stLocal;
	    GetLocalTime(&stLocal);
        wsprintf(szTime, L"%d:%02d", stLocal.wHour, stLocal.wMinute);
    }

    // Battery level & charging state
    // http://msdn.microsoft.com/en-us/library/aa456240.aspx
    if (uWhich & TB_BATTERY_MASK) {
        RegistryGetDWORD(
            SN_POWERBATTERYSTRENGTH_ROOT, 
            SN_POWERBATTERYSTRENGTH_PATH, 
            SN_POWERBATTERYSTRENGTH_VALUE,
            &dw);
        nBattery = (dw & SN_POWERBATTERYSTRENGTH_BITMASK) >> 16;
        nBattery = (int)(nBattery * 0.172834 + 0.5);
        bCharging = (dw & BATTERY_STATE_CHARGING) != 0;
    }

    // Bluetooth on/off
    if (uWhich & TB_BLUETOOTH_MASK) {
        RegistryGetDWORD(
            SN_BLUETOOTHSTATEPOWERON_ROOT,
            SN_BLUETOOTHSTATEPOWERON_PATH,
            SN_BLUETOOTHSTATEPOWERON_VALUE,
            &dw);
        bBluetooth = (dw & SN_BLUETOOTHSTATEPOWERON_BITMASK) != 0;

        // this is not needed, since this is the same RegKey as SN_BLUETOOTH*
        //RegistryGetDWORD(
        //    SN_BLUETOOTHSTATEA2DPCONNECTED_ROOT,
        //    SN_BLUETOOTHSTATEA2DPCONNECTED_PATH,
        //    SN_BLUETOOTHSTATEA2DPCONNECTED_VALUE,
        //    &dw);
        bA2dp = (dw & SN_BLUETOOTHSTATEA2DPCONNECTED_BITMASK) != 0;
    }

    // Speaker / volume
    if (uWhich & TB_VOLUME_MASK) {
        RegistryGetDWORD(
		    SN_RINGMODE_ROOT,
		    SN_RINGMODE_PATH,	
		    SN_RINGMODE_VALUE,
            &dw);
	    bSpeakerOn = dw == SN_RINGMODE_SOUND;
        bVibrate = dw == SN_RINGMODE_VIBRATE;
    }

    // Connections & Wifi
    if (uWhich & TB_CONNECTIONS_MASK) {
        RegistryGetDWORD(
            SN_WIFISTATEPOWERON_ROOT,
            SN_WIFISTATEPOWERON_PATH,
            SN_WIFISTATEPOWERON_VALUE,
            &dw);
        bWifi = (dw & SN_WIFISTATEPOWERON_BITMASK) != 0;

        RegistryGetDWORD(
            SN_CONNECTIONSCOUNT_ROOT,
            SN_CONNECTIONSCOUNT_PATH,
            SN_CONNECTIONSCOUNT_VALUE,
            &dw);
        bConnection = dw != 0;
    }

    if (uWhich & TB_OTHER_MASK) {
        bLocked = false; // TODO?
        bAlarm = false; // TODO?
    }
}

void DrawTitlebarOn(HDC hdc, RECT rTitlebar, HDC hdcSkin, HBRUSH hbrBackground) {
    RECT rc;
    HGDIOBJ hOld;

    HBRUSH hbrForeground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	hOld = SelectObject(hdc, hfTitleFont);
	SetTextColor(hdc, RGB(255, 255, 255));
	SetBkColor(hdc, RGB(0, 0, 0));
    SetBkMode(hdc, TRANSPARENT);

    // Fill in background
	FillRect(hdc, &rTitlebar, hbrBackground);

    // LEFT SIDE
    // Bars
    rc.left = rTitlebar.left;
    rc.bottom = rTitlebar.bottom - 3;
    for (int i = 0; i < 5; i++) {
        rc.right = rc.left + 3;
        rc.top = nBars > i ? rc.bottom - (2 * i + 3) : rc.bottom - 1;
        FillRect(hdc, &rc, hbrForeground);
        rc.left = rc.right + 1;
    }

    // Carrier
    rc.top = rTitlebar.top + 1;
    rc.left = rc.right + 2;
	SetTextAlign(hdc, TA_LEFT);
    
    DrawText(hdc, szCarrier, -1, &rc, DT_LEFT | DT_TOP | DT_NOCLIP | DT_NOPREFIX);

    // Time
    rc.left = rTitlebar.left;
    rc.right = rTitlebar.right;
    DrawText(hdc, szTime, -1, &rc, DT_CENTER | DT_TOP | DT_NOCLIP);

    // Battery
    int x = rTitlebar.right - BATTERY_WIDTH;
    int y = rTitlebar.top;
	BitBlt(hdc, x, y, BATTERY_WIDTH, TITLE_BAR_HEIGHT, 
        hdcSkin, BATTERY_X_OFFSET, 0, SRCCOPY);

    if (nBattery) {
        rc.top = 5;
        rc.bottom = rc.top + 6;
        rc.left = x + 3;
        rc.right = rc.left + nBattery;
        FillRect(hdc, &rc, hbrForeground);
    }

    // Speaker
    if (bVibrate) {
        x -= VIBRATE_WIDTH + TITLE_BAR_ICON_SPACING;
	    BitBlt(hdc, x, y, VIBRATE_WIDTH, TITLE_BAR_HEIGHT, 
            hdcSkin, VIBRATE_X_OFFSET, 0, SRCCOPY);
    }
    else if (bSpeakerOn) {
        x -= SPEAKER_ON_WIDTH + TITLE_BAR_ICON_SPACING;
	    BitBlt(hdc, x, y, SPEAKER_ON_WIDTH, TITLE_BAR_HEIGHT, 
            hdcSkin, SPEAKER_ON_X_OFFSET, 0, SRCCOPY);
    }
    else {
        x -= SPEAKER_OFF_WIDTH + TITLE_BAR_ICON_SPACING;
	    BitBlt(hdc, x, y, SPEAKER_OFF_WIDTH, TITLE_BAR_HEIGHT, 
            hdcSkin, SPEAKER_OFF_X_OFFSET, 0, SRCCOPY);
    }

    // Bluetooth
    if (bBluetooth) {
        x -= BLUETOOTH_WIDTH + TITLE_BAR_ICON_SPACING;
        BitBlt(hdc, x, y, BLUETOOTH_WIDTH, TITLE_BAR_HEIGHT, 
            hdcSkin, BLUETOOTH_X_OFFSET, 0, SRCCOPY);
    }

    // WiFi Connection
    if (bWifi) {
        x -= WIFI_WIDTH + TITLE_BAR_ICON_SPACING;
        BitBlt(hdc, x, y, WIFI_WIDTH, TITLE_BAR_HEIGHT, 
            hdcSkin, WIFI_X_OFFSET, 0, SRCCOPY);
    }

    // Data Connection
    if (bConnection) {
        x -= CONNECTION_WIDTH + TITLE_BAR_ICON_SPACING;
        BitBlt(hdc, x, y, CONNECTION_WIDTH, TITLE_BAR_HEIGHT, 
            hdcSkin, CONNECTION_X_OFFSET, 0, SRCCOPY);
    }

    //cleanup
    SelectObject(hdc, hOld);
}