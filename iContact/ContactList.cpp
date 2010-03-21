#include <stdafx.h>
#include <windows.h>
#include <math.h>
#include <string.h>

#include "ContactList.h"
#include "GraphicFunctions.h"
#include "poom.h"
#include <phone.h>
#include "ThemeSupport.h"
#include "SimpleIni.h"
#include <pimstore.h>


#define MAX_LIST_ITEMS				2000
#define DEFAULT_ITEM_HEIGHT			40

#define SLA_NUMBER		10

#define SLA_TEXT		00
#define SLA_CALL		10
#define SLA_SMS			20
#define SLA_EMAIL		30

#define IDT_TIMER_SCROLL			10010
#define IDT_TIMER_BOUNCE			10011
#define IDT_TIMER_SCROLL_TO			10012
#define IDT_TIMER_RESIZE_TO			10013
#define IDT_TIMER_KEYBOARD_SWITCH	10014
#define IDT_TIMER_SHOW_PLUS			10015
#define IDT_TIMER_SHOW_FAVORITES	10016

#define LIST_DISPLAY_LIST			10
#define LIST_DISPLAY_DETAILS		20
#define LIST_DISPLAY_KEYBOARD		30
#define LIST_DISPLAY_ADDFAVORITES	40


bool	display_favorites = false;

int LIST_DISPLAY_TYPE = LIST_DISPLAY_LIST;

CSimpleIniW ini(false, false, false);
TCHAR ini_path[MAX_PATH];

/// Handles
HWND	hwndParent;
HWND	hContainer;

/// DATA
struct Data
{
	int		ID;
	LONG	oId;
	WCHAR *	Text;
	int		ItemHeight;
	int		StartingPosition;
	bool	GroupHeader;
	int		TextLength;
	int		Group;
	bool	Favorite;
};

IFolder * pCurrFldr = NULL;
IContact * pContact2 = NULL;
IPOutlookItemCollection * pItemCol2;
WCHAR * contact_Company;
WCHAR * contact_MobileNumberPrivate;
WCHAR * contact_MobileNumberWork;
WCHAR * contact_HomeNumber;
WCHAR * contact_WorkNumber;
WCHAR * contact_Email;
WCHAR * contact_Notes;


struct SubListAction {
	int		id;
	LPTSTR	text;
	TCHAR	label[25];
	int		action;
	bool	sms;
};
SubListAction ActionsList[SLA_NUMBER];
int SubListCurrentAction = -1;
int ActionsNumber = 0;
bool sms = false;


Data	ListData[MAX_LIST_ITEMS];
Data	dItem;


struct Group
{
	int		ID;
	RECT	Rect;
};

Group	GroupArray[30];		// Group references to ListData[] items
int		GroupCounter = 0;	// Used when adding group items
int		GroupWidth = 0;		// Final group width
int		GroupHeight = 0;	// Final group height
int		CurrentGroup = -1;	// Currently selected group

int		ListHeight = 0;
int		ListCounter = 0;
int		PixelToItem[MAX_LIST_ITEMS * DEFAULT_ITEM_HEIGHT];
int		ListItemSelected = -1;
int		ListItemHover = -1;
int		LastListItemSelected = -1;
bool	ItemEdit = false;
bool	ItemBack = false;



// Graphic
PAINTSTRUCT	ps;
HDC			hdc;
RECT		rCL;
RECT		rClient;
RECT		rListBack;
RECT		rItem;
HFONT		ListFont;
HFONT		GroupFont;
HDC			hdcMem;
HBITMAP		hbmMem;
HANDLE		hOld;

struct theme_struct Theme;

// List
HBRUSH		ListBackgroundBrush;

// Standard Item
int			ItemFontSize = 20;
HBRUSH		ListItemBackgroundBrush;
int			ListItemIndent = 18;

// Selected Item
bool		ListSelectedItemBackgroundGradient = true;
HBRUSH		ListSelectedBackgroundBrush;
bool		ListSelectedItemShadow = false;
HBRUSH		ListSelectedBrush;
HPEN		ListSelectedPen;
HBRUSH		ListSelectedItemDetailsButtonBrush;
HPEN		ListSelectedItemDetailsButtonPen;
HBRUSH		ListUnSelectedItemDarken;

// Hover Item
HBRUSH		ListItemHoverBackgroundBrush;

// Item Details
HFONT		ItemDetailsButtonFont;
int			ItemDetailsButtonFontSize = 12;

// Group Header
int			GroupItemFontSize = 13;
int			ListGroupItemIndent = 14;
bool		ListGroupBackgroundGradient = false;
bool		ListGroupBackgroundBitmapStretch = false;
HBRUSH		ListGroupBackgroundBrush;

// Separator
HBRUSH		ListSeparatorBrush;
int			ListSeparatorHeight = 1;

// List Indicator
HPEN		ListIndicatorPen;
HBRUSH		ListIndicatorBrush;
bool		bListIndicatorBackground = false;
HFONT		ListIndicatorFont;
int			ListIndicatorFontSize = 80;

// Keyboard Button
HPEN		KeyboardButtonPen;
HBRUSH		KeyboardButtonBrush;
HFONT		KeyboardButtonFont;
int			KeyboardButtonFontSize = 14;
RECT		rKeyboardButton;

// Keyboard
HFONT		KeyboardFont;
HBRUSH		KeyboardBackgroundBrush;
HBRUSH		KeyboardKeyBrush;
HBRUSH		KeyboardSelectedBrush;
HPEN		KeyboardGridPen;
HDC			hdcKeyboard;
HBITMAP		hbmKeyboard;

// Strings
const wchar_t * abc_button_string;
const wchar_t * email_account;

const wchar_t * mobile_string;
const wchar_t * home_string;
const wchar_t * work_string;
const wchar_t * company_string;
const wchar_t * email_string;
const wchar_t * sms_string;

// Scrolling
bool		bDragging = false;
bool		bScrolling = false;
bool		bStoping = false;
bool		bClickable = true;
int			Scrolled = 0;
bool		DrawList = true;
int			StartX;
int			StartY;
int			EndY;
int			LastX;
int			LastY;
int			tStartTime;
int			tEndTime;
double		Speed = 0;
double		Offset = 0;

int friction = 30;
int InitSpeedMultiplier = 30;
int RefreshRate = 22;

// Scroll To
double Scroll_TimeCounter = 0;
double Scroll_StartPosition = 0;
double Scroll_Change = 0;
double Scroll_Duration = 0;

// Indent To
double Indent_TimeCounter = 0;
double Indent_StartPosition = 0;
double Indent_Change = 0;
double Indent_Duration = 0;
double Indent;

// Resize To
double	Resize_StartHeight = 0;
double	Resize_Change = 0;
double	Resize_Duration = 0;
double	Resize_TimeCounter = 0;
double	LastHeight = 0;
double	LastPosition = 0;
bool	bResizing = false;

// Keyboard Switch
double	Keyboard_Start = 0;
double	Keyboard_Change = 0;
double	Keyboard_Duration = 0;
double	Keyboard_TimeCounter = 0;
int		Keyboard_Ratio = 100;
RECT	rKeyboardFull;
bool	KeyboardShown = true;

// Show Plus Sign Switch
double	ShowPlus_Start = 0;
double	ShowPlus_Change = 0;
double	ShowPlus_Duration = 0;
double	ShowPlus_TimeCounter = 0;
int		ShowPlus_Indent = 0;
bool	ShowPlus_Shown = false;
bool	First_Drag = false;
bool	Plus_Down = false;

// Show Favorites Switch
double	ShowFavorites_Start = 0;
double	ShowFavorites_Change = 0;
double	ShowFavorites_Duration = 0;
double	ShowFavorites_TimeCounter = 0;
double	ShowFavorites_Amount = 0;

WNDPROC OldCntListProc;
LRESULT CALLBACK CntListProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

bool (*rePopulate)() = NULL;

bool InitListData();
bool InitSubListData();
bool InitSurface();

void MoveList(double Amount);
void ScrollTo (int Position, int Duration);
void ResizeTo(int height, int duration);
void SwitchKeyboard(int action, int duration);
void ShowPlus(int action, int duration);
void ShowFavorites(int action, int duration);

void Call();
void SendSMS();
void SendMail();
void AddToFavorites(LONG oId, int ID);
void RemoveFromFavorites(LONG oId, int ID);

LRESULT CALLBACK CntListProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
	case WM_ERASEBKGND:
		{ return 1; }
		break;

	case WM_PAINT:
		{
			hdc = BeginPaint( hwnd, &ps );

			int sItem, Item = 0;
			int itemLeft;

			sItem = Item = PixelToItem[Scrolled];

			// ******* DRAW LIST BACKGROUND
			if (Offset != 0 || ListHeight < (rClient.bottom - rClient.top))
				FillRect(hdcMem, &rClient, ListBackgroundBrush);

			if (LIST_DISPLAY_TYPE == LIST_DISPLAY_ADDFAVORITES)
			{
				ListUnSelectedItemDarken = CreateSolidBrush((COLORREF) RGB(255,255,255));
				FillRect(hdcKeyboard, &rClient, ListUnSelectedItemDarken);
				ListUnSelectedItemDarken = CreateSolidBrush((COLORREF) RGB(255 - (3 * ShowPlus_Indent), 255 - (3 * ShowPlus_Indent), 255 - (3 * ShowPlus_Indent)));
			}
			

			while(DrawList == true)
			{
				dItem = ListData[Item];

				if(display_favorites == true) {
					if(dItem.Favorite == false) {
						Item++;
						continue;
					}
				}

				if (dItem.ID < 0)
					break;

				if (dItem.StartingPosition >= (rClient.bottom - rClient.top + Scrolled - Offset))
					break;

				rItem = rClient;
				rItem.top = dItem.StartingPosition - Scrolled + (int)Offset;
				rItem.bottom = rItem.top + dItem.ItemHeight;
				itemLeft = rItem.left;

				SetBkMode(hdcMem, TRANSPARENT);

				if (dItem.GroupHeader == true)
				{
					// ****** GroupHeader background
					if (ListGroupBackgroundGradient == true)
						DrawGradientGDI(hdcMem, rItem, Theme.ListGroupBackgroundColor, Theme.ListGroupBackgroundColor2);
					else
						FillRect(hdcMem, &rItem, ListGroupBackgroundBrush);

					// ******* Draw Item Text
					rItem.left += ListGroupItemIndent;
					SelectObject(hdcMem, GroupFont);
					SetTextColor(hdcMem, Theme.ListGroupTextColor);
					ExtTextOut(hdcMem, ListGroupItemIndent, rItem.top + ((dItem.ItemHeight - GroupItemFontSize) / 2), NULL, NULL, dItem.Text,  dItem.TextLength, 0);
				} else {
					if (Item == ListItemSelected)
					{
						// ********* FAVORITES PLUS SIGN
						if (LIST_DISPLAY_TYPE == LIST_DISPLAY_ADDFAVORITES)
						{
							if(Plus_Down)
								DrawGradientGDI(hdcMem, rItem, Theme.ListGroupBackgroundColor, Theme.ListItemHoverBackgroundColor);
							else
								FillRect(hdcMem, &rItem, KeyboardBackgroundBrush);
							
							RECT plus;
							
							plus.left = rItem.left + 15;
							plus.right = plus.left + 20;
							plus.top = rItem.top + (rItem.bottom - rItem.top) / 2 - 2;
							plus.bottom = plus.top + 4;
							FillRect(hdcMem, &plus, KeyboardButtonBrush);							

							if(dItem.Favorite == false) {
								plus.left = 50 / 2 - 2;
								plus.right = plus.left + 4;
								plus.top = rItem.top + (rItem.bottom - rItem.top) / 2 - 10;
								plus.bottom = plus.top + 20;
								FillRect(hdcMem, &plus, KeyboardButtonBrush);
							}

							rItem.left += ShowPlus_Indent;
						}

						// ******* DRAW ITEM BACKGROUND
						if (ListSelectedItemBackgroundGradient == true)
						{
							DrawGradientGDI(hdcMem, rItem, Theme.ListSelectedBackgroundColor, Theme.ListSelectedBackgroundColor2);
						} else {
							SelectObject(hdcMem, ListSelectedBrush);
							SelectObject(hdcMem, ListSelectedPen);
							RoundRect(hdcMem, rItem.left, rItem.top, rItem.right, rItem.bottom, 20, 20);
							//FillRect(hdcMem, &rItem, ListSelectedBackgroundBrush);
						}
						RECT rItem2 = rItem;
						// ****** Edit contact button
						if(ItemEdit == true) {
							rItem2.left = rItem.left + 50;
							rItem2.bottom = rItem2.top + 40;
							DrawGradientGDI(hdcMem, rItem2, Theme.ListGroupBackgroundColor, Theme.ListItemHoverBackgroundColor);
						}

						// ****** Draw Item Text
						rItem.left += ListItemIndent;
						SelectObject(hdcMem, ListFont);

						if (ListSelectedItemShadow == true)
						{
							SetTextColor(hdcMem, Theme.ListSelectedItemTextShadowColor);
							ExtTextOut(hdcMem, (int)(rItem.left + Indent), (int)(rItem.top + 1 + ((LastHeight - ItemFontSize) / 2)), ETO_OPAQUE, NULL, dItem.Text, dItem.TextLength, 0);
						}
						SetTextColor(hdcMem, Theme.ListSelectedItemTextColor);
						ExtTextOut(hdcMem, (int)(rItem.left + Indent), (int)(rItem.top + ((LastHeight - ItemFontSize) / 2)), ETO_OPAQUE, NULL, dItem.Text, dItem.TextLength, 0);

						if (LIST_DISPLAY_TYPE == LIST_DISPLAY_DETAILS) {
							// DRAW ITEM DETAILS **************	
							int ButtonOffset = 40;
							RECT rButton = rItem;

							for (int c = 0; c < SLA_NUMBER; c++) {
								if(ActionsList[c].id == -1) continue;						

								rButton.top += ButtonOffset;
								rButton.bottom = rButton.top + 1;
								rButton.left = itemLeft;

								FillRect(hdcMem, &rButton, CreateSolidBrush((COLORREF) RGB(160,165,170)));

								rButton.left = 0;
								rButton.top += 1;
								rButton.bottom = rButton.top + 40;

								if(c == SubListCurrentAction) {
									if((ActionsList[SubListCurrentAction].action == SLA_CALL || ActionsList[SubListCurrentAction].action == SLA_EMAIL) && !sms)
										DrawGradientGDI(hdcMem, rButton, Theme.ListGroupBackgroundColor, Theme.ListItemHoverBackgroundColor);
								}

								rButton.left = 5;

								rButton.right = rButton.left + 44;

								SelectObject(hdcMem, ItemDetailsButtonFont);
								DrawText(hdcMem, ActionsList[c].label, _tcslen(ActionsList[c].label), &rButton, DT_RIGHT | DT_VCENTER);

								rButton.right = rItem.right;
								rButton.bottom = rButton.top + 20;

								SelectObject(hdcMem, ListFont);
								if (ListSelectedItemShadow == true)
								{
									SetTextColor(hdcMem, Theme.ListSelectedItemTextShadowColor);
									ExtTextOut(hdcMem, rButton.left + 52, rButton.top + 1 + (ItemFontSize) / 2, ETO_OPAQUE, NULL, ActionsList[c].text, SysStringLen(ActionsList[c].text), 0);
								}
								SetTextColor(hdcMem, Theme.ListSelectedItemTextColor);
								ExtTextOut(hdcMem, rButton.left + 52, rButton.top + (ItemFontSize) / 2, ETO_OPAQUE, NULL, ActionsList[c].text, SysStringLen(ActionsList[c].text), 0);

								if(ActionsList[c].sms == true) {
									rButton.bottom = rButton.top + 40;
									if (sms == true && c == SubListCurrentAction) {
										int old = rButton.left;
										rButton.left = rButton.right - 40;
										DrawGradientGDI(hdcMem, rButton, Theme.ListGroupBackgroundColor, Theme.ListItemHoverBackgroundColor);
										rButton.left = old;
									}
									rButton.right -= 10;
									SelectObject(hdcMem, ItemDetailsButtonFont);
									DrawText(hdcMem, sms_string, 3, &rButton, DT_RIGHT | DT_VCENTER);
									rButton.right += 10;
								}
							}

							if(ItemBack == true) {
								rItem.left = 0;
								rItem2.right = rItem2.left + 50;
								rItem2.bottom = rItem2.top + 40;
								DrawGradientGDI(hdcMem, rItem2, Theme.ListGroupBackgroundColor, Theme.ListItemHoverBackgroundColor);
							}
							
							SelectObject(hdcMem, CreateSolidBrush(RGB(220,220,220)));
							SelectObject(hdcMem, CreatePen(PS_SOLID, 1, RGB(220,220,220)));
							POINT rectPoints[3];
							rectPoints[0].x = (LONG)Indent - 22	;
							rectPoints[0].y = rItem.top + 20;
							rectPoints[1].x = (LONG)Indent - 10;
							rectPoints[1].y = rItem.top + 13;
							rectPoints[2].x = (LONG)Indent - 10;
							rectPoints[2].y = rItem.top + 27;
							Polygon(hdcMem, rectPoints, 3);
						}

					}
					else
					{
						if(Item == ListItemHover) {
						// Draw Item
							DrawGradientGDI(hdcMem, rItem, Theme.ListGroupBackgroundColor, Theme.ListItemHoverBackgroundColor);
							//FillRect(hdcMem, &rItem, ListItemHoverBackgroundBrush);
							rItem.left += ListItemIndent;
							SelectObject(hdcMem, ListFont);
							SetTextColor(hdcMem, Theme.ListGroupBackgroundColor);
							ExtTextOut(hdcMem, ListItemIndent, rItem.top + 3 + ((dItem.ItemHeight - ItemFontSize) / 2), ETO_OPAQUE, NULL, dItem.Text, dItem.TextLength, 0);
							SetTextColor(hdcMem, Theme.ListItemTextColor);
							ExtTextOut(hdcMem, ListItemIndent, rItem.top + ((dItem.ItemHeight - ItemFontSize) / 2), ETO_OPAQUE, NULL, dItem.Text, dItem.TextLength, 0);
						} else {
							FillRect(hdcMem, &rItem, ListItemBackgroundBrush);
							rItem.left += ListItemIndent;
							SelectObject(hdcMem, ListFont);
							SetTextColor(hdcMem, Theme.ListItemTextColor);
							ExtTextOut(hdcMem, ListItemIndent, rItem.top + ((dItem.ItemHeight - ItemFontSize) / 2), ETO_OPAQUE, NULL, dItem.Text, dItem.TextLength, 0);
						}
					}
					
					/*
					// Display favorites indicator
					if(ListData[Item].Favorite == true && LIST_DISPLAY_TYPE == LIST_DISPLAY_LIST) {
						SelectObject(hdcMem, CreateSolidBrush(RGB(110,110,110)));
						SelectObject(hdcMem, CreatePen(PS_SOLID, 1, RGB(110,110,110)));
						Ellipse(hdcMem, rItem.left - 13, rItem.top + 18, rItem.left - 6, rItem.top + 25);
					}
					*/
				}

				// Draw Item Separator
				if (ListSeparatorHeight > 0)
				{
					rItem.top = rItem.bottom;
					rItem.bottom = rItem.top + ListSeparatorHeight;
					rItem.left = itemLeft;
					FillRect(hdcMem, &rItem, ListSeparatorBrush);
				}

				if ((LIST_DISPLAY_TYPE == LIST_DISPLAY_ADDFAVORITES) && (Item != ListItemSelected))
				{
					rItem.top = ListData[Item].StartingPosition - Scrolled;
					FillRect(hdcKeyboard, &rItem, ListUnSelectedItemDarken);
				}

				// Next Item
				Item++;
			}
			if (LIST_DISPLAY_TYPE == LIST_DISPLAY_ADDFAVORITES)
			{
				BltAlpha(hdcMem, rClient.left, rClient.top, rClient.right - rClient.left, rClient.bottom - rClient.top, hdcKeyboard, 30);
			}

			// Draw list indicator
			if (fabs(Speed) > 10 && !display_favorites)
			{
				RECT rInd = rClient;
				rInd.top = 10;

				if(bListIndicatorBackground == true)
					FillRect(hdcMem, &rInd, ListIndicatorBrush);

				SelectObject(hdcMem, ListIndicatorFont);
				SetTextColor(hdcMem, Theme.ListIndicatorFontColor);
				SetBkMode(hdcMem, TRANSPARENT);
				SetBkColor(hdcMem, Theme.ListIndicatorBrushColor);
				DrawText(hdcMem, ListData[ListData[sItem+1].Group].Text, -1, &rInd, DT_CENTER | DT_TOP | DT_NOCLIP);
			}

			/*
			optimizerTime2 = GetTickCount() - optimizerTime1;
			TCHAR buf[10];
			wsprintf(buf,_T("%d - %d\n"),optimizerTime2, sItem);
			OutputDebugString(buf);
			*/

			// BOTTOM MENUS
			if (LIST_DISPLAY_TYPE == LIST_DISPLAY_LIST && !display_favorites)
			{
				SelectObject(hdcMem, KeyboardButtonFont);
				SetTextColor(hdcMem, Theme.KeyboardButtonFontColor);

				rKeyboardButton = rClient;
				rKeyboardButton.left = rKeyboardButton.right - 40;
				rKeyboardButton.top = rKeyboardButton.bottom - 30;
			
				DrawText(hdcMem, (LPCWSTR)abc_button_string, -1, &rKeyboardButton, DT_NOCLIP);
				/*
				rKeyboardButton = rClient;
				rKeyboardButton.right = rKeyboardButton.left + 40;
				rKeyboardButton.top = rKeyboardButton.bottom - 30;
				DrawText(hdcMem, TEXT("TOOLS"), -1, &rKeyboardButton, DT_NOCLIP);
				*/
			}


			// DRAW KEYBOARD SHORTCUTS
			if (LIST_DISPLAY_TYPE == LIST_DISPLAY_KEYBOARD)
			{
				rKeyboardFull = rClient;
				rKeyboardFull.left = rClient.right - ((rClient.right - rClient.left) * Keyboard_Ratio / 100);
				rKeyboardFull.top = rClient.bottom - ((rClient.bottom - rClient.top) * Keyboard_Ratio / 100);
				FillRect(hdcKeyboard, &rKeyboardFull, KeyboardBackgroundBrush);

				BltAlpha(hdcMem, rKeyboardFull.left, rKeyboardFull.top, rKeyboardFull.right - rKeyboardFull.left, rKeyboardFull.bottom - rKeyboardFull.top, hdcKeyboard, 30);

				if (Keyboard_Ratio > 50)
				{
					KeyboardFont = BuildFont(30 * Keyboard_Ratio / 100, TRUE, FALSE);
					SelectObject(hdcMem, KeyboardFont);
					SelectObject(hdcMem, KeyboardGridPen);
					SetBkMode(hdcMem, TRANSPARENT);
					SelectObject(hdcMem, KeyboardKeyBrush);

					GroupWidth = 1 + ((rKeyboardFull.right - rKeyboardFull.left)-6) / 5 * Keyboard_Ratio / 100;
					GroupHeight = 1 + ((rKeyboardFull.bottom - rKeyboardFull.top)-7) / 6 * Keyboard_Ratio / 100;

					int cnt2 = 0;
					for(int h = 0; h < 6; h++)
					{
						Rectangle(hdcMem, rKeyboardFull.left, rKeyboardFull.top + h * GroupHeight + h, rKeyboardFull.right, rKeyboardFull.top + h * GroupHeight + h + 1);
						for(int g = 0; g < 5; g++)
						{
							if (h == 0)
							{
								Rectangle(hdcMem, rKeyboardFull.left + (g * GroupWidth) + g, rKeyboardFull.top, rKeyboardFull.left + (g * GroupWidth) + g + 1, rKeyboardFull.bottom);
							}

							GroupArray[cnt2].Rect.left = rKeyboardFull.left + (g + 1) + (g * GroupWidth);
							GroupArray[cnt2].Rect.top = rKeyboardFull.top + (h + 1) + (h * GroupHeight);
							GroupArray[cnt2].Rect.right = GroupArray[cnt2].Rect.left + GroupWidth;
							GroupArray[cnt2].Rect.bottom = GroupArray[cnt2].Rect.top + GroupHeight;

							if (cnt2 == CurrentGroup)
							{
								FillRect(hdcMem, &GroupArray[cnt2].Rect, KeyboardSelectedBrush);
								SetTextColor(hdcMem, (COLORREF) Theme.KeyboardSelectedFontColor);
							}
							else
								SetTextColor(hdcMem, Theme.KeyboardFontColor);

							if (GroupArray[cnt2].ID > -1)
							{
								DrawText(hdcMem, ListData[GroupArray[cnt2].ID].Text, -1, &GroupArray[cnt2].Rect, DT_CENTER | DT_VCENTER | DT_NOCLIP);
							}
							cnt2++;
						}
					}
					Rectangle(hdcMem, rKeyboardFull.right - 1, rKeyboardFull.top, rKeyboardFull.right, rKeyboardFull.bottom);
					Rectangle(hdcMem, rKeyboardFull.left, rKeyboardFull.bottom - 1, rKeyboardFull.right, rKeyboardFull.bottom);
				}

			}


			BitBlt(hdc, rClient.left, rClient.top, rClient.right - rClient.left, rClient.bottom - rClient.top, hdcMem, rClient.left, rClient.top, SRCCOPY);

			EndPaint(hwnd, &ps);
			return 0;
		}
		break;



	case WM_LBUTTONDOWN:
		{
			POINT pt;
			pt.x = LOWORD(lParam);
			pt.y = HIWORD(lParam);

			if (LIST_DISPLAY_TYPE == LIST_DISPLAY_ADDFAVORITES)
			{
				if(PixelToItem[pt.y + Scrolled] == ListItemSelected && pt.x < 50) {
					Plus_Down = true;
					RedrawList();
				}
			}

			if ((LIST_DISPLAY_TYPE == LIST_DISPLAY_LIST) && bClickable)
			{
				if (PtInRect(&rKeyboardButton, pt))
				{
					LIST_DISPLAY_TYPE = LIST_DISPLAY_KEYBOARD;
					KillTimer(hContainer, IDT_TIMER_SCROLL);
					SwitchKeyboard(1, 200);
				}
				else
				{
					LastX = StartX = pt.x;
					LastY = StartY = pt.y;
					tStartTime = GetTickCount();
					bStoping = false;
					if (ListData[PixelToItem[pt.y + Scrolled]].Group != PixelToItem[pt.y + Scrolled])
					{
						ListItemHover = PixelToItem[pt.y + Scrolled];
						RedrawList();
					}
					if (bScrolling == true)
					{
						KillTimer(hwnd, IDT_TIMER_SCROLL);
						bScrolling = false;
						bStoping = true;
						Speed = 0;
						RedrawList();
					}
				}
			}

			if(LIST_DISPLAY_TYPE == LIST_DISPLAY_DETAILS) {
				if((pt.y > rClient.top) && (pt.y < rClient.top + 40)) {
					if(pt.x <= 60) {
						ItemBack = true;
						RedrawList();
					} else {
						ItemEdit = true;
						RedrawList();
					}
				} else {
					SubListCurrentAction = (pt.y - 40) / 41;
					if(ActionsList[SubListCurrentAction].sms == true && pt.x > 200) sms = true;
					RedrawList();
				}
			}
			return true;
		}
		break;

	case WM_MOUSEMOVE:
		{
			POINT pt;
			pt.x = LOWORD(lParam);
			pt.y = HIWORD(lParam);

			if (LIST_DISPLAY_TYPE == LIST_DISPLAY_LIST)
			{
				int CurrentX = pt.x;
				int CurrentY = pt.y;

				int DeltaX = CurrentX - LastX;
				int DeltaY = CurrentY - LastY;
				
				if (bDragging == false && bScrolling == false && (abs(DeltaX) > abs(DeltaY)) && (CurrentX > (StartX + 10)))
				{	
				
					// FAVORITES
					//MessageBox(hContainer, TEXT("favorite"), TEXT("Alert"), MB_OK);
					
					int CurrItem = PixelToItem[HIWORD(lParam) + Scrolled];
					if (ListData[CurrItem].Group != CurrItem)
					{
						ListItemSelected = CurrItem;
						LastHeight = ListData[ListItemSelected].ItemHeight;
						LIST_DISPLAY_TYPE = LIST_DISPLAY_ADDFAVORITES;
						ShowPlus(1, 200);
						First_Drag = true;
					}
					
				}
				else
				{
					// SCROLL
					bDragging = true;
					if (abs(DeltaY) > 3)
					{
						MoveList(DeltaY);
						LastY = CurrentY;
					}

					if (!((StartY < LastY) || (StartY > LastY)))
					{
						bDragging = false;
					}
				}
				LastX = CurrentX;
			}

			if (LIST_DISPLAY_TYPE == LIST_DISPLAY_DETAILS) {
			
			}

			if (LIST_DISPLAY_TYPE == LIST_DISPLAY_KEYBOARD)
			{
				if (KeyboardShown == false)
					break;

				for(int i = 0; i < GroupCounter; i++)
				{
					if (PtInRect(&GroupArray[i].Rect, pt))
					{
						if (CurrentGroup != i)
						{
							CurrentGroup = i;
							RedrawList();
						}
						return 1;
					}
				}
				if (CurrentGroup != -1)
				{
					CurrentGroup = -1;
					RedrawList();
				}
			}
			return 0;
		}
		break;

	case WM_LBUTTONUP:
		{
			POINT pt;
			pt.x = LOWORD(lParam);
			pt.y = HIWORD(lParam);

			ItemEdit = false;
			ItemBack = false;
			RedrawList();

			if (LIST_DISPLAY_TYPE == LIST_DISPLAY_ADDFAVORITES)
			{
				if(First_Drag == false) {
					if (ListItemSelected == PixelToItem[pt.y + Scrolled] && pt.x < 50)
					{
						if(ListData[ListItemSelected].Favorite == true)
							RemoveFromFavorites(ListData[ListItemSelected].oId, ListItemSelected);
						else
							AddToFavorites(ListData[ListItemSelected].oId, ListItemSelected);
					}
					ShowPlus(3,300);
				} else {
					First_Drag = false;
				}
				Plus_Down = false;
			}
			else if (LIST_DISPLAY_TYPE == LIST_DISPLAY_KEYBOARD)
			{	
				if (PtInRect(&rKeyboardButton, pt))
				{
					return TRUE;
				} else {
					SwitchKeyboard(3, 200);
					if (CurrentGroup > -1)
					{
						if ( (ListHeight - ListData[GroupArray[CurrentGroup].ID].StartingPosition) < (rClient.bottom - rClient.top))
							ScrollTo(ListHeight - (rClient.bottom - rClient.top), 300);
						else
							ScrollTo(ListData[GroupArray[CurrentGroup].ID].StartingPosition, 300);

						CurrentGroup = -1;
					}
				}
			}
			else if (LIST_DISPLAY_TYPE == LIST_DISPLAY_LIST)
			{
				ListItemHover = -1;
				RedrawList();
				if (bDragging == true)
				{
					int pos = HIWORD(lParam) + Scrolled;
					tEndTime = GetTickCount();
					Speed = ((HIWORD(lParam) - StartY) * InitSpeedMultiplier) / (tEndTime - tStartTime);
					if (fabs(Speed) > 6){
						SetTimer(hContainer, IDT_TIMER_SCROLL, RefreshRate, (TIMERPROC) NULL);
						bStoping = false;
						bScrolling = true;
					} else {

						bStoping = false;
						bScrolling = false;
						bDragging = false;
					}
				} else {
					if (bStoping == true)
					{
						bStoping = false;
					} else {
						if (ListData[PixelToItem[HIWORD(lParam) + Scrolled]].Group != PixelToItem[HIWORD(lParam) + Scrolled])
						{
							ListItemSelected = PixelToItem[HIWORD(lParam) + Scrolled];
							LastHeight = ListData[ListItemSelected].ItemHeight;

							GetItemData();
						}
					}
				}
			}
			else if (LIST_DISPLAY_TYPE == LIST_DISPLAY_DETAILS)
			{
				if(HIWORD(lParam) > rClient.top && HIWORD(lParam) < rClient.top + 40)
				{
					if(pt.x > 60) {
						pItemCol2->Item(ListData[ListItemSelected].ID + 1, reinterpret_cast<IDispatch**>(&pContact2));
						pContact2->Display();
						pContact2->Release();
						ResizeTo(ListItemSelected, 200);
					} else {
						LastListItemSelected = -1;
						ResizeTo(ListItemSelected, 200);
					}
				} else {
					// HANDLE SUBLIST EVENTS
					if ((pt.y - 40) / 41 == SubListCurrentAction) {
						switch(ActionsList[SubListCurrentAction].action) {
							case SLA_CALL:
								if(sms) SendSMS();
								else Call();
								LastListItemSelected = -1;
								ResizeTo(ListItemSelected, 200);
								LIST_DISPLAY_TYPE = LIST_DISPLAY_LIST;
								break;
							case SLA_EMAIL:
								SendMail();
								LastListItemSelected = -1;
								ResizeTo(ListItemSelected, 200);
								LIST_DISPLAY_TYPE = LIST_DISPLAY_LIST;
								break;
						}
					}
					// HANDLE SUBLIST EVENTS
				}
				SubListCurrentAction = -1;
				sms = false;
				RedrawList();
			}
			return TRUE;
		}
		break;
	case WM_TIMER:
		switch (wParam)
		{
			///// TIMER for scrolling
		case IDT_TIMER_SCROLL:
			{
				Offset = 0;
				if (fabs(Speed) <= 0.01)
				{
					KillTimer(hwnd, IDT_TIMER_SCROLL);
					bScrolling = false;
					bStoping = false;
					Speed = 0;
				} else {
					bScrolling = true;
					Speed = Speed - Speed / friction;
					MoveList(Speed);
				}
				return 0;
			}

			///// TIMER for bouncing
		case IDT_TIMER_BOUNCE:
			{
				if ( fabs(Offset) <= 0.01 ) {
					KillTimer(hwnd, IDT_TIMER_BOUNCE);
					Offset = 0;
				} else {
					Offset = Offset - Offset / friction * 4;
				}

				RedrawList();
				return 0;
			}

			///// TIMER for scroll to
		case IDT_TIMER_SCROLL_TO:
			{
				Offset = 0;
				KillTimer(hContainer, IDT_TIMER_SCROLL);
				if ((Scroll_TimeCounter < Scroll_Duration) && bStoping == false)
				{
					double amount;
					//double sc;
					bScrolling = true;

					// Cubic
					amount = Scroll_Change * (pow(Scroll_TimeCounter/Scroll_Duration-1, 3) + 1) + Scroll_StartPosition;
					Speed = amount;

					// Bounce
					/*
					sc = Scroll_TimeCounter;
					if ((sc/=Scroll_Duration) < (1/2.75)) {
						amount = Scroll_Change*(7.5625*sc*sc) + Scroll_StartPosition;
					} else if (sc < (2/2.75)) {
						amount = Scroll_Change*(7.5625*(sc-=(1.5/2.75))*sc + .75) + Scroll_StartPosition;
					} else if (sc < (2.5/2.75)) {
						amount = Scroll_Change*(7.5625*(sc-=(2.25/2.75))*sc + .9375) + Scroll_StartPosition;
					} else {
						amount = Scroll_Change*(7.5625*(sc-=(2.625/2.75))*sc + .984375) + Scroll_StartPosition;
					}
					*/

					// Back
					/*
					double s = 1.70158;
					sc = Scroll_TimeCounter;
					amount = Scroll_Change*((sc=sc/Scroll_Duration-1)*sc*((s+1)*sc + s) + 1) + Scroll_StartPosition;
					*/

					// Elastic
					/*
					double a = 0; double p = 0; double s = 0;
					double PI = 3.14159265;
					sc = Scroll_TimeCounter;
					if (sc==0)
						amount = Scroll_StartPosition;
					else
					{
						if ((sc/=Scroll_Duration)==1)
							amount = Scroll_StartPosition+Scroll_Change;
						else
						{
							if (!p) p=Scroll_Duration*.3;
							if (!a || a < abs(Scroll_Change))
								{ a=Scroll_Change; s=p/4; }
							else
								s = p/(2*PI) * asin (Scroll_Change/a);
							amount = (a*pow(2,-10*sc) * sin( (sc*Scroll_Duration-s)*(2*PI)/p ) + Scroll_Change + Scroll_StartPosition);
						}
					}
					*/


					MoveList(Scrolled - amount);
					Scroll_TimeCounter++;
				}
				else
				{
					bScrolling = false;
					Speed = 0;
					KillTimer(hwnd, IDT_TIMER_SCROLL_TO);
				}
				RedrawList();
				return 0;
			}

			///// TIMER for scroll to
		case IDT_TIMER_RESIZE_TO:
			{
				if (Resize_TimeCounter < Resize_Duration)
				{
					bClickable = false;
					double amount; double amount2;

					// Cubic
					amount2 = Resize_Change * (pow(Resize_TimeCounter/Resize_Duration-1, 3) + 1) + Resize_StartHeight;
					ListData[ListItemSelected].ItemHeight = (int)amount2;
					CalculateHeights();
					Resize_TimeCounter++;

					amount = Scroll_Change * (pow(Scroll_TimeCounter/Scroll_Duration-1, 3) + 1) + Scroll_StartPosition;
					Offset = Scrolled - amount;
					Scroll_TimeCounter++;

					Indent = Indent_Change * (pow(Indent_TimeCounter/Indent_Duration-1, 3) + 1) + Indent_StartPosition;
					Indent_TimeCounter++;
				}
				else
				{
					bClickable = true;
					KillTimer(hwnd, IDT_TIMER_RESIZE_TO);
					if (LIST_DISPLAY_TYPE == LIST_DISPLAY_LIST)
						ListItemSelected = LastListItemSelected;
				}
				RedrawList();
				return 0;
			}

			///// TIMER for keyboard show and hide
		case IDT_TIMER_KEYBOARD_SWITCH:
			{
				ListItemSelected = -1;
				if (Keyboard_TimeCounter < Keyboard_Duration)
				{
					// Cubic
					Keyboard_Ratio = (int)(Keyboard_Change * (pow(Keyboard_TimeCounter/Keyboard_Duration-1, 3) + 1) + Keyboard_Start);
					Keyboard_TimeCounter++;
				}
				else
				{
					KillTimer(hwnd, IDT_TIMER_KEYBOARD_SWITCH);
					KeyboardShown = true;
					if (Keyboard_Ratio <= 11)
					{
						LIST_DISPLAY_TYPE = LIST_DISPLAY_LIST;
						ListItemSelected = LastListItemSelected;
						KeyboardShown = false;
					}
				}
				RedrawList();
				return 0;
			}

			///// TIMER for plus sign show and hide
		case IDT_TIMER_SHOW_PLUS:
			{
				if (ShowPlus_TimeCounter < ShowPlus_Duration)
				{
					// Cubic
					ShowPlus_Indent = (int)(ShowPlus_Change * (pow(ShowPlus_TimeCounter/ShowPlus_Duration-1, 3) + 1) + ShowPlus_Start);
					ShowPlus_TimeCounter++;
				}
				else
				{
					KillTimer(hwnd, IDT_TIMER_SHOW_PLUS);
					ShowPlus_Shown = true;
					if (ShowPlus_Indent <= 2)
					{
						LIST_DISPLAY_TYPE = LIST_DISPLAY_LIST;
						ListItemSelected = LastListItemSelected;
						ShowPlus_Shown = false;
					}
				}
				RedrawList();
				return 0;
			}

			///// TIMER for displaying of favorites
		case IDT_TIMER_SHOW_FAVORITES:
			{
				if (ShowFavorites_TimeCounter < ShowFavorites_Duration)
				{
					// Cubic
					ShowFavorites_Amount = (ShowFavorites_Change * (pow(ShowFavorites_TimeCounter/ShowFavorites_Duration-1, 3) + 1) + ShowFavorites_Start);
					ShowFavorites_TimeCounter++;
					CalculateHeights();
				}
				else
				{
					KillTimer(hwnd, IDT_TIMER_SHOW_FAVORITES);
				}
				RedrawList();
				return 0;
			}

		} 
		return TRUE;

	case WM_KEYDOWN:
		switch (wParam)
		{
			// scroll up
		case 33:
			if (LIST_DISPLAY_TYPE == LIST_DISPLAY_LIST)
				((Scrolled - 1000) >= 0) ? ( ScrollTo(Scrolled - 1000, 1000) ) : ( ScrollTo( -40, 1000) );
			break;
			// scroll down
		case 34:
			if (LIST_DISPLAY_TYPE == LIST_DISPLAY_LIST)
				((Scrolled + 1000) <= ListHeight - (rClient.bottom - rClient.top) ) ? ( ScrollTo(Scrolled + 1000, 1000) ) : ( ScrollTo( ListHeight - (rClient.bottom - rClient.top) + 40, 1000) );
			break;
			// left
		case 37:
			{
				if (LIST_DISPLAY_TYPE == LIST_DISPLAY_DETAILS) {
					LastListItemSelected = ListItemSelected;
					GetItemData();
					SubListCurrentAction = -1;
				} else if (LIST_DISPLAY_TYPE == LIST_DISPLAY_LIST) {
					if(display_favorites == false) break;
					display_favorites = false;
					InitListData();
					//ClearList();
					rePopulate();
					CalculateHeights();
					RedrawList();
				}
			}
			break;

			// up
		case 38:
			if (LIST_DISPLAY_TYPE == LIST_DISPLAY_LIST) {
				if ((ListData[ListItemSelected].StartingPosition < Scrolled) ||
					(ListData[ListItemSelected].StartingPosition > (Scrolled + (rClient.bottom - rClient.top))))
				{
					ScrollTo(ListData[PixelToItem[Scrolled]].StartingPosition, 200);
					ListItemSelected = PixelToItem[Scrolled];
				}
				else if (ListItemSelected < 0)
				{
					ListItemSelected = PixelToItem[Scrolled];
				}
				else
				{
					ListItemSelected--;

					if (ListData[ListItemSelected].Group == ListItemSelected)
						ListItemSelected--;

					if (ListItemSelected < 0)
						ListItemSelected = ListItemSelected + 2;

					if (ListData[ListItemSelected].StartingPosition < Scrolled)
					{
						if (ListData[ListItemSelected].StartingPosition - (rClient.bottom - rClient.top) + ListData[ListItemSelected].ItemHeight < 0)
							ScrollTo(0, 200);
						else
							ScrollTo(ListData[ListItemSelected].StartingPosition - (rClient.bottom - rClient.top) + ListData[ListItemSelected].ItemHeight, 200);
					}
				}
				LastHeight = ListData[ListItemSelected].ItemHeight;
				RedrawList();
				break;
			} else if (LIST_DISPLAY_TYPE == LIST_DISPLAY_DETAILS) {
				if(SubListCurrentAction > 0) {
					SubListCurrentAction--;
					if(ActionsList[SubListCurrentAction].action == SLA_TEXT) {
						if (SubListCurrentAction > 0)
							SubListCurrentAction--;
						else
							SubListCurrentAction = ActionsNumber - 1;
					}
				} else {
					SubListCurrentAction = ActionsNumber - 1;
					if(ActionsList[SubListCurrentAction].action == SLA_TEXT) {
						if (SubListCurrentAction > 0)
							SubListCurrentAction--;
						else
							SubListCurrentAction = ActionsNumber - 1;
					}
				}
				RedrawList();
				break;
			}
			// right
		case 39:
			{
				if (LIST_DISPLAY_TYPE == LIST_DISPLAY_DETAILS) {
					if (SubListCurrentAction > -1 && ActionsList[SubListCurrentAction].sms == true) {
						SendSMS();
						LastListItemSelected = -1;
						ResizeTo(ListItemSelected, 200);
						LIST_DISPLAY_TYPE = LIST_DISPLAY_LIST;
						break;
					}
				} else if (LIST_DISPLAY_TYPE == LIST_DISPLAY_LIST) {
					if(display_favorites == true) break;
					display_favorites = true;
					Scrolled = 0;
					InitListData();
					//ClearList();
					rePopulate();
					CalculateHeights();
					RedrawList();
					//ShowFavorites(1, 300);
				}
			}
			break;
			// down
		case 40:
			if (LIST_DISPLAY_TYPE == LIST_DISPLAY_LIST) {
				if ((ListData[ListItemSelected].StartingPosition < Scrolled) ||
					(ListData[ListItemSelected].StartingPosition > (Scrolled + (rClient.bottom - rClient.top))))
				{
					ScrollTo(ListData[PixelToItem[Scrolled]].StartingPosition, 200);
					ListItemSelected = PixelToItem[Scrolled];
				}
				else if (ListItemSelected < 0)
				{
					ListItemSelected = PixelToItem[Scrolled];
				}
				else
				{
					ListItemSelected++;
					if (ListData[ListItemSelected].ID < 0)
					{
						ListItemSelected--;
					}

					if (ListData[ListItemSelected].Group == ListItemSelected)
						ListItemSelected++;

					if ((ListData[ListItemSelected].StartingPosition + ListData[ListItemSelected].ItemHeight) > Scrolled + (rClient.bottom - rClient.top))
					{
						if ((ListData[ListItemSelected].StartingPosition + (rClient.bottom - rClient.top)) > ListHeight)
							ScrollTo(ListHeight - (rClient.bottom - rClient.top), 200);
						else
							ScrollTo(ListData[ListItemSelected].StartingPosition, 200);
					}
				}
				LastHeight = ListData[ListItemSelected].ItemHeight;
				RedrawList();
				break;
			} else if (LIST_DISPLAY_TYPE == LIST_DISPLAY_DETAILS) {
				if(SubListCurrentAction > -1) {
					if(SubListCurrentAction < ActionsNumber - 1) {
						SubListCurrentAction++;
						if(ActionsList[SubListCurrentAction].action == SLA_TEXT) {
							if (SubListCurrentAction < ActionsNumber - 1)
								SubListCurrentAction++;
							else
								SubListCurrentAction = 0;
						}
					} else {
						SubListCurrentAction = 0;
						if(ActionsList[SubListCurrentAction].action == SLA_TEXT) {
							if (SubListCurrentAction < ActionsNumber - 1)
								SubListCurrentAction++;
							else
								SubListCurrentAction = 0;
						}
					}
				} else {
					SubListCurrentAction++;
				}
				RedrawList();
				break;
			}
			// enter
		case 13:
			if (LIST_DISPLAY_TYPE == LIST_DISPLAY_LIST) {
				if (ListItemSelected > -1)
				{
					LastListItemSelected = ListItemSelected;
					GetItemData();
				}
				break;
			} else if (LIST_DISPLAY_TYPE == LIST_DISPLAY_DETAILS) {
				if(SubListCurrentAction > -1) {
					switch(ActionsList[SubListCurrentAction].action) {
						case SLA_CALL:
							Call();
							LastListItemSelected = -1;
							SubListCurrentAction = -1;
							ResizeTo(ListItemSelected, 200);
							LIST_DISPLAY_TYPE = LIST_DISPLAY_LIST;
							break;
						case SLA_EMAIL:
							SendMail();
							LastListItemSelected = -1;
							SubListCurrentAction = -1;
							ResizeTo(ListItemSelected, 200);
							LIST_DISPLAY_TYPE = LIST_DISPLAY_LIST;
							break;
					}
				} else {
					LastListItemSelected = ListItemSelected;
					GetItemData();
				}

			}
		}
		break;
	}
	return TRUE;
	//return CallWindowProc (OldCntListProc, hwnd, message, wParam, lParam);
}

HWND CreateContactList(HWND hParent, RECT rSize, bool (*rePop)())
{

	WNDCLASS wc;

	wc.style         = 0;
	wc.lpfnWndProc   = CntListProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.hInstance     = 0;
	wc.hIcon         = 0;
	wc.hCursor       = 0;
	wc.hbrBackground = (HBRUSH) GetStockObject(BLACK_BRUSH);
	wc.lpszMenuName  = 0;
	wc.lpszClassName = TEXT("ScrollList");

	RegisterClass(&wc);

	rePopulate = rePop;

	rCL = rSize;
	hwndParent = hParent;
	hContainer = CreateWindow(TEXT("ScrollList"), NULL, WS_CHILD | WS_VISIBLE, rCL.left, rCL.top, rCL.right-rCL.left, rCL.bottom-rCL.top, hParent, NULL, NULL, NULL);
	//OldCntListProc = (WNDPROC)SetWindowLong(hContainer, GWL_WNDPROC, (LONG)CntListProc);
	
	//Get program file path
	TCHAR buf[MAX_PATH];
	TCHAR filename[] = TEXT("\\settings.ini");
	//wsprintf(buf,_T("%d - %d\n"),optimizerTime2, sItem);
	
	GetModuleFileName(NULL, buf, MAX_PATH);

	//Get program folder
	char * pch;
	char * pch2;
	pch = (char*) memchr (buf, '\\', wstrlen(buf));
	//pch = strrchr(buf,'\\');
	while(pch != NULL) {
		pch = (char*) memchr (pch + 2, '\\', wstrlen(buf));
		if(pch != NULL) pch2 = pch;
	}
	memcpy(ini_path, buf,(char *)pch2 - (char *)buf);
	memcpy(ini_path + wstrlen(ini_path), filename, wstrlen(filename)*2+1);
	//OutputDebugString(path);

	ini.LoadFile(ini_path);
	email_account = ini.GetValue(TEXT("main"), TEXT("email_account"), TEXT("default") /*default*/);
	abc_button_string = ini.GetValue(TEXT("abc_button"), TEXT("abc_button_string"),  TEXT("ABC") /*default*/);

	mobile_string	= ini.GetValue(TEXT("contact_details"), TEXT("mobile_string"), TEXT("Mobile") /*default*/);
	home_string		= ini.GetValue(TEXT("contact_details"), TEXT("home_string"), TEXT("Home") /*default*/);
	work_string		= ini.GetValue(TEXT("contact_details"), TEXT("work_string"), TEXT("Work") /*default*/);
	company_string	= ini.GetValue(TEXT("contact_details"), TEXT("company_string"), TEXT("Company") /*default*/);
	email_string	= ini.GetValue(TEXT("contact_details"), TEXT("email_string"), TEXT("E-mail") /*default*/);
	sms_string		= ini.GetValue(TEXT("contact_details"), TEXT("sms_string"), TEXT("SMS") /*default*/);

	InitListData();
	InitSurface();

	DWORD th = 20;
	SwitchTheme(&Theme, (int)th);

	ListBackgroundBrush = CreateSolidBrush(Theme.ListBackgroundColor);

	ListFont = BuildFont(ItemFontSize, FALSE, FALSE);
	ListItemBackgroundBrush = CreateSolidBrush(Theme.ListItemBackgroundColor);
	ListItemHoverBackgroundBrush = CreateSolidBrush(Theme.ListItemHoverBackgroundColor);

	ListSelectedBackgroundBrush = CreateSolidBrush(Theme.ListSelectedBackgroundColor);
	ListSelectedPen = CreatePen(PS_SOLID, 1, Theme.ListSelectedEditBackgroundColor);
	ListSelectedBrush = CreateSolidBrush(Theme.ListSelectedEditBackgroundColor);
	ListSelectedItemDetailsButtonBrush = CreateSolidBrush(Theme.ListSelectedItemDetailsButtonColor);
	ListSelectedItemDetailsButtonPen = CreatePen(PS_SOLID, 1, Theme.ListSelectedItemDetailsButtonColor);

	ItemDetailsButtonFont = BuildFont(ItemDetailsButtonFontSize, FALSE, FALSE);

	ListSeparatorBrush = CreateSolidBrush(Theme.ListSeparatorColor);
	GroupFont = BuildFont(GroupItemFontSize, TRUE, FALSE);
	ListGroupBackgroundBrush = CreateSolidBrush(Theme.ListGroupBackgroundColor);

	ListIndicatorPen = CreatePen(PS_SOLID, 1, Theme.ListIndicatorBrushColor);
	ListIndicatorBrush = CreateSolidBrush(Theme.ListIndicatorBrushColor);
	ListIndicatorFont = BuildFont(ListIndicatorFontSize, TRUE, FALSE);

	KeyboardButtonPen = CreatePen(PS_SOLID, 1, Theme.KeyboardButtonBrushColor);
	KeyboardButtonBrush = CreateSolidBrush(Theme.KeyboardButtonBrushColor);
	KeyboardButtonFont = BuildFont(KeyboardButtonFontSize, TRUE, FALSE);
	rKeyboardButton.left = rClient.right - 45;
	rKeyboardButton.top = rClient.bottom - 25;
	rKeyboardButton.right = rClient.right + 10;
	rKeyboardButton.bottom = rClient.bottom + 10;

	KeyboardBackgroundBrush = CreateSolidBrush(Theme.KeyboardBackgroundBrushColor);
	rKeyboardFull = rKeyboardButton;
	KeyboardKeyBrush = CreateSolidBrush(Theme.KeyboardKeyBrushColor);
	KeyboardSelectedBrush = CreateSolidBrush(Theme.KeyboardSelectedBrushColor);
	KeyboardGridPen = CreatePen(PS_SOLID, 1, Theme.KeyboardGridColor);

	ShowWindow(hContainer, SW_SHOW);
	UpdateWindow(hContainer);

	GetPoomFolder(olFolderContacts, &pCurrFldr);
	pCurrFldr->get_Items(&pItemCol2);

	return hContainer;
}

void DestroyList()
{
	ListCounter = 0;
	for(int i = 0; i < MAX_LIST_ITEMS; i++)
	{
		delete ListData[i].Text;
	}
	pItemCol2->Release();
	pCurrFldr->Release();
	DestroyWindow(hContainer);
}

void RedrawList()
{
	InvalidateRect(hContainer, &rClient, FALSE);
	UpdateWindow(hContainer);
}

bool ClearList()
{
	ListCounter = 0;
	GroupCounter = 0;
	for(int i = 0; i < MAX_LIST_ITEMS; i++)
	{
		//delete ListData[i].Text;
		ListData[i].ID = -1;
		ListData[i].ItemHeight = DEFAULT_ITEM_HEIGHT;
		ListData[i].GroupHeader = false;
	}
	for(int j = 0; j < 30; j++)
	{
		GroupArray[j].ID = -1;
	}

	InitListData();
	InitSurface();

	return true;
}

void ResizeList(RECT r)
{
	rCL = r;
	MoveWindow(hContainer, rCL.left, rCL.top, rCL.right - rCL.left, rCL.bottom - rCL.top, TRUE);
	InitSurface();
}

bool InitListData()
{
	ListCounter = 0;
	GroupCounter = 0;
	for(int i = 0; i < MAX_LIST_ITEMS; i++)
	{
		ListData[i].ID = -1;
		ListData[i].ItemHeight = DEFAULT_ITEM_HEIGHT;
		ListData[i].GroupHeader = false;
	}
	for(int j = 0; j < 30; j++)
	{
		GroupArray[j].ID = -1;
	}
	return true;
}

bool InitSubListData()
{
	for(int j = 0; j < SLA_NUMBER; j++)
	{
		ActionsList[j].id = -1;
		ActionsList[j].action = -1;
		ActionsList[j].sms = false;
	}
	return true;
}

bool InitSurface()
{
	PAINTSTRUCT  ps;
	HDC          hdc;
	hdc = BeginPaint( hContainer, &ps );
	GetClientRect(hContainer, &rClient);

	hdcMem = CreateCompatibleDC(hdc);
	hbmMem = CreateCompatibleBitmap(hdc, rClient.right - rClient.left, rClient.bottom - rClient.top);
	hOld   = SelectObject(hdcMem, hbmMem);

	hdcKeyboard = CreateCompatibleDC(hdc);
	hbmKeyboard = CreateCompatibleBitmap(hdc, rClient.right - rClient.left, rClient.bottom - rClient.top);
	hOld   = SelectObject(hdcKeyboard, hbmKeyboard);

	EndPaint(hContainer, &ps);
	DeleteDC(hdc);

	return true;
}

bool AddItem(int ID, WCHAR * tszTxt, bool GroupHeader, int ItemHeight, int Group, LONG oId)
{
	TCHAR buf[20];
	wsprintf(buf, TEXT("%li"), oId);
	if(display_favorites && memcmp(ini.GetValue(TEXT("favorites"), buf, TEXT("0") /*default*/), TEXT("1"), 2) != 0)
		return false;
	if(GroupHeader == true)
	{
		CurrentGroup = ListCounter;
		GroupArray[GroupCounter].ID = CurrentGroup;
		GroupCounter++;
	}
	ListData[ListCounter].ID = ID;
	ListData[ListCounter].Text = tszTxt;
	ListData[ListCounter].TextLength = wcslen(tszTxt);
	ListData[ListCounter].GroupHeader = GroupHeader;
	ListData[ListCounter].ItemHeight = ItemHeight;
	ListData[ListCounter].Group = CurrentGroup;
	ListData[ListCounter].oId = oId;
	
	if(memcmp(ini.GetValue(TEXT("favorites"), buf, TEXT("0") /*default*/), TEXT("1"), 2) == 0) {
		ListData[ListCounter].Favorite = true;	
	} else {
		ListData[ListCounter].Favorite = false;
	}
	ListCounter++;
	return true;
}

void CalculateHeights()
{
	int c = 0;
	for(int i = 0; i < MAX_LIST_ITEMS; i++)
	{
		if (ListData[i].ID < 0)
			break;
		ListData[i].StartingPosition = c;

		for(int a = 0; a < ListData[i].ItemHeight + ListSeparatorHeight; a++)
		{
			PixelToItem[c] = i;
			c++;
		}
/*
		if(display_favorites == true) {
			if(ListData[i].GroupHeader == true) continue;
			if(ListData[i].Favorite == true)
				for(int a = 0; a < ListData[i].ItemHeight + ListSeparatorHeight; a++)
				{
					PixelToItem[c] = i;
					c++;
				}
			else
				for(int a = 0; a < (ListData[i].ItemHeight * ShowFavorites_Amount); a++)
				{
					PixelToItem[c] = i;
					c++;
				}		
		} else {
			for(int a = 0; a < ListData[i].ItemHeight + ListSeparatorHeight; a++)
			{
				PixelToItem[c] = i;
				c++;
			}		
		}
*/
		/*
		if (ListData[i+1].ID > -1)
		c += ListSeparatorHeight;
		*/
	}
	ListHeight = c;
}

void GetItemData()
{
	int c = 0;
	InitSubListData();

	if(ListData[ListItemSelected].ID == 10000) {
		//pItemCol2->Add(reinterpret_cast<IDispatch**>(&pContact2));
		HRESULT hr = 0;
		IPOutlookApp * pOutlook;
		IContact * pContact3;
		pItemCol2->get_Application(&pOutlook);
		pOutlook->CreateItem(olContactItem, (IDispatch**)&pContact3);
		hr = pContact3->Display();
		pContact3->Release();
		pOutlook->Release();
		return;
	}
	
	// GET CURRENT CONTACT
	pItemCol2->Item(ListData[ListItemSelected].ID + 1, reinterpret_cast<IDispatch**>(&pContact2));

	// GET CONTACT DETAILS
	pContact2->get_MobileTelephoneNumber(&contact_MobileNumberPrivate);
	if(SysStringLen(contact_MobileNumberPrivate) > 0) {
		ActionsList[c].id = c + 1;
		ActionsList[c].text = contact_MobileNumberPrivate;
		ActionsList[c].action = SLA_CALL;
		ActionsList[c].sms = true;
		wsprintf(ActionsList[c].label, mobile_string);
		c++;
	}

	pContact2->get_HomeTelephoneNumber(&contact_HomeNumber);
	if(SysStringLen(contact_HomeNumber) > 0) {
		ActionsList[c].id = c + 1;
		ActionsList[c].text = contact_HomeNumber;
		ActionsList[c].action = SLA_CALL;
		wsprintf(ActionsList[c].label, home_string);
		c++;
	}
	/*
	pContact2->get_Business2TelephoneNumber(&contact_MobileNumberWork);
	if(SysStringLen(contact_MobileNumberWork) > 0) {
		ActionsList[c].id = c + 1;
		ActionsList[c].text = contact_MobileNumberWork;
		ActionsList[c].action = SLA_CALL;
		wsprintf(ActionsList[c].label, _T("Work mobile"));
		c++;
	}*/
	
	pContact2->get_BusinessTelephoneNumber(&contact_WorkNumber);
	if(SysStringLen(contact_WorkNumber) > 0) {
		ActionsList[c].id = c + 1;
		ActionsList[c].text = contact_WorkNumber;
		ActionsList[c].action = SLA_CALL;
		ActionsList[c].sms = true;
		wsprintf(ActionsList[c].label, work_string);
		c++;
	}
	
	pContact2->get_CompanyName(&contact_Company);
	if(SysStringLen(contact_Company) > 0) {
		ActionsList[c].id = c + 1;
		ActionsList[c].text = contact_Company;
		ActionsList[c].action = SLA_TEXT;
		wsprintf(ActionsList[c].label, company_string);
		c++;
	}
	
	pContact2->get_Email1Address(&contact_Email);
	if(SysStringLen(contact_Email) > 0) {
		ActionsList[c].id = c + 1;
		ActionsList[c].text = contact_Email;
		ActionsList[c].action = SLA_EMAIL;
		wsprintf(ActionsList[c].label, email_string);
		c++;
	}
	
	/*
	pContact2->get_Body(&contact_Notes);
	if(SysStringLen(contact_Notes) > 0) {
		ActionsList[c].id = c + 1;
		ActionsList[c].text = contact_Notes;
		ActionsList[c].action = SLA_TEXT;
		wsprintf(ActionsList[c].label, _T("Notes"));
		c++;
	}
	*/

	ActionsNumber = c;

	RedrawList();
	ResizeTo(ListItemSelected, 200);

	pContact2->Release();
}

void Call() {
	PHONEMAKECALLINFO mci = {0};
	LRESULT  lResult;

/*
	TCHAR tszPhoneNumber[64];

	memset(tszPhoneNumber, _T('\0'), sizeof(tszPhoneNumber));
	_tcscpy(tszPhoneNumber, _T("0989330399"));
*/
	mci.cbSize = sizeof(mci);
	mci.dwFlags = 0;
	//mci.pszDestAddress = tszPhoneNumber;
	mci.pszDestAddress = ActionsList[SubListCurrentAction].text;
	lResult = PhoneMakeCall(&mci);
}

void SendSMS() {
    PROCESS_INFORMATION pi;
	TCHAR tszAppName[100];
    TCHAR tszCommandLine[500];

	_tcscpy(tszAppName, TEXT("tmail.exe"));
	wsprintf(tszCommandLine,  TEXT("-service \"SMS\" -to \"%s\""), ActionsList[SubListCurrentAction].text);
    CreateProcess(tszAppName, tszCommandLine, NULL, NULL, FALSE, NULL, NULL, NULL, NULL, &pi);
}

void SendMail() {
    PROCESS_INFORMATION pi;
	TCHAR tszAppName[100];
    TCHAR tszCommandLine[500];

	_tcscpy(tszAppName, TEXT("tmail.exe"));
	wsprintf(tszCommandLine,  TEXT("-service \"%s\" -to \"%s\""), (LPCWSTR)email_account, ActionsList[SubListCurrentAction].text);
	CreateProcess(tszAppName, tszCommandLine, NULL, NULL, FALSE, NULL, NULL, NULL, NULL, &pi);
}

void AddToFavorites(LONG oId, int ID) {
	TCHAR buf[20];
	wsprintf(buf, TEXT("%li"), oId);
	ini.SetValue(TEXT("favorites"), buf, TEXT("1"));
	ini.SaveFile(ini_path);
	ListData[ID].Favorite = true;
}

void RemoveFromFavorites(LONG oId, int ID) {
	TCHAR buf[20];
	wsprintf(buf, TEXT("%li"), oId);
	ini.Delete(TEXT("favorites"), buf, true);
	ini.SaveFile(ini_path);
	ListData[ID].Favorite = false;
	if(display_favorites == true) {
		Scrolled = 0;
		InitListData();
		rePopulate();
		CalculateHeights();
		RedrawList();
	}
}

void MoveList(double Amount)
{
	int tmpScrolled;
	tmpScrolled = Scrolled;

	if(ListHeight < (rClient.bottom - rClient.top))
		return;

	if ((Scrolled - Amount) < 0)
	{
		if(bScrolling)
		{
			SetTimer(hContainer, IDT_TIMER_BOUNCE, RefreshRate, NULL);
			Offset = Amount;
		}
		tmpScrolled = 0;
		bScrolling = false;
		bStoping = false;
		KillTimer(hContainer, IDT_TIMER_SCROLL);
		KillTimer(hContainer, IDT_TIMER_SCROLL_TO);
		Speed = 0;
	}
	else if ((ListHeight - rClient.bottom + rClient.top) < (Scrolled - Amount))
	{
		tmpScrolled = ListHeight - (rClient.bottom - rClient.top);
		if(bScrolling)
		{
			SetTimer(hContainer, IDT_TIMER_BOUNCE, RefreshRate, NULL);
			Offset = Amount;
		}
		bScrolling = false;
		bStoping = false;
		KillTimer(hContainer, IDT_TIMER_SCROLL);
		KillTimer(hContainer, IDT_TIMER_SCROLL_TO);
		Speed = 0;
	}
	else
	{
		tmpScrolled -= (int)Amount;
	}

	if (Scrolled == tmpScrolled)
	{
		RedrawList();
		return;
	}
	Scrolled = tmpScrolled;
	RedrawList();
}

void ScrollTo(int position, int Duration)
{
	Scroll_StartPosition = Scrolled;
	Scroll_Change = position - Scroll_StartPosition;
	Scroll_Duration = Duration / RefreshRate;
	Scroll_TimeCounter = 0;
	SetTimer(hContainer, IDT_TIMER_SCROLL_TO, RefreshRate, NULL);
}

void ResizeTo(int Item, int duration)
{
	if (LIST_DISPLAY_TYPE == LIST_DISPLAY_LIST)
	{
		LastPosition = Scroll_StartPosition = Scrolled;
		Scroll_Change = ListData[Item].StartingPosition - Scroll_StartPosition + 1;
		Scroll_Duration = duration / RefreshRate;
		Scroll_TimeCounter = 0;

		Indent_StartPosition = 0;
		Indent_Change = 40;
		Indent_Duration = duration / RefreshRate;
		Indent_TimeCounter = 0;

		LastHeight = ListData[Item].ItemHeight;
		Resize_StartHeight = ListData[Item].ItemHeight;
		Resize_Change = rClient.bottom - rClient.top - Resize_StartHeight + 1;
		Resize_Duration = duration / RefreshRate;
		Resize_TimeCounter = 0;
		SetTimer(hContainer, IDT_TIMER_RESIZE_TO, RefreshRate, NULL);
		LIST_DISPLAY_TYPE = LIST_DISPLAY_DETAILS;
	}
	else if (LIST_DISPLAY_TYPE == LIST_DISPLAY_DETAILS)
	{
		Scroll_StartPosition = ListData[Item].StartingPosition;
		Scroll_Change = LastPosition - Scroll_StartPosition;
		Scroll_Duration = duration / RefreshRate;
		Scroll_TimeCounter = 0;

		Indent_StartPosition = Indent;
		Indent_Change = -40;
		Indent_Duration = duration / RefreshRate;
		Indent_TimeCounter = 0;

		Resize_StartHeight = ListData[Item].ItemHeight;
		Resize_Change = LastHeight - Resize_StartHeight;
		Resize_Duration = duration / RefreshRate;
		Resize_TimeCounter = 0;
		SetTimer(hContainer, IDT_TIMER_RESIZE_TO, RefreshRate, NULL);
		LIST_DISPLAY_TYPE = LIST_DISPLAY_LIST;
	}
}

void SwitchKeyboard(int action, int duration)
{
	if (action == 1)
	{
		LastListItemSelected = ListItemSelected;

		Keyboard_Start = 10;
		Keyboard_Change = 91;
		Keyboard_Duration = duration / RefreshRate;
		Keyboard_TimeCounter = 0;
	}
	else
	{
		Keyboard_Start = 101;
		Keyboard_Change = -91;
		Keyboard_Duration = duration / RefreshRate;
		Keyboard_TimeCounter = 0;
	}
	SetTimer(hContainer, IDT_TIMER_KEYBOARD_SWITCH, RefreshRate, NULL);
}

void ShowPlus(int action, int duration)
{
	if (action == 1)
	{
		LastListItemSelected = ListItemSelected;

		ShowPlus_Start = 0;
		ShowPlus_Change = 50;
		ShowPlus_Duration = duration / RefreshRate;
		ShowPlus_TimeCounter = 0;
	}
	else
	{
		ShowPlus_Start = 50;
		ShowPlus_Change = -50;
		ShowPlus_Duration = duration / RefreshRate;
		ShowPlus_TimeCounter = 0;
	}
	SetTimer(hContainer, IDT_TIMER_SHOW_PLUS, RefreshRate, NULL);
}

void ShowFavorites(int action, int duration)
{
	if (action == 1)
	{
		ShowFavorites_Start = 1;
		ShowFavorites_Change = -1;
		ShowFavorites_Duration = duration / RefreshRate;
		ShowFavorites_TimeCounter = 0;
	}
	else
	{
		ShowFavorites_Start = 0;
		ShowFavorites_Change = 1;
		ShowFavorites_Duration = duration / RefreshRate;
		ShowFavorites_TimeCounter = 0;
	}
	SetTimer(hContainer, IDT_TIMER_SHOW_FAVORITES, RefreshRate, NULL);
}
