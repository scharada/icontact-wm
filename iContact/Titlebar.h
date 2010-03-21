#pragma once

// Define this when taking screenshots to get a nice titlebar
//#define DEBUG_SCREENSHOTS

// Default heights and spacing of icons
#define TITLE_BAR_HEIGHT        16
#define TITLE_BAR_ICON_SPACING  3

// Position of elements within skin.png
#define BATTERY_X_OFFSET        0
#define BATTERY_WIDTH           21
#define SPEAKER_OFF_X_OFFSET    23
#define SPEAKER_OFF_WIDTH       17
#define SPEAKER_ON_X_OFFSET     44
#define SPEAKER_ON_WIDTH        16
#define VIBRATE_X_OFFSET        67
#define VIBRATE_WIDTH           17
#define BLUETOOTH_X_OFFSET      90
#define BLUETOOTH_WIDTH         11
#define WIFI_X_OFFSET           110
#define WIFI_WIDTH              21
#define LOCK_X_OFFSET           132
#define LOCK_WIDTH              8
#define ALARM_X_OFFSET          139
#define ALARM_WIDTH             10
#define A2DP_X_OFFSET           152
#define A2DP_WIDTH              17
#define CONNECTION_X_OFFSET     171
#define CONNECTION_WIDTH        17

// Window Message for notifications of changes
#define WM_TITLEBAR             WM_USER + 1

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
void RefreshTitlebar(UINT);
void DrawTitlebarOn(HDC, RECT, HDC, COLORREF, COLORREF, COLORREF, COLORREF);