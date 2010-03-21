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

#include "stdafx.h"
#include "Sensor.h"

// ---- HTC Touch Diamond sensor --------------------------------------------
// ------- by Scott Seligman <scott@scottandmichelle.net> -------------------
// ------- GESTURE_TOUCH related information from KriX <kriix@hotmail.com> --

// -- Polling the sensor using the SDK dll ------------------------

PFN_HTCSensorOpen           pfnHTCSensorOpen;
PFN_HTCSensorClose          pfnHTCSensorClose;
PFN_HTCSensorGetDataOutput  pfnHTCSensorGetDataOutput;

PFN_HTCNavOpen              pfnHTCNavOpen;
PFN_HTCNavSetMode           pfnHTCNavSetMode;
PFN_HTCNavClose             pfnHTCNavClose;


// ---- End of HTC Touch Diamond sensor -------------------------------------
// --------------------------------------------------------------------------
HANDLE      g_hSensor;

bool SensorGestureInit(HWND hWnd) {
    HMODULE hHTCAPI = LoadLibrary(HTCAPI_DLL);

    if (hHTCAPI == NULL) {
        return false;
    }

    pfnHTCNavOpen = (PFN_HTCNavOpen)
        GetProcAddress(hHTCAPI, L"HTCNavOpen");
    pfnHTCNavSetMode = (PFN_HTCNavSetMode)
        GetProcAddress(hHTCAPI, L"HTCNavSetMode");
    pfnHTCNavClose = (PFN_HTCNavClose)
        GetProcAddress(hHTCAPI, L"HTCNavClose");

    if (pfnHTCNavOpen == NULL ||
        pfnHTCNavSetMode == NULL ||
        pfnHTCNavClose == NULL) {
        
		MessageBox(hWnd,
            _T("Unable to find entry point"),
            _T("Error"), MB_TOPMOST);
		
		pfnHTCNavOpen = NULL;
		pfnHTCNavSetMode = NULL;
		pfnHTCNavClose = NULL;

        return false;
    }

    pfnHTCNavOpen(hWnd, GESTURE_API);
    pfnHTCNavSetMode(hWnd, GESTURE_GESTURE);
    pfnHTCNavSetMode(hWnd, GESTURE_TOUCH);

    return true;
}

void SensorGesturePoll(PSENSORDATA psd) {
    pfnHTCSensorGetDataOutput(g_hSensor, psd);
}

bool SensorGestureUninit() {
	if (pfnHTCNavClose) {
		pfnHTCNavClose(GESTURE_TOUCH);
		pfnHTCNavClose(GESTURE_GESTURE);
	}
    return true;
}

bool SensorPollingInit() {
    HMODULE hSensorLib = LoadLibrary(SENSOR_DLL);

    if (hSensorLib == NULL)
        return false;

    pfnHTCSensorOpen = (PFN_HTCSensorOpen)
        GetProcAddress(hSensorLib, L"HTCSensorOpen");
    pfnHTCSensorClose = (PFN_HTCSensorClose)
        GetProcAddress(hSensorLib, L"HTCSensorClose");
    pfnHTCSensorGetDataOutput = (PFN_HTCSensorGetDataOutput)
        GetProcAddress(hSensorLib, L"HTCSensorGetDataOutput");

    if (pfnHTCSensorOpen == NULL ||
        pfnHTCSensorClose == NULL ||
        pfnHTCSensorGetDataOutput == NULL) {
        MessageBox(NULL,
            TEXT("Unable to find entry point"),
            TEXT("Error"), MB_TOPMOST);

		pfnHTCSensorOpen = NULL;
		pfnHTCSensorClose = NULL;
		pfnHTCSensorGetDataOutput = NULL;
        return false;
    }

    g_hSensor = pfnHTCSensorOpen(SENSOR_TILT);

    return true;
}

bool SensorPollingUninit() {
	if(pfnHTCSensorClose)
		pfnHTCSensorClose(g_hSensor);

	return true;
}
