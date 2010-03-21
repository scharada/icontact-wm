#pragma once

#include "stdafx.h"

#include <pimstore.h>

#include "Settings.h"
#include "ListData.h"

#define ADD_NEW_CONTACT_ID 10000

class ListDataPoom : public ListData {
private:
    IPOutlookApp * polApp;
    IFolder * pCurrFldr;
    IPOutlookItemCollection * pItemCol;
	bool _bOnlyFavorites;

    bool InitPoom();
    void ShutdownPoom();

public:

    ListDataPoom(Settings *);
	ListDataPoom(Settings *, bool);
    void Clear(void);
    void Populate(void);
    int PopulateDetailsFor(int);
    void ToggleFavorite(int);
    void DisplayItem(int);
};
