/*******************************************************************
This file is part of iContact.

iContact is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

iContact is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with iContact.  If not, see <http://www.gnu.org/licenses/>.
*******************************************************************/

#pragma once
#include "resourceppc.h"

#define MAX_LOADSTRING 100
#define MAX_OPTIONS 32

//----------------------------------------------------------------------
// Generic defines and data types
//
struct decodeUINT {                             // Structure associates
    UINT Code;                                  // messages 
                                                // with a function. 
    LRESULT (*Fxn)(HWND, UINT, WPARAM, LPARAM);
}; 

enum SettingType {
    stList,
    stOnOff,
};

struct SingleSetting {
    const TCHAR * caption;
    const TCHAR * key;
    const TCHAR * def;
    SettingType type;
    int (*filler)(TCHAR [MAX_OPTIONS][MAX_LOADSTRING], TCHAR *);
};

enum TransitionType {
    ttExpand,
    ttContract,
};

enum PopupType {
    ptYesNo,
};

#define SZ_APP_NAME                 TEXT("iContact Settings")
#define MAX_LIST_ITEMS              50

#define SZ_DOWNLOAD_SKINS           TEXT("download more skins...")
#define URL_DOWNLOAD_SKINS          TEXT("http://supware.net/iContact/skins/?app=1")
#define SZ_DOWNLOAD_LANGUAGES       TEXT("download more languages...")
#define URL_DOWNLOAD_LANGUAGES      TEXT("http://supware.net/iContact/language/?app=1")

#define SZ_ON                       TEXT("ON")
#define SZ_OFF                      TEXT("OFF")
#define SZ_AUTO                     TEXT("AUTO")

#define SCROLL_THRESHOLD            5

#define IDT_TIMER_SCROLL			10010
#define IDT_TIMER_SCROLL_TO			10012
#define IDT_TIMER_TRANSITION        10015

#define SCROLL_TO_PERIOD            300
#define EXPAND_DETAILS_PERIOD       300
#define TRANSITION_PERIOD           300
#define EXPAND_PERIOD               300
#define CONTRACT_PERIOD             200

#define	REFRESH_RATE                11
#define FRICTION_COEFF              0.001

#define DEFAULT_ITEM_HEIGHT			36
#define DEFAULT_GROUP_HEIGHT        17
#define HEADER_HEIGHT               24
#define HEADER_CLICK_HEIGHT         40
#define MENU_BAR_HEIGHT             40
#define MENU_BAR_ICON_WIDTH         48

#define TITLEBAR_FONT_SIZE          13
#define	ITEM_FONT_SIZE              20
#define	ITEM_SECONDARY_FONT_SIZE    12

#define	LIST_ITEM_INDENT            8
#define	LIST_SEPARATOR_HEIGHT       1


//----------------------------------------------------------------------
// EDB constants
//
#define DB_VOL_FN TEXT("pim.vol")
#define CATEGORY_DB_NAME TEXT("CategoryMainDB")

#define MAKEPROP(n,t)    ((n<<16)|CEVT_##t)
#define PROPID_CAT_NAME		MAKEPROP(0x02, LPWSTR)


//----------------------------------------------------------------------
// Function prototypes
//
HWND InitInstance(HINSTANCE, LPWSTR, int);
int TermInstance(HINSTANCE, int);

// Window procedures
LRESULT CALLBACK MainWndProc(HWND, UINT, WPARAM, LPARAM);

// Message handlers
LRESULT DoPaintMain(HWND, UINT, WPARAM, LPARAM);
LRESULT DoActivate(HWND, UINT, WPARAM, LPARAM);
LRESULT DoTitlebarCallback (HWND, UINT, WPARAM, LPARAM);
LRESULT DoSize(HWND, UINT, WPARAM, LPARAM);
LRESULT DoMouseDown(HWND, UINT, WPARAM, LPARAM);
LRESULT DoMouseMove(HWND, UINT, WPARAM, LPARAM);
LRESULT DoMouseUp(HWND, UINT, WPARAM, LPARAM);
LRESULT DoTimer(HWND, UINT, WPARAM, LPARAM);
LRESULT DoKeyDown(HWND, UINT, WPARAM, LPARAM);
LRESULT DoDestroyMain(HWND, UINT, WPARAM, LPARAM);

void InitSurface(HWND);

COLORREF GetSkinRGB(int index);
void InitializeSkin(HDC hdc);
void InitializeCanvas();

void DrawCanvasOn(HDC hdc, RECT rect);
void DrawScreenOn(HDC, RECT);
void DrawListDetailsOn(HDC hdc, RECT rect, RECT rClip, const TCHAR * value);
void DrawRect(HDC hdc, LPRECT prc, COLORREF clr);

void ScrollBar(int);
void ScrollTo (HWND, int, int = SCROLL_TO_PERIOD);
void CalculateHeights();
void ScrollTo(HWND hWnd, int position, int duration);
void StartTransition(HWND, TransitionType, int = TRANSITION_PERIOD);
void ExpandIt(HWND hWnd, int index);
HFONT BuildFont(int iFontSize, BOOL bBold, BOOL bItalic);

// load/save
void LoadSettings(void);
void SaveSettings(void);
void LoadDefaults(void);

// generic setting handlers
void DrawOnOff(HDC, RECT, bool);
void DrawValue(HDC, RECT, const TCHAR *);

// setting handlers
int emailAccountFiller(TCHAR [MAX_OPTIONS][MAX_LOADSTRING], TCHAR * value);
int favoriteCategoryFiller(TCHAR [MAX_OPTIONS][MAX_LOADSTRING], TCHAR * value);
HRESULT _initPoom(void);

int skinFiller(TCHAR [MAX_OPTIONS][MAX_LOADSTRING], TCHAR * value);
int languageFiller(TCHAR [MAX_OPTIONS][MAX_LOADSTRING], TCHAR * value);
