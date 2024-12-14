// CXDC.h  Version 1.2 - see article at http://www.codeproject.com
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
//              NAME                             DESCRIPTION
//   ---------------------------  ---------------------------------------------
//   Construction
//      CXDC()                    Ctor
//      CXDC(HWND)                Ctor from HWND
//   Attributes
//      HDC()                     Operator HDC - returns HDC
//      m_hDC                     HDC member variable
//   Operations
//      Attach()                  Attaches a HDC
//      BitBlt()                  Copies bits from source DC to this DC
//      CreateCompatibleDC()      Creates a memory DC compatible with specified device
//      CreateCompatibleBitmap()  Creates a bitmap compatible with the device that 
//                                is associated with the specified DC
//      Detach()                  Detaches HDC
//      DrawFocusRect()           Draws a rectangle in the style used to indicate 
//                                that the rectangle has the focus
//      DrawImage()               Draw (stretch) an image to a rect
//      DrawText()                Draws formatted text in the specified rectangle
//      ExtTextOut()              Draws text using the currently selected font, 
//                                background color, and text color.
//      FillSolidRect()           Fills rect with RGB color
//      GetPixel()                Gets RGB color at pixel
//      GetTabbedTextExtent()     Retrieves the width and height of the specified 
//                                string of text.  If the string contains one or 
//                                more tab characters, the width of the string is 
//                                based upon the specified tab stops. 
//      GetTextExtent             Retrieves the width and height of the specified 
//                                string of text
//      GetTextMetrics()          Retrieves metrics for currently selected font
//      LineTo()                  Draws a line from the current position up to, 
//                                but not including, the point specified by x and y
//      MoveTo()                  Moves the current position to the point 
//                                specified by x and y
//      SelectObject()            Selects an object into the specified DC
//      SetBkColor()              Sets the background color
//      SetBkMode()               Sets the background mix mode of the specified DC
//      SetPixelV()               Sets RGB color
//      SetTextColor()            Sets the text color
//      SetWindowOrgEx()          Specifies which window point maps to the 
//                                viewport origin (0,0). 
//      StretchBlt                The StretchBlt function copies a bitmap from a 
//                                source rectangle into a destination rectangle, 
//                                stretching or compressing the bitmap to fit 
//                                the dimensions of the destination rectangle, 
//                                if necessary. 
//      TabbedTextOut()           Writes a character string at a specified location, 
//                                expanding tabs to the values specified in an array 
//                                of tab-stop positions. Text is written in the 
//                                currently selected font, background color, and 
//                                text color.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef CXDC_H
#define CXDC_H

#include <crtdbg.h>

#pragma warning(push)
#pragma warning(disable : 4127)	// for _ASSERTE: conditional expression is constant
#pragma warning(disable : 4710) // function not inlined

#ifndef HD_NO_COLOR
#define HD_NO_COLOR ((COLORREF)-1)
#endif

namespace HansDietrich {

	class CXDC
	{
		// Construction
	public:
		CXDC() : m_hDC(0), m_hWnd(0), m_bRelease(FALSE), m_bDelete(FALSE) {}
		CXDC(HWND hWnd) : m_hDC(0), m_hWnd(0), m_bRelease(FALSE), m_bDelete(FALSE)
		{
			m_hWnd = hWnd;
			if (m_hWnd)		// might be NULL
			{
				_ASSERTE(IsWindow(m_hWnd));
			}
			if ((m_hWnd == NULL) || IsWindow(m_hWnd))
			{
				m_hDC = ::GetDC(m_hWnd);
				_ASSERTE(m_hDC);
				m_bRelease = m_hDC != 0;
			}
		}
		CXDC(HDC hdc) : m_hDC(hdc), m_hWnd(0), m_bRelease(FALSE), m_bDelete(FALSE) {}

		__forceinline virtual ~CXDC()
		{
			if (m_hDC && m_bRelease)
			{
				::ReleaseDC(m_hWnd, m_hDC);
			}
			if (m_hDC && m_bDelete)
			{
				::DeleteDC(m_hDC);
			}
			m_hDC = 0;
			m_hWnd = 0;
		}

		// Attributes
	public:
		HDC m_hDC;
		operator HDC() const
		{
			return this == 0 ? 0 : m_hDC;
		}

		// Operations
	public:
		BOOL Attach(HDC hdc)
		{
			BOOL rc = hdc != 0;
			_ASSERTE(hdc);
			_ASSERTE(m_hDC == 0);
			m_hDC = hdc;
			m_bRelease = FALSE;
			m_bDelete = FALSE;
			return rc;
		}

		HDC Detach()
		{
			HDC rc = m_hDC;
			m_hDC = 0;
			return rc;
		}

		COLORREF GetPixel(int x,	// x-coordinate of pixel
			int y)	// y-coordinate of pixel
		{
			COLORREF cr = HD_NO_COLOR;
			_ASSERTE(m_hDC);
			if (m_hDC)
				cr = ::GetPixel(m_hDC, x, y);
			return cr;
		}

		BOOL SetPixelV(int x,		// x-coordinate of pixel
			int y,		// y-coordinate of pixel
			COLORREF cr)	// new pixel color
		{
			BOOL rc = FALSE;
			_ASSERTE(m_hDC);
			if (m_hDC)
				rc = ::SetPixelV(m_hDC, x, y, cr);
			return rc;
		}

		BOOL BitBlt(int x,			// x-coord of destination upper-left corner
			int y,			// y-coord of destination upper-left corner
			int nWidth,		// width of destination rectangle
			int nHeight,	// height of destination rectangle
			CXDC *pSrcDC,	// pointer to source CXDC object
			int xSrc,		// x-coordinate of source upper-left corner
			int ySrc,		// y-coordinate of source upper-left corner
			DWORD dwRop)	// raster operation code
		{
			BOOL rc = FALSE;
			_ASSERTE(m_hDC);
			_ASSERTE(pSrcDC);
			if (m_hDC && pSrcDC && pSrcDC->m_hDC)
				rc = ::BitBlt(m_hDC, x, y, nWidth, nHeight, pSrcDC->m_hDC,
					xSrc, ySrc, dwRop);
			return rc;
		}

		BOOL StretchBlt(int x,			// x-coord of destination upper-left corner
			int y,			// y-coord of destination upper-left corner
			int nWidth,		// width of destination rectangle
			int nHeight,	// height of destination rectangle
			CXDC *pSrcDC,	// pointer to source CXDC object
			int xSrc,		// x-coordinate of source upper-left corner
			int ySrc,		// y-coordinate of source upper-left corner
			int nSrcWidth,	// width of source rectangle
			int nSrcHeight,	// height of source rectangle
			DWORD dwRop)	// raster operation code
		{
			BOOL rc = FALSE;
			_ASSERTE(m_hDC);
			_ASSERTE(pSrcDC);
			if (m_hDC && pSrcDC && pSrcDC->m_hDC)
				rc = ::StretchBlt(m_hDC, x, y, nWidth, nHeight, pSrcDC->m_hDC,
					xSrc, ySrc, nSrcWidth, nSrcHeight, dwRop);
			return rc;
		}

		BOOL DrawImage(CXDC *pDC, LPRECT lpRect, HBITMAP hBitmap)
		{
			BOOL rc = FALSE;
			_ASSERTE(m_hDC);
			_ASSERTE(pDC);
			_ASSERTE(lpRect);
			_ASSERTE(hBitmap);
			if (m_hDC && pDC && lpRect && hBitmap)
			{
				// set to halftone so we can apply filter
				::SetStretchBltMode(pDC->m_hDC, HALFTONE);

				// set color filter
				COLORADJUSTMENT ca = { 0 };
				ca.caSize = sizeof(ca);
				ca.caFlags = CA_LOG_FILTER;				// Specifies that a logarithmic 
														// function should be applied to 
														// the final density of the output 
														// colors. This will increase the 
														// color contrast when the 
														// luminance is low
				ca.caIlluminantIndex = ILLUMINANT_A;	// light source = tungsten lamp
				ca.caBrightness = 70;					// amount of brightness; value 
														// between -100 and 100
				::SetColorAdjustment(pDC->m_hDC, &ca);

				// get bitmap info for size
				BITMAP bm;
				::GetObject(hBitmap, sizeof(BITMAP), &bm);

				// select bitmap into temp dc
				CXDC dcbmp;
				dcbmp.CreateCompatibleDC(pDC);
				HBITMAP hOldBitmap = (HBITMAP)dcbmp.SelectObject(hBitmap);

				// stretch bitmap into rect
				pDC->StretchBlt(lpRect->left, lpRect->top,
					lpRect->right - lpRect->left, lpRect->bottom - lpRect->top,
					&dcbmp, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);

				if (hOldBitmap)
					dcbmp.SelectObject(hOldBitmap);

				rc = TRUE;
			}
			return rc;
		}

		void FillRect(LPRECT lpRect, HBRUSH hBrush)
		{
			_ASSERTE(m_hDC);
			_ASSERTE(lpRect);
			_ASSERTE(hBrush);
			if (m_hDC && lpRect && hBrush)
			{
				HBRUSH hOldBrush = (HBRUSH)::SelectObject(m_hDC, hBrush);
				::ExtTextOut(m_hDC, 0, 0, ETO_OPAQUE, lpRect, 0, 0, 0);
				if (hOldBrush)
					::SelectObject(m_hDC, hOldBrush);
			}
		}

		void FillSolidRect(int x, int y, int cx, int cy, COLORREF cr)
		{
			_ASSERTE(m_hDC);
			if (m_hDC)
			{
				::SetBkColor(m_hDC, cr);
				RECT rect = { x, y, x + cx, y + cy };
				::ExtTextOut(m_hDC, 0, 0, ETO_OPAQUE, &rect, 0, 0, 0);
			}
		}

		void FillSolidRect(RECT *pRect, COLORREF cr)
		{
			FillSolidRect(pRect->left, pRect->top,
				pRect->right - pRect->left, pRect->bottom - pRect->top, cr);
		}

		BOOL CreateCompatibleDC(HDC hdc)
		{
			BOOL rc = FALSE;
			_ASSERTE(hdc);
			_ASSERTE(m_hDC == 0);
			if (hdc)
			{
				m_hDC = ::CreateCompatibleDC(hdc);
				_ASSERTE(m_hDC);
				rc = m_hDC != 0;
				m_bDelete = TRUE;
				m_bRelease = FALSE;
			}
			return rc;
		}

		BOOL CreateCompatibleDC(CXDC *pDC)
		{
			BOOL rc = FALSE;
			_ASSERTE(pDC && pDC->m_hDC);
			_ASSERTE(m_hDC == 0);
			if (pDC && pDC->m_hDC)
			{
				m_hDC = ::CreateCompatibleDC(pDC->m_hDC);
				_ASSERTE(m_hDC);
				rc = m_hDC != 0;
				m_bDelete = TRUE;
				m_bRelease = FALSE;
			}
			return rc;
		}

		HBITMAP CreateCompatibleBitmap(int nWidth, int nHeight)
		{
			HBITMAP hbitmap = 0;
			_ASSERTE(m_hDC);
			if (m_hDC)
				hbitmap = ::CreateCompatibleBitmap(m_hDC, nWidth, nHeight);
			return hbitmap;
		}

		HGDIOBJ SelectObject(HGDIOBJ hgdiobj)
		{
			HGDIOBJ old = 0;
			_ASSERTE(m_hDC);
			if (m_hDC)
				old = ::SelectObject(m_hDC, hgdiobj);
			return old;
		}

		COLORREF SetTextColor(COLORREF crColor)
		{
			COLORREF crPrevious = RGB(0, 0, 0);
			_ASSERTE(m_hDC);
			if (m_hDC)
			{
				crPrevious = ::SetTextColor(m_hDC, crColor);
			}
			return crPrevious;
		}

		COLORREF SetBkColor(COLORREF crColor)
		{
			COLORREF crPrevious = RGB(0, 0, 0);
			_ASSERTE(m_hDC);
			if (m_hDC)
			{
				crPrevious = ::SetBkColor(m_hDC, crColor);
			}
			return crPrevious;
		}

		int SetBkMode(int nBkMode)
		{
			int nPrevBkMode = OPAQUE;
			_ASSERTE(m_hDC);
			if (m_hDC)
			{
				nPrevBkMode = ::SetBkMode(m_hDC, nBkMode);
			}
			return nPrevBkMode;
		}

		BOOL GetTextMetrics(LPTEXTMETRIC lptm)
		{
			_ASSERTE(m_hDC);
			_ASSERTE(lptm);
			if (m_hDC)
				return ::GetTextMetrics(m_hDC, lptm);
			else
				return FALSE;
		}

		int DrawText(LPCTSTR lpString, int nCount, LPRECT lpRect, UINT uFormat)
		{
			int rc = 0;
			_ASSERTE(m_hDC);
			if (m_hDC)
				rc = ::DrawText(m_hDC, lpString, nCount, lpRect, uFormat);
			return rc;
		}

		LONG TabbedTextOut(int X,
			int Y,
			LPCTSTR lpString,
			int nCount,
			int nTabPositions,
			CONST LPINT lpnTabStopPositions,
			int nTabOrigin)
		{
			LONG lResult = 0;
			_ASSERTE(m_hDC);
			if (m_hDC)
				lResult = ::TabbedTextOut(m_hDC, X, Y, lpString, nCount,
					nTabPositions, lpnTabStopPositions, nTabOrigin);
			return lResult;
		}

		BOOL ExtTextOut(int X,
			int Y,
			UINT fuOptions,
			CONST RECT* lprc,
			LPCTSTR lpString,
			UINT cbCount,
			CONST INT* lpDx)
		{
			BOOL lResult = 0;
			_ASSERTE(m_hDC);
			if (m_hDC)
				lResult = ::ExtTextOut(m_hDC, X, Y, fuOptions, lprc, lpString,
					cbCount, lpDx);
			return lResult;
		}

		BOOL DrawFocusRect(CONST RECT* lprc)
		{
			BOOL rc = FALSE;
			_ASSERTE(m_hDC);
			if (m_hDC)
				rc = ::DrawFocusRect(m_hDC, lprc);
			return rc;
		}

		BOOL LineTo(int x, int y)
		{
			BOOL rc = FALSE;
			_ASSERTE(m_hDC);
			if (m_hDC)
				rc = ::LineTo(m_hDC, x, y);
			return rc;
		}

		BOOL MoveTo(int x, int y)
		{
			BOOL rc = FALSE;
			_ASSERTE(m_hDC);
			if (m_hDC)
				rc = ::MoveToEx(m_hDC, x, y, NULL);
			return rc;
		}

		BOOL SetWindowOrgEx(int X, int Y, LPPOINT lpPoint)
		{
			BOOL rc = FALSE;
			_ASSERTE(m_hDC);
			if (m_hDC)
				rc = ::SetViewportOrgEx(m_hDC, X, Y, lpPoint);
			return rc;
		}

		BOOL GetWindowOrgEx(LPPOINT lpPoint)
		{
			BOOL rc = FALSE;
			_ASSERTE(m_hDC);
			if (m_hDC)
				rc = ::GetWindowOrgEx(m_hDC, lpPoint);
			return rc;
		}

		SIZE GetTextExtent(LPCTSTR lpszString)
		{
			SIZE size = { 0 };
			_ASSERTE(lpszString);
			_ASSERTE(m_hDC);
			if (m_hDC && lpszString)
				::GetTextExtentPoint32(m_hDC, lpszString, _tcslen(lpszString), &size);
			return size;
		}

		SIZE GetTabbedTextExtent(LPCTSTR lpszString,
			int nTabPositions,
			LPINT lpnTabStopPositions)
		{
			_ASSERTE(lpszString);
			_ASSERTE(lpnTabStopPositions);
			_ASSERTE(m_hDC);
			DWORD dw = 0;
			if (m_hDC && lpszString)
				dw = ::GetTabbedTextExtent(m_hDC, lpszString, _tcslen(lpszString),
					nTabPositions, lpnTabStopPositions);
			SIZE size;
			size.cx = LOWORD(dw);
			size.cy = HIWORD(dw);
			return size;
		}

		// Implementation
	protected:
		HWND	m_hWnd;
		BOOL	m_bRelease;
		BOOL	m_bDelete;
	};
}
#pragma warning(pop)

#endif //CXDC_H
