//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this source code is subject to the terms of the Microsoft end-user
// license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
// If you did not accept the terms of the EULA, you are not authorized to use
// this source code. For a copy of the EULA, please see the LICENSE.RTF on your
// install media.
//

///////////////////////////////////////////////////////////////////////////////

#pragma once


#define _ErrorLabel Error


#define CHR(hResult) \
    if(FAILED(hResult)) { hr = (hResult); goto _ErrorLabel;} 


#define CPR(pPointer) \
    if(NULL == (pPointer)) { hr = (E_OUTOFMEMORY); goto _ErrorLabel;} 


#define CBR(fBool) \
    if(!(fBool)) { hr = (E_FAIL); goto _ErrorLabel;} 


#define ARRAYSIZE(s) (sizeof(s) / sizeof(s[0]))


#define RELEASE_OBJ(s)  \
    if (s != NULL)      \
    {                   \
        s->Release();   \
        s = NULL;       \
    }


  

