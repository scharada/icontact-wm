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

BOOL CreateShortcutFile(const TCHAR * szShortcutTitle,
                        const TCHAR * szObjectPath,
                        const TCHAR * szArguments,
                        const TCHAR * szIconsPath);

BOOL RemoveShortcutFile(const TCHAR * szShortcutTitle);

BOOL GetShortcutFilename(TCHAR * szShortcutPath, 
						 const TCHAR * szShortcutTitle);