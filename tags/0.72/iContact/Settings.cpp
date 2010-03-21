#include <stdafx.h>
#include <string.h>
#include "GraphicFunctions.h"

#include "Settings.h"

Settings::Settings(void) {
    this->ini = CSimpleIniW(false, false, false);

    wchar_t szIniPath[MAX_PATH];
    wchar_t szLanguagePath[MAX_PATH];
    wchar_t szThemePath[MAX_PATH];

    // Get program file path
	GetModuleFileName(NULL, szIniPath, MAX_PATH);

    wchar_t * pstr = wcsrchr(szIniPath, '\\');
	if (pstr) *(++pstr) = '\0';

    wcsncpy(szLanguagePath, szIniPath, MAX_PATH);
    wcsncpy(szThemePath, szIniPath, MAX_PATH);

    wcscat(szIniPath, L"settings.ini");

    //////////////////////////////////////////////////
    // Settings
	this->ini.LoadFile(szIniPath);

    this->email_account = ini.GetValue(
        L"main", L"EmailAccount", L"default");

    this->favorite_category = ini.GetValue(
        L"main", L"FavoriteCategory", L"Favorites");

	this->doExitOnAction = 0 == wmemcmp(ini.GetValue(
		L"main", L"ExitOnAction", L"0"), L"1", 1);

	this->doExitOnMinimize = 0 == wmemcmp(ini.GetValue(
		L"main", L"ExitOnMinimize", L"0"), L"1", 1);
    
    wcscat(szLanguagePath, ini.GetValue(
        L"main", L"Language", L"english"));
    wcscat(szLanguagePath, L".lng");

    wcscat(szThemePath, ini.GetValue(
        L"main", L"Skin", L"default"));
    wcscat(szThemePath, L".skn");

    this->skin_name = ini.GetValue(
        L"main", L"Skin", L"default");

    //////////////////////////////////////////////////
    // Language
	this->ini.LoadFile(szLanguagePath);

    // Language
    this->alphabet = ini.GetValue(
        L"language", L"alphabet", L"ABCDEFGHIJKLMNOPQRSTUVWXYZ");

	this->add_new_contact_string = ini.GetValue(
		L"language", L"addNewContact", L"Add new contact");

    this->mobile_string = ini.GetValue(
        L"language", L"mobile", L"Mobile");

    this->home_string = ini.GetValue(
        L"language", L"home", L"Home");

    this->work_string = ini.GetValue(
        L"language", L"work", L"Work");

    this->company_string = ini.GetValue(
        L"language", L"company", L"Company");

    this->email_string = ini.GetValue(
        L"language", L"email", L"E-mail");

    this->sms_string = ini.GetValue(
        L"language", L"sms", L"SMS");

    this->today_string = ini.GetValue(
        L"language", L"today", L"Today");

    this->yesterday_string = ini.GetValue(
        L"language", L"yesterday", L"Yesterday");

    this->sunday_string = ini.GetValue(
        L"language", L"sunday", L"Sunday");

    this->monday_string = ini.GetValue(
        L"language", L"monday", L"Monday");

    this->tuesday_string = ini.GetValue(
        L"language", L"tuesday", L"Tuesday");

    this->wednesday_string = ini.GetValue(
        L"language", L"wednesday", L"Wednesday");

    this->thursday_string = ini.GetValue(
        L"language", L"thursday", L"Thursday");

    this->friday_string = ini.GetValue(
        L"language", L"friday", L"Friday");

    this->saturday_string = ini.GetValue(
        L"language", L"saturday", L"Saturday");

    this->older_string = ini.GetValue(
        L"language", L"older", L"Older");

    this->type_string = ini.GetValue(
        L"language", L"type", L"Type");

    this->number_string = ini.GetValue(
        L"language", L"number", L"Number");

    this->date_string = ini.GetValue(
        L"language", L"date", L"Date");

    this->duration_string = ini.GetValue(
        L"language", L"duration", L"Duration");

    this->seconds_string = ini.GetValue(
        L"language", L"seconds", L"Seconds");

    this->outgoing_string = ini.GetValue(
        L"language", L"outgoing", L"Outgoing");

    this->missed_string = ini.GetValue(
        L"language", L"missed", L"Missed");

    this->incoming_string = ini.GetValue(
        L"language", L"incoming", L"Incoming");

    //////////////////////////////////////////////////
    // Theme
	this->ini.LoadFile(szThemePath);

    // Colors, Brushes & Fonts
    this->rgbTitlebarBackground = this->initColor(
        L"theme", L"TitlebarBackground", L"000000");

    this->rgbTitlebarText = this->initColor(
        L"theme", L"TitlebarText", L"ffffff");

    this->rgbTitlebarSignal = this->initColor(
        L"theme", L"TitlebarSignal", L"ffffff");

    this->hbrListBackground = CreateSolidBrush(this->initColor(
        L"theme", L"ListBackground", L"182633"));

	this->hbrListGroupBackground = CreateSolidBrush(this->initColor(
		L"theme", L"ListGroupBackground", L"000000"));

	this->rgbListGroupText = this->initColor(
		L"theme", L"ListGroupText", L"ffffff");

    this->hbrListItemBackground = CreateSolidBrush(this->initColor(
		L"theme", L"ListItemBackground", L"1e1e1e"));

	this->rgbListItemText = this->initColor(
		L"theme", L"ListItemText", L"dcdcdc");
    this->hbrListItemForeground = CreateSolidBrush(this->rgbListItemText);
    this->hpenListItemForeground = CreatePen(PS_SOLID, 1, this->rgbListItemText);

    this->rgbListItemHoverBackground1 = this->initColor(
		L"theme", L"ListItemHoverBackground1", L"000000");
    
    this->rgbListItemHoverBackground2 = this->initColor(
		L"theme", L"ListItemHoverBackground2", L"282828");

    this->rgbListItemSelectedBackground1 = this->initColor(
        L"theme", L"ListItemSelectedBackground1", L"4b5ab5");

    this->rgbListItemSelectedBackground2 = this->initColor(
        L"theme", L"ListItemSelectedBackground2", L"162793");

	this->rgbListItemSelectedText = this->initColor(
		L"theme", L"ListItemSelectedText", L"e6e6e6");

	this->rgbListItemSelectedShadow = this->initColor(
		L"theme", L"ListItemSelectedShadow", L"505050");

	this->hbrListItemSeparator = CreateSolidBrush(this->initColor(
		L"theme", L"ListItemSeparator", L"323232"));

    this->hbrDetailItemSeparator = CreateSolidBrush(this->initColor(
        L"theme", L"DetailItemSeparator", L"999999"));

	this->rgbListIndicatorText = this->initColor(
		L"theme", L"ListIndicatorText", L"ffffff");

    this->rgbKeyboardText = this->initColor(
		L"theme", L"KeyboardText", L"dcdcdc");

    this->hbrKeyboardBackground = CreateSolidBrush(this->initColor(
        L"theme", L"KeyboardBackground", L"464646"));

    this->hpenKeyboardGrid = CreatePen(PS_SOLID, 1, this->initColor(
		L"theme", L"KeyboardGrid", L"646464"));
}

Settings::~Settings(void) {
    // Deallocate all memory used by the ini object
    this->ini.Reset();
}

int Settings::initColor(wchar_t * wp_section, wchar_t * wp_key,
                        wchar_t * wp_default_hex) {

    const wchar_t * wpColor = ini.GetValue(
        wp_section, wp_key, wp_default_hex);

    return parseColor(wpColor, wp_default_hex);
}

int parseColor(const wchar_t * color_hex, const wchar_t * default_hex) {
    const wchar_t * source = wcslen(color_hex) == 6 
        ? color_hex : default_hex;

    wchar_t red[3];
    wchar_t green[3];
    wchar_t blue[3];

    red[2] = 0;
    green[2] = 0;
    blue[2] = 0;

	wcsncpy(red, source, 2);
	wcsncpy(green, source + 2, 2);
	wcsncpy(blue, source + 4, 2);

	int r = wcstol(red, NULL, 16);
	int g = wcstol(green, NULL, 16);
	int b = wcstol(blue, NULL, 16);

    int rgbcolor = RGB(r, g, b);

	return rgbcolor;
}