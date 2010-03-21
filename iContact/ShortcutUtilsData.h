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

// portions of this file were originally from Google Gears
// (http://gears.googlecode.com)
// which is licensed as follows:

// Copyright 2008, Google Inc.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//  1. Redistributions of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//  2. Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//  3. Neither the name of Google Inc. nor the names of its contributors may be
//     used to endorse or promote products derived from this software without
//     specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
// WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
// EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
// OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
// ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


// The resource ID of the icon in the DLL. Note that this value is baked into
// the DLL data in the arrays defined below, so should not be changed.
#define DLL_ICON_ID 2001

// The DLL data before the embedded icon resource.
extern const int kIcon16DllBegin[];
extern const int kIcon16DllBeginSize;
extern const int kIcon32and16DllBegin[];
extern const int kIcon32and16DllBeginSize;
// The DLL data for the icon header.
extern const int kIcon16DllHeader[];
extern const int kIcon16DllHeaderSize;
extern const int kIcon32and16DllHeader[];
extern const int kIcon32and16DllHeaderSize;
// The DLL data for the icon image footers.
extern const int kIcon16Image16Footer[];
extern const int kIcon16Image16FooterSize;
extern const int kIcon32and16Image32Footer[];
extern const int kIcon32and16Image32FooterSize;
extern const int kIcon32and16Image16Footer[];
extern const int kIcon32and16Image16FooterSize;
// The DLL data after the embedded icon resource.
extern const int kIcon16DllEnd[];
extern const int kIcon16DllEndSize;
extern const int kIcon32and16DllEnd[];
extern const int kIcon32and16DllEndSize;
