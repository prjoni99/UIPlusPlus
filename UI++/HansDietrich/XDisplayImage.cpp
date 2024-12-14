// XDisplayImage.cpp  Version 1.3.813
//
// Author: Hans Dietrich
//         hdietrich@gmail.com
//
// Description:
//     XDisplayImage.cpp implements DisplayImageFromResource() and
//     DisplayImageFromUrl(), APIs that will display a BMP, GIF, JPG, 
//     or PNG image from an embedded resource ot external file.
//     For more information see 
//         http://www.hdsoft.org/xdisplayimage.html
//
// History
//     Version 1.3 - 2009 June 11
//     - Added ability to overlay control with standard Win32 static text boxes
//
//     Version 1.2 - 2009 June 4
//     - Control now sends WM_LBUTTONDOWN, WM_LBUTTONUP, and WM_MOUSEMOVE
//       messages to parent
//
//     Version 1.1 - 2009 May 29
//     - Added API for string / MAKEINTRESOURCE
//     - Added ability to display icons from resources
//
//     Version 1.0 - 2009 May 1
//     - Initial public release
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
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
// include the following line if compiling an MFC app
#include "stdafx.h"
///////////////////////////////////////////////////////////////////////////////


#ifndef _MFC_VER
#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include <crtdbg.h>
#include <tchar.h>
#pragma message("    compiling for Win32")
#else
#pragma message("    compiling for MFC")
#endif

#include "XDisplayImage.h"

#ifdef _DEBUG
#ifndef _MFC_VER
static void * operator new(size_t nSize, LPCSTR lpszFileName, int nLine)
{
	return ::operator new(nSize, _NORMAL_BLOCK, lpszFileName, nLine);
}
#define DEBUG_NEW new(THIS_FILE, __LINE__)
#endif //_MFC_VER

#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif //_DEBUG

#ifndef __noop
#if _MSC_VER < 1300
#define __noop ((void)0)
#endif
#endif

#undef TRACE
#undef TRACERECT
#undef TRACEPOINT
#define TRACE __noop
#define TRACERECT __noop
#define TRACEPOINT __noop

//=============================================================================
// if you want to see the TRACE output, uncomment this line:
//#include "XTrace.h"
//=============================================================================


#pragma warning(disable : 4127)	// _ASSERTE conditional expression is constant
#pragma warning(disable : 4996)	// disable bogus deprecation warning

namespace HansDietrich
{

#ifndef LVBKIF_TYPE_WATERMARK
#define LVBKIF_TYPE_WATERMARK   0x10000000
#endif

struct XDISPLAYIMAGEPARAMS
{
	LONG_PTR wndproc;
	HBITMAP  hbm;
	BOOL bIsInitialized;
	BOOL bTransparent;
};

//=============================================================================
//
// RedrawParentWindowForControl()
//
// Purpose:		Redraw the control's rectangle on the parent.
// 
// Parameters:	hWndControl      - HWND control
//				bTransparentOnly - TRUE = children will not be redrawn because
//                                 this call is used to retrieve the background 
//                                 of the dialog
//
// Returns:		None
//
static
BOOL RedrawParentWindowForControl(HWND hWndControl, BOOL bTransparentOnly)
{
	if (!::IsWindow(hWndControl)) 
		return FALSE;

	// get parent handle
	HWND hWndParent = ::GetParent(hWndControl);
	if (!::IsWindow(hWndParent)) 
		return FALSE;

	// get the control's window rect
	RECT rectControl;
	::GetWindowRect(hWndControl, &rectControl);
	// convert to parent coords
	::ScreenToClient(hWndParent, (LPPOINT)&rectControl);
	::ScreenToClient(hWndParent, ((LPPOINT)&rectControl)+1);

	// set up the right flags
	UINT nFlags = RDW_NOCHILDREN | RDW_INVALIDATE | RDW_ERASE | RDW_ERASENOW | 
					RDW_UPDATENOW;
	if (!bTransparentOnly)
		nFlags = RDW_ALLCHILDREN;

	// now redraw the control's rectangle in the parent
	return ::RedrawWindow(hWndParent, &rectControl, NULL, nFlags);
}

//=============================================================================
//
// DefWindowProcX()
//
// Purpose:     Dispatch WM_LBUTTONDOWN, WM_LBUTTONUP, and WM_MOUSEMOVE 
//              messages to parent, and cleanup when WM_DESTROY message
//              arrives.
//
// Parameters:  Standard windows message parameters.
//
// Returns:     LRESULT - The return value is the result of the message 
//                        processing and depends on the message.
//
static 
LRESULT __stdcall DefWindowProcX(HWND hWnd,		// handle to window
								 UINT message,	// message identifier
								 WPARAM wParam,	// first message parameter
								 LPARAM lParam)	// second message parameter
{
	switch (message)
	{
		case WM_MOUSEMOVE:
		{
			TRACE(_T("sending WM_MOUSEMOVE to parent\n"));
			HWND hwndParent = ::GetParent(hWnd);
			if (IsWindow(hwndParent))
			{
				// return cursor position relative to parent
				POINT point;
				::GetCursorPos(&point);
				::ScreenToClient(hwndParent, &point);
				::SendMessage(hwndParent, message, wParam, 
					MAKELPARAM(point.x, point.y));
			}
			break;
		}

		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		{
			TRACE(_T("sending WM_LBUTTONDOWN/UP to parent\n"));
			HWND hwndParent = ::GetParent(hWnd);
			if (IsWindow(hwndParent))
			{
				// return cursor position relative to parent
				POINT point;
				::GetCursorPos(&point);
				::ScreenToClient(hwndParent, &point);
				::SendMessage(hwndParent, message, wParam, 
					MAKELPARAM(point.x, point.y));
			}
			// Returning TRUE for WM_LBUTTONDOWN allows us to see the
			// WM_LBUTTONUP messages, and also suppresses the mouse
			// selection rect annoyance
			return TRUE;
			break;
		}

		case WM_DESTROY:
		{
			if (IsWindow(hWnd))
			{
				// get current value, set to 0
				XDISPLAYIMAGEPARAMS *pxdi = 
					(XDISPLAYIMAGEPARAMS *) ::SetWindowLongPtr(hWnd, GWLP_USERDATA, 0);
				if (pxdi)
				{
					if (pxdi->hbm)
					{
						// hbm member will be non-zero for icons
						TRACE(_T("WM_DESTROY:  deleting hbm for 0x%X\n"), hWnd);
						::DeleteObject(pxdi->hbm);
					}
					pxdi->hbm = 0;
					LONG_PTR wndproc = pxdi->wndproc;
					delete pxdi;
					if (wndproc)
					{
						// restore saved window proc 
						TRACE(_T("WM_DESTROY:  restoring wndproc for 0x%X\n"), hWnd);
						::SetWindowLongPtr(hWnd, GWLP_WNDPROC, wndproc);
						return ::CallWindowProc((WNDPROC)wndproc, hWnd, message, 
									wParam, lParam);
					}
				}
			}
			break;
		}

		default:
			//TRACE(_T("message=0x%X\n"), message);
			break;
	}

	if (IsWindow(hWnd))
	{
		XDISPLAYIMAGEPARAMS *pxdi = 
			(XDISPLAYIMAGEPARAMS *) ::GetWindowLongPtr(hWnd, GWLP_USERDATA);
		if (pxdi)
		{
			// initialize if required
			if (!pxdi->bIsInitialized)
			{
				// sleep due to timing problems
				::Sleep(10);

				// redraw because of drawing problems
				RedrawParentWindowForControl(hWnd, pxdi->bTransparent);

				// now we are initialized
				pxdi->bIsInitialized = TRUE;
			}

			LONG_PTR wndproc = pxdi->wndproc;
			if (wndproc)
			{
				// dispatch via saved window proc 
				return ::CallWindowProc((WNDPROC)wndproc, hWnd, message, 
							wParam, lParam);
			}
			else
			{
				TRACE(_T("WARNING: wndproc = 0 for message = 0x%04X\n"), message);
			}
		}
		else
		{
			TRACE(_T("WARNING: GWLP_USERDATA = 0 for message = 0x%04X\n"), message);
		}
	}

	return ::DefWindowProc(hWnd, message, wParam, lParam);
}

//=============================================================================
//
// BitmapFromIcon()
//
// Purpose:     Get bitmap from icon resource.
//
// Parameters:  hwndParent         - HWND of parent
//              hResModule         - module handle of resource
//              rectCtrl           - RECT of control
//              nIconWidth         - icon width;  LR_DEFAULTSIZE will be used
//                                   to load icon if 0
//              nIconHeight        - icon width;  LR_DEFAULTSIZE will be used
//                                   to load icon if 0
//              lpszIconResourceId - icon resource id (string or integer via
//                                   MAKEINTRESOURCE)
//
// Returns:     HBITMAP - The return value is the bitmap handle of the icon,
//                        as drawn to a memory dc.
//
static
HBITMAP BitmapFromIcon(HWND hwndParent, 
					   HMODULE hResModule,
					   RECT& rectCtrl, 
					   int nIconWidth,
					   int nIconHeight,
					   LPCTSTR lpszIconResourceId)
{
	TRACERECT(rectCtrl);
	HBITMAP hbm = NULL;

	if (!IsWindow(hwndParent))
		return NULL;

	SIZE sizeIcon = { nIconWidth, nIconHeight };

	// check if default size requested
	UINT flags = 0;
	if ((sizeIcon.cx == 0) || (sizeIcon.cy == 0))
		flags = LR_DEFAULTSIZE;

	HICON hIcon = (HICON) ::LoadImage(hResModule, 
									  lpszIconResourceId,
									  IMAGE_ICON, 
									  sizeIcon.cx, sizeIcon.cy, 
									  flags);
	
	if (!hIcon)
		return NULL;

	int w = rectCtrl.right-rectCtrl.left;
	int h = rectCtrl.bottom-rectCtrl.top;
	
	::RedrawWindow(hwndParent, &rectCtrl, NULL, 
		RDW_INVALIDATE | RDW_ERASE | RDW_ERASENOW | RDW_UPDATENOW);

	HDC hdcParent = ::GetDC(hwndParent);
	TRACE(_T("hwndParent=0x%X\n"), hwndParent);
	
	if (hdcParent)
	{
		ICONINFO iconinfo;
		::GetIconInfo(hIcon, &iconinfo);

		if (iconinfo.hbmColor)
		{
			// get actual bitmap size (in case default size is requested)

			BITMAPINFO bmi;
			memset(&bmi, 0, sizeof(bmi));
			bmi.bmiHeader.biSize = sizeof(bmi.bmiHeader);

			// get bitmap info ==> bmi
			int rc = ::GetDIBits(
				hdcParent,			// handle to DC
				iconinfo.hbmColor,	// handle to bitmap
				0,					// first scan line to set
				0,					// number of scan lines to copy
				0,					// array for bitmap bits
				&bmi,				// bitmap data buffer
				DIB_RGB_COLORS);	// RGB or palette index

			TRACE(_T("~~~~~ rc=%d  h=%d  w=%d  bpp=%d\n"), 
				rc, bmi.bmiHeader.biHeight, bmi.bmiHeader.biWidth, bmi.bmiHeader.biBitCount);

			if (rc)
			{
				if (bmi.bmiHeader.biWidth)
					sizeIcon.cx = bmi.bmiHeader.biWidth;
				if (bmi.bmiHeader.biHeight)
					sizeIcon.cy = bmi.bmiHeader.biHeight;
			}
		}

		// clean up bitmaps
		if (iconinfo.hbmColor)
			::DeleteObject(iconinfo.hbmColor);
		iconinfo.hbmColor = 0;
		if (iconinfo.hbmMask)
			::DeleteObject(iconinfo.hbmMask);
		iconinfo.hbmMask = 0;

		// create a compatible DC
		HDC hdcMem = ::CreateCompatibleDC(hdcParent);

		if (hdcMem)
		{
			// create a new bitmap of icon size
			hbm = ::CreateCompatibleBitmap(hdcParent, w, h);
			
			if (hbm)
			{
				// select it into the compatible DC
				HBITMAP oldbmp = (HBITMAP)::SelectObject(hdcMem, hbm);

				RECT rectParent;
				::GetWindowRect(hwndParent, &rectParent);
				TRACERECT(rectParent);
				::BitBlt(hdcMem, 0, 0, w, h, 
					hdcParent, 
					rectCtrl.left, rectCtrl.top, SRCCOPY);
				
				// draw the icon into the compatible DC
				::DrawIconEx(hdcMem, (w-sizeIcon.cx)/2, (h-sizeIcon.cy)/2, hIcon, 
					sizeIcon.cx, sizeIcon.cy,
					0, 0, DI_NORMAL);
				
				::SelectObject(hdcMem, oldbmp);
			}
			::DeleteDC(hdcMem);
		}
	}
	::ReleaseDC(hwndParent, hdcParent);
	::DestroyIcon(hIcon);
	
	return hbm;
}

//=============================================================================
//
// SubClassXDisplayImage()
//
// Purpose:     Subclass control in order to forward WM_LBUTTONDOWN, 
//              WM_LBUTTONUP, and WM_MOUSEMOVE messages to parent, and 
//              clean up when WM_DESTROY message arrives.
//
// Parameters:  hwnd         - HWND of control
//              hbm          - HBITMAP of icon (deleted when WM_DESTROY received)
//              bTransparent - TRUE = image should be drawn transparently
//
// Returns:     None
//
static
void SubClassXDisplayImage(HWND hwnd, HBITMAP hbm, BOOL bTransparent)
{
	if (!IsWindow(hwnd))
		return;

	// make sure it's not already subclassed
	LONG_PTR userdata = ::GetWindowLongPtr(hwnd, GWLP_USERDATA);
	_ASSERTE(userdata == 0);

	if (userdata == 0)
	{
		XDISPLAYIMAGEPARAMS *pxdi = new XDISPLAYIMAGEPARAMS;
		_ASSERTE(pxdi);

		pxdi->hbm = hbm;
		pxdi->bTransparent = bTransparent;
		pxdi->bIsInitialized = FALSE;
		pxdi->wndproc = ::GetWindowLongPtr(hwnd, GWLP_WNDPROC);

		::SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR) pxdi);

		// subclass the list control
		::SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)DefWindowProcX);
	}
}

//=============================================================================
//
// SetBkImage()
//
// Purpose:     Send LVM_SETBKIMAGE message to control.
//
// Parameters:  hwnd     - HWND of control
//              lpszPath - pointer to string containing resource path or url;
//                         can be res:, file:, or http: protocol.  hbm and 
//                         lpszPath cannot both be NULL
//              hbm      - HBITMAP of icon; hbm and lpszPath cannot both be NULL
//              ulFlags  - flags used for LVM_SETBKIMAGE message;  see
//                           http://msdn.microsoft.com/en-us/library/bb774742(VS.85).aspx
//
// Returns:     BOOL - TRUE = LVM_SETBKIMAGE succeeded;  FALSE = LVM_SETBKIMAGE
//                     failed, probably because OleInitialize() or CoInitialize()
//                     was not called.
//
static
BOOL SetBkImage(HWND hwnd, 
				LPCTSTR lpszPath, 
				HBITMAP hbm = 0, 
				ULONG ulFlags = LVBKIF_STYLE_NORMAL | LVBKIF_SOURCE_URL)
{
	BOOL rc = FALSE;

	if (IsWindow(hwnd) && (lpszPath || hbm))
	{
		LVBKIMAGE image;
		memset(&image, 0, sizeof(image));
		image.ulFlags        = ulFlags;
		image.xOffsetPercent = 50;	// center
		image.yOffsetPercent = 50;
		image.hbm            = hbm;
		image.pszImage       = (LPTSTR) lpszPath;
		LRESULT lRes = ::SendMessage(hwnd, LVM_SETBKIMAGE, 0, (LPARAM)&image);
		if (!lRes)
		{
			TRACE(_T("ERROR - LVM_SETBKIMAGE failed.\n"));
			TRACE(_T("Did you call CoInitialize(NULL) or OleInitialize(NULL)?\n"));
			_ASSERTE(lRes);
		}
		rc = lRes != 0;
	}
	return rc;
}

//=============================================================================
//
// GetIntOrString()
//
// Purpose:     Returns resource string from a path (string) or an integer
//              (passed via MAKEINTRESOURCE).
//
// Parameters:  lpszString  - path or MAKEINTRESOURCE integer
//              lpszOutbuf  - pointer to buffer that will receive the string
//                            (path or integer formatted with "#%u")
//              nOutbufSize - size in TCHARs of lpszOutbuf
//
// Returns:     Returns string via lpszOutbuf
//
static
void GetIntOrString(LPCTSTR lpszString, 
					LPTSTR lpszOutbuf, 
					size_t nOutbufSize)	// TCHARs
{
	_ASSERTE(lpszString);
	_ASSERTE(lpszOutbuf);
	_ASSERTE(nOutbufSize > 2);
	if (lpszOutbuf)
		lpszOutbuf[0] = 0;
	if (lpszString && lpszOutbuf && (nOutbufSize > 2))
	{
		// is this a string or number
		if (HIWORD(lpszString) == 0)
		{
			// number
			WORD w = LOWORD((UINT)(UINT_PTR)lpszString);
			_sntprintf(lpszOutbuf, nOutbufSize, _T("#%u"), w);
		}
		else
		{
			// string
			_tcsncpy(lpszOutbuf, lpszString, nOutbufSize);
		}
		lpszOutbuf[nOutbufSize-1] = 0;
	}
}

//=============================================================================
//
// MoveToBack()
//
// Purpose:     Move control behind other child controls.
//
// Parameters:  hwndParent   - HWND of parent
//              hwndCtrl     - HWND of control
//              bTransparent - TRUE = draw image transparently
//
// Returns:     None
//
static
void MoveToBack(HWND hwndParent, HWND hwndCtrl, BOOL bTransparent)
{
	HWND hwndChild = ::GetWindow(hwndParent, GW_CHILD);
	while (hwndChild)
	{
		HWND hwndNext = ::GetWindow(hwndChild, GW_HWNDNEXT);
		if (hwndChild != hwndCtrl)
		{
			::SetWindowPos(hwndChild, hwndCtrl, 0, 0, 0, 0, 
				SWP_NOMOVE|SWP_NOSIZE);
			RedrawParentWindowForControl(hwndChild, bTransparent);
		}
		hwndChild = hwndNext;
	}
	if (!RedrawParentWindowForControl(hwndCtrl, bTransparent))
	{
		TRACE(_T("ERROR - Failed to redraw parent window\n"));
	}
}

//=============================================================================
//
// DisplayImage()
//
// Purpose:     Load the image from the resource specified by lpszImageResourceId
//              and display it in the area specified by rect.
//              
// Parameters:  hWndParent            - parent window
//              hResModule            - handle of module that contains resource
//              lpszImageResourceType - resource type - can be any string or
//                                      MAKEINTRESOURCE value
//              lpszImageResourceId   - resource id - can be any string or
//                                      MAKEINTRESOURCE value
//              nCtrlId               - child control identifier
//              rect                  - position of image window in client 
//                                      coordinates
//              nIconWidth            - icon width (icons only); 0 for default size
//              nIconHeight           - icon height (icons only); 0 for default size
//              bTransparent          - TRUE = set image background transparent
//              bSendToBack           - TRUE = move image behind other controls
//
// Returns:     HWND - handle of window created to display image, or 0 if error;  
//                     the caller is responsible for destroying this window when 
//                     it is no longer needed.
//
static
HWND DisplayImage(HWND hWndParent,
				  HMODULE hResModule,
				  LPCTSTR lpszImageResourceType, 
				  LPCTSTR lpszImageResourceId,
				  UINT nCtrlId,
				  RECT& rect,
				  int nIconWidth,
				  int nIconHeight,
				  BOOL bTransparent,
				  BOOL bSendToBack)
{
	TRACE(_T("in DisplayImage\n"));
	HWND hwnd = ::CreateWindowEx(0, //WS_EX_STATICEDGE,
								 WC_LISTVIEW,
								 _T(""),
								 WS_CHILD|WS_VISIBLE,
								 rect.left, rect.top, 
								 rect.right-rect.left, rect.bottom-rect.top,
								 hWndParent,
								 (HMENU)nCtrlId,
								 NULL,
								 NULL);
	_ASSERTE(hwnd);

	if (hwnd)
	{
		if (bTransparent)
		{
			ListView_SetTextBkColor(hwnd, CLR_NONE);
			ListView_SetBkColor(hwnd, CLR_NONE);
		}

		TCHAR szModule[_MAX_PATH*2];
		DWORD rc = ::GetModuleFileName(hResModule, szModule, 
						sizeof(szModule)/sizeof(TCHAR));
		if (rc)
		{
			szModule[sizeof(szModule)/sizeof(TCHAR)-1] = 0;

			TCHAR szType[_MAX_PATH];
			GetIntOrString(lpszImageResourceType, szType, 
				sizeof(szType)/sizeof(TCHAR));

			TCHAR szID[_MAX_PATH];
			GetIntOrString(lpszImageResourceId, szID, 
				sizeof(szID)/sizeof(TCHAR));

			TCHAR szPath[_MAX_PATH*3+10];
			_sntprintf(szPath, sizeof(szPath)/sizeof(TCHAR),
				_T("res://%s/%s/%s"), szModule, szType, szID);
			szPath[sizeof(szPath)/sizeof(TCHAR)-1] = 0;

#ifdef _DEBUG
			HRSRC hres = ::FindResource(hResModule, szID, szType);
			if (hres == NULL)
			{
				TRACE(_T("ERROR - resource '%s' not found\n"), szPath);
			}
#endif

			BOOL ok = FALSE;
			HBITMAP hbm = 0;
			if (lpszImageResourceType == RT_GROUP_ICON)
			{
				hbm = BitmapFromIcon(hWndParent, hResModule, rect, 
					nIconWidth, nIconHeight, lpszImageResourceId);
				_ASSERTE(hbm);
				ok = SetBkImage(hwnd, 0, hbm, LVBKIF_TYPE_WATERMARK);
			}
			else
			{
				ok = SetBkImage(hwnd, szPath);
			}

			if (ok)
			{
				SubClassXDisplayImage(hwnd, hbm, bTransparent);

				// check if we should send the control to the back
				if (bSendToBack)
				{
					MoveToBack(hWndParent, hwnd, bTransparent);
				}
			}
			else
			{
				TRACE(_T("ERROR - SetBkImage() failed\n"));
				::DestroyWindow(hwnd);
				hwnd = 0;
			}
		}
		else
		{
			TRACE(_T("ERROR - GetModuleFileName() failed\n"));
			::DestroyWindow(hwnd);
			hwnd = 0;
		}
	}

	return hwnd;
}

//=============================================================================
//
// DisplayImageFromResource()
//
// Purpose:     Load the image from the resource specified by lpszImageResourceId
//              and display it in the area specified by rect.
//              
// Parameters:  hWndParent            - parent window
//              hResModule            - handle of module that contains resource
//              lpszImageResourceType - resource type - can be any string or
//                                      MAKEINTRESOURCE value
//              lpszImageResourceId   - resource id - can be any string or
//                                      MAKEINTRESOURCE value
//              nCtrlId               - child control identifier
//              rect                  - position of image window in client 
//                                      coordinates
//              bTransparent          - TRUE = set image background transparent
//              bSendToBack           - TRUE = move image behind other controls
//
// Returns:     HWND - handle of window created to display image, or 0 if error;  
//                     the caller is responsible for destroying this window when 
//                     it is no longer needed.
//
HWND DisplayImageFromResource(HWND hWndParent,
							  HMODULE hResModule,
							  LPCTSTR lpszImageResourceType, 
							  LPCTSTR lpszImageResourceId,
							  UINT nCtrlId,
							  RECT& rect,
							  BOOL bTransparent,
							  BOOL bSendToBack)
{
	TRACE(_T("in DisplayImage\n"));

	return DisplayImage(hWndParent,
						hResModule,
						lpszImageResourceType,
						lpszImageResourceId,
						nCtrlId,
						rect,
						0,
						0,
						bTransparent,
						bSendToBack);
}

//=============================================================================
//
// DisplayImageFromResource()
//
// Purpose:     Load the image from the resource specified by nImageResourceId
//              and display it in the area specified by rect.
//              
// Parameters:  hWndParent         - parent window
//              hResModule         - handle of module that contains resource
//              nImageResourceType - resource type (can be any integer value not 
//                                   already used by system)
//              nImageResourceId   - resource id
//              nCtrlId            - child control identifier
//              rect               - position of image window in client 
//                                   coordinates
//              bTransparent       - TRUE = set image background transparent
//              bSendToBack        - TRUE = move image behind other controls
//
// Returns:     HWND - handle of window created to display image, or 0 if error;  
//                     the caller is responsible for destroying this window when 
//                     it is no longer needed.
//
HWND DisplayImageFromResource(HWND hWndParent,
							  HMODULE hResModule,
							  UINT nImageResourceType, 
							  UINT nImageResourceId,
							  UINT nCtrlId,
							  RECT& rect,
							  BOOL bTransparent,
							  BOOL bSendToBack)
{
	TRACE(_T("in DisplayImageFromResource 1\n"));

	return DisplayImage(hWndParent,
						hResModule,
						MAKEINTRESOURCE(nImageResourceType),
						MAKEINTRESOURCE(nImageResourceId),
						nCtrlId,
						rect,
						0,
						0,
						bTransparent,
						bSendToBack);
}

//=============================================================================
//
// DisplayImageFromUrl()
//
// Purpose:     Load the image from the url specified by lpszImageUrl
//              and display it in the area specified by rect.
//              
// Parameters:  hWndParent   - parent window
//              lpszImageUrl - url string
//              nCtrlId      - child control identifier
//              rect         - position of image window in client 
//                             coordinates
//              bTransparent - TRUE = set image background transparent
//              bSendToBack  - TRUE = move image behind other controls
//
// Returns:     HWND - handle of window created to display image, or 0 if error;  
//                     the caller is responsible for destroying this window when 
//                     it is no longer needed.
//
HWND DisplayImageFromUrl(HWND hWndParent,
						 LPCTSTR lpszImageUrl,
						 UINT nCtrlId,
						 RECT& rect,
						 BOOL bTransparent,
						 BOOL bSendToBack)
{
	TRACE(_T("in DisplayImageFromUrl\n"));

	HWND hwnd = 0;

	_ASSERTE(lpszImageUrl && (lpszImageUrl[0] != 0));

	if (lpszImageUrl && (lpszImageUrl[0] != 0))
	{
		hwnd = ::CreateWindowEx(0, //WS_EX_STATICEDGE,
								WC_LISTVIEW,
								_T(""),
								WS_CHILD|WS_VISIBLE,
								rect.left, rect.top, 
								rect.right-rect.left, rect.bottom-rect.top,
								hWndParent,
								(HMENU)nCtrlId,
								NULL,
								NULL);
		_ASSERTE(hwnd);

		if (hwnd)
		{
			if (bTransparent)
			{
				ListView_SetTextBkColor(hwnd, CLR_NONE);
				ListView_SetBkColor(hwnd, CLR_NONE);
			}

			if (SetBkImage(hwnd, lpszImageUrl))
			{
				SubClassXDisplayImage(hwnd, 0, bTransparent);
				// check if we should send the control to the back
				if (bSendToBack)
				{
					MoveToBack(hWndParent, hwnd, bTransparent);
				}
			}
			else
			{
				TRACE(_T("ERROR - SetBkImage() failed\n"));
				::DestroyWindow(hwnd);
				hwnd = 0;
			}
		}
	}

	return hwnd;
}

//=============================================================================
//
// DisplayIconFromResource()
//
// Purpose:     Load the icon from the resource specified by lpszImageResourceId
//              and display it in the area specified by rect.
//              
// Parameters:  hWndParent          - parent window
//              hResModule          - handle of module that contains resource
//              lpszImageResourceId - resource id - can be any string or
//                                    MAKEINTRESOURCE value
//              nCtrlId             - child control identifier
//              rect                - position of image window in client 
//                                    coordinates
//              nIconWidth          - icon width, or 0 to use system default
//              nIconHeight         - icon height, or 0 to use system default
//              bTransparent        - TRUE = set image background transparent
//              bSendToBack         - TRUE = move image behind other controls
//
// Returns:     HWND - handle of window created to display image, or 0 if error;  
//                     the caller is responsible for destroying this window when 
//                     it is no longer needed.
//
HWND DisplayIconFromResource(HWND hWndParent,
							 HMODULE hResModule,
							 LPCTSTR lpszImageResourceId,
							 UINT nCtrlId,
							 RECT& rect,
							 int nIconWidth,
							 int nIconHeight,
							 BOOL bTransparent,
							 BOOL bSendToBack)
{
	TRACE(_T("in DisplayIconFromResource 1\n"));
	return DisplayImage(hWndParent,
						hResModule,
						RT_GROUP_ICON,
						lpszImageResourceId,
						nCtrlId,
						rect,
						nIconWidth,
						nIconHeight,
						bTransparent,
						bSendToBack);
}

//=============================================================================
//
// DisplayIconFromResource()
//
// Purpose:     Load the icon from the resource specified by nImageResourceId
//              and display it in the area specified by rect.
//              
// Parameters:  hWndParent       - parent window
//              hResModule       - handle of module that contains resource
//              nImageResourceId - resource id
//              nCtrlId          - child control identifier
//              rect             - position of image window in client 
//                                 coordinates
//              nIconWidth       - icon width, or 0 to use system default
//              nIconHeight      - icon height, or 0 to use system default
//              bTransparent     - TRUE = set image background transparent
//              bSendToBack      - TRUE = move image behind other controls
//
// Returns:     HWND - handle of window created to display image, or 0 if error;  
//                     the caller is responsible for destroying this window when 
//                     it is no longer needed.
//
HWND DisplayIconFromResource(HWND hWndParent,
							 HMODULE hResModule,
							 UINT nImageResourceId,
							 UINT nCtrlId,
							 RECT& rect,
							 int nIconWidth,
							 int nIconHeight,
							 BOOL bTransparent,
							 BOOL bSendToBack)
{
	TRACE(_T("in DisplayIconFromResource 2\n"));

	return DisplayImage(hWndParent,
						hResModule,
						RT_GROUP_ICON,
						MAKEINTRESOURCE(nImageResourceId),
						nCtrlId,
						rect,
						nIconWidth,
						nIconHeight,
						bTransparent,
						bSendToBack);
}
}