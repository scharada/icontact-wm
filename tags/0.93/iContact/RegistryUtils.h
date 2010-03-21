
#include "stdafx.h"

void SaveSetting(const TCHAR * lpSubKey, const TCHAR * szValue,
                 const TCHAR * szKeyName);

void LoadSetting(TCHAR * szValue, int cchValue, const TCHAR * lpSubKey,
                 const TCHAR * szKeyName, const TCHAR * szDefault = NULL);