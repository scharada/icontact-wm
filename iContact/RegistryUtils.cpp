
#include "stdafx.h"
#include "RegistryUtils.h"

void SaveSetting(const TCHAR * lpSubKey, const TCHAR * szValue,
                 const TCHAR * szKeyName) {

    HKEY  hkey = 0;
    DWORD dwDisposition = 0;
    DWORD dwType = REG_SZ;
    DWORD dwSize = 0;

    // create (or open) the specified registry key
    LONG result = RegCreateKeyEx(HKEY_CURRENT_USER, lpSubKey, 
        0, NULL, 0, 0, NULL, &hkey, &dwDisposition);

    if (result != ERROR_SUCCESS)
        return;

    dwSize = (_tcslen(szValue) + 1) * sizeof(TCHAR);
    result = RegSetValueEx(hkey, szKeyName, NULL, dwType, (PBYTE)szValue, dwSize);

    if (hkey != NULL)
        RegCloseKey(hkey);
}

void SaveSetting(const TCHAR * lpSubKey, DWORD dwValue,
                 const TCHAR * szKeyName) {
    HKEY  hkey = 0;
    DWORD dwDisposition = 0;
    DWORD dwType = REG_DWORD;
    DWORD dwSize = 0;

    // create (or open) the specified registry key
    LONG result = RegCreateKeyEx(HKEY_CURRENT_USER, lpSubKey, 
        0, NULL, 0, 0, NULL, &hkey, &dwDisposition);

    if (result != ERROR_SUCCESS)
        return;

    dwSize = sizeof(DWORD);
    result = RegSetValueEx(hkey, szKeyName, NULL, dwType, (PBYTE)&dwValue, dwSize);

    if (hkey != NULL)
        RegCloseKey(hkey);
}

void LoadSetting(TCHAR * szValue, int cchValue, const TCHAR * lpSubKey,
                 const TCHAR * szKeyName, const TCHAR * szDefault) {

    HKEY  hkey = 0;
    DWORD dwDisposition = 0;
    DWORD dwType = REG_SZ;
    DWORD dwSize = cchValue;
    szValue[0] = 0;

    LONG result = RegOpenKeyEx(HKEY_CURRENT_USER, lpSubKey, 0, 0, &hkey);

    // could not open key
    if (result != ERROR_SUCCESS) {

        // no default value exists
        if (!szDefault)
            return;

        // try to create the key instead
        result = RegCreateKeyEx(HKEY_CURRENT_USER, lpSubKey, 
            0, NULL, 0, 0, NULL, &hkey, &dwDisposition);

        // also couldn't create key, just use default
        if (result != ERROR_SUCCESS) {
            StringCchCopy(szValue, cchValue, szDefault);
            return;
        }
    
        // opened key (also creating it)
        if (dwDisposition == REG_CREATED_NEW_KEY) {
            RegCloseKey(hkey);
            SaveSetting(lpSubKey, szDefault, szKeyName);
            StringCchCopy(szValue, cchValue, szDefault);
            return;
        }

    }

    // load the value
    result = RegQueryValueEx(hkey, szKeyName, NULL, &dwType,
        (PBYTE)szValue, &dwSize);
    if (result != ERROR_SUCCESS) {
        RegCloseKey(hkey);
        if (szDefault) {
            SaveSetting(lpSubKey, szDefault, szKeyName);
            StringCchCopy(szValue, cchValue, szDefault);
        }
        return;
    }

    RegCloseKey(hkey);
}