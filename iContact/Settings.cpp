#include <stdafx.h>
#include <string.h>
#include "GraphicFunctions.h"

#include "Settings.h"


Settings::Settings(void) {
    this->ini = CSimpleIniW(false, false, false);

    TCHAR szIniPath[MAX_PATH];
    TCHAR szLanguagePath[MAX_PATH];
    TCHAR szThemePath[MAX_PATH];

    // Get program file path
	GetModuleFileName(NULL, szIniPath, MAX_PATH);

    TCHAR * pstr = _tcsrchr(szIniPath, '\\');
	if (pstr) *(++pstr) = '\0';

    StringCchCopy(szLanguagePath, MAX_PATH, szIniPath);
    StringCchCopy(szThemePath, MAX_PATH, szIniPath);

    StringCchCat(szIniPath, MAX_PATH, TEXT("settings.ini"));

    //////////////////////////////////////////////////
    // Settings
	this->ini.LoadFile(szIniPath);

    this->email_account = ini.GetValue(
        MAIN_SECTION, TEXT("EmailAccount"), TEXT(""));

    this->favorite_category = ini.GetValue(
        MAIN_SECTION, TEXT("FavoriteCategory"), TEXT("Favorites"));

	this->doExitOnAction = 0 == wmemcmp(ini.GetValue(
		MAIN_SECTION, TEXT("ExitOnAction"), TEXT("0")), TEXT("1"), 1);

	this->doExitOnMinimize = 0 == wmemcmp(ini.GetValue(
		MAIN_SECTION, TEXT("ExitOnMinimize"), TEXT("0")), TEXT("1"), 1);

    this->doFastGraphics = 0 == wmemcmp(ini.GetValue(
        MAIN_SECTION, TEXT("FastGraphics"), TEXT("0")), TEXT("1"), 1);
    
    StringCchCat(szLanguagePath, MAX_PATH, ini.GetValue(
        MAIN_SECTION, TEXT("Language"), TEXT("english")));
    StringCchCat(szLanguagePath, MAX_PATH, TEXT(".lng"));

    StringCchCat(szThemePath, MAX_PATH, ini.GetValue(
        MAIN_SECTION, TEXT("Skin"), TEXT("default")));
    StringCchCat(szThemePath, MAX_PATH, TEXT(".skn"));

    this->skin_name = ini.GetValue(
        MAIN_SECTION, TEXT("Skin"), TEXT("default"));

    //////////////////////////////////////////////////
    // Language
	this->ini.LoadFile(szLanguagePath);

    // Language
    this->categories_field = ini.GetValue(
        LANGUAGE_SECTION, TEXT("categoriesField"), TEXT("Categories"));

    this->alphabet = ini.GetValue(
        LANGUAGE_SECTION, TEXT("alphabet"), TEXT(""));

    this->mobile_string = ini.GetValue(
        LANGUAGE_SECTION, TEXT("mobile"), TEXT("Mobile"));

    this->home_string = ini.GetValue(
        LANGUAGE_SECTION, TEXT("home"), TEXT("Home"));

    this->work_string = ini.GetValue(
        LANGUAGE_SECTION, TEXT("work"), TEXT("Work"));

    this->car_string = ini.GetValue(
        LANGUAGE_SECTION, TEXT("car"), TEXT("Car"));

    this->assistant_string = ini.GetValue(
        LANGUAGE_SECTION, TEXT("assistant"), TEXT("Assistant"));

    this->fax_string = ini.GetValue(
        LANGUAGE_SECTION, TEXT("fax"), TEXT("Fax"));

    this->pager_string = ini.GetValue(
        LANGUAGE_SECTION, TEXT("pager"), TEXT("Pager"));

    this->radio_string = ini.GetValue(
        LANGUAGE_SECTION, TEXT("radio"), TEXT("Radio"));

    this->email_string = ini.GetValue(
        LANGUAGE_SECTION, TEXT("email"), TEXT("E-mail"));

    this->sms_string = ini.GetValue(
        LANGUAGE_SECTION, TEXT("sms"), TEXT("SMS"));

    this->today_string = ini.GetValue(
        LANGUAGE_SECTION, TEXT("today"), TEXT("Today"));

    this->yesterday_string = ini.GetValue(
        LANGUAGE_SECTION, TEXT("yesterday"), TEXT("Yesterday"));

    this->sunday_string = ini.GetValue(
        LANGUAGE_SECTION, TEXT("sunday"), TEXT("Sunday"));

    this->monday_string = ini.GetValue(
        LANGUAGE_SECTION, TEXT("monday"), TEXT("Monday"));

    this->tuesday_string = ini.GetValue(
        LANGUAGE_SECTION, TEXT("tuesday"), TEXT("Tuesday"));

    this->wednesday_string = ini.GetValue(
        LANGUAGE_SECTION, TEXT("wednesday"), TEXT("Wednesday"));

    this->thursday_string = ini.GetValue(
        LANGUAGE_SECTION, TEXT("thursday"), TEXT("Thursday"));

    this->friday_string = ini.GetValue(
        LANGUAGE_SECTION, TEXT("friday"), TEXT("Friday"));

    this->saturday_string = ini.GetValue(
        LANGUAGE_SECTION, TEXT("saturday"), TEXT("Saturday"));

    this->older_string = ini.GetValue(
        LANGUAGE_SECTION, TEXT("older"), TEXT("Older"));

    this->type_string = ini.GetValue(
        LANGUAGE_SECTION, TEXT("type"), TEXT("Type"));

    this->number_string = ini.GetValue(
        LANGUAGE_SECTION, TEXT("number"), TEXT("Number"));

    this->date_string = ini.GetValue(
        LANGUAGE_SECTION, TEXT("date"), TEXT("Date"));

    this->duration_string = ini.GetValue(
        LANGUAGE_SECTION, TEXT("duration"), TEXT("Duration"));

    this->seconds_string = ini.GetValue(
        LANGUAGE_SECTION, TEXT("seconds"), TEXT("Seconds"));

    this->outgoing_string = ini.GetValue(
        LANGUAGE_SECTION, TEXT("outgoing"), TEXT("Outgoing"));

    this->missed_string = ini.GetValue(
        LANGUAGE_SECTION, TEXT("missed"), TEXT("Missed"));

    this->incoming_string = ini.GetValue(
        LANGUAGE_SECTION, TEXT("incoming"), TEXT("Incoming"));

    this->unknown_string = ini.GetValue(
        LANGUAGE_SECTION, TEXT("unknown"), TEXT("Unknown"));

    this->returncall_string = ini.GetValue(
        LANGUAGE_SECTION, TEXT("returncall"), TEXT("Return Call"));

    this->savecontact_string = ini.GetValue(
        LANGUAGE_SECTION, TEXT("savecontact"), TEXT("Save Contact"));
 
    this->showsummary_string = ini.GetValue(
        LANGUAGE_SECTION, TEXT("showsummary"), TEXT("Show Summary"));

    this->editcontact_string = ini.GetValue(
        LANGUAGE_SECTION, TEXT("editcontact"), TEXT("Edit Contact"));

    this->recents_string = ini.GetValue(
        LANGUAGE_SECTION, TEXT("recents"), TEXT("Recents"));

    this->allcontacts_string = ini.GetValue(
        LANGUAGE_SECTION, TEXT("allcontacts"), TEXT("All Contacts"));

    this->details_string = ini.GetValue(
        LANGUAGE_SECTION, TEXT("details"), TEXT("Details"));

    //////////////////////////////////////////////////
    // Theme
	this->ini.LoadFile(szThemePath);

    // Colors, Brushes & Fonts
    this->rgbTitlebarBackground = this->initColor(
        THEME_SECTION, TEXT("TitlebarBackground"), TEXT("000000"));

    this->rgbTitlebarText = this->initColor(
        THEME_SECTION, TEXT("TitlebarText"), TEXT("ffffff"));

    this->rgbTitlebarSignal = this->initColor(
        THEME_SECTION, TEXT("TitlebarSignal"), TEXT("ffffff"));

    this->rgbTitlebarBattery = this->initColor(
        THEME_SECTION, TEXT("TitlebarBattery"), TEXT("ffffff"));

    this->hbrListBackground = CreateSolidBrush(this->initColor(
        THEME_SECTION, TEXT("ListBackground"), TEXT("182633")));

	this->hbrListGroupBackground = CreateSolidBrush(this->initColor(
		THEME_SECTION, TEXT("ListGroupBackground"), TEXT("000000")));

	this->rgbListGroupText = this->initColor(
		THEME_SECTION, TEXT("ListGroupText"), TEXT("ffffff"));

    this->hbrListItemBackground = CreateSolidBrush(this->initColor(
		THEME_SECTION, TEXT("ListItemBackground"), TEXT("1e1e1e")));

	this->rgbListItemText = this->initColor(
		THEME_SECTION, TEXT("ListItemText"), TEXT("dcdcdc"));
    this->hbrListItemForeground = CreateSolidBrush(this->rgbListItemText);
    this->hpenListItemForeground = CreatePen(PS_SOLID, 1, this->rgbListItemText);

    this->rgbListItemSelectedBackground1 = this->initColor(
        THEME_SECTION, TEXT("ListItemSelectedBackground1"), TEXT("4b5ab5"));

    this->rgbListItemSelectedBackground2 = this->initColor(
        THEME_SECTION, TEXT("ListItemSelectedBackground2"), TEXT("162793"));

	this->rgbListItemSelectedText = this->initColor(
		THEME_SECTION, TEXT("ListItemSelectedText"), TEXT("e6e6e6"));

	this->rgbListItemSelectedShadow = this->initColor(
		THEME_SECTION, TEXT("ListItemSelectedShadow"), TEXT("505050"));

	this->hbrListItemSeparator = CreateSolidBrush(this->initColor(
		THEME_SECTION, TEXT("ListItemSeparator"), TEXT("323232")));

    this->hbrDetailItemSeparator = CreateSolidBrush(this->initColor(
        THEME_SECTION, TEXT("DetailItemSeparator"), TEXT("999999")));

    this->rgbKeyboardText = this->initColor(
		THEME_SECTION, TEXT("KeyboardText"), TEXT("dcdcdc"));

    this->hbrKeyboardBackground = CreateSolidBrush(this->initColor(
        THEME_SECTION, TEXT("KeyboardBackground"), TEXT("464646")));

    this->hpenKeyboardGrid = CreatePen(PS_SOLID, 1, this->initColor(
		THEME_SECTION, TEXT("KeyboardGrid"), TEXT("646464")));
}

Settings::~Settings(void) {
    // Deallocate all memory used by the ini object
    this->ini.Reset();
}

int Settings::initColor(TCHAR * wp_section, TCHAR * wp_key,
                        TCHAR * wp_default_hex) {

    const TCHAR * wpColor = ini.GetValue(
        wp_section, wp_key, wp_default_hex);

    return parseColor(wpColor, wp_default_hex);
}

int parseColor(const TCHAR * color_hex, const TCHAR * default_hex) {
    const TCHAR * source = _tcslen(color_hex) == 6 
        ? color_hex : default_hex;

    TCHAR red[3];
    TCHAR green[3];
    TCHAR blue[3];

	StringCchCopy(red, 3, source);
	StringCchCopy(green, 3, source + 2);
	StringCchCopy(blue, 3, source + 4);

	int r = _tcstol(red, NULL, 16);
	int g = _tcstol(green, NULL, 16);
	int b = _tcstol(blue, NULL, 16);

	return RGB(r, g, b);
}