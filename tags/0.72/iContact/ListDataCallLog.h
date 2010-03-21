#pragma once
#include <phone.h>

#include "iContact.h"
#include "Settings.h"
#include "ListData.h"


class ListDataCallLog : public ListData {
public:
    ListDataCallLog(Settings *);
    void Clear(void);
    void Populate(void);
    int PopulateDetailsFor(int);
    void GetItemGroup(int, wchar_t *);
    void ToggleFavorite(int);
    void DisplayItem(int);
};
