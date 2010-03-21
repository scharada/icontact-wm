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
#include "FileUtils.h"

// Thanks l3v5y!
bool GetIDialerFilename(TCHAR * szFilename) {

	HKEY hkey;

    // Where app install data is stored
	LONG result = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
        TEXT("Software\\Apps\\Supware.net iDialer"), 0, 
		KEY_QUERY_VALUE, &hkey);

    // Try to find iDialer from installation in registry
    if (0 == result) {
		DWORD buffersize = MAX_PATH;
		RegQueryValueEx(hkey, TEXT("InstallDir"), NULL, NULL,
            (PBYTE)szFilename, &buffersize);
		RegCloseKey(hkey);
	}

    // Otherwise, assume it's stored in the same location
    // (either both in memory or both on storage card)
	else {
        // generate path name for iContact
        ::GetModuleFileName(NULL, szFilename, MAX_PATH);
        TCHAR * pstr = _tcsrchr(szFilename, '\\');
        if (pstr) *(pstr) = '\0';

        // Go "up 1" directory. We should then be in \Program Files
        pstr = _tcsrchr(szFilename, '\\');
        if (pstr) *(++pstr) = '\0';

        // Concatenate iDialer program directory
        StringCchCat(szFilename, MAX_PATH, TEXT("iDialer"));
	}

    // Concatenate iDialer exe path
	StringCchCat(szFilename, MAX_PATH, TEXT("\\iDialer.exe"));

    // test if file exists
    HANDLE h = CreateFile(szFilename, GENERIC_READ,
        FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, NULL, NULL);
    bool exists = h != INVALID_HANDLE_VALUE;
    if (exists)
        CloseHandle(h);

    return exists;
}

void GetCurDirFilename(TCHAR * szFilename, const TCHAR * filename, 
                       const TCHAR * extension) {

    ::GetModuleFileName(NULL, szFilename, MAX_PATH);
    TCHAR * pstr = _tcsrchr(szFilename, '\\');
    if (pstr) *(++pstr) = '\0';
    StringCchCat(szFilename, MAX_PATH, filename);
    if (extension != NULL) {
        StringCchCat(szFilename, MAX_PATH, TEXT("."));
        StringCchCat(szFilename, MAX_PATH, extension);
    }
}