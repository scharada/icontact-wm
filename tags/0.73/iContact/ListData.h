#pragma once

#include "Settings.h"

#define MAX_LIST_ITEMS 2000
#define MAX_SUBLIST_ITEMS 10
#define MAX_GROUP_ITEMS 30

#define PRIMARY_TEXT_LENGTH 64
#define SECONDARY_TEXT_LENGTH 16

#define SLA_TEXT		00
#define SLA_CALL		10
#define SLA_SMS			20
#define SLA_EMAIL		30

/// List DATA
struct Data {
	int		    ID;
	LONG    	oId;
	TCHAR       szPrimaryText[PRIMARY_TEXT_LENGTH];
	TCHAR		szSecondaryText[10];
    TCHAR       wcGroup;
	int		    nPrimaryTextLength;
	int			nSecondaryTextLength;
    bool        isFavorite;
};

struct DataDetail {
	int		    id;
	TCHAR       text[PRIMARY_TEXT_LENGTH];
	TCHAR	    label[SECONDARY_TEXT_LENGTH];
	int		    action;
	bool	    canSms;
};

class ListData {
protected:
    Settings * _settings;

    Data * _items;
    DataDetail * _actionsList; //TODO: change the name of this

    int _arrayLength;
    int _listCounter;
    int _subListCurrentAction;
    int _actionsNumber;
    int _currentGroup;
    bool _sms;
    bool _canFavorite;

public:

    ListData();
    ListData(Settings * pSettings);
    ~ListData(void);
    
    virtual void Init();
    virtual void Clear();
    virtual void Populate();
    
    Data GetItem(int index);
    virtual void GetItemGroup(int index, TCHAR * pszGroup);

    bool CanSms(void);
    void SetSms(bool is);
    bool CanFavorite(void);

    void Select(int index);

    virtual void ToggleFavorite(int index);

    int GetItemCount(void);

    bool AddItem(int id, const TCHAR * tszPrimary, const TCHAR * tszSecondary,
		TCHAR tszGroup, LONG oId);

    virtual void DisplayItem(int index);

    virtual int PopulateDetailsFor(int id);
    bool InitDetailData(void);
    int GetItemDetailCount(void);
    int GetSubListCurrentActionIndex(void);//TODO: change the name of this
    void SetSubListCurrentActionIndex(int index);//TODO: change the name of this
    void SelectPreviousSubListAction(void);//TODO: change the name of this
    void SelectNextSubListAction(void);//TODO: change the name of this
    DataDetail GetSubListCurrentAction(void); //TODO: change the name of this
    DataDetail GetItemDetail(int index);
    bool IsItemNewGroup(int index);
};
