// CXRect.h  Version 1.2 - see article at http://www.codeproject.com
//
// Author:  Hans Dietrich
//          hdietrich@gmail.com
//
// License:
//     This file is Copyright © 2011 Hans Dietrich. All Rights Reserved.
//
//     This software is released under the Code Project Open License (CPOL),
//     which may be found here:  http://www.codeproject.com/info/eula.aspx
//     You are free to use this software in any way you like, except that you 
//     may not sell this source code.
//
//     This software is provided "as is" with no expressed or implied warranty.
//     I accept no liability for any damage or loss of business that this 
//     software may cause.
//
// Public Members:
//           NAME                             DESCRIPTION
//   ---------------------   -------------------------------------------------
//   Construction
//      CXRect()             Ctor
//      CXRect(RECT)         Ctor from RECT
//      CXRect(LPCRECT)      Ctor from LPCRECT
//
//   Attributes
//      IsRectEmpty()        Determines whether CXRect is empty. CXRect is empty
//                           if the width and/or height are 0.
//      IsRectNull()         Determines whether the top, bottom, left, and right 
//                           member variables are all equal to 0
//      Width()              Returns width of rect
//      Height()             Returns height of rect
//      PtInRect()           Determines if point is in rect
//
//   Operations
//      BottomRight()        Returns bottom right corner of rect
//      ClientToScreen()     Converts the client-area coordinates of rect to 
//                           screen coordinates
//      CopyRect()           Copies the dimensions of a source rectangle to 
//                           CXRect
//      InflateRect()        Increases the width and height of rect
//      DeflateRect()        Decreases the width and height of rect
//      OffsetRect()         Moves rect by the specified offsets
//      ScreenToClient()     Converts the screen coordinates of rect on the 
//                           screen to client-area coordinates. 
//      SetRect()            Sets the dimensions of CXRect
//      SetRectEmpty()       Sets CXRect to an empty rectangle (all coordinates
//                           equal to 0)
//      SwapLeftRight()      Swaps left and right
//     	TopLeft()            Returns top left corner of rect
//
//      operator LPRECT      Converts a CXRect to an LPRECT
//      operator LPCRECT     Converts a CXRect to an LPCRECT
//      operator=            Copies the dimensions of a rectangle to CXRect
//      operator==           Determines whether CXRect is equal to a rectangle
//
///////////////////////////////////////////////////////////////////////////////

#ifndef CXRECT_H
#define CXRECT_H

#include <crtdbg.h>

#pragma warning(push)
#pragma warning(disable : 4127)	// for _ASSERTE: conditional expression is constant

namespace HansDietrich {

	class CXRect : public tagRECT
	{
		// Construction
	public:
		CXRect(int l = 0, int t = 0, int r = 0, int b = 0)
		{
			left = l;
			top = t;
			right = r;
			bottom = b;
		}
		CXRect(const RECT& srcRect) { ::CopyRect(this, &srcRect); }
		CXRect(LPCRECT lpSrcRect) { ::CopyRect(this, lpSrcRect); }

		// Attributes
	public:
		BOOL IsRectEmpty() const { return ::IsRectEmpty(this); }
		BOOL IsRectNull() const { return (left == 0 && right == 0 && top == 0 && bottom == 0); }
		int Width() const { return right - left; }
		int Height() const { return bottom - top; }
		BOOL PtInRect(POINT point) const { return ::PtInRect(this, point); }

		// Operations
	public:
		void CopyRect(LPCRECT lpSrcRect) { ::CopyRect(this, lpSrcRect); }
		void InflateRect(int x, int y) { ::InflateRect(this, x, y); }
		void InflateRect(SIZE size) { ::InflateRect(this, size.cx, size.cy); }
		void DeflateRect(int x, int y) { ::InflateRect(this, -x, -y); }
		void DeflateRect(SIZE size) { ::InflateRect(this, -size.cx, -size.cy); }
		void DeflateRect(int l, int t, int r, int b) { left += l; top += t; right -= r; bottom -= b; }
		void OffsetRect(int x, int y) { ::OffsetRect(this, x, y); }
		void OffsetRect(POINT point) { ::OffsetRect(this, point.x, point.y); }
		void OffsetRect(SIZE size) { ::OffsetRect(this, size.cx, size.cy); }
		void SetRect(int x1, int y1, int x2, int y2) { ::SetRect(this, x1, y1, x2, y2); }
		void SetRectEmpty() { ::SetRectEmpty(this); }
		void SwapLeftRight() { SwapLeftRight(LPRECT(this)); }
		static void SwapLeftRight(LPRECT lpRect) {
			LONG temp = lpRect->left;
			lpRect->left = lpRect->right;
			lpRect->right = temp;
		}
		POINT TopLeft() { POINT pt; pt.x = left; pt.y = top; return pt; }
		POINT BottomRight() { POINT pt; pt.x = right; pt.y = bottom; return pt; }

		operator LPRECT() { return this; }
		operator LPCRECT() const { return this; }
		void operator=(const RECT& srcRect) { ::CopyRect(this, &srcRect); }
		BOOL operator==(const RECT& rect) const { return ::EqualRect(this, &rect); }

		static void ScreenToClient(HWND hWnd, LPRECT lpRect)
		{
			_ASSERTE(::IsWindow(hWnd));
			::ScreenToClient(hWnd, (LPPOINT)lpRect);
			::ScreenToClient(hWnd, ((LPPOINT)lpRect) + 1);
			if (::GetWindowLong(hWnd, GWL_EXSTYLE) & WS_EX_LAYOUTRTL)
				SwapLeftRight(lpRect);
		}

		static void ClientToScreen(HWND hWnd, LPRECT lpRect)
		{
			_ASSERTE(::IsWindow(hWnd));
			::ClientToScreen(hWnd, (LPPOINT)lpRect);
			::ClientToScreen(hWnd, ((LPPOINT)lpRect) + 1);
			if (::GetWindowLong(hWnd, GWL_EXSTYLE) & WS_EX_LAYOUTRTL)
				SwapLeftRight(lpRect);
		}
	};
}
#pragma warning(pop)

#endif //CXRECT_H
