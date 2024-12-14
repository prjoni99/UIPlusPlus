// CXPoint.h  Version 1.0
//
// Author:  Hans Dietrich
//          hdietrich@gmail.com
//
// History
//     Version 1.0 - 2009 May 12
//     - Initial release
//
// License:
//     This file is Copyright © 2009 Hans Dietrich. All Rights Reserved.
//
//     This source file is the property of Hans Dietrich and is not to be
//     re-distributed by any means whatsoever without the expressed written
//     consent of Hans Dietrich.
//
//     This source code can only be used under the Terms of Use set forth
//     on the Hans Dietrich Software web site. Hans Dietrich Software grants 
//     to you (one software developer) the limited right to use this software.
//
//     This software is provided "as is" with no expressed or implied warranty.
//     Hans Dietrich accepts no liability for any damage or loss of business 
//     that this software may cause.
//
// Public Members:
//           NAME                             DESCRIPTION
//   ---------------------   -------------------------------------------------
//   Construction
//      CXPoint()            Ctor
//      CXPoint(int, int)    Ctor from ints
//      CXPoint(POINT)       Ctor from POINT
//      CXPoint(SIZE)        Ctor from SIZE
//      CXPoint(DWORD)       Ctor from DWORD
//   Operations
//      Offset(int, int)     Adds int values to the x and y members of the CXPoint
//      Offset(POINT)        Adds POINT values to the x and y members of the CXPoint
//      Offset(SIZE)         Adds SIZE values to the x and y members of the CXPoint
//      operator==           Checks for equality between two points
//      operator!=           Checks for inequality between two points
//      operator+=           Offsets CXPoint by adding a size or point
//      operator-=           Offsets CXPoint by subtracting a size or point
//      operator+            Returns the sum of a CXPoint and a size or point
//      operator-            Returns the difference of a CXPoint and a size or point
//
///////////////////////////////////////////////////////////////////////////////

#ifndef CXPOINT_H
#define CXPOINT_H

#pragma message("including CXPoint.h")

namespace HansDietrich {

	class CXPoint : public tagPOINT
	{
	public:
		// Constructors

			// create an uninitialized point
		CXPoint() { }
		// create from two integers
		CXPoint(int initX, int initY) { x = initX; y = initY; }
		// create from another point
		CXPoint(POINT initPt) { *(POINT*)this = initPt; }
		// create from a size
		CXPoint(SIZE initSize) { *(SIZE*)this = initSize; }
		// create from a dword: x = LOWORD(dw) y = HIWORD(dw)
		CXPoint(DWORD pt) { x = (short)LOWORD(pt); y = (short)HIWORD(pt); }

		// Operations

		// translate the point
		void Offset(int xOffset, int yOffset) { x += xOffset; y += yOffset; }
		void Offset(POINT point) { x += point.x; y += point.y; }
		void Offset(SIZE size) { x += size.cx; y += size.cy; }

		BOOL operator==(POINT point) const { return (x == point.x && y == point.y); }
		BOOL operator!=(POINT point) const { return (x != point.x || y != point.y); }
		void operator+=(SIZE size) { x += size.cx; y += size.cy; }
		void operator-=(SIZE size) { x -= size.cx; y -= size.cy; }
		void operator+=(POINT point) { x += point.x; y += point.y; }
		void operator-=(POINT point) { x -= point.x; y -= point.y; }

		// Operators returning CXPoint values
		CXPoint operator+(SIZE size) const { return CXPoint(x + size.cx, y + size.cy); }
		CXPoint operator-(SIZE size) const { return CXPoint(x - size.cx, y - size.cy); }
		CXPoint operator-() const { return CXPoint(-x, -y); }
		CXPoint operator+(POINT point) const { return CXPoint(x + point.x, y + point.y); }

		// Operators returning CSize values
			//CSize operator-(POINT point) const;

		// Operators returning CRect values
			//CXRect operator+(const RECT* lpRect) const { return CXRect(lpRect) + *this; }
			//CXRect operator-(const RECT* lpRect) const { return CXRect(lpRect) - *this; }
	};
}
#endif //CXPOINT_H
