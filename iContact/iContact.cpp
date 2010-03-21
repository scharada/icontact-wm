// iContact.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "iContact.h"
#include "Settings.h"
#include "ListData.h"
#include "ListDataCallLog.h"
#include "ListDataPoom.h"
#include "GraphicFunctions.h"
#include "PhoneUtils.h"
#include "Titlebar.h"

#ifdef _DEBUG

#endif

//----------------------------------------------------------------
// Global data
//
const TCHAR szAppName[] = TEXT ("iContact");
HINSTANCE   hInst;                     // Program instance handle

Settings *  pSettings;
ListData *  pListData;

int         nListDisplayType = LIST_DISPLAY_LIST;
int         nCurrentTab = 2;

Data	    dItem;              // temporary storage for data item

int		    GroupWidth = 0;		// Final group width
int		    GroupHeight = 0;	// Final group height

int		    ListHeight = 0;
int		    PixelToItem[MAX_LIST_ITEMS * DEFAULT_ITEM_HEIGHT];
int         StartPosition[MAX_LIST_ITEMS];
int		    ListItemHover = -1;
int		    ListItemSelected = -1;
int		    LastListItemSelected = -1;
bool	    ItemEdit = false;
bool	    ItemBack = false;

// Graphic
RECT		rScreen;
int         nScreenHeight;
RECT        rTitlebar;
RECT		rMenubar;
RECT		rList;

HFONT		PrimaryListFont;
HFONT		SecondaryListFont;
HFONT		GroupFont;
HBITMAP		hbmMem;
HANDLE		hOld;
HBITMAP		hbmTmp;
HDC			hdcSkin;
HBITMAP		hbmSkin;

// Standard Item
#define 	ITEM_FONT_SIZE 20
#define		ITEM_SECONDARY_FONT_SIZE 12
#define 	LIST_ITEM_INDENT 8

// Item Details
#define		ITEM_DETAILS_BUTTON_FONT_SIZE 12
HFONT		ItemDetailsButtonFont;

// Group Header
#define		GROUP_ITEM_FONT_SIZE 13
#define		LIST_GROUP_ITEM_INDENT 14

// Separator
#define		LIST_SEPARATOR_HEIGHT 1

// List Indicator
#define		LIST_INDICATOR_BACKGROUND false
#define		LIST_INDICATOR_FONT_SIZE 80
HFONT		ListIndicatorFont;

// Scrolling
bool		bDragging = false;
bool		bScrolling = false;
bool		bClickable = true;
int			Scrolled = 0;
int			StartX;
int			StartY;
int			LastX;
int			LastY;
int			tStartTime;
int			tEndTime;
double		Speed = 0;
double		Offset = 0;

#define		FRICTION 30
#define		SPEED_MULTIPLIER 30
#define		REFRESH_RATE 22

// Scroll To
double		Scroll_TimeCounter = 0;
double		Scroll_StartPosition = 0;
double		Scroll_Change = 0;
double		Scroll_Duration = 0;

// Indent To
double		Indent_TimeCounter = 0;
double		Indent_StartPosition = 0;
double		Indent_Change = 0;
double		Indent_Duration = 0;
double		Indent;

// Resize To
double		Resize_ItemHeight = 0;
double		Resize_StartHeight = 0;
double		Resize_Change = 0;
double		Resize_Duration = 0;
double		Resize_TimeCounter = 0;
double		LastHeight = 0;
double		LastPosition = 0;
bool		bResizing = false;

bool		Plus_Down = false;



// Message dispatch table for MainWindowProc
const struct decodeUINT MainMessages[] = {
    WM_PAINT, DoPaintMain,
    WM_DESTROY, DoDestroyMain,
    WM_ACTIVATE, DoActivate,
    WM_SIZE, DoSize,
    WM_LBUTTONDOWN, DoLButtonDown,
    WM_MOUSEMOVE, DoMouseMove,
    WM_LBUTTONUP, DoLButtonUp,
    WM_TIMER, DoTimer,
    WM_KEYDOWN, DoKeyDown,
    WM_TITLEBAR, DoTitlebarCallback,
};

//======================================================================
//
// Program entry point
//
int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance,
                    LPWSTR lpCmdLine, int nCmdShow) {
    MSG msg;
    HWND hwndMain;

    // Initialize this instance.
    hwndMain = InitInstance(hInstance, lpCmdLine, nCmdShow);
    if (hwndMain == 0)
        return 0x10;

    // Application message loop
    while (GetMessage (&msg, NULL, 0, 0)) {
        TranslateMessage (&msg);
        DispatchMessage (&msg);
    }
    // Instance cleanup
    return TermInstance (hInstance, msg.wParam);
}
//----------------------------------------------------------------------
// InitInstance - Instance initialization
//
HWND InitInstance (HINSTANCE hInstance, LPWSTR lpCmdLine, int nCmdShow){
    WNDCLASS wc;
    HWND hWnd;

    // Save program instance handle in global variable.
    hInst = hInstance;

#if defined(WIN32_PLATFORM_PSPC) || defined(WIN32_PLATFORM_WFSP)
    // If Windows Mobile, allow only one instance of the application.
    hWnd = FindWindow (szAppName, NULL);
    if (hWnd) {
        SetForegroundWindow ((HWND)(((DWORD)hWnd) | 0x01));    
        return 0;
    }
#endif

    // Register application main window class.
    wc.style = 0;                             // Window style
    wc.lpfnWndProc = MainWndProc;             // Callback function
    wc.cbClsExtra = 0;                        // Extra class data
    wc.cbWndExtra = 0;                        // Extra window data
    wc.hInstance = hInstance;                 // Owner handle
    wc.hIcon = NULL,                          // Application icon
    wc.hCursor = LoadCursor (NULL, IDC_ARROW);// Default cursor
    wc.hbrBackground = (HBRUSH) GetStockObject (BLACK_BRUSH);
    wc.lpszMenuName =  NULL;                  // Menu name
    wc.lpszClassName = szAppName;             // Window class name

    if (RegisterClass (&wc) == 0) return 0;
    // Create main window.
    hWnd = CreateWindowEx (WS_EX_NODRAG,      // Ex Style
                         szAppName,           // Window class
                         TEXT("iContact"),    // Window title
                         WS_VISIBLE,          // Style flags
                         CW_USEDEFAULT,       // x position
                         CW_USEDEFAULT,       // y position
                         CW_USEDEFAULT,       // Initial width
                         CW_USEDEFAULT,       // Initial height
                         NULL,                // Parent
                         NULL,                // Menu, must be null
                         hInstance,           // Application instance
                         NULL);               // Pointer to create
                                              // parameters
    // Return fail code if window not created.
    if (!IsWindow (hWnd)) return 0;

    // Initialize COM libraries (for POOM)
    if (FAILED(CoInitializeEx(NULL, 0)))
        return 0;

    // Additional initialization

    // Initialize titlebar callbacks
    InitTitlebar(hWnd);

    // Create data lists
    pSettings = new Settings();
	pListData = new ListDataPoom(pSettings);

    CalculateHeights();
    InitSurface(hWnd);

	PrimaryListFont = BuildFont(ITEM_FONT_SIZE, FALSE, FALSE);
	SecondaryListFont = BuildFont(ITEM_SECONDARY_FONT_SIZE, TRUE, FALSE);
	ItemDetailsButtonFont = BuildFont(ITEM_DETAILS_BUTTON_FONT_SIZE, FALSE, FALSE);
	GroupFont = BuildFont(GROUP_ITEM_FONT_SIZE, TRUE, FALSE);
	ListIndicatorFont = BuildFont(LIST_INDICATOR_FONT_SIZE, TRUE, FALSE);

    // Standard show and update calls
    ShowWindow (hWnd, nCmdShow);
    UpdateWindow (hWnd);
    return hWnd;
}

//----------------------------------------------------------------------
// TermInstance - Program cleanup
//
int TermInstance (HINSTANCE hInstance, int nDefRC) {

    return nDefRC;
}
//======================================================================
// Message handling procedures for MainWindow
//

//----------------------------------------------------------------------
// MainWndProc - Callback function for application window
//
LRESULT CALLBACK MainWndProc (HWND hWnd, UINT wMsg, WPARAM wParam, 
                              LPARAM lParam) {
    INT i;
    //
    // Search message list to see if we need to handle this
    // message. If in list, call procedure.
    //
    for (i = 0; i < dim(MainMessages); i++) {
        if (wMsg == MainMessages[i].Code)
            return (*MainMessages[i].Fxn)(hWnd, wMsg, wParam, lParam);
    }
    return DefWindowProc (hWnd, wMsg, wParam, lParam);
}

//----------------------------------------------------------------------
// DoPaintMain - Process WM_PAINT message for window.
//
LRESULT DoPaintMain (HWND hWnd, UINT wMsg, WPARAM wParam, 
                     LPARAM lParam) {
    PAINTSTRUCT ps;
    RECT rect;
    HDC hdc;

    GetClientRect (hWnd, &rect);
    hdc = BeginPaint (hWnd, &ps); 

    HDC hdcMem = CreateCompatibleDC(hdc);
    HANDLE hbmMemOld = SelectObject(hdcMem, hbmMem);

    HDC hdcTmp = CreateCompatibleDC(hdc);
    HANDLE hbmTmpOld = SelectObject(hdcTmp, hbmTmp);

	// DRAW LIST
	DrawListAt(hdcMem, hdcTmp, rList);

	// TITLE BAR
	DrawTitlebarOn(hdcMem, rTitlebar, hdcSkin, pSettings->hbrTitlebarBackground);

	// DRAW BOTTOM MENUS
    // draw all unselected buttons
	BitBlt(hdcMem, rMenubar.left, rMenubar.top, 
        rMenubar.right - rMenubar.left, MENU_BAR_HEIGHT, 
        hdcSkin, 0, MENU_BAR_Y_OFFSET, SRCCOPY);
    // draw current selected button
    int x = 48 * nCurrentTab;
    BitBlt(hdcMem, x + rMenubar.left, rMenubar.top, 
        48, MENU_BAR_HEIGHT, 
        hdcSkin, x, MENU_BAR_SELECTED_Y_OFFSET, SRCCOPY);

    // Transfer everything to the actual screen
	BitBlt(hdc, rScreen.left, rScreen.top, rScreen.right - rScreen.left,
        nScreenHeight, hdcMem, rScreen.left, rScreen.top, SRCCOPY);
    
    SelectObject(hdcTmp, hbmTmpOld);
    DeleteDC(hdcTmp);

    SelectObject(hdcMem, hbmMemOld);
    DeleteDC(hdcMem);

    EndPaint (hWnd, &ps); 
    return 0;
}
//----------------------------------------------------------------------
// DoActivate - Process WM_ACTIVATE message for window
//
LRESULT DoActivate (HWND hWnd, UINT wMsg, WPARAM wParam,
                    LPARAM lParam) {

    DWORD dwState;
    RECT rc;

    if (wParam == WA_CLICKACTIVE || wParam == WA_ACTIVE) {
        // To switch to full screen mode, first hide all of the shell parts.
        dwState = (SHFS_HIDETASKBAR 
            | SHFS_HIDESTARTICON 
            | SHFS_HIDESIPBUTTON);

        SHFullScreen(hWnd, dwState);

        // Next resize the main window to the size of the screen.
        SetRect(&rc, 0, 0, GetSystemMetrics(SM_CXSCREEN), 
            GetSystemMetrics(SM_CYSCREEN));
        MoveWindow(hWnd, rc.left, rc.top, rc.right-rc.left, 
            rc.bottom-rc.top, TRUE);

        ShowWindow(hWnd, SW_SHOWNORMAL);
    }

    // The window is being deactivated... restore it to non-fullscreen
    else {
        // To switch to normal mode, first show all of the shell parts.
        dwState = (SHFS_SHOWTASKBAR 
            | SHFS_SHOWSTARTICON 
            | SHFS_SHOWSIPBUTTON);

        SHFullScreen(hWnd, dwState);

        // Next resize the main window to the size of the work area.
        SystemParametersInfo(SPI_GETWORKAREA, 0, &rc, FALSE);

        MoveWindow(hWnd, rc.left, rc.top, rc.right-rc.left,
            rc.bottom-rc.top, TRUE);
    }

    return DefWindowProc (hWnd, wMsg, wParam, lParam);
}

//----------------------------------------------------------------------
// DoTitlebarCallback - Process WM_TITLEBAR message for window
//
LRESULT DoTitlebarCallback (HWND hWnd, UINT wMsg, WPARAM wParam,
                    LPARAM lParam) {
    RefreshTitlebar(lParam);
    InvalidateRect(hWnd, &rTitlebar, true);
    return 0;
}

//----------------------------------------------------------------------
// DoSize - Process WM_SIZE message for window
//
LRESULT DoSize (HWND hWnd, UINT wMsg, WPARAM wParam,
                LPARAM lParam) {

	InitRects(hWnd);
    return DefWindowProc (hWnd, wMsg, wParam, lParam);
}

void InitRects(HWND hWnd) {
	GetClientRect(hWnd, &rScreen);
    nScreenHeight = rScreen.bottom - rScreen.top;

	rTitlebar = rScreen;
	rTitlebar.bottom = rTitlebar.top + TITLE_BAR_HEIGHT;

	rMenubar = rScreen;
	rMenubar.top = rMenubar.bottom - MENU_BAR_HEIGHT;

	rList = rScreen;
	rList.top = rTitlebar.bottom;
	rList.bottom = rMenubar.top;
}

//----------------------------------------------------------------------
// DoLButtonDown - Process WM_LBUTTONDOWN message for window
//
LRESULT DoLButtonDown (HWND hWnd, UINT wMsg, WPARAM wParam,
                       LPARAM lParam) {

    POINT pt;
	pt.x = LOWORD(lParam);
	pt.y = HIWORD(lParam);

	if (PtInRect(&rTitlebar, pt)) {
        ShowWindow(hWnd, SW_MINIMIZE);
	}

	// They clicked in the bottom menus
	else if (PtInRect(&rMenubar, pt)) {
		KillTimer(hWnd, IDT_TIMER_SCROLL);
        Scrolled = 0;
        Speed = 0;
        bScrolling = false;

        if (pt.x < 144) {
            pListData->Clear();
            delete pListData;
            nListDisplayType = LIST_DISPLAY_LIST;
            nCurrentTab = pt.x / 48;
            ListItemSelected = -1;

            pListData = 
                // Favorites
                pt.x < 48 ? (ListData *)new ListDataPoom(pSettings, true)

                // Call Log
                : pt.x < 96 ? (ListData *)new ListDataCallLog(pSettings)

                // Contacts
                : (ListData *)new ListDataPoom(pSettings);

            CalculateHeights();
        }

		else if (pt.x < 192) { // Dialer
            RunDialer();
		}

		else { // Voicemail
            CallVmail();
		}
	}

	else if (nListDisplayType == LIST_DISPLAY_LIST && bClickable) {

		// "scroll bar"
		if (pt.x > rList.right - 20) {
			KillTimer(hWnd, IDT_TIMER_SCROLL);
    	    KillTimer(hWnd, IDT_TIMER_SCROLL_TO);
			bScrolling = true;
			Speed = 20;
			double pct = (double)(pt.y - rList.top) 
                / ((double)rList.bottom - (double)rList.top);
			Scrolled = (int)((ListHeight - rList.bottom + rList.top) * pct);
		}

		else {
			LastX = StartX = pt.x;
			LastY = StartY = pt.y;
			tStartTime = GetTickCount();
			if (bScrolling) {
				KillTimer(hWnd, IDT_TIMER_SCROLL);
        	    KillTimer(hWnd, IDT_TIMER_SCROLL_TO);
				bScrolling = false;
				Speed = 0;
			}
		}
	}

	else if (nListDisplayType == LIST_DISPLAY_DETAILS) {
		if ((pt.y > rList.top) && (pt.y < rList.top + DEFAULT_ITEM_HEIGHT)) {
			if (pt.x <= 40) {
				ItemBack = true;
			}
            else if (pt.x >= rList.right - 40 && pListData->CanFavorite()) {
                Plus_Down = true;
            }
            else {
				ItemEdit = true;
			}
		} 
        else {
            int subListIndex = (pt.y - rList.top) / DEFAULT_ITEM_HEIGHT - 1;
            pListData->SetSubListCurrentActionIndex(subListIndex);
			if (pListData->GetSubListCurrentAction().canSms && pt.x > 200)
				pListData->SetSms(true);
		}
	}
	RedrawList(hWnd);
    return 0;
}

//----------------------------------------------------------------------
// DoMouseMove - Process WM_MOUSEMOVE message for window
//
LRESULT DoMouseMove (HWND hWnd, UINT wMsg, WPARAM wParam,
                     LPARAM lParam) {

    POINT pt;
	pt.x = LOWORD(lParam);
	pt.y = HIWORD(lParam);
	int DeltaX = 0;
	int DeltaY = 0;

	switch (nListDisplayType) {
		case LIST_DISPLAY_LIST:
			if (bScrolling) {
				double pct = (double)(pt.y - rList.top) / ((double)rList.bottom - (double)rList.top);
				Scrolled = (int)((ListHeight - rList.bottom + rList.top) * pct);
				RedrawList(hWnd);
				break;
			}

			DeltaX = pt.x - LastX;
			DeltaY = pt.y - LastY;

			// SCROLL
			bDragging = true;
			if (abs(DeltaY) > 3) {
				MoveList(hWnd, DeltaY);
				LastY = pt.y;
			}

			if (StartY == LastY) {
				bDragging = false;
			}

			LastX = pt.x;
			break;

		case LIST_DISPLAY_DETAILS:
			break;
    }

	return 0;
}

//----------------------------------------------------------------------
// DoLButtonUp - Process WM_LBUTTONUP message for window
//
LRESULT DoLButtonUp (HWND hWnd, UINT wMsg, WPARAM wParam,
                       LPARAM lParam) {

    POINT pt;
	pt.x = LOWORD(lParam);
	pt.y = HIWORD(lParam);
    ItemEdit = false;
	ItemBack = false;
	RedrawList(hWnd);

    switch (nListDisplayType) {
        case LIST_DISPLAY_LIST:
            if (bDragging || Scrolled < -1 
                || (ListHeight > rList.bottom - rList.top 
                && Scrolled > ListHeight - rList.bottom + rList.top + 1)) {

                tEndTime = GetTickCount();
			    Speed = (pt.y - StartY) * SPEED_MULTIPLIER / (tEndTime - tStartTime);

                if (Scrolled < -1) {
                    ScrollTo(hWnd, 1, 300);
                }

                else if (Scrolled > ListHeight - rList.bottom + rList.top + 1) {
                    ScrollTo(hWnd, ListHeight - rList.bottom + rList.top, 300);
                }
                
			    else if (fabs(Speed) > 6) {
				    SetTimer(hWnd, IDT_TIMER_SCROLL, REFRESH_RATE, (TIMERPROC) NULL);
				    bScrolling = true;
			    } 
                else {
				    bScrolling = false;
				    bDragging = false;
			    }
		    } 

            else {
                int pos = pt.y + Scrolled - rList.top;
			    ListItemSelected = PixelToItem[pos];
                LastHeight = DEFAULT_ITEM_HEIGHT;

                if (pListData->PopulateDetailsFor(PixelToItem[pos]))
                    ExpandDetails(hWnd);
		    }

			ListItemHover = -1;
		    RedrawList(hWnd);
	        break;

        case LIST_DISPLAY_DETAILS:
            Plus_Down = false;

            // They clicked in the bottom menus
			if (PtInRect(&rMenubar, pt)) {	
				break;
			}

		    if (pt.y > rList.top && pt.y < rList.top + DEFAULT_ITEM_HEIGHT) {
			    if (pt.x <= 40) {
				    LastListItemSelected = -1;
				    ExpandDetails(hWnd);
                }

                else if (pt.x >= rList.right - 40) {
				    pListData->ToggleFavorite(ListItemSelected);
                }

                else {
                    pListData->DisplayItem(ListItemSelected);
				    ExpandDetails(hWnd);
			    } 
		    }
            else {
			    // HANDLE SUBLIST EVENTS
                int subListIndex = (pt.y - rList.top) / DEFAULT_ITEM_HEIGHT - 1;
			    if (subListIndex == pListData->GetSubListCurrentActionIndex()) {
                    DataDetail sla = pListData->GetSubListCurrentAction();
				    switch (sla.action) {
					    case SLA_CALL:
                            if (pListData->CanSms()) SendSMS(sla.text);
                            else Call(sla.text, sla.text);
						    LastListItemSelected = -1;
						    ExpandDetails(hWnd);
						    nListDisplayType = LIST_DISPLAY_LIST;
						    break;
					    case SLA_EMAIL:
                            SendEMail(pSettings->email_account, sla.text);
						    LastListItemSelected = -1;
						    ExpandDetails(hWnd);
						    nListDisplayType = LIST_DISPLAY_LIST;
						    break;
				    }
			    }
		    }
		    pListData->SetSubListCurrentActionIndex(-1);
		    pListData->SetSms(false);
		    RedrawList(hWnd);
	        break;

        default:
            break;
    }

	return 0;
}

//----------------------------------------------------------------------
// DoTimer - Process WM_TIMER message for window
//
LRESULT DoTimer (HWND hWnd, UINT wMsg, WPARAM wParam,
                       LPARAM lParam) {

	switch (wParam)	{

        ///// TIMER for scrolling
	    case IDT_TIMER_SCROLL:
		    Offset = 0;
		    if (fabs(Speed) <= 0.01) {
			    KillTimer(hWnd, IDT_TIMER_SCROLL);
			    bScrolling = false;
			    Speed = 0;
		    }
            else {
			    bScrolling = true;
			    Speed = Speed - Speed / FRICTION;
			    MoveList(hWnd, Speed);
		    }
		    break;

	    ///// TIMER for bouncing
	    case IDT_TIMER_BOUNCE:
	        if ( fabs(Offset) <= 0.01 ) {
		        KillTimer(hWnd, IDT_TIMER_BOUNCE);
		        Offset = 0;
	        }
            else {
		        Offset = Offset - Offset / FRICTION * 4;
	        }
            break;

	    ///// TIMER for scroll to
	    case IDT_TIMER_SCROLL_TO:
		    Offset = 0;
		    KillTimer(hWnd, IDT_TIMER_SCROLL);
		    if (Scroll_TimeCounter < Scroll_Duration) {
			    double amount;
			    bScrolling = true;

			    // Cubic
			    amount = Scroll_Change
                    * (pow(Scroll_TimeCounter/Scroll_Duration - 1, 3) + 1)
                    + Scroll_StartPosition;
			    Speed = amount;
			    MoveList(hWnd, Scrolled - amount);
			    Scroll_TimeCounter++;
		    }
		    else {
			    bScrolling = false;
			    Speed = 0;
			    KillTimer(hWnd, IDT_TIMER_SCROLL_TO);
		    }
		    break;

	    ///// TIMER for expand details
	    case IDT_TIMER_EXPAND_DETAILS:
		    if (Resize_TimeCounter < Resize_Duration) {
			    bClickable = false;
			    double amount; 

			    // Cubic
			    Resize_ItemHeight = Resize_Change
                    * (pow(Resize_TimeCounter/Resize_Duration - 1, 3) + 1)
                    + Resize_StartHeight;
			    CalculateHeights();
			    Resize_TimeCounter++;

			    amount = Scroll_Change 
                    * (pow(Scroll_TimeCounter/Scroll_Duration - 1, 3) + 1)
                    + Scroll_StartPosition;
			    Offset = Scrolled - amount;
			    Scroll_TimeCounter++;

			    Indent = Indent_Change 
                    * (pow(Indent_TimeCounter/Indent_Duration - 1, 3) + 1) 
                    + Indent_StartPosition;
			    Indent_TimeCounter++;
		    }
		    else {
			    bClickable = true;
			    KillTimer(hWnd, IDT_TIMER_EXPAND_DETAILS);
			    if (nListDisplayType == LIST_DISPLAY_LIST)
				    ListItemSelected = LastListItemSelected;
		    }
            break;

	} 

    RedrawList(hWnd);
	return 0;
}

//----------------------------------------------------------------------
// DoKeyDown - Process WM_KEYDOWN message for window
//
LRESULT DoKeyDown (HWND hWnd, UINT wMsg, WPARAM wParam,
                       LPARAM lParam) {
    int top = 0;
    int bot = 0;
    int rListHeight = rList.bottom - rList.top;

	switch (wParam) {

	    // scroll up
	    case 33:
		    if (nListDisplayType == LIST_DISPLAY_LIST)
			    Scrolled >= 1000
                ? ScrollTo(hWnd, Scrolled - 1000, 1000)
                : ScrollTo(hWnd, -DEFAULT_ITEM_HEIGHT, 1000);
		    break;

	    // scroll down
	    case 34:
		    if (nListDisplayType == LIST_DISPLAY_LIST)
			    Scrolled + 1000 <= ListHeight - rListHeight
                ? ScrollTo(hWnd, Scrolled + 1000, 1000)
                : ScrollTo(hWnd, ListHeight - rListHeight 
                    + DEFAULT_ITEM_HEIGHT, 1000);
		    break;

        // left
	    case 37:
		    if (nListDisplayType == LIST_DISPLAY_DETAILS) {
			    LastListItemSelected = ListItemSelected;
                pListData->SetSubListCurrentActionIndex(-1);
			    ExpandDetails(hWnd);
		    } 
		    break;

	    // right
	    case 39:
		    if (nListDisplayType == LIST_DISPLAY_DETAILS) {
			    if (pListData->GetSubListCurrentActionIndex() > -1 
                    && pListData->GetSubListCurrentAction().canSms) {

                    SendSMS(pListData->GetSubListCurrentAction().text);
				    LastListItemSelected = -1;
				    ExpandDetails(hWnd);
				    nListDisplayType = LIST_DISPLAY_LIST;
				    break;
			    }
		    }
		    break;

	    // up
	    case 38:
            if (nListDisplayType == LIST_DISPLAY_LIST) {
                if (ListItemSelected < 0) {
                    ListItemSelected = PixelToItem[Scrolled + rListHeight];
			    }
                else {
					ListItemSelected = max(0, ListItemSelected - 1);
                }

                // make sure the selected item is visible
                top = StartPosition[ListItemSelected];
                bot = StartPosition[ListItemSelected + 1];

                if (top < Scrolled || bot > Scrolled + rListHeight)
                    ScrollTo(hWnd, max(0, bot - rListHeight), 200);

			    LastHeight = bot - top;
		    }

            else if (nListDisplayType == LIST_DISPLAY_DETAILS) {
                pListData->SelectPreviousSubListAction();
		    }

		    RedrawList(hWnd);
		    break;

	    // down
	    case 40:
		    if (nListDisplayType == LIST_DISPLAY_LIST) {
                if (ListItemSelected < 0) {
                    ListItemSelected = PixelToItem[Scrolled];
			    }
                else {
					ListItemSelected = min(pListData->GetItemCount(), 
                        ListItemSelected + 1);
                }

                // make sure the selected item is visible
                top = StartPosition[ListItemSelected];
                bot = StartPosition[ListItemSelected + 1];

                if (top < Scrolled || bot > Scrolled + rListHeight) 
				    ScrollTo(hWnd, min(top, ListHeight - rListHeight), 200);

			    LastHeight = bot - top;
		    }
            
            else if (nListDisplayType == LIST_DISPLAY_DETAILS) {
                pListData->SelectNextSubListAction();
		    }

            RedrawList(hWnd);
		    break;

	    // enter
	    case 13:
		    if (nListDisplayType == LIST_DISPLAY_LIST) {
			    if (ListItemSelected > -1) {
				    LastListItemSelected = ListItemSelected;
                    pListData->PopulateDetailsFor(LastListItemSelected);
                    RedrawList(hWnd);
                    ExpandDetails(hWnd);
			    }
		    }

            else if (nListDisplayType == LIST_DISPLAY_DETAILS) {
                DataDetail sla = pListData->GetSubListCurrentAction();
			    if (pListData->GetSubListCurrentActionIndex() > -1) {
				    switch (sla.action) {
					    case SLA_CALL:
                            Call(sla.text, sla.text);
						    LastListItemSelected = -1;
                            pListData->SetSubListCurrentActionIndex(-1);
						    ExpandDetails(hWnd);
						    nListDisplayType = LIST_DISPLAY_LIST;
						    break;

					    case SLA_EMAIL:
                            SendEMail(pSettings->email_account, sla.text);
						    LastListItemSelected = -1;
                            pListData->SetSubListCurrentActionIndex(-1);
						    ExpandDetails(hWnd);
						    nListDisplayType = LIST_DISPLAY_LIST;
						    break;
				    }
			    }

                else {
				    LastListItemSelected = ListItemSelected;
                    pListData->PopulateDetailsFor(LastListItemSelected);
                    RedrawList(hWnd);
                    ExpandDetails(hWnd);
			    }

		    }
            break;
	}

    return 0;
}

//----------------------------------------------------------------------
// DoDestroyMain - Process WM_DESTROY message for window
//
LRESULT DoDestroyMain (HWND hWnd, UINT wMsg, WPARAM wParam, 
                       LPARAM lParam) {
   
    // Uninitialize the COM classes
    CoUninitialize();

    // Quit
    PostQuitMessage (0);
    return 0;
}

//----------------------------------------------------------------------
// Miscellaneous Drawing Functions
//
void DrawListAt(HDC hdc, HDC hdcTmp, RECT rList) {
	int sItem, Item;
    sItem = Item = Scrolled < 0 ? 0 : PixelToItem[Scrolled + (int)Offset];

    RECT rItem;
    rItem = rList;

    rItem.bottom = rList.top + StartPosition[Item] - Scrolled + (int)Offset;

	// ******* DRAW LIST BACKGROUND
	FillRect(hdc, &rList, pSettings->hbrListBackground);
	SetBkMode(hdc, TRANSPARENT);

    // ******* DRAW LIST ITEMS
	while (1) {
		dItem = pListData->GetItem(Item);

        // we've reached the last item in the list
		if (dItem.ID < 0)
			break;

        // This item would be off the bottom of the screen
		if (rItem.bottom >= rList.bottom)
			break;

        rItem.top = rItem.bottom;
        rItem.bottom = rItem.top + StartPosition[Item+1] - StartPosition[Item];
        
        if (Item == ListItemSelected && nListDisplayType == LIST_DISPLAY_LIST) {
            DrawItemSelectedAt(hdc, dItem, rItem);
		}
        else if (Item == ListItemSelected && nListDisplayType == LIST_DISPLAY_DETAILS) {
            DrawItemDetailsAt(hdc, dItem, rItem);
		}
        else if (Item == ListItemHover) {
            DrawItemHoverAt(hdc, dItem, rItem);
		}
        else {
            DrawItemAt(hdc, dItem, rItem);
		}

        // ****** Group Header
		if (nListDisplayType == LIST_DISPLAY_LIST
			&& pListData->IsItemNewGroup(Item)
            && dItem.nGroupLength && rItem.top >= rList.top) {

            DrawGroupHeaderAt(hdc, dItem, rItem);
		} 

		// Next Item
		Item++;
	}

    // Special: Draw the group of the list item that's at the top of the list
    if (nListDisplayType != LIST_DISPLAY_DETAILS && Scrolled >= 0) {
        dItem = pListData->GetItem(sItem);
        if (dItem.nGroupLength) {
            RECT rTopGroup = {rList.left, 0, rList.right, DEFAULT_GROUP_HEIGHT};
            DrawGroupHeaderAt(hdcTmp, dItem, rTopGroup);

            int nHeight = DEFAULT_GROUP_HEIGHT;
            int nBottom = nHeight;

            if (pListData->IsItemNewGroup(sItem + 1)) {
                nBottom = min(nBottom, StartPosition[sItem + 1] - Scrolled + (int)Offset);
            }

            // account for the fact that the list
            // doesn't start at the top of the screen
            nBottom += rList.top;

            int nLeft = rList.left;
            int nWidth = rList.right - rList.left;
            int nTop = nBottom - nHeight;

            BltAlpha(hdc, nLeft, nTop, nWidth, nHeight, hdcTmp, 200);
        }
    }

	// Draw list indicator if scrolling quickly
	if (fabs(Speed) > 10) {
		SelectObject(hdc, ListIndicatorFont);
	    SetTextAlign(hdc, TA_CENTER);
		SetTextColor(hdc, pSettings->rgbListIndicatorText);
		SetBkMode(hdc, TRANSPARENT);
	    dItem = pListData->GetItem(sItem);
        ExtTextOut(hdc, (rList.right - rList.left) / 2 + rList.left, 
            rList.top + 10, NULL, NULL, dItem.szGroup, dItem.nGroupLength, 0);
        
	}
}

void DrawGroupHeaderAt(HDC hdc, Data dItem, RECT rItem) {
    RECT rHeader = rItem;
    rHeader.bottom = rHeader.top + DEFAULT_GROUP_HEIGHT;

	// ****** GroupHeader background
	FillRect(hdc, &rHeader, pSettings->hbrListGroupBackground);

    // separator
    RECT rSep = rHeader;
    rSep.top = rHeader.bottom - LIST_SEPARATOR_HEIGHT;
    FillRect(hdc, &rSep, pSettings->hbrListItemSeparator);
	SetTextAlign(hdc, TA_LEFT);

	// ******* Draw Group Header Text
	SelectObject(hdc, GroupFont);
   	SetBkMode(hdc, TRANSPARENT);
	SetTextColor(hdc, pSettings->rgbListGroupText);
	ExtTextOut(hdc, rItem.left + LIST_GROUP_ITEM_INDENT, 
        rHeader.top - 1 + ((DEFAULT_GROUP_HEIGHT - GROUP_ITEM_FONT_SIZE) / 2),
        NULL, NULL, dItem.szGroup, dItem.nGroupLength, 0);
}

void DrawItemSelectedAt(HDC hdc, Data dItem, RECT rItem) {
	// ******* DRAW ITEM BACKGROUND
	DrawGradientGDI(hdc, rItem,
        pSettings->rgbListItemSelectedBackground1,
        pSettings->rgbListItemSelectedBackground2);

	// ****** Draw Item Text
	SelectObject(hdc, PrimaryListFont);
	SetTextAlign(hdc, TA_LEFT);

    // Item Shadow Text
	SetTextColor(hdc, pSettings->rgbListItemSelectedShadow);
	ExtTextOut(hdc, rItem.left + LIST_ITEM_INDENT,
        rItem.bottom - ((DEFAULT_ITEM_HEIGHT + ITEM_FONT_SIZE) / 2),
        ETO_OPAQUE, NULL, dItem.szPrimaryText, dItem.nPrimaryTextLength, 0);

    // Item Text
	SetTextColor(hdc, pSettings->rgbListItemSelectedText);
	ExtTextOut(hdc, rItem.left + LIST_ITEM_INDENT, 
        rItem.bottom - 2 - ((DEFAULT_ITEM_HEIGHT + ITEM_FONT_SIZE) / 2),
        ETO_OPAQUE, NULL, dItem.szPrimaryText, dItem.nPrimaryTextLength, 0);
}

void DrawItemHoverAt(HDC hdc, Data dItem, RECT rItem) {
    // Item Background
	DrawGradientGDI(hdc, rItem, 
        pSettings->rgbListItemHoverBackground1,
        pSettings->rgbListItemHoverBackground2);

    // separator
    RECT rSep = rItem;
    rSep.top = rItem.bottom - LIST_SEPARATOR_HEIGHT;
    FillRect(hdc, &rSep, pSettings->hbrListItemSeparator);

	SelectObject(hdc, PrimaryListFont);
	SetTextAlign(hdc, TA_LEFT);

    // Item Shadow Text
	SetTextColor(hdc, pSettings->rgbListGroupText);
	ExtTextOut(hdc, rItem.left + LIST_ITEM_INDENT,
        rItem.bottom - ((DEFAULT_ITEM_HEIGHT + ITEM_FONT_SIZE) / 2),
        ETO_OPAQUE, NULL, dItem.szPrimaryText, dItem.nPrimaryTextLength, 0);

    // Item Text
    SetTextColor(hdc, pSettings->rgbListItemText);
	ExtTextOut(hdc, rItem.left + LIST_ITEM_INDENT,
        rItem.bottom - 2 - ((DEFAULT_ITEM_HEIGHT + ITEM_FONT_SIZE) / 2),
        ETO_OPAQUE, NULL, dItem.szPrimaryText, dItem.nPrimaryTextLength, 0);

}

void DrawItemAt(HDC hdc, Data dItem, RECT rItem) {
    // Item Background
    FillRect(hdc, &rItem, pSettings->hbrListItemBackground);

    // separator
    RECT rSep = rItem;
    rSep.top = rItem.bottom - LIST_SEPARATOR_HEIGHT;
    FillRect(hdc, &rSep, pSettings->hbrListItemSeparator);
	SetTextAlign(hdc, TA_LEFT);

    // Item Primary Text
	SelectObject(hdc, PrimaryListFont);
	SetTextColor(hdc, pSettings->rgbListItemText);
	ExtTextOut(hdc, rItem.left + LIST_ITEM_INDENT,
        rItem.bottom - 2 - ((DEFAULT_ITEM_HEIGHT + ITEM_FONT_SIZE) / 2),
        ETO_OPAQUE, NULL, dItem.szPrimaryText, dItem.nPrimaryTextLength, 0);

	// Item Secondary Text
	SelectObject(hdc, SecondaryListFont);
	SetTextAlign(hdc, TA_RIGHT);
	SetTextColor(hdc, pSettings->rgbListItemText);
	ExtTextOut(hdc, rItem.right - LIST_ITEM_INDENT,
        rItem.bottom - 2 - ((DEFAULT_ITEM_HEIGHT + ITEM_SECONDARY_FONT_SIZE) / 2),
		ETO_OPAQUE, NULL, dItem.szSecondaryText, dItem.nSecondaryTextLength, 0);

}

void DrawItemDetailsAt(HDC hdc, Data dItem, RECT rItem) {
	// ******* DRAW ITEM BACKGROUND
	DrawGradientGDI(hdc, rItem,
        pSettings->rgbListItemSelectedBackground1,
        pSettings->rgbListItemSelectedBackground2);

	RECT rItem2 = rItem;
	// ****** Edit contact button
	if (ItemEdit) {
		rItem2.left += 40;
        if (pListData->CanFavorite()) 
            rItem2.right -= 40;
		rItem2.bottom = rItem2.top + DEFAULT_ITEM_HEIGHT;
		DrawGradientGDI(hdc, rItem2,
            pSettings->rgbListItemHoverBackground1,
            pSettings->rgbListItemHoverBackground2);
	}

	// ****** Draw Item Text
	rItem.left += LIST_ITEM_INDENT;
	SelectObject(hdc, PrimaryListFont);
	SetTextAlign(hdc, TA_LEFT);

	SetTextColor(hdc, pSettings->rgbListItemSelectedShadow);
	ExtTextOut(hdc, rItem.left + (int)Indent,
        (int)(rItem.top + 1 + ((LastHeight - ITEM_FONT_SIZE) / 2)),
        ETO_OPAQUE, NULL, dItem.szPrimaryText, dItem.nPrimaryTextLength, 0);
	
	SetTextColor(hdc, pSettings->rgbListItemSelectedText);
	ExtTextOut(hdc, rItem.left + (int)Indent, 
        (int)(rItem.top + ((LastHeight - ITEM_FONT_SIZE) / 2)),
        ETO_OPAQUE, NULL, dItem.szPrimaryText, dItem.nPrimaryTextLength, 0);


    // DRAW ITEM DETAILS **************	
	int ButtonOffset = DEFAULT_ITEM_HEIGHT;
	RECT rButton = rItem;

    int iItemCount = pListData->GetItemDetailCount();
    int iCurrentIndex = pListData->GetSubListCurrentActionIndex();

	for (int c = 0; c < iItemCount; c++) {
        DataDetail currentAction = pListData->GetItemDetail(c);

        // draw separator
		rButton.top += ButtonOffset;
		rButton.bottom = rButton.top + 1;
		rButton.left = 0;
		FillRect(hdc, &rButton, pSettings->hbrDetailItemSeparator);

		rButton.top = rButton.bottom;
		rButton.bottom = rButton.top + DEFAULT_ITEM_HEIGHT;

		if (c == iCurrentIndex) {
			if(!pListData->CanSms() && (currentAction.action == SLA_CALL
                || currentAction.action == SLA_EMAIL)) {
				DrawGradientGDI(hdc, rButton,
                    pSettings->rgbListItemHoverBackground1,
                    pSettings->rgbListItemHoverBackground2);
            }
		}

		rButton.left = 5;

		rButton.right = rButton.left + 44;

		SelectObject(hdc, ItemDetailsButtonFont);
		DrawText(hdc, currentAction.label, -1,
            &rButton, DT_RIGHT | DT_VCENTER);

		rButton.right = rItem.right;
		rButton.bottom = rButton.top + 20;

		SelectObject(hdc, PrimaryListFont);
		SetTextColor(hdc, pSettings->rgbListItemSelectedShadow);
		ExtTextOut(hdc, rButton.left + 52,
            rButton.top + 1 + (ITEM_FONT_SIZE) / 2,
            ETO_OPAQUE, NULL, currentAction.text,
            wcslen(currentAction.text), 0);

		SetTextColor(hdc, pSettings->rgbListItemSelectedText);
		ExtTextOut(hdc, rButton.left + 52,
            rButton.top + (ITEM_FONT_SIZE) / 2,
            ETO_OPAQUE, NULL, currentAction.text,
            wcslen(currentAction.text), 0);

		if (currentAction.canSms) {
			rButton.bottom = rButton.top + 40;
			if (currentAction.canSms && c == iCurrentIndex) {
				int old = rButton.left;
				rButton.left = rButton.right - 40;
				DrawGradientGDI(hdc, rButton,
                    pSettings->rgbListItemHoverBackground1,
                    pSettings->rgbListItemHoverBackground2);
				rButton.left = old;
			}
			rButton.right -= 10;
			SelectObject(hdc, ItemDetailsButtonFont);
			DrawText(hdc, pSettings->sms_string, 3,
                &rButton, DT_RIGHT | DT_VCENTER);
			rButton.right += 10;
		}
	}

    // draw the < "Back" triangle
    // TODO: change this to a nice picture
	if (ItemBack == true) {
		rItem.left = 0;
		rItem2.right = rItem2.left + 40;
		rItem2.bottom = rItem2.top + 40;
		DrawGradientGDI(hdc, rItem2,
            pSettings->rgbListItemHoverBackground1,
            pSettings->rgbListItemHoverBackground2);
	}

	SelectObject(hdc, pSettings->hbrListItemForeground);
	SelectObject(hdc, pSettings->hpenListItemForeground);
	POINT rectPoints[3];
	rectPoints[0].x = (LONG)Indent - 27;
	rectPoints[0].y = rItem.top + 20;
	rectPoints[1].x = (LONG)Indent - 15;
	rectPoints[1].y = rItem.top + 13;
	rectPoints[2].x = (LONG)Indent - 15;
	rectPoints[2].y = rItem.top + 27;
	Polygon(hdc, rectPoints, 3);

    // draw the + "favorite" symbol
    // TODO: change this to a nice picture
    if (pListData->CanFavorite()) {
        if (Plus_Down) {
            rItem2.top = rItem.top;
            rItem2.bottom = rItem2.top + 40;
            rItem2.right = rItem.right;
            rItem2.left = rItem2.right - 40;
		    DrawGradientGDI(hdc, rItem2,
                pSettings->rgbListItemHoverBackground1,
                pSettings->rgbListItemHoverBackground2);
        }
	    RECT plus;
	    plus.left = rItem.right - 30;
	    plus.right = plus.left + 20;
	    plus.top = rItem.top + 18;
	    plus.bottom = plus.top + 4;
	    FillRect(hdc, &plus, pSettings->hbrListItemForeground);							
	    if (!dItem.isFavorite) {
		    plus.left = rItem.right - 22;
		    plus.right = plus.left + 4;
		    plus.top = rItem.top + 10;
		    plus.bottom = plus.top + 20;
		    FillRect(hdc, &plus, pSettings->hbrListItemForeground);
	    }
    }

}

//----------------------------------------------------------------------
// Utility functions
//

void RedrawList(HWND hWnd) {
	InvalidateRect(hWnd, &rScreen, FALSE);
	UpdateWindow(hWnd);
}

void InitSurface(HWND hWnd) {
	HDC hdc;
	hdc = GetDC(hWnd);
    
	InitRects(hWnd);
    int nScreenWidth = rScreen.right - rScreen.left;

	if (!hbmMem)
		hbmMem = CreateCompatibleBitmap(hdc, nScreenWidth, nScreenHeight);

	if (!hbmTmp)
		hbmTmp = CreateCompatibleBitmap(hdc, nScreenWidth, nScreenHeight);

	if (!hbmSkin) {
		// Calculate skin filename
		wchar_t szSkinFileName[MAX_PATH];
		GetModuleFileName(NULL, szSkinFileName, MAX_PATH);
		wchar_t * pstr = wcsrchr(szSkinFileName, '\\');
		if (pstr) *(++pstr) = '\0';
		wcscat(szSkinFileName, L"skin.png");

		// Load skin
		hdcSkin = CreateCompatibleDC(hdc);
		hbmSkin = SHLoadImageFile(szSkinFileName);
		hOld = SelectObject(hdcSkin, hbmSkin);
		GetObject(hbmSkin, sizeof(hbmSkin), &hbmSkin);
	}

	DeleteDC(hdc);
}

void CalculateHeights() {
	int c = 0;
    int count = pListData->GetItemCount();

	for (int i = 0; i < count; i++) {
        StartPosition[i] = c;

        int h = DEFAULT_ITEM_HEIGHT;
        if (pListData->IsItemNewGroup(i) && pListData->GetItem(i).nGroupLength)
            h += DEFAULT_GROUP_HEIGHT;

        if (nListDisplayType == LIST_DISPLAY_DETAILS && ListItemSelected == i)
            h = (int)Resize_ItemHeight;

        for (int a = 0; a < h; a++) {
			PixelToItem[c] = i;
			c++;
		}
	}

    StartPosition[count] = c;
	ListHeight = c;
}


void MoveList(HWND hWnd, double Amount) {
	if (ListHeight < rList.bottom - rList.top)
		return;

    int tmpScrolled = Scrolled - (int)Amount;
    int minScrolled = 0;
    int maxScrolled = ListHeight - rList.bottom + rList.top;
    bool bounce = false;

    if (bScrolling && Scrolled > minScrolled && tmpScrolled <= minScrolled) {
        bounce = true;
        tmpScrolled = minScrolled;
    }

    else if (bScrolling && Scrolled < maxScrolled && tmpScrolled >= maxScrolled) {
        bounce = true;
	    tmpScrolled = maxScrolled;
    }

    if (bounce) {
	    SetTimer(hWnd, IDT_TIMER_BOUNCE, REFRESH_RATE, NULL);
	    Offset = Amount;
	    bScrolling = false;
	    KillTimer(hWnd, IDT_TIMER_SCROLL);
	    KillTimer(hWnd, IDT_TIMER_SCROLL_TO);
	    Speed = 0;
    }

    Scrolled = tmpScrolled;
	RedrawList(hWnd);
}

void ScrollTo(HWND hWnd, int position, int Duration) {
	Scroll_StartPosition = Scrolled;
	Scroll_Change = position - Scroll_StartPosition;
	Scroll_Duration = Duration / REFRESH_RATE;
	Scroll_TimeCounter = 0;
	SetTimer(hWnd, IDT_TIMER_SCROLL_TO, REFRESH_RATE, NULL);
}

void ExpandDetails(HWND hWnd) {
    Data dItem = pListData->GetItem(ListItemSelected);
    int duration = 200;
    int iStartPos = StartPosition[ListItemSelected];
    int iItemHeight = DEFAULT_ITEM_HEIGHT;

	LastHeight = iItemHeight;
	Scroll_TimeCounter = 0;
	Indent_TimeCounter = 0;
	Resize_TimeCounter = 0;
	Scroll_Duration = duration / REFRESH_RATE;
	Indent_Duration = duration / REFRESH_RATE;
	Resize_Duration = duration / REFRESH_RATE;

	if (nListDisplayType == LIST_DISPLAY_LIST) {
		LastPosition = Scroll_StartPosition = Scrolled;
		Scroll_Change = iStartPos - Scroll_StartPosition + 1;

		Indent_StartPosition = 0;
		Indent_Change = 40;

		Resize_StartHeight = iItemHeight;
		Resize_Change = rList.bottom - rList.top - Resize_StartHeight + 1;
		nListDisplayType = LIST_DISPLAY_DETAILS;
	}

	else if (nListDisplayType == LIST_DISPLAY_DETAILS) {
		Scroll_StartPosition = iStartPos;
		Scroll_Change = LastPosition - Scroll_StartPosition;

		Indent_StartPosition = Indent;
		Indent_Change = -40;

		Resize_Change = LastHeight - Resize_StartHeight;
		nListDisplayType = LIST_DISPLAY_LIST;
	}

	SetTimer(hWnd, IDT_TIMER_EXPAND_DETAILS, REFRESH_RATE, NULL);
}

