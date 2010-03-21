#include "SimpleIni.h"
#include "stdafx.h"

#pragma once

int parseColor(const wchar_t * color_hex, const wchar_t * default_hex);

class Settings {
private:
    CSimpleIniW ini;

    int initColor(wchar_t *, wchar_t *, wchar_t *);

public:
    Settings(void);
    ~Settings(void);

    // POOM settings
    const wchar_t * favorite_category;
    const wchar_t * email_account;

    // Strings
    const wchar_t * alphabet;
	const wchar_t * add_new_contact_string;
    const wchar_t * mobile_string;
    const wchar_t * home_string;
    const wchar_t * work_string;
    const wchar_t * company_string;
    const wchar_t * email_string;
    const wchar_t * sms_string;
    const wchar_t * today_string;
    const wchar_t * yesterday_string;
    const wchar_t * sunday_string;
    const wchar_t * monday_string;
    const wchar_t * tuesday_string;
    const wchar_t * wednesday_string;
    const wchar_t * thursday_string;
    const wchar_t * friday_string;
    const wchar_t * saturday_string;
    const wchar_t * older_string;
    const wchar_t * type_string;
    const wchar_t * number_string;
    const wchar_t * date_string;
    const wchar_t * duration_string;
    const wchar_t * seconds_string;
    const wchar_t * outgoing_string;
    const wchar_t * missed_string;
    const wchar_t * incoming_string;

	bool doExitOnAction;
	bool doExitOnMinimize;

    const wchar_t * skin_name;

    // Theme
    COLORREF rgbTitlebarBackground;
    COLORREF rgbTitlebarText;
    COLORREF rgbTitlebarSignal;
    HBRUSH hbrListBackground;
    HBRUSH hbrListGroupBackground;
    COLORREF rgbListGroupText;
    HBRUSH hbrListItemBackground;
    COLORREF rgbListItemHoverBackground1;
    COLORREF rgbListItemHoverBackground2;
	COLORREF rgbListItemText;
    HBRUSH hbrListItemForeground;
    HPEN hpenListItemForeground;
	COLORREF rgbListItemSelectedBackground1;
	COLORREF rgbListItemSelectedBackground2;
    COLORREF rgbListItemSelectedText;
	COLORREF rgbListItemSelectedShadow;
    HBRUSH hbrListItemSeparator;
    HBRUSH hbrDetailItemSeparator;
	COLORREF rgbListIndicatorText;
	COLORREF rgbKeyboardText;
    HBRUSH hbrKeyboardBackground;
    HPEN hpenKeyboardGrid;
};
