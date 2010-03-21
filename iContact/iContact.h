#pragma once
#include "resourceppc.h"
#include "ListData.h"

// Returns number of elements
#define dim(x) (sizeof(x) / sizeof(x[0]))   

//----------------------------------------------------------------------
// Generic defines and data types
//
struct decodeUINT {                             // Structure associates
    UINT Code;                                  // messages 
                                                // with a function. 
    LRESULT (*Fxn)(HWND, UINT, WPARAM, LPARAM);
}; 

enum TransitionType {
    ttSlideLeft,
    ttSlideRight,
    ttKeyboardExpand,
    ttKeyboardShrink,
};

enum ScreenType {
    stList,
    stDetails,
};

enum PopupType {
    ptKeyboard,
};

#define SZ_APP_NAME                 TEXT("iContact")
#define SZ_ABOUT                    TEXT("iContact v0.74 - supware.net")
#define ABOUT_LENGTH                28
#define DEFAULT_ITEM_HEIGHT			36
#define DEFAULT_GROUP_HEIGHT        17
#define HEADER_HEIGHT               24
#define HEADER_CLICK_HEIGHT         40
#define HEADER_Y_OFFSET             16
#define MENU_BAR_HEIGHT             40
#define MENU_BAR_Y_OFFSET           40
#define MENU_BAR_SELECTED_Y_OFFSET  80

#define IDT_TIMER_SCROLL			10010
#define IDT_TIMER_SCROLL_TO			10012
#define IDT_TIMER_TRANSITION        10015
#define IDT_TIMER_LOADLIST          10016

#define SCROLL_TO_PERIOD            300
#define EXPAND_DETAILS_PERIOD       300
#define TRANSITION_PERIOD           300
#define EXPAND_KEYBOARD_PERIOD      300
#define SHRINK_KEYBOARD_PERIOD      200
#define ALPHABET_MAX_SIZE           128

#define LIST_DISPLAY_LIST			10
#define LIST_DISPLAY_DETAILS		20
#define LIST_DISPLAY_KEYBOARD		30
#define LIST_DISPLAY_ADDFAVORITES	40

#define MAX_LOADSTRING              100

#define	ITEM_FONT_SIZE              20
#define	ITEM_SECONDARY_FONT_SIZE    12
#define	KEYBOARD_FONT_SIZE          40
#define	ITEM_DETAILS_FONT_SIZE      12
#define	GROUP_ITEM_FONT_SIZE        13
#define	LIST_INDICATOR_FONT_SIZE    55

#define	LIST_ITEM_INDENT            8
#define	LIST_GROUP_ITEM_INDENT      14
#define	LIST_SEPARATOR_HEIGHT       1
#define	REFRESH_RATE                11

#define FRICTION_COEFF              0.001

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
LRESULT DoLButtonDown(HWND, UINT, WPARAM, LPARAM);
LRESULT DoMouseMove(HWND, UINT, WPARAM, LPARAM);
LRESULT DoLButtonUp(HWND, UINT, WPARAM, LPARAM);
LRESULT DoTimer(HWND, UINT, WPARAM, LPARAM);
LRESULT DoKeyDown(HWND, UINT, WPARAM, LPARAM);
LRESULT DoDestroyMain(HWND, UINT, WPARAM, LPARAM);

void InitSurface(HWND);

void DrawScreenOn(HDC, ScreenType, HDC, RECT, int);
void DrawListOn(HDC, HDC, RECT, int);
void DrawGroupHeaderOn(HDC, int, RECT);
void DrawItemDetailsOn(HDC, Data, int);
void DrawItemSelectedOn(HDC hdc, Data, RECT);
void DrawItemHoverOn(HDC, Data, RECT);
void DrawItemOn(HDC, Data, RECT);
void DrawKeyboardOn(HDC, RECT);
void DrawHeaderOn(HDC, ScreenType, RECT, HDC);

void CalculateHeights();
int GetPixelToItem(int);
void ScrollBar(int);
void ScrollTo (HWND, int, int = SCROLL_TO_PERIOD);
void StartTransition(HWND, TransitionType, int = TRANSITION_PERIOD);
void SwitchTab(HWND, int);