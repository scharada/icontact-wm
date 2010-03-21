#pragma once
#include <phone.h>

#include "iContact.h"
#include "Settings.h"
#include "ListData.h"


class ListDataCallLog : public ListData {
public:
    ListDataCallLog(Settings *);
    void Release(void);
    HRESULT Populate(void);
    HRESULT PopulateDetailsFor(int);
    void GetItemGroup(int, TCHAR *);
    void ToggleFavorite();
    void DisplayItem();
    void EditItem();
};

TCHAR _getPastTime(FILETIME ftTime, TCHAR * pszSecondary, int strLength);
void _printDate(TCHAR * str, int strLength, FILETIME ft);
void _printDuration(TCHAR * str, int strLength, FILETIME ftStart,
                    FILETIME ftEnd, Settings * pSettings);