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

#pragma once

// Define this when taking screenshots to get a nice titlebar
//#define DEBUG_SCREENSHOTS

// Default heights and spacing of icons
#define TITLE_BAR_ICON_SPACING  3

// Position and size of elements within skin.png
#define TITLE_BAR_HEIGHT        16
#define TITLE_BAR_WIDTH         240
#define BACKGROUND_X_OFFSET     208
#define BACKGROUND_WIDTH        4
#define FOREGROUND_X_OFFSET     212
#define FOREGROUND_WIDTH        4
#define SIGNAL_X_OFFSET         0
#define SIGNAL_WIDTH            20
#define SIGNAL_OFF_X_OFFSET     20
#define SIGNAL_OFF_WIDTH        20
#define SPEAKER_OFF_X_OFFSET    40
#define SPEAKER_OFF_WIDTH       17
#define SPEAKER_ON_X_OFFSET     61
#define SPEAKER_ON_WIDTH        16
#define VIBRATE_X_OFFSET        84
#define VIBRATE_WIDTH           17
#define BLUETOOTH_X_OFFSET      107
#define BLUETOOTH_WIDTH         11
#define WIFI_X_OFFSET           127
#define WIFI_WIDTH              21
#define LOCK_X_OFFSET           149
#define LOCK_WIDTH              8
#define ALARM_X_OFFSET          156
#define ALARM_WIDTH             10
#define A2DP_X_OFFSET           169
#define A2DP_WIDTH              17
#define CONNECTION_X_OFFSET     188
#define CONNECTION_WIDTH        17
#define BATTERY_X_OFFSET        219
#define BATTERY_WIDTH           21

// Window Message for notifications of changes
#define WM_TITLEBAR             WM_USER + 99
#define WM_REGISTRY             WM_USER + 100

// Which values to refresh
#define TB_SIGNAL_MASK          0x01
#define TB_OPERATOR_MASK        0x02
#define TB_TIME_MASK            0x04
#define TB_BATTERY_MASK         0x08
#define TB_BLUETOOTH_MASK       0x10
#define TB_VOLUME_MASK          0x20
#define TB_CONNECTIONS_MASK     0x40
#define TB_OTHER_MASK           0x80

// Registry key locations & flags
// also see snapi.h

#define SN_RINGMODE_ROOT        HKEY_CURRENT_USER
#define SN_RINGMODE_PATH        TEXT("ControlPanel\\Notifications\\ShellOverrides")
#define SN_RINGMODE_VALUE       TEXT("Mode")

#define SN_RINGMODE_SOUND       0
#define SN_RINGMODE_VIBRATE     1
#define SN_RINGMODE_MUTE        2


#define BATTERY_STATE_NORMAL     0
#define BATTERY_STATE_NOTPRESENT 1
#define BATTERY_STATE_CHARGING   2
#define BATTERY_STATE_LOW        4
#define BATTERY_STATE_CRITICAL   8

// copied from WM6 snapi.h
////////////////////////////////////////////////////////////////////////////////
// BluetoothStatePowerOn
// Gets a value indicating whether Bluetooth is powered on
#define SN_BLUETOOTHSTATEPOWERON_ROOT HKEY_LOCAL_MACHINE
#define SN_BLUETOOTHSTATEPOWERON_PATH TEXT("System\\State\\Hardware")
#define SN_BLUETOOTHSTATEPOWERON_VALUE TEXT("Bluetooth")
#define SN_BLUETOOTHSTATEPOWERON_BITMASK 1

// copied from WM6 snapi.h
////////////////////////////////////////////////////////////////////////////////
// BluetoothStateA2DPConnected
// Gets a value indicating whether Bluetooth A2DP is connected
#define SN_BLUETOOTHSTATEA2DPCONNECTED_ROOT HKEY_LOCAL_MACHINE
#define SN_BLUETOOTHSTATEA2DPCONNECTED_PATH TEXT("System\\State\\Hardware")
#define SN_BLUETOOTHSTATEA2DPCONNECTED_VALUE TEXT("Bluetooth")
#define SN_BLUETOOTHSTATEA2DPCONNECTED_BITMASK 4

// copied from WM6 snapi.h
////////////////////////////////////////////////////////////////////////////////
// WiFiStatePowerOn
// Gets a value indicating whether Wi-Fi is powered on
#define SN_WIFISTATEPOWERON_ROOT HKEY_LOCAL_MACHINE
#define SN_WIFISTATEPOWERON_PATH TEXT("System\\State\\Hardware")
#define SN_WIFISTATEPOWERON_VALUE TEXT("WiFi")
#define SN_WIFISTATEPOWERON_BITMASK 2

// copied from WM6 snapi.h
////////////////////////////////////////////////////////////////////////////////
// CellNetwork Connected
// Gets a value indicating what cellular system is used for connection 
#define SN_CELLSYSTEMCONNECTED_ROOT HKEY_LOCAL_MACHINE
#define SN_CELLSYSTEMCONNECTED_PATH TEXT("System\\State\\Phone")
#define SN_CELLSYSTEMCONNECTED_VALUE TEXT("Cellular System Connected")
#define SN_CELLSYSTEMCONNECTED_GPRS_BITMASK     1
#define SN_CELLSYSTEMCONNECTED_1XRTT_BITMASK    2
#define SN_CELLSYSTEMCONNECTED_1XEVDO_BITMASK   4
#define SN_CELLSYSTEMCONNECTED_EDGE_BITMASK     8
#define SN_CELLSYSTEMCONNECTED_UMTS_BITMASK     16
#define SN_CELLSYSTEMCONNECTED_EVDV_BITMASK     32
#define SN_CELLSYSTEMCONNECTED_HSDPA_BITMASK    64
#define SN_CELLSYSTEMCONNECTED_CSD_BITMASK      2147483648 

////////////////////////////////////////////////////////////////////////////////
// public functions
void InitTitlebar(HWND);
void DestroyTitlebar();
void RefreshTitlebar(UINT);
void DrawTitlebarOn(HDC, RECT, HDC, HFONT, const TCHAR * = NULL);