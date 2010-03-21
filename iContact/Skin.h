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

#include "stdafx.h"

//----------------------------------------------------------------------
// Skin defines and data types
//

#define SKIN_HEADER_HEIGHT          24
#define SKIN_HEADER_Y_OFFSET        16
#define SKIN_MENU_BAR_HEIGHT        40
#define SKIN_MENU_BAR_Y_OFFSET      40
#define SKIN_MENU_BAR_SEL_Y_OFFSET  80
#define SKIN_MENU_BAR_ICON_WIDTH    48
#define SKIN_CANVAS_HEIGHT          4
#define SKIN_CANVAS_Y_OFFSET        120
#define SKIN_COLORS_Y_OFFSET		124
#define SKIN_WIDTH                  240

// These sizes are for screens of 96 DPI only
// For 192 DPI, these will be adjusted
#define DEFAULT_DPI                 96
#define DEFAULT_SCREEN_WIDTH		240
#define DEFAULT_SKIN_HEIGHT			125

// X-Location of skin colors
#define SKIN_COLOR_HIGHLIGHT				0   //4b5ab5
#define SKIN_COLOR_HEADER_TEXT				1	//ffffff
#define SKIN_COLOR_HEADER_LOADING_TEXT		2	//7f7f7f
#define SKIN_COLOR_LIST_GROUP_BACKGROUND	3	//000000
#define SKIN_COLOR_LIST_GROUP_TEXT			4	//ffffff
#define SKIN_COLOR_LIST_ITEM_BACKGROUND		5	//1e1e1e
#define SKIN_COLOR_LIST_ITEM_TEXT			6	//dcdcdc
#define SKIN_COLOR_LIST_ITEM_MISSED			7	//ff0000
#define SKIN_COLOR_LIST_ITEM_FAVORITE		8	//ffff00
#define SKIN_COLOR_LIST_ITEM_SEPARATOR		9	//323232
#define SKIN_COLOR_DETAIL_MAIN_TEXT			10	//e6e6e6
#define SKIN_COLOR_DETAIL_MAIN_SHADOW		11	//505050
#define SKIN_COLOR_DETAIL_ITEM_SEPARATOR	12	//999999
#define SKIN_COLOR_KEYBOARD_TEXT			13	//dcdcdc
#define SKIN_COLOR_KEYBOARD_BACKGROUND		14	//464646
#define SKIN_COLOR_KEYBOARD_GRID			15	//646464
