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

// portions of this file were originally from Google Gears
// (http://gears.googlecode.com)
// which is licensed as follows:
//
// Copyright 2008, Google Inc.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//  1. Redistributions of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//  2. Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//  3. Neither the name of Google Inc. nor the names of its contributors may be
//     used to endorse or promote products derived from this software without
//     specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
// WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
// EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
// OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
// ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//

#include "stdafx.h"

#include "FileUtils.h"
#include "ShortcutUtils.h"
#include "ShortcutUtilsData.h"

static bool CreateShellLink(const TCHAR * szLinkPath,
                            const TCHAR * szIconPath,
                            const TCHAR * szObjectPath,
                            const TCHAR * szArguments) {

    // IShellLink does not exist on WinCE and the alternative (SHCreateShortcut)
    // does not allow us to set a custom icon. So we write the shortcut by hand.
    // The format is ...
    // <n>#"<target>" [[-]<argument>][?<module>,<icon index>]
    // where ...
    // <> = variable
    // [] = optional
    // n = number of characters after the hash, up to and including the last comma
    // eg. 63#\Windows\iexplore.exe -http://www.google.com?\Windows\test.exe,101
    if (!szObjectPath) {
        return false;
    }

    TCHAR buffer[MAX_PATH];
    StringCchPrintf(buffer, MAX_PATH, TEXT("\"%s\""), szObjectPath);

    if (szArguments) {
        // It seems that the system splits the argument from the module at the first
        // occurence of '?'. So if the supplied argument contains a '?', the parsing
        // will be incorrect. This allows exploits where the system is forced to try
        // to load an icon from a location other than that in szIconPath. (Note that
        // in the case where szArguments is a URL, this also prevents the use of query
        // params.)
        //
        // For this reason, we terminate szArguments just before the occurence of the
        // first '?'.
        int num_characters = _tcscspn(szArguments, TEXT("?"));
        if (num_characters > 0) {
            StringCchCat(buffer, MAX_PATH, TEXT(" "));
            StringCchCatN(buffer, MAX_PATH, szArguments, num_characters);
        }
    }
    if (szIconPath) {
        StringCchCat(buffer, MAX_PATH, TEXT("?"));
        StringCchCat(buffer, MAX_PATH, szIconPath);
        StringCchCat(buffer, MAX_PATH, TEXT(","));
    }

    TCHAR shortcut[MAX_PATH];
    StringCchPrintf(shortcut, MAX_PATH, TEXT("%d#%s"),
        _tcslen(buffer), buffer);

    if (szIconPath) {
        StringCchPrintf(buffer, MAX_PATH, TEXT("%d"), DLL_ICON_ID);
        StringCchCat(shortcut, MAX_PATH, buffer);
    }

    // actually write the file
    HANDLE hShortcut = CreateFile(szLinkPath, GENERIC_READ, 0, NULL, 
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    
    if (hShortcut == INVALID_HANDLE_VALUE)
        return false;

    DWORD dwNumberOfBytesWritten = 0;
    bool success = 0 != WriteFile(hShortcut, shortcut, _tcslen(shortcut),
        &dwNumberOfBytesWritten, NULL);

    CloseHandle(hShortcut);
    return success;
}

bool CreateShortcutFile(const TCHAR * szShortcutTitle,
                        const TCHAR * szObjectPath,
                        const TCHAR * szArguments,
                        const TCHAR * szIconsPath) {

    TCHAR szLinkPath[MAX_PATH] = {0};
    if (!SHGetSpecialFolderPath(NULL, szLinkPath, CSIDL_PROGRAMS, FALSE))
        return false;

    StringCchCat(szLinkPath, MAX_PATH, TEXT("\\"));
    StringCchCat(szLinkPath, MAX_PATH, szShortcutTitle);
    StringCchCat(szLinkPath, MAX_PATH, TEXT(".lnk"));

    // Return immediately if shortcut already exists.
    if (FileExists(szLinkPath))
        return true;

    return CreateShellLink(szLinkPath, szIconsPath, szObjectPath, szArguments);
}
