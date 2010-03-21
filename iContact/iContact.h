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

#define SZ_APP_NAME                 TEXT("iContact")
#define SZ_ABOUT                    TEXT("iContact v0.71 - supware.net")
#define ABOUT_LENGTH                28
#define DEFAULT_ITEM_HEIGHT			40
#define DEFAULT_GROUP_HEIGHT        17
#define MENU_BAR_HEIGHT             40
#define MENU_BAR_Y_OFFSET           16
#define MENU_BAR_SELECTED_Y_OFFSET  56

#define IDT_TIMER_SCROLL			10010
#define IDT_TIMER_BOUNCE			10011
#define IDT_TIMER_SCROLL_TO			10012
#define IDT_TIMER_EXPAND_DETAILS    10013
#define IDT_TIMER_KEYBOARD_SWITCH	10014

#define KEYBOARD_SCROLL_PERIOD      50
#define EXPAND_DETAILS_PERIOD       200
#define	KEYBOARD_FONT_SIZE          40

#define LIST_DISPLAY_LIST			10
#define LIST_DISPLAY_DETAILS		20
#define LIST_DISPLAY_KEYBOARD		30
#define LIST_DISPLAY_ADDFAVORITES	40

#define MAX_LOADSTRING              100

#define	ITEM_FONT_SIZE              20
#define	ITEM_SECONDARY_FONT_SIZE    12
#define	LIST_ITEM_INDENT            8
#define	ITEM_DETAILS_FONT_SIZE      12
#define	GROUP_ITEM_FONT_SIZE        13
#define	LIST_GROUP_ITEM_INDENT      14
#define	LIST_SEPARATOR_HEIGHT       1
#define	LIST_INDICATOR_FONT_SIZE    55
#define	FRICTION                    30
#define	SPEED_MULTIPLIER            30
#define	REFRESH_RATE                22

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
void InitRects(HWND);

void DrawListAt(HDC, HDC, RECT);
void DrawGroupHeaderAt(HDC, Data, RECT);
void DrawItemDetailsAt(HDC, Data, RECT);
void DrawItemSelectedAt(HDC hdc, Data, RECT);
void DrawItemHoverAt(HDC, Data, RECT);
void DrawItemAt(HDC, Data, RECT);
void DrawKeyboardAt(HDC, RECT);

void CalculateHeights();
void RedrawList(HWND);
void MoveList(HWND, double);
void ScrollTo (HWND, int, int);
void StopScrolling(HWND);
void ExpandDetails(HWND);
void SwitchKeyboard(HWND, int, int);
void SwitchTab(HWND, int);