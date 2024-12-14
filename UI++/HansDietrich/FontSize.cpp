// FontSize.cpp  Version 1.4
//
// Author:  Hans Dietrich
//          hdietrich@gmail.com
//
// License:
//     This file is Copyright © 2010 Hans Dietrich. All Rights Reserved.
//
//     This source file is the property of Hans Dietrich.  This source code 
//     can be used freely for personal and commercial projects without 
//     restriction, provided you keep intact the copyright notices.
//
//     This software is provided "as is" with no expressed or implied warranty.
//     Hans Dietrich accepts no liability for any damage or loss of business 
//     that this software may cause.
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FontSize.h"

namespace HansDietrich {

///////////////////////////////////////////////////////////////////////////////
// GetFontPointSize()
int GetFontPointSize(int nHeight)
{
	HDC hdc = ::CreateDC(_T("DISPLAY"), NULL, NULL, NULL);
	ASSERT(hdc);
	int cyPixelsPerInch = ::GetDeviceCaps(hdc, LOGPIXELSY);
	::DeleteDC(hdc);

	int nPointSize = MulDiv(nHeight, 72, cyPixelsPerInch);
	if (nPointSize < 0)
		nPointSize = -nPointSize;

	return nPointSize;
}

///////////////////////////////////////////////////////////////////////////////
// GetFontHeight()
int GetFontHeight(int nPointSize)
{
	HDC hdc = ::CreateDC(_T("DISPLAY"), NULL, NULL, NULL);
	ASSERT(hdc);
	int cyPixelsPerInch = ::GetDeviceCaps(hdc, LOGPIXELSY);
	::DeleteDC(hdc);

	int nHeight = -MulDiv(nPointSize, cyPixelsPerInch, 72);

	return nHeight;
}

}