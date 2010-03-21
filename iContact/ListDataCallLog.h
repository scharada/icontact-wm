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
    void AddToFavorites(void);
    void RemoveFromFavorites(void);
    void DisplayItem(int);
};
