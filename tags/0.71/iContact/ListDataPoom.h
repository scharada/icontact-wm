#pragma once

#include "stdafx.h"

#include <pimstore.h>

#include "Settings.h"
#include "ListData.h"

class ListDataPoom : public ListData {
private:
    IPOutlookApp * polApp;
    IFolder * pCurrFldr;
    IPOutlookItemCollection * pItemCol;
	bool _bOnlyFavorites;

    bool InitPoom();
    bool GetPoomFolder(int nFolder, IFolder ** ppFolder);
    bool SetPoomCategories(LONG oId, BSTR bstrCategories);
    void ShutdownPoom();

public:

    ListDataPoom(Settings *);
	ListDataPoom(Settings *, bool);
    void Clear(void);
    void Populate(void);
    int PopulateDetailsFor(int);
    void AddToFavorites(int);
    void RemoveFromFavorites(int);
    void DisplayItem(int);
};
