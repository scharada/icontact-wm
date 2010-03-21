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


// ---- HTC Touch Diamond sensor --------------------------------------------
// ------- by Scott Seligman <scott@scottandmichelle.net> -------------------
// ------- GESTURE_TOUCH related information from KriX <kriix@hotmail.com> --


// -- Polling the sensor using the SDK dll ------------------------
#define SENSOR_DLL      L"HTCSensorSDK.dll"

typedef struct _SENSORDATA
{
    SHORT   TiltX;          // From -1000 to 1000 (about), 0 is flat
    SHORT   TiltY;          // From -1000 to 1000 (about), 0 is flat
    SHORT   Orientation;		    // From -1000 to 1000 (about)
							// 0 = Straight up, -1000 = Flat, 1000 = Upside down

    WORD    Unknown1;       // Always zero
    DWORD   AngleY;         // From 0 to 359
    DWORD   AngleX;         // From 0 to 359
    DWORD   Unknown2;       // Bit field?
} SENSORDATA, *PSENSORDATA;

#define SENSOR_TILT     1

typedef HANDLE (WINAPI * PFN_HTCSensorOpen)(DWORD);
typedef void (WINAPI * PFN_HTCSensorClose)(HANDLE);
typedef DWORD (WINAPI * PFN_HTCSensorGetDataOutput)(HANDLE, PSENSORDATA);


// -- Notifications via the sensor's event ------------------------
#define SN_GSENSOR_ROOT     HKEY_LOCAL_MACHINE
#define SN_GSENSOR_PATH     _T("Software\\HTC\\HTCSensor\\GSensor")
#define SN_GSENSOR_VALUE    _T("EventChanged")
#define SN_GSENSOR_BITMASK  0xF

#define SENSOR_START        _T("HTC_GSENSOR_SERVICESTART")
#define SENSOR_STOP         _T("HTC_GSENSOR_SERVICESTOP")

#define ORIENTATION_LANDSCAPE           0
#define ORIENTATION_REVERSE_LANDSCAPE   1
#define ORIENTATION_PORTRAIT            2
#define ORIENTATION_UPSIDE_DOWN         3
#define ORIENTATION_FACE_DOWN           4
#define ORIENTATION_FACE_UP             5

// TODO: Modify these two constants to your needs
#define SN_GSENSOR          1
#define WM_STATECHANGE      (WM_USER + 1)



// -- Notifications for gestures around the d-pad -----------------
#define HTCAPI_DLL      L"HTCAPI.dll"

typedef DWORD (WINAPI * PFN_HTCNavOpen)(HWND, DWORD);
typedef DWORD (WINAPI * PFN_HTCNavSetMode)(HWND, DWORD);
typedef DWORD (WINAPI * PFN_HTCNavClose)(DWORD);

#define GESTURE_API         1
#define GESTURE_GESTURE     4   // Generates WM_HTC_GESTURE messages
#define GESTURE_TOUCH       5   // Generates WM_HTC_TOUCH messages
                                //  (with the HTCTOUCH structs)

#define WM_HTC_GESTURE      (WM_USER + 200)
#define WM_HTC_TOUCH        (WM_USER + 209)

#define ROTATION_MASK       0x00000F00  // WPARAM
#define ROTATION_CLOCKWISE  0x00000800
#define ROTATION_COUNTER    0x00000900

#define COMPLETION_MASK     0x00F00000 // LPARAM
#define DIRECTION_MASK      0x0000F000 // LPARAM

typedef struct tagHTCTOUCH_WPARAM
{
    BYTE Up;    //0=KeyDown,1=KeyUp
    BYTE Where; //Where the click occurs (left pane, wheel, right pane)
                //Pos for the Right Pane click
    BYTE xPosRP;
    BYTE yPosRP;
} HTCTOUCH_WPARAM;

typedef struct tagHTCTOUCH_LPARAM
{
        //Pos for the Left Pane click
    BYTE xPosLP;
    BYTE yPosLP;
        //Wheel click angle
    BYTE WheelAngle;
    BYTE Unknown4;
} HTCTOUCH_LPARAM;

bool SensorGestureInit(HWND hWnd);
void SensorGesturePoll(PSENSORDATA sd);
bool SensorGestureUninit();
bool SensorPollingInit();
bool SensorPollingUninit();