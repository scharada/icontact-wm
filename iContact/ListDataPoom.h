#pragma once

#include "stdafx.h"
#include "macros.h"

#include <pimstore.h>

#include "Settings.h"
#include "ListData.h"

class ListDataPoom : public ListData {
private:
    IPOutlookApp2 * polApp;
    IFolder * pCurrFldr;
    IPOutlookItemCollection * pItemCol;
	bool _bOnlyFavorites;

    void _construct(Settings *, bool);

    HRESULT _initPoom();

    HRESULT _loadBitmap(int size);

public:

    ListDataPoom(Settings *);
	ListDataPoom(Settings *, bool);

    void Release(void);
    HRESULT Populate(void);
    HRESULT PopulateDetailsFor(int);

    void ToggleFavorite();
    void AddItem();
    HRESULT DisplayItem();
    void EditItem();
};
