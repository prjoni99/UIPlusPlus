// XHTMLCtrl.cpp  Version 2.5
//
// Author:  Hans Dietrich
//          hdietrich@gmail.com
//
// History
//     Version 2.5 - 2011 July 1
//     - Added resource instance handle to Create()
//     - Used resource instance handle to call DisplayImageFromResource()
//
//     Version 2.4 - 2011 April 14
//     - Fixed problem with left margin
//     - Fixed problem with <center>
//     - Implemented <HTML>, <BODY>, <TITLE>
//     - Implemented GetTitleText()
//
//     Version 2.2 - 2009 November 3
//     - GDI leak: deleted m_hBitmap in dtor
//
//     Version 2.1 - 2009 October 12
//     - fixed some minor bugs
//     - changed "Set" APIs to return object ref
//
//     Version 2.0.1 - 2009 May 25
//     - Fixed problem with space after text attribute
//
//     Version 2.0 - 2009 May 18
//     - Implemented <IMG> tag.
//     - Implemented <P> tag.
//     - Implemented <H1>...<H6> tags.
//     - Added TITLE attribute to <A> tag
//     - Control can now be resized (WM_SIZE).
//     - Removed MFC dependency; control can now be used in MFC or non-MFC app.
//
//     Version 1.4 - 2007 October 19
//     - Fixed bug where text could be written outside of control's client 
//       rect, reported (with fix) by David Pritchard.
//     - Expanded table of character entities.
//     - Added VS2005 project.
//     - Implemented memory DC to improve performance, suggested by conan.ks.
//     - Switched from WM_TIMER to WM_MOUSEMOVE for display of hand cursor, 
//       suggested by rm2 and RichardC.  This prevents hand cursor from 
//       appearing on overlapping windows.
//
//     Version 1.3 - 2006 August 15
//     - Added transparency support, suggested by Anna
//     - Added support for WM_PRINT, suggested by beaus07
//     - Added support for <center>, requested by several readers
//     - Added support for tooltips for hyperlinks
//     - Load hand cursor from IDC_HAND, suggested by kamnas
//     - Fixed font object leak, reported by furbo
//     - Fixed problem with SetWindowText() reported by Andro67;
//       the background and text colors are no longer reset
//       when SetWindowText() is called.
//     - Fixed bug when control is hidden, reported by RichardC
//
//     Version 1.2 - 2004 June 12
//     - Changed APP: hyperlink to use HWND instead of GetParent();
//     - Added wParam to XHTMLCTRL_APP_COMMAND struct
//     - Added function SetTextColor(LPCTSTR lpszColor)
//     - Added function SetLogFont(const LOGFONT * pLogFont)
//     - Added function SetWindowText() to call Init and RedrawWindow
//     - Fixed bug with XNamedColors in handling of "255,0,0" style 
//       in SetColorFromString()
//     - Fixed bug with descenders of large serif fonts
//
//     Version 1.1 - 2004 May 20
//     - Implemented SUB tag
//     - Implemented SUP tag
//     - Implemented BIG tag
//     - Implemented SMALL tag
//     - Implemented CODE tag
//     - Implemented HR tag
//     - Implemented APP: hyperlink
//     - Implemented common character entities
//     - Improved parsing performance
//     - Bug fixes
//
//     Version 1.0 - 2002 September 16
//     - Initial public release
//
// License:
//     This file is Copyright © 2011 Hans Dietrich. All Rights Reserved.
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


//=============================================================================
// NOTE ABOUT PRECOMPILED HEADERS
//
// If you are using this control in a non-MFC project, this file does not need 
// to be compiled with precompiled headers (.pch).  To disable this, go to 
// Project | Settings | C/C++ | Precompiled Headers and select "Not using 
// precompiled headers".  Be sure to do this for all build configurations.
//
// If you want to use this control in an MFC project, un-comment the next line,
// and be sure to define XHTMLCTRL_USE_MFC in XHTMLCtrl.h.  This will cause the
// standard MFC classes to be used and reduce the memory footprint of the control.

#include "stdafx.h"

#include <windows.h>
#include <crtdbg.h>
#include "XNamedColors.h"
#include "XDisplayImage.h"
#include "XHTMLCtrl.h"

#ifdef _MFC_VER
	#ifndef XHTMLCTRL_USE_MFC
		#error >>>>> Compiling for MFC, but XHTMLCTRL_USE_MFC is not defined
	#endif
#endif


#ifdef XHTMLCTRL_USE_MFC
	#pragma message("    compiling for MFC")
	#define CXDC			CDC
	#define CXWaitCursor	CWaitCursor
#else
	#pragma message("    compiling non MFC")
	#include "CXDC.h"
	#include "CXWaitCursor.h"
#endif // XHTMLCTRL_USE_MFC

#ifndef GET_X_LPARAM
#define GET_X_LPARAM(lp)	((int)(short)LOWORD(lp))
#define GET_Y_LPARAM(lp)	((int)(short)HIWORD(lp))
#endif

#ifdef _DEBUG
#ifndef XHTMLCTRL_USE_MFC
static void * operator new(size_t nSize, LPCSTR lpszFileName, int nLine)
{
	return ::operator new(nSize, _NORMAL_BLOCK, lpszFileName, nLine);
}
#define DEBUG_NEW new(THIS_FILE, __LINE__)
#endif //XHTMLCTRL_USE_MFC

#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif //_DEBUG

//=============================================================================
// if you want to see the TRACE output, uncomment this line:
//#include "XTrace.h"
//=============================================================================

#ifndef XTRACE_H
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
#endif // XTRACE_H

#pragma warning(disable : 4996)	// disable bogus deprecation warning
#pragma warning(disable : 4127)	// for _ASSERTE: conditional expression is constant


//=============================================================================
// diagnostics
//=============================================================================
#undef _VERIFY
#ifdef _DEBUG
#define _VERIFY(f) _ASSERTE(f)
#else
#define _VERIFY(f) ((void)(f))
#endif //_DEBUG

#undef UNUSED
#define UNUSED(x) x

//=============================================================================
// determine number of elements in an array (not bytes)
//=============================================================================
#undef _countof
#define _countof(array) (sizeof(array)/sizeof(array[0]))


namespace HansDietrich {
	//=============================================================================
	// control id for images
	//=============================================================================
	UINT CXHTMLCtrl::m_nNextImageControlId = XHTML_FIRST_IMAGE_CONTROL_ID;

	//=============================================================================
	// some common character entities
	//=============================================================================
	XHTMLCTRL_CHAR_ENTITIES CXHTMLCtrl::m_aCharEntities[] =
	{
		{ _T("&amp;"),		0,	_T('&') },		// ampersand
		{ _T("&bull;"),		0,	_T('\x95') },	// bullet      NOT IN MS SANS SERIF
		{ _T("&cent;"),		0,	_T('\xA2') },	// cent sign
		{ _T("&copy;"),		0,	_T('\xA9') },	// copyright
		{ _T("&deg;"),		0,	_T('\xB0') },	// degree sign
		{ _T("&euro;"),		0,	_T('\x80') },	// euro sign
		{ _T("&frac12;"),	0,	_T('\xBD') },	// fraction one half
		{ _T("&frac14;"),	0,	_T('\xBC') },	// fraction one quarter
		{ _T("&gt;"),		0,	_T('>') },		// greater than
		{ _T("&iquest;"),	0,	_T('\xBF') },	// inverted question mark
		{ _T("&lt;"),		0,	_T('<') },		// less than
		{ _T("&micro;"),	0,	_T('\xB5') },	// micro sign
		{ _T("&middot;"),	0,	_T('\xB7') },	// middle dot = Georgian comma
		{ _T("&nbsp;"),		0,	_T(' ') },		// nonbreaking space
		{ _T("&para;"),		0,	_T('\xB6') },	// pilcrow sign = paragraph sign
		{ _T("&plusmn;"),	0,	_T('\xB1') },	// plus-minus sign
		{ _T("&pound;"),	0,	_T('\xA3') },	// pound sign
		{ _T("&quot;"),		0,	_T('"') },		// quotation mark
		{ _T("&reg;"),		0,	_T('\xAE') },	// registered trademark
		{ _T("&sect;"),		0,	_T('\xA7') },	// section sign
		{ _T("&sup1;"),		0,	_T('\xB9') },	// superscript one
		{ _T("&sup2;"),		0,	_T('\xB2') },	// superscript two
		{ _T("&sup3;"),		0,	_T('\xB3') },	// superscript three
		{ _T("&times;"),	0,	_T('\xD7') },	// multiplication sign
		{ _T("&trade;"),	0,	_T('\x99') },	// trademark   NOT IN MS SANS SERIF
		{ NULL,				0,	0 }				// MUST BE LAST
	};
}
#define TIMER_MOUSEMOVE		1
#define TIMER_HOVER			2
#define TIMER_OUT_OF_RECT	3
#define TIMER_AUTOPOP		4

namespace HansDietrich {
//=============================================================================
CXHTMLCtrl::CXHTMLCtrl()
//=============================================================================
{
	m_hResourceInstance   = NULL;			//+++2.5
	m_strText             = _T("");
	m_strTitleText        = _T("");			//+++2.4
	m_bRefresh            = TRUE;
	m_bToolTip            = TRUE;			// TRUE = display tooltip
	m_bInHtml             = FALSE;			//+++2.4
	m_bInBody             = FALSE;			//+++2.4
	m_hLinkCursor         = NULL;
	m_paAppCommands       = NULL;
	m_hMemDC              = NULL;
	m_hBitmap             = NULL;
	m_hOldBitmap          = NULL;
	m_pImageInfo          = NULL;			//+++2.0
	m_nImages             = 0;
	m_nAppCommands        = 0;
	m_nLeftMargin         = 0;
	m_nRightMargin        = 0;
	m_nImageTooltipActive = -1;
	m_hLinkCursor = ::LoadCursor(0, IDC_HAND);		//+++2.0
	_ASSERTE(m_hLinkCursor);


#if 0  // -----------------------------------------------------------

	// this is probably necessary only for Win95

	if (m_hLinkCursor == NULL)			// Still no cursor handle - 
										// load the WinHelp hand cursor
	{
		TRACE(_T("Loading hand cursor from winhlp32.exe\n"));

		// This retrieves cursor #106 from winhlp32.exe, which is a hand pointer

		// get hand cursor
		CWTLString strWndDir;
		GetWindowsDirectory(strWndDir.GetBuffer(MAX_PATH), MAX_PATH);
		strWndDir.ReleaseBuffer();
		strWndDir += _T("\\winhlp32.exe");
		HMODULE hModule = ::LoadLibrary(strWndDir);
		if (hModule) 
		{
			HCURSOR hHandCursor = ::LoadCursor(hModule, MAKEINTRESOURCE(106));
			if (hHandCursor)
				m_hLinkCursor = CopyCursor(hHandCursor);
		}
		::FreeLibrary(hModule);
	}
#endif // -----------------------------------------------------------

	m_prevPoint.x = 100000;

	InitCharEntities();

	ResetAll();
}

//=============================================================================
CXHTMLCtrl::~CXHTMLCtrl()
//=============================================================================
{
	ResetAll();

	if (m_hLinkCursor)
		::DestroyCursor(m_hLinkCursor);
	m_hLinkCursor = NULL;

	if (m_hMemDC)
		DeleteDC(m_hMemDC);
	m_hMemDC = NULL;

	if (m_hBitmap)						//+++2.2
		::DeleteObject(m_hBitmap);
	m_hBitmap = NULL;

	m_AnchorInfo.RemoveAll(0);
}

//=============================================================================
//
// DefWindowProcX()
//
// Purpose:     Initial window proc to dispatch messages to 
//              CXHTMLCtrl::WindowProc().  This allows us to set up
//              the 'this' pointer to CXHTMLCtrl instance.
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
		case WM_CREATE:
		{
			// save 'this' pointer in windows extra memory - the lParam
			// is set when ::CreateWindowEx() is called
			CREATESTRUCT *pcs = (CREATESTRUCT *) lParam;
			if (!pcs)
			{
				TRACE(_T("ERROR - CREATESTRUCT lParam is zero\n"));
				_ASSERTE(pcs);
				return -1;		// abort creation
			}
			::SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)(pcs->lpCreateParams));
			return 0;
		}
		break;

		default:
		{
			// dispatch via saved 'this' pointer
			LONG_PTR lData = ::GetWindowLongPtr(hWnd, GWLP_USERDATA);
			if (lData)
			{
				CXHTMLCtrl *pCtrl = (CXHTMLCtrl *) lData;
				return pCtrl->WindowProc(message, wParam, lParam);
			}
			else
			{
				// probably some WM_NCxxxx message
				TRACE(_T("GWLP_USERDATA = 0 for message = 0x%04X\n"), message);
			}
		}
		break;
	}

	return ::DefWindowProc(hWnd, message, wParam, lParam);
}

//=============================================================================
//
// Create()
//
// Purpose:     This virtual function creates the html control window.
//
// Parameters:  hInstance      - handle to the instance that contains 
//                               the window procedure 
//              dwStyle        - specifies the window style attributes
//              rect           - the size and position of the window
//              hParent        - the parent window HWND
//              nID            - the ID of the child window
//
// Returns:     BOOL - TRUE = window created successfully
//
BOOL CXHTMLCtrl::Create(HINSTANCE hInstance,
						HINSTANCE hResourceInstance,					//+++2.5
						DWORD dwStyle,
						const RECT& rect,
						HWND hParent,
						UINT nID)
{
	TRACE(_T("in CXHTMLCtrl::Create\n"));

	m_hParent = hParent;
	_ASSERTE(m_hParent);
	if (!m_hParent)
		return FALSE;

	m_hResourceInstance = hResourceInstance;

	m_rectCtrl = rect;
	TRACERECT(m_rectCtrl);

	m_nId = nID;

	const TCHAR * pszClassName = _T("XHTMLCtrl");

	WNDCLASS wc =
	{
		0,											// class style
		DefWindowProcX,								// window proc
		0,											// class extra bytes
		0,											// window extra bytes
		hInstance,									// instance handle
		0,											// icon
		::LoadCursor(0, IDC_ARROW),					// cursor
		0,											// background brush
		0,											// menu name
		pszClassName								// class name
	};

	if (!::RegisterClass(&wc))
	{
		DWORD dwLastError = GetLastError();
		if (dwLastError != ERROR_CLASS_ALREADY_EXISTS)
		{
			TRACE(_T("ERROR - RegisterClass failed, GetLastError() returned %u\n"), dwLastError);
			_ASSERTE(FALSE);
			return FALSE;
		}
	}

	// we pass 'this' pointer as lpParam, so DefWindowProcX will see it 
	// in WM_CREATE message
	m_hWnd = ::CreateWindowEx(0, pszClassName, _T(""), dwStyle, 
		m_rectCtrl.left, m_rectCtrl.top, m_rectCtrl.Width(), m_rectCtrl.Height(),
		hParent, (HMENU)nID, hInstance, this);

	if (m_hWnd == 0)
	{
#ifdef _DEBUG
		DWORD dwLastError = GetLastError();
		UNUSED(dwLastError);
		TRACE(_T("ERROR - CreateWindowEx failed, GetLastError() returned %u\n"), dwLastError);
		_ASSERTE(m_hWnd);
#endif
		return FALSE;
	}

	InitControl();

	return m_hWnd != 0;
}

//=============================================================================
void CXHTMLCtrl::InitControl()
//=============================================================================
{
#ifdef XHTMLCTRL_USE_MFC
	m_CWnd.Attach(m_hWnd);
#endif

	// create the tooltip

#ifdef XHTMLCTRL_USE_MFC
	_VERIFY(m_ToolTip.Create(&m_CWnd));
#else
	_VERIFY(m_ToolTip.Create(m_hWnd));
#endif //XHTMLCTRL_USE_MFC

	m_ToolTip.SetMaxTipWidth(SHRT_MAX);
	m_nInitialDelay = m_ToolTip.GetDelayTime(TTDT_INITIAL);
	m_nAutoPop = m_ToolTip.GetDelayTime(TTDT_AUTOPOP);
	TRACE(_T("m_nInitialDelay=%d  m_nAutoPop=%d  .....\n"), m_nInitialDelay, m_nAutoPop);
	if (m_nInitialDelay == 0)
		m_nInitialDelay = 600;
	if (m_nAutoPop == 0)
		m_nAutoPop = 6000;

	m_sizeCursor = GetCursorOffset(m_hWnd);

	m_CurrentFontInfo.Reset();
	GetSafeFont(m_CurrentFontInfo.lf);

	TRACE(_T("m_CurrentFontInfo.lf.lfHeight=%d\n"), m_CurrentFontInfo.lf.lfHeight);

	//m_CurrentFontInfo.lf.lfHeight -= 4;	// test

	int inc = 2;
	if (m_CurrentFontInfo.lf.lfHeight < 0)
		inc = -2;

	// set up 6 header fonts
	// H1 = biggest, H6 = smallest
	// H5 = current font size
	int nPointSize = 8;
	int i = 0;
	for (i = 5; i >= 0; i--)
	{
		if (i == 0)
			nPointSize += 2;	// make H1 = H2 + 4 points
		m_HeaderFontInfo[i] = m_CurrentFontInfo;
		if (i < 3)		// set H1, H2, and H3 = blue text
			m_HeaderFontInfo[i].crText = RGB(0,0,255);
		m_HeaderFontInfo[i].lf.lfWeight = FW_BOLD;	// all headers are bold
		m_HeaderFontInfo[i].lf.lfHeight = GetFontHeight(nPointSize);
		nPointSize += 2;		// next heading will be 2 points bigger
	}

#ifdef _DEBUG
	for (i = 0; i < 6; i++)
	{
		int nPointSize = GetFontPointSize(m_HeaderFontInfo[i].lf.lfHeight);
		UNUSED(nPointSize);
		TRACE(_T("m_HeaderFontInfo[%d].lf.lfHeight=%d  point size=%d,,,,,\n"), 
			i, m_HeaderFontInfo[i].lf.lfHeight, nPointSize);
	}
#endif

}

//=============================================================================
//
// WindowProc()
//
// Purpose:     This function is the window proc for CXHTMLCtrl object.  
//              Messages are forwarded to this function from DefWindowProcX().
//
// Parameters:  message - message identifier
//              wParam  - first message parameter
//              lParam  - second message parameter
//
// Returns:     LRESULT - The return value is the result of the message 
//                        processing and depends on the message.
//
LRESULT CXHTMLCtrl::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	CXRect rect;

	switch (message)
	{
		case WM_DESTROY:
			KillTimer(TIMER_MOUSEMOVE);
			KillTimer(TIMER_HOVER);
			KillTimer(TIMER_OUT_OF_RECT);
			KillTimer(TIMER_AUTOPOP);
			CleanupImageWindows();
#ifdef XHTMLCTRL_USE_MFC
			m_CWnd.Detach();
#endif //XHTMLCTRL_USE_MFC
			break;

		case WM_PAINT:
			if (GetUpdateRect(m_hWnd, &rect, FALSE))
			{
				PAINTSTRUCT ps;
				HDC hdc = BeginPaint(m_hWnd, &ps);
				if (hdc)
				{
					Draw(hdc);
					EndPaint(m_hWnd, &ps);
				}
			}
			break;

		case WM_PRINT:
		case WM_PRINTCLIENT:
			if (wParam)
			{
				if (lParam & PRF_CHECKVISIBLE)
				{
					if (!IsWindowVisible(m_hWnd))
						break;
				}
				Draw((HDC)wParam);
			}
			break;

		case WM_MOUSEMOVE:
		{
#ifdef XHTMLCTRL_USE_MFC
			MSG msg;
			msg.hwnd    = m_hWnd;
			msg.message = message;
			msg.wParam  = wParam;
			msg.lParam  = lParam;
			msg.pt.x    = GET_X_LPARAM(lParam);
			msg.pt.y    = GET_Y_LPARAM(lParam);
			m_ToolTip.RelayEvent(&msg);
#endif
			SetTimer(TIMER_MOUSEMOVE, 100, NULL);
			break;
		}

		case WM_LBUTTONDOWN:
		{
			TRACE(_T("WM_LBUTTONDOWN\n"));
			POINT point;
			if (::GetCursorPos(&point))
			{
				::ScreenToClient(m_hWnd, &point);

				BOOL bOnHyperlink = FALSE;

				int n = m_AnchorInfo.SizeUsed();

				if (n != 0)
				{
					int i = 0;
					for (i = 0; i < n; i++)
					{
						CXRect rect = m_AnchorInfo[i].rect;

						if (rect.PtInRect(point))		 // Cursor is currently over control
						{
							bOnHyperlink = TRUE;
							break;
						}
					}

					if (bOnHyperlink)
					{
						CXWaitCursor wait;
						GotoURL(m_AnchorInfo[i].szUrl, SW_SHOW);
					}
				}
			}
			TRACE(_T("exiting WM_LBUTTONDOWN\n"));
			::SetFocus(m_hWnd);
			break;
		}

		case WM_SIZE:
		{
			LRESULT lResult = ::DefWindowProc(m_hWnd, message, wParam, lParam);
			m_bRefresh = TRUE;
			if (::IsWindow(m_hWnd))
			{
				::InvalidateRect(m_hWnd, NULL, TRUE);
			}
			return lResult;
		}

		case WM_SETCURSOR:
			if (m_bOnHyperlink)	
				return TRUE;
			break;

		case WM_TIMER:
			OnTimer((UINT)wParam);
			break;

		case WM_ERASEBKGND:
			return TRUE;

		default:
			break;
	}

	return ::DefWindowProc(m_hWnd, message, wParam, lParam);
}

//=============================================================================
void CXHTMLCtrl::ResetAll()
//=============================================================================
{
	TRACE(_T("in CXHTMLCtrl::ResetAll\n"));

	Reset();

	m_CurrentFontInfo.Reset();
	GetSafeFont(m_CurrentFontInfo.lf);

	m_nLeftMargin	= 0;
	m_nRightMargin	= 0;

	m_AnchorInfo.RemoveAll(10);

	if (m_paAppCommands)
		delete [] m_paAppCommands;
	m_paAppCommands = NULL;

	m_nAppCommands = 0;
}

//=============================================================================
// Reset    (now called by SetWindowText() instead of ResetAll())
void CXHTMLCtrl::Reset()
//=============================================================================
{
	TRACE(_T("in CXHTMLCtrl::Reset\n"));

	m_bRefresh				= TRUE;
	m_bCenter				= FALSE;
	m_bHorizontalRule		= FALSE;
	m_nHorizontalRuleSize	= 2;
	m_bOnHyperlink			= FALSE;
	m_hPrevCursor			= NULL;
	m_bInAnchor				= FALSE;
	m_bGeneratedText		= FALSE;
}

//=============================================================================
void CXHTMLCtrl::InitCharEntities()
//=============================================================================
{
	for (int i = 0; m_aCharEntities[i].pszName != NULL; i++)
	{
		m_aCharEntities[i].cCode = (TCHAR) (i + 2);	// don't use 0 or 1
	}
}

//=============================================================================
TCHAR CXHTMLCtrl::GetCharEntity(TCHAR cCode)
//=============================================================================
{
	TCHAR c = _T(' ');

	for (int i = 0; m_aCharEntities[i].pszName != NULL; i++)
	{
		if (cCode == m_aCharEntities[i].cCode)
		{
			c = m_aCharEntities[i].cSymbol;
			break;
		}
	}

	return c;
}

//=============================================================================
void CXHTMLCtrl::Draw(HDC hDC)
//=============================================================================
{
	TRACE(_T("in CXHTMLCtrl::Draw\n"));

	_ASSERTE(hDC);
	if (hDC == NULL)
		return;

	CXRect rect;
	::GetClientRect(m_hWnd, &rect);
	TRACERECT(rect);

	int nRectWidth  = rect.Width();
	int nRectHeight = rect.Height();

	// get text from control
	CWTLString strText = m_strText;

	m_bInHtml = FALSE;			//+++2.4
	m_bInBody = FALSE;			//+++2.4
	m_strTitleText = _T("");	//+++2.4

	if (m_bRefresh)
	{
		// MODIFIED BY CATENALOGIC - START

		if (::IsWindow(m_hParent))
		{
			CXRect rectWindow;
			::GetWindowRect(m_hWnd, &rectWindow);
			::ScreenToClient(m_hParent, (LPPOINT)&rectWindow);
			::ScreenToClient(m_hParent, ((LPPOINT)&rectWindow)+1);
			TRACERECT(rectWindow);
			if (GetWindowLongPtr(m_hParent, GWL_EXSTYLE) & WS_EX_LAYOUTRTL)
				CXRect::SwapLeftRight(&rectWindow);
			// redraw the control's rectangle in the parent
			::RedrawWindow(m_hParent, &rectWindow, NULL, 
				RDW_NOCHILDREN | RDW_INVALIDATE | RDW_ERASE | RDW_ERASENOW | RDW_UPDATENOW);
		}

		// MODIFIED BY CATENALOGIC - END

		TRACE(_T("creating bitmap ..................................\n"));

		if (m_hOldBitmap && m_hMemDC)
			SelectObject(m_hMemDC, m_hOldBitmap);
		m_hOldBitmap = NULL;

		if (m_hBitmap)
			DeleteObject(m_hBitmap);
		m_hBitmap = NULL;

		if (m_hMemDC)
			DeleteDC(m_hMemDC);
		m_hMemDC = CreateCompatibleDC(hDC);
		_ASSERTE(m_hMemDC);

		// create bitmap for entire client area
		m_hBitmap = CreateCompatibleBitmap(hDC, nRectWidth, nRectHeight);
		_ASSERTE(m_hBitmap);

		m_hOldBitmap = (HBITMAP) SelectObject(m_hMemDC, m_hBitmap);

		// copy background from parent
		BitBlt(m_hMemDC, 0, 0, nRectWidth, nRectHeight, hDC, 0, 0, SRCCOPY);

		// release memory used for images
		CleanupImageWindows();
		m_nImages = CountImageTags(strText);	// scan text for <IMG tags
		m_pImageInfo = new XHTMLCTRL_IMAGE_INFO [m_nImages + 1];
		memset(m_pImageInfo, 0, sizeof(XHTMLCTRL_IMAGE_INFO) * (m_nImages + 1));

		m_bRefresh = FALSE;
	}
	else
	{
		TRACE(_T("restoring bitmap ============================================\n"));
		// restore cached bitmap
		_ASSERTE(m_hMemDC);
		_ASSERTE(m_hBitmap);
		BitBlt(hDC, 0, 0, nRectWidth, nRectHeight, m_hMemDC, 0, 0, SRCCOPY);
		return;
	}

	int nAnchors = m_AnchorInfo.SizeUsed();

	int i = 0;

	for (i = 0; i < nAnchors; i++)
	{
		if (m_bToolTip && m_ToolTip.GetToolCount())
		{
			TRACE(_T("deleting tool %d -----\n"), i+1);
#ifdef XHTMLCTRL_USE_MFC
			m_ToolTip.DelTool(&m_CWnd, i+1);
#else
			m_ToolTip.DelTool(m_hWnd, i+1);
#endif
		}
	}

	m_AnchorInfo.RemoveAll(10);

	UINT nToolId = 1;	// next tool id

	// replace character entity names with codes

	TCHAR ent[3] = { 0 };
	ent[0] = _T('\001');	// each entity name is replaced with a two-character
							// code that begins with \001

	for (i = 0; m_aCharEntities[i].pszName != NULL; i++)
	{
		ent[1] = m_aCharEntities[i].cCode;
		strText.Replace(m_aCharEntities[i].pszName, ent);
	}

	CWTLString str1 = _T("");
	int index = 0;

	if (!(GetWindowLong(m_hWnd, GWL_EXSTYLE) & WS_EX_TRANSPARENT))
	{
		HBRUSH hbrush = CreateSolidBrush(m_CurrentFontInfo.crBackground); 
		_ASSERTE(hbrush);
		FillRect(m_hMemDC, &rect, hbrush);
		if (hbrush)
			DeleteObject(hbrush);
	}

	// nothing to do if no text or not visible
	if (strText.IsEmpty() || !IsWindowVisible(m_hWnd))
		return;

	int n = strText.GetLength();

	// allow for margins
	//rect.left += m_nLeftMargin;
	rect.right -= m_nRightMargin;

	int nInitialXOffset = 0;
	int nNextInitialXOffset = 0;
	m_yStart = rect.top;

	// create initial font
	HFONT hNewFont = CreateFontIndirect(&m_CurrentFontInfo.lf);
	_ASSERTE(hNewFont);
	HFONT hOldFont = (HFONT) SelectObject(m_hMemDC, hNewFont);

	CWTLString strAnchorText = _T("");

	BOOL bSizeChange = FALSE;
	TEXTMETRIC tm = { 0 };
	GetTextMetrics(m_hMemDC, &tm);

	int nImageWidth = 0;
	int nImageHeight = 0;
	enum ImageAlign { none = 0, left, right, top, middle, bottom };
	ImageAlign eImageAlign = none;
	CWTLString strSrc = _T("");
	CWTLString strAnchorTitle = _T("");
	int nResId = 0;
	int nResType = 0;

	m_sizeLeftIndent.cx = 0;
	m_sizeLeftIndent.cy = 0;
	m_sizeRightIndent.cx = 0;
	m_sizeRightIndent.cy = 0;

	while (n > 0)
	{
		TRACE(_T("..... beginning of while\n"));

		TRACERECT(rect);

		///////////////////////////////////////////////////////////////////////
		if (_tcsnicmp(strText, _T("<HTML>"), 6) == 0)	// check for <HTML>			//+++2.4
		{
			m_bInHtml = TRUE;
			n -= 6;
			strText = strText.Mid(6);
			continue;
		}
		///////////////////////////////////////////////////////////////////////
		else if (_tcsnicmp(strText, _T("</HTML>"), 7) == 0)	// check for <\HTML>	//+++2.4
		{
			m_bInHtml = FALSE;
			n -= 7;
			strText = strText.Mid(7);
			continue;
		}
		///////////////////////////////////////////////////////////////////////
		else if (_tcsnicmp(strText, _T("<BODY>"), 6) == 0)	// check for <BODY>		//+++2.4
		{
			m_bInBody = TRUE;
			n -= 6;
			strText = strText.Mid(6);
			continue;
		}
		///////////////////////////////////////////////////////////////////////
		else if (_tcsnicmp(strText, _T("</BODY>"), 7) == 0)	// check for <\BODY>	//+++2.4
		{
			m_bInBody = FALSE;
			n -= 7;
			strText = strText.Mid(7);
			continue;
		}
		///////////////////////////////////////////////////////////////////////
		else if (_tcsnicmp(strText, _T("<TITLE>"), 7) == 0)	// check for <TITLE>	//+++2.4
		{
			strText = strText.Mid(7);
			index = strText.Find(_T("</TITLE>"));
			if (index >= 0) 
			{
				m_strTitleText = strText.Left(index);
				strText = strText.Mid(index+8);
			}
			continue;
		}
		///////////////////////////////////////////////////////////////////////
		else if (_tcsnicmp(strText, _T("<B>"), 3) == 0)	// check for <b> or <B>
		{
			n -= 3;
			index = strText.Find(_T('>'));
			if (index != -1)
				strText = strText.Mid(index+1);
			m_FontInfo.Push(m_CurrentFontInfo);
			m_CurrentFontInfo.lf.lfWeight = FW_BOLD;
			continue;
		}
		///////////////////////////////////////////////////////////////////////
		else if (_tcsnicmp(strText, _T("</B>"), 4) == 0)	// check for </B>
		{
			n -= 4;
			index = strText.Find(_T('>'));
			if (index != -1)
				strText = strText.Mid(index+1);
			m_CurrentFontInfo = m_FontInfo.Pop();
			continue;
		}
		///////////////////////////////////////////////////////////////////////
		else if (_tcsnicmp(strText, _T("<I>"), 3) == 0)	// check for <I>
		{
			n -= 3;
			index = strText.Find(_T('>'));
			if (index != -1)
				strText = strText.Mid(index+1);
			m_FontInfo.Push(m_CurrentFontInfo);
			m_CurrentFontInfo.lf.lfItalic = TRUE;
			continue;
		}
		///////////////////////////////////////////////////////////////////////
		else if (_tcsnicmp(strText, _T("</I>"), 4) == 0)	// check for </I>
		{
			n -= 4;
			index = strText.Find(_T('>'));
			if (index != -1)
				strText = strText.Mid(index+1);
			m_CurrentFontInfo = m_FontInfo.Pop();
			continue;
		}
		///////////////////////////////////////////////////////////////////////
		else if (_tcsnicmp(strText, _T("<U>"), 3) == 0)		// check for <U>
		{
			n -= 3;
			index = strText.Find(_T('>'));
			if (index != -1)
				strText = strText.Mid(index+1);
			m_FontInfo.Push(m_CurrentFontInfo);
			m_CurrentFontInfo.lf.lfUnderline  = TRUE;
			continue;
		}
		///////////////////////////////////////////////////////////////////////
		else if (_tcsnicmp(strText, _T("</U>"), 4) == 0)	// check for </U>
		{
			n -= 4;
			index = strText.Find(_T('>'));
			if (index != -1)
				strText = strText.Mid(index+1);
			m_CurrentFontInfo = m_FontInfo.Pop();
			continue;
		}
		///////////////////////////////////////////////////////////////////////
		else if (_tcsnicmp(strText, _T("<STRIKE>"), 8) == 0)	// check for <STRIKE>
		{
			n -= 8;
			index = strText.Find(_T('>'));
			if (index != -1)
				strText = strText.Mid(index+1);
			m_FontInfo.Push(m_CurrentFontInfo);
			m_CurrentFontInfo.lf.lfStrikeOut   = TRUE;
			continue;
		}
		///////////////////////////////////////////////////////////////////////
		else if (_tcsnicmp(strText, _T("</STRIKE>"), 9) == 0)	// check for </STRIKE>
		{
			n -= 9;
			index = strText.Find(_T('>'));
			if (index != -1)
				strText = strText.Mid(index+1);
			m_CurrentFontInfo = m_FontInfo.Pop();
			continue;
		}
		///////////////////////////////////////////////////////////////////////
		else if (_tcsnicmp(strText, _T("<CENTER>"), 8) == 0)	// check for <CENTER>
		{
			n -= 8;
			index = strText.Find(_T('>'));
			if (index != -1)
				strText = strText.Mid(index+1);
			m_bCenter++;// = TRUE;
			continue;
		}
		///////////////////////////////////////////////////////////////////////
		else if (_tcsnicmp(strText, _T("</CENTER>"), 9) == 0)	// check for </CENTER>
		{
			n -= 9;
			index = strText.Find(_T('>'));
			if (index != -1)
				strText = strText.Mid(index+1);
			if (m_bCenter)
				m_bCenter--;// = FALSE;
			continue;
		}
		///////////////////////////////////////////////////////////////////////
		else if (_tcsnicmp(strText, _T("<BIG>"), 5) == 0)	// check for <BIG>
		{
			n -= 5;
			index = strText.Find(_T('>'));
			if (index != -1)
				strText = strText.Mid(index+1);
			m_FontInfo.Push(m_CurrentFontInfo);
			if (m_CurrentFontInfo.lf.lfHeight > 0)
				m_CurrentFontInfo.lf.lfHeight++;
			else
				m_CurrentFontInfo.lf.lfHeight--;
			continue;
		}
		///////////////////////////////////////////////////////////////////////
		else if (_tcsnicmp(strText, _T("</BIG>"), 6) == 0)	// check for </BIG>
		{
			n -= 6;
			index = strText.Find(_T('>'));
			if (index != -1)
				strText = strText.Mid(index+1);
			m_CurrentFontInfo = m_FontInfo.Pop();
			continue;
		}
		///////////////////////////////////////////////////////////////////////
		else if (_tcsnicmp(strText, _T("<SMALL>"), 7) == 0)	// check for <SMALL>
		{
			n -= 7;
			index = strText.Find(_T('>'));
			if (index != -1)
				strText = strText.Mid(index+1);
			m_FontInfo.Push(m_CurrentFontInfo);
			if (m_CurrentFontInfo.lf.lfHeight > 0)
				m_CurrentFontInfo.lf.lfHeight--;
			else
				m_CurrentFontInfo.lf.lfHeight++;
			continue;
		}
		///////////////////////////////////////////////////////////////////////
		else if (_tcsnicmp(strText, _T("</SMALL>"), 8) == 0)	// check for </SMALL>
		{
			n -= 8;
			index = strText.Find(_T('>'));
			if (index != -1)
				strText = strText.Mid(index+1);
			m_CurrentFontInfo = m_FontInfo.Pop();
			continue;
		}
		///////////////////////////////////////////////////////////////////////
		else if (_tcsnicmp(strText, _T("<SUB>"), 5) == 0)	// check for <SUB>
		{
			n -= 5;
			index = strText.Find(_T('>'));
			if (index != -1)
				strText = strText.Mid(index+1);
			// nBaselineAdjust is positive for subscript, negative for superscript
			m_FontInfo.Push(m_CurrentFontInfo);
			::GetTextMetrics(m_hMemDC, &tm);

			//m_CurrentFontInfo.nBaseLineAdjust += 6;
			m_CurrentFontInfo.nBaseLineAdjust += tm.tmAscent/2 + 1;
			//m_yStart += 1;//m_CurrentFontInfo.nBaseLineAdjust;
			//m_bSubscript++;// = TRUE;
			continue;
		}
		///////////////////////////////////////////////////////////////////////
		else if (_tcsnicmp(strText, _T("</SUB>"), 6) == 0)	// check for </SUB>
		{
			n -= 6;
			index = strText.Find(_T('>'));
			if (index != -1)
				strText = strText.Mid(index+1);
			m_CurrentFontInfo = m_FontInfo.Pop();
			//m_yStart -= 1;//m_CurrentFontInfo.nBaseLineAdjust;
			//if (m_bSubscript)
			//	m_bSubscript--;// = FALSE;
			continue;
		}
		///////////////////////////////////////////////////////////////////////
		else if (_tcsnicmp(strText, _T("<SUP>"), 5) == 0)	// check for <SUP>
		{
			n -= 5;
			index = strText.Find(_T('>'));
			if (index != -1)
				strText = strText.Mid(index+1);
			// nBaselineAdjust is positive for subscript, negative for superscript
			m_FontInfo.Push(m_CurrentFontInfo);
			m_CurrentFontInfo.nBaseLineAdjust -= 1;
			//m_yStart -= 1;//m_CurrentFontInfo.nBaseLineAdjust;
			//m_bSuperscript++;// = TRUE;
			continue;
		}
		///////////////////////////////////////////////////////////////////////
		else if (_tcsnicmp(strText, _T("</SUP>"), 6) == 0)	// check for </SUP>
		{
			n -= 6;
			index = strText.Find(_T('>'));
			if (index != -1)
				strText = strText.Mid(index+1);
			m_CurrentFontInfo = m_FontInfo.Pop();
			//m_yStart += 1;//m_CurrentFontInfo.nBaseLineAdjust;
//			if (m_bSuperscript)
//				m_bSuperscript--;// = FALSE;
			continue;
		}
		///////////////////////////////////////////////////////////////////////
		else if (_tcsnicmp(strText, _T("<FONT"), 5) == 0)	// check for <FONT
		{
			index = strText.Find(_T('>'));
			if (index != -1)
			{
				m_FontInfo.Push(m_CurrentFontInfo);

				CWTLString strAttributes = strText.Mid(5, index-5);
				int m = strAttributes.GetLength();
				strText = strText.Mid(index+1);

				// loop to parse FONT attributes
				while (m > 0)
				{
					// trim left whitespace
					if ((strAttributes.GetLength() > 0) && 
						(strAttributes[0] == _T(' ')))
					{
						m--;
						strAttributes = strAttributes.Mid(1);
						continue;
					}

					///////////////////////////////////////////////////////////
					if (_tcsnicmp(strAttributes, _T("COLOR"), 5) == 0)
					{
						int index2 = strAttributes.Find(_T('"'));
						if (index2 != -1)
						{
							m -= index2 + 1;
							strAttributes = strAttributes.Mid(index2+1);

							index2 = strAttributes.Find(_T('"'));
							if (index2 != -1)
							{
								CWTLString strColor = strAttributes.Left(index2);
								CXNamedColors nc(strColor);
								m_CurrentFontInfo.crText = nc.GetRGB();
								strAttributes = strAttributes.Mid(index2+1);
								m = strAttributes.GetLength();
							}
						}
						else
							break;
					}
					///////////////////////////////////////////////////////////
					else if (_tcsnicmp(strAttributes, _T("BGCOLOR"), 7) == 0)
					{
						int index2 = strAttributes.Find(_T('"'));
						if (index2 != -1)
						{
							m -= index2 + 1;
							strAttributes = strAttributes.Mid(index2+1);

							index2 = strAttributes.Find(_T('"'));
							if (index2 != -1)
							{
								CWTLString strBgColor = strAttributes.Left(index2);
								CXNamedColors nc(strBgColor);
								m_CurrentFontInfo.crBackground = nc.GetRGB();
								strAttributes = strAttributes.Mid(index2+1);
								m = strAttributes.GetLength();
							}
						}
						else
							break;
					}
					///////////////////////////////////////////////////////////
					else if (_tcsnicmp(strAttributes, _T("FACE"), 4) == 0)
					{
						int index2 = strAttributes.Find(_T('"'));
						if (index2 != -1)
						{
							m -= index2 + 1;
							strAttributes = strAttributes.Mid(index2+1);
							index2 = strAttributes.Find(_T('"'));
							if (index2 != -1)
							{
								memset(m_CurrentFontInfo.lf.lfFaceName, 0, 
									sizeof(m_CurrentFontInfo.lf.lfFaceName));
								_tcsncpy(m_CurrentFontInfo.lf.lfFaceName, strAttributes, index2);

								m -= index2 + 1;
								if (m > 0)
									strAttributes = strAttributes.Mid(index2+1);
								else
									strAttributes = _T("");
								m = strAttributes.GetLength();
							}
						}
						else
							break;
					}
					///////////////////////////////////////////////////////////
					else if (_tcsnicmp(strAttributes, _T("SIZE"), 4) == 0)
					{
						int index2 = strAttributes.Find(_T('"'));
						if (index2 != -1)
						{
							m -= index2 + 1;
							strAttributes = strAttributes.Mid(index2+1);
							index2 = strAttributes.Find(_T('"'));
							if (index2 != -1)
							{
								int nSize = 0;
								nSize = _ttoi(strAttributes);
								m_CurrentFontInfo.lf.lfHeight -= nSize;
								TRACE(_T("nSize=%d\n"), nSize);
								bSizeChange = TRUE;

								m -= index2 + 1;
								if (m > 0)
									strAttributes = strAttributes.Mid(index2+1);
								else
									strAttributes = _T("");
								m = strAttributes.GetLength();
							}
						}
						else
							break;
					}
					else
					{
						while ((strAttributes.GetLength() > 0) && 
							   (strAttributes[0] != _T(' ')))
						{
							m--;
							strAttributes = strAttributes.Mid(1);
						}
					}
				}
				TRACE(_T("strText=<%20.20s>\n"), strText);
			}
			else
			{
				// no ending >
				strText = strText.Mid(1);
			}

			n = strText.GetLength();
			continue;
		}
		///////////////////////////////////////////////////////////////////////
		else if (_tcsnicmp(strText, _T("</FONT>"), 7) == 0)	// check for </FONT>
		{
			n -= 7;
			index = strText.Find(_T('>'));
			if (index != -1)
				strText = strText.Mid(index+1);
			m_CurrentFontInfo = m_FontInfo.Pop();
			//if (bSizeChange)
			//++++++	m_yStart += tm.tmDescent;
			bSizeChange = FALSE;
			continue;
		}
		///////////////////////////////////////////////////////////////////////
		else if (_tcsnicmp(strText, _T("<IMG"), 4) == 0)	// check for <IMG	//+++2.0
		{
			index = strText.Find(_T('>'));
			if (index != -1)
			{
				nImageWidth = 0;
				nImageHeight = 0;
				eImageAlign = bottom;
				strSrc = _T("");
				CWTLString strImageTitle = _T("");
				nResId = 0;
				nResType = 0;

				CWTLString strAttributes = strText.Mid(4, index-4);
				int m = strAttributes.GetLength();
				strText = strText.Mid(index+1);

				// loop to parse IMG attributes
				while (m > 0)
				{
					// trim left whitespace
					if ((strAttributes.GetLength() > 0) && 
						(strAttributes[0] == _T(' ')))
					{
						m--;
						strAttributes = strAttributes.Mid(1);
						continue;
					}

					///////////////////////////////////////////////////////////
					if (_tcsnicmp(strAttributes, _T("SRC"), 3) == 0)
					{
						int index2 = strAttributes.Find(_T('"'));
						if (index2 != -1)
						{
							m -= index2 + 1;
							strAttributes = strAttributes.Mid(index2+1);

							index2 = strAttributes.Find(_T('"'));
							if (index2 != -1)
							{
								strSrc = strAttributes.Left(index2);
								strAttributes = strAttributes.Mid(index2+1);
								m = strAttributes.GetLength();
							}
						}
						else
							break;
					}
					///////////////////////////////////////////////////////////
					else if (_tcsnicmp(strAttributes, _T("TITLE"), 5) == 0)
					{
						int index2 = strAttributes.Find(_T('"'));
						if (index2 != -1)
						{
							m -= index2 + 1;
							strAttributes = strAttributes.Mid(index2+1);

							index2 = strAttributes.Find(_T('"'));
							if (index2 != -1)
							{
								strImageTitle = strAttributes.Left(index2);
								strAttributes = strAttributes.Mid(index2+1);
								m = strAttributes.GetLength();
							}
						}
						else
							break;
					}
					///////////////////////////////////////////////////////////
					else if (_tcsnicmp(strAttributes, _T("ID"), 2) == 0)
					{
						int index2 = strAttributes.Find(_T('"'));
						if (index2 != -1)
						{
							m -= index2 + 1;
							strAttributes = strAttributes.Mid(index2+1);

							index2 = strAttributes.Find(_T('"'));
							if (index2 != -1)
							{
								CWTLString str = strAttributes.Left(index2);
								nResId = _ttoi(str);
								strAttributes = strAttributes.Mid(index2+1);
								m = strAttributes.GetLength();
							}
						}
						else
							break;
					}
					///////////////////////////////////////////////////////////
					else if (_tcsnicmp(strAttributes, _T("TYPE"), 4) == 0)
					{
						int index2 = strAttributes.Find(_T('"'));
						if (index2 != -1)
						{
							m -= index2 + 1;
							strAttributes = strAttributes.Mid(index2+1);

							index2 = strAttributes.Find(_T('"'));
							if (index2 != -1)
							{
								CWTLString str = strAttributes.Left(index2);
								nResType = _ttoi(str);
								strAttributes = strAttributes.Mid(index2+1);
								m = strAttributes.GetLength();
							}
						}
						else
							break;
					}
					///////////////////////////////////////////////////////////
					else if (_tcsnicmp(strAttributes, _T("WIDTH"), 5) == 0)
					{
						int index2 = strAttributes.Find(_T('"'));
						if (index2 != -1)
						{
							m -= index2 + 1;
							strAttributes = strAttributes.Mid(index2+1);

							index2 = strAttributes.Find(_T('"'));
							if (index2 != -1)
							{
								CWTLString str = strAttributes.Left(index2);
								nImageWidth = _ttoi(str);
								strAttributes = strAttributes.Mid(index2+1);
								m = strAttributes.GetLength();
							}
						}
						else
							break;
					}
					///////////////////////////////////////////////////////////
					else if (_tcsnicmp(strAttributes, _T("HEIGHT"), 6) == 0)
					{
						int index2 = strAttributes.Find(_T('"'));
						if (index2 != -1)
						{
							m -= index2 + 1;
							strAttributes = strAttributes.Mid(index2+1);

							index2 = strAttributes.Find(_T('"'));
							if (index2 != -1)
							{
								CWTLString str = strAttributes.Left(index2);
								nImageHeight = _ttoi(str);
								TRACE(_T("nImageHeight=%d\n"), nImageHeight);
								strAttributes = strAttributes.Mid(index2+1);
								m = strAttributes.GetLength();
							}
						}
						else
							break;
					}
					///////////////////////////////////////////////////////////
					else if (_tcsnicmp(strAttributes, _T("ALIGN"), 5) == 0)
					{
						int index2 = strAttributes.Find(_T('"'));
						if (index2 != -1)
						{
							m -= index2 + 1;
							strAttributes = strAttributes.Mid(index2+1);

							index2 = strAttributes.Find(_T('"'));
							if (index2 != -1)
							{
								CWTLString str = strAttributes.Left(index2);
								eImageAlign = bottom;
								if (_tcsnicmp(str, _T("left"), 4) == 0)
									eImageAlign = left;
								else if (_tcsnicmp(str, _T("right"), 5) == 0)
									eImageAlign = right;
								else if (_tcsnicmp(str, _T("top"), 3) == 0)
									eImageAlign = top;
								else if (_tcsnicmp(str, _T("middle"), 6) == 0)
									eImageAlign = middle;
								else if (_tcsnicmp(str, _T("bottom"), 6) == 0)
									eImageAlign = bottom;
								strAttributes = strAttributes.Mid(index2+1);
								m = strAttributes.GetLength();
							}
						}
						else
							break;
					}
					else
					{
						while ((strAttributes.GetLength() > 0) && 
							   (strAttributes[0] != _T(' ')))
						{
							m--;
							strAttributes = strAttributes.Mid(1);
						}
					}
				} // while

				// IMG tag does not require </IMG> tag

				if (nImageWidth == 0 || nImageHeight == 0)
				{
					TRACE(_T("ERROR - width and height must be specified\n"));
				}
				else
				{
					m_sizeLeftIndent.cx = 0;
					m_sizeLeftIndent.cy = 0;
					m_sizeRightIndent.cx = 0;
					m_sizeRightIndent.cy = 0;
					RECT rectImage;
					if (eImageAlign == left)
					{
						rectImage.left   = rect.left + m_nLeftMargin;
						rectImage.right  = rectImage.left + nImageWidth;
						rectImage.top    = m_yStart + tm.tmHeight/4;
						rectImage.bottom = rectImage.top + nImageHeight;
						m_sizeLeftIndent.cx = nImageWidth + 5;
						m_sizeLeftIndent.cy = nImageHeight + 1;
					}
					else if (eImageAlign == right)
					{
						rectImage.left   = rect.right - nImageWidth;
						rectImage.right  = rect.right;
						rectImage.top    = m_yStart + tm.tmHeight/4;
						rectImage.bottom = rectImage.top + nImageHeight;
						m_sizeRightIndent.cx = nImageWidth + 5;
						m_sizeRightIndent.cy = nImageHeight + 1;
					}
					else if (eImageAlign == bottom)
					{
						int temp = nInitialXOffset + nImageWidth;
						if (temp < rect.right)
						{
							// image will fit on current line
							rectImage.left   = nInitialXOffset;
							nInitialXOffset += nImageWidth + 5;
							rectImage.right  = rectImage.left + nImageWidth;
							rectImage.top    = m_yStart;
							rectImage.bottom = rectImage.top + nImageHeight;
							m_sizeLeftIndent.cx = nImageWidth + 5;
							m_sizeLeftIndent.cy = tm.tmHeight - 2;
							m_yStart = rectImage.bottom - tm.tmHeight + 2;
						}
						else
						{
							// image will not fit on current line -
							// start it on next line
							rectImage.left   = rect.left;
							rectImage.right  = rectImage.left + nImageWidth;
							rectImage.top    = m_yStart + tm.tmHeight;
							rectImage.bottom = rectImage.top + nImageHeight;
							m_sizeLeftIndent.cx = nImageWidth + 5;
							m_sizeLeftIndent.cy = tm.tmHeight - 2;

							str1 = _T("\r\n");
							m_bGeneratedText = TRUE;
							nNextInitialXOffset = nImageWidth + 5;
						}
					}

					HWND hwnd = 0;
					
					TRACE(_T("m_nNextImageControlId=%d\n"), m_nNextImageControlId);

					if (strSrc.IsEmpty())
					{
						hwnd = HansDietrich::DisplayImageFromResource(m_hWnd,
									m_hResourceInstance,				//+++2.5
									nResType,
									nResId,
									m_nNextImageControlId++,
									rectImage, 
									TRUE,
									FALSE);
					}
					else
					{
						hwnd = HansDietrich::DisplayImageFromUrl(m_hWnd,
									strSrc,
									m_nNextImageControlId++,
									rectImage, 
									TRUE,
									FALSE);
					}

					_ASSERTE(hwnd);
					if (hwnd)
					{
						CWTLString strTooltip = _T("");
						if (m_bInAnchor)
						{
							if (!strAnchorTitle.IsEmpty())
							{
								// use title if it was specified
								strTooltip = strAnchorTitle;
							}
							else
							{
								// no title - display url if this is not an app: command
								if (_tcsnicmp(strAnchorText, _T("APP:"), 4) != 0)
								{
									strTooltip = strAnchorText;
								}
							}
						}

						// add tooltip for image window
						if (strTooltip.IsEmpty())
							strTooltip = strImageTitle;
						int nId = nToolId;
						if (strTooltip.IsEmpty())
							nId = 0;				// no tooltip
						int index = AddImageWindow(hwnd, nId, rectImage);
						if (index >= 0)
						{
							if (m_bToolTip && !strImageTitle.IsEmpty())
							{
								m_nImageTooltipActive = -1;
								TCHAR szTip[200];
								szTip[0] = 0;
								//if (!strImageTitle.IsEmpty())
									_tcscpy(szTip, strImageTitle);
								TRACE(_T("adding tool=%d size.cy=%d <%s> ======\n"), nToolId, m_sizeCursor.cy, strImageTitle);
								TRACERECT(rectImage);

								TOOLINFO ti = { 0 };
								ti.cbSize   = sizeof(ti);
								ti.uFlags   = TTF_TRACK | TTF_ABSOLUTE | TTF_TRANSPARENT;
								ti.hwnd     = m_hWnd;
								ti.uId      = nToolId;
								ti.lpszText = szTip;
								ti.rect     = rectImage;

								//m_ToolTip.SendMessage(TTM_ADDTOOL, 0, (LPARAM)&ti);
								if (IsWindow(m_ToolTip.m_hWnd))
									::SendMessage(m_ToolTip.m_hWnd, TTM_ADDTOOL, 
										0, (LPARAM)&ti);
							}

							if (m_bInAnchor)
							{
								XHTMLCTRL_ANCHOR_INFO info;
								info.rect = rectImage;
								_tcsncpy(info.szUrl, strAnchorText, _countof(info.szUrl));
								info.szUrl[_countof(info.szUrl)-1] = 0;
								m_AnchorInfo.Push(info);

								// save rect for this image - save in window coordinates
								TRACE(_T("added anchor:  <%s>\n"), strAnchorText);
								TRACERECT(rectImage);

								if (m_bToolTip)
								{
									if (!strTooltip.IsEmpty())
									{
										TRACE(_T("adding tool %d <%s>\n"), nToolId, strTooltip);
#ifdef XHTMLCTRL_USE_MFC
										m_ToolTip.AddTool(&m_CWnd, strTooltip, &rectImage, nToolId++);
#else
										m_ToolTip.AddTool(m_hWnd, strTooltip, &rectImage, nToolId++);
#endif //XHTMLCTRL_USE_MFC
									}
								}
							}
						}
						else
						{
							TRACE(_T("ERROR - AddImageWindow failed\n"));
							_ASSERTE(FALSE);
						}
					}
					nToolId++;
				}

				TRACE(_T("after <IMG: strText=<%20.20s>\n"), strText);
			}
			else
			{
				// no ending >
				strText = strText.Mid(1);
			}

			n = strText.GetLength();

			if (!m_bGeneratedText)
				continue;
		}
		///////////////////////////////////////////////////////////////////////
		else if (_tcsnicmp(strText, _T("<CODE>"), 6) == 0)	// check for <CODE>
		{
			n -= 6;
			index = strText.Find(_T('>'));
			if (index != -1)
				strText = strText.Mid(index+1);
			m_FontInfo.Push(m_CurrentFontInfo);
			_tcscpy(m_CurrentFontInfo.lf.lfFaceName, _T("Courier New"));
			continue;
		}
		///////////////////////////////////////////////////////////////////////
		else if (_tcsnicmp(strText, _T("</CODE>"), 7) == 0)	// check for </CODE>
		{
			n -= 7;
			index = strText.Find(_T('>'));
			if (index != -1)
				strText = strText.Mid(index+1);
			m_CurrentFontInfo = m_FontInfo.Pop();
			continue;
		}
		///////////////////////////////////////////////////////////////////////
		// <a title="whatever" href="www.xyz.com">XYZ Web Site</a>
		else if (_tcsnicmp(strText, _T("<A"), 2) == 0)	// check for <A
		{
			strAnchorText = _T("");
			strAnchorTitle = _T("");

			index = strText.Find(_T('>'));
			if (index != -1)
			{
				CWTLString strAttributes = strText.Mid(3, index-2);
				int m = strAttributes.GetLength();
				strText = strText.Mid(index+1);

				// loop to parse <A attributes
				while (m > 0)
				{
					// trim left whitespace
					if ((strAttributes.GetLength() > 0) && 
						(strAttributes[0] == _T(' ')))
					{
						m--;
						strAttributes = strAttributes.Mid(1);
						continue;
					}

					///////////////////////////////////////////////////////////
					if (_tcsnicmp(strAttributes, _T("HREF"), 4) == 0)
					{
						int index2 = strAttributes.Find(_T('"'));
						if (index2 != -1)
						{
							m -= index2 + 1;
							strAttributes = strAttributes.Mid(index2+1);

							index2 = strAttributes.Find(_T('"'));
							if (index2 != -1)
							{
								strAnchorText = strAttributes.Left(index2);
								TRACE(_T("strAnchorText=<%s>\n"), strAnchorText);
								strAttributes = strAttributes.Mid(index2+1);
								m = strAttributes.GetLength();
							}
						}
						else
							break;
					}
					///////////////////////////////////////////////////////////
					else if (_tcsnicmp(strAttributes, _T("TITLE"), 5) == 0)	//+++2.0
					{
						int index2 = strAttributes.Find(_T('"'));
						if (index2 != -1)
						{
							m -= index2 + 1;
							strAttributes = strAttributes.Mid(index2+1);

							index2 = strAttributes.Find(_T('"'));
							if (index2 != -1)
							{
								strAnchorTitle = strAttributes.Left(index2);
								strAttributes = strAttributes.Mid(index2+1);
								m = strAttributes.GetLength();
							}
						}
						else
							break;
					}
					else
					{
						while ((strAttributes.GetLength() > 0) && 
							   (strAttributes[0] != _T(' ')))
						{
							m--;
							strAttributes = strAttributes.Mid(1);
						}
					}
				} // while


				TRACE(_T("after <A: strText=<%20.20s>\n"), strText);
			}
			else
			{
				// no ending >
				strText = strText.Mid(1);
			}

			n = strText.GetLength();
			m_bInAnchor = TRUE;
			continue;
		}
		///////////////////////////////////////////////////////////////////////
		else if (_tcsnicmp(strText, _T("</A>"), 4) == 0)	// check for </A>
		{
			strText = strText.Mid(4);
			n -= 4;
			m_bInAnchor = FALSE;
			continue;
		}
		///////////////////////////////////////////////////////////////////////
		else if (_tcsnicmp(strText, _T("<HR"), 3) == 0)	// check for <HR>
		{
			index = strText.Find(_T('>'));
			if (index != -1)
			{
				CWTLString strAttributes = strText.Mid(3);
				int m = strAttributes.GetLength();
				strText = strText.Mid(index+1);

				m_nHorizontalRuleSize = 2;

				// loop to parse attributes
				while (m > 0)
				{
					// trim left whitespace
					if ((strAttributes.GetLength() > 0) && 
						(strAttributes[0] == _T(' ')))
					{
						m--;
						strAttributes = strAttributes.Mid(1);
						continue;
					}

					///////////////////////////////////////////////////////////
					if (_tcsnicmp(strAttributes, _T("SIZE"), 4) == 0)
					{
						int index2 = strAttributes.Find(_T('='));
						if (index2 != -1)
						{
							m -= index2 + 1;
							strAttributes = strAttributes.Mid(index2+1);

							index2 = strAttributes.Find(_T('>'));
							if (index2 != -1)
							{
								CWTLString strSize = strAttributes.Left(index2);
								m_nHorizontalRuleSize = _ttoi(strSize);
								strAttributes = strAttributes.Mid(index2+1);
								m = strAttributes.GetLength();
							}
						}
						else
							break;
					}
					else
					{
						while ((strAttributes.GetLength() > 0) && 
							   (strAttributes[0] != _T(' ')))
						{
							m--;
							strAttributes = strAttributes.Mid(1);
						}
					}
				}
			}
			else
			{
				// no ending >
				strText = strText.Mid(1);
			}

			n = strText.GetLength();
			m_bHorizontalRule++;// = TRUE;
			str1 = _T("\r\n");
			m_bGeneratedText = TRUE;
		}
		///////////////////////////////////////////////////////////////////////
		else if (_tcsnicmp(strText, _T("<H"), 2) == 0)	// check for <H1>, <H2>, etc.
		{
			index = strText.Find(_T('>'));
			if (index != -1)
			{
				TCHAR szHeader[2] = { 0 };
				szHeader[0] = strText[2];
				strText = strText.Mid(index+1);
				int nHeader = _ttoi(szHeader);
				if (nHeader > 0 && nHeader <= 6)
				{
					m_FontInfo.Push(m_CurrentFontInfo);
					m_CurrentFontInfo = m_HeaderFontInfo[nHeader-1];
					bSizeChange = TRUE;
					TRACE(_T("lfWeight=%d\n"), m_CurrentFontInfo.lf.lfWeight);
				}
			}
			else
			{
				// no ending >
				strText = strText.Mid(1);
			}
			n = strText.GetLength();
			continue;
		}
		///////////////////////////////////////////////////////////////////////
		else if (_tcsnicmp(strText, _T("</H"), 3) == 0)	// check for </H1>, </H2>, etc.
		{
			index = strText.Find(_T('>'));
			if (index != -1)
			{
				strText = strText.Mid(index+1);
				int nPointSize = GetFontPointSize(m_CurrentFontInfo.lf.lfHeight);
				m_CurrentFontInfo = m_FontInfo.Pop();
				TRACE(_T("bSizeChange=%d  tmDescent=%d  tmAscent=%d\n"), bSizeChange, tm.tmDescent, tm.tmAscent);
				
				int inc = nPointSize / 2; //tm.tmAscent/4;
				if (bSizeChange && (inc > 6))
				{
					TRACE(_T("applying inc=%d\n"), inc);
					m_yStart += inc;
				}
				bSizeChange = FALSE;
			}
			else
			{
				// no ending >
				strText = strText.Mid(1);
			}
			n = strText.GetLength();
			str1 = _T("\r\n\r\n");
			m_bGeneratedText = TRUE;
		}
		///////////////////////////////////////////////////////////////////////
		// <br> or \r\n or plain text
		else
		{
			str1 = strText;
			index = str1.Find(_T('<'));
			if (index != -1)
			{
				if (_tcsnicmp(strText, _T("<P>"), 3) == 0)	// check for <P>	//+++2.0
				{
					n -= 3;
					str1 = _T("\r\n\r\n");
					m_bGeneratedText = TRUE;
					strText = strText.Mid(3);
				}
				else if (_tcsnicmp(strText, _T("<BR>"), 4) == 0)	// check for <BR>
				{
					n -= 4;
					str1 = _T("\r\n");
					m_bGeneratedText = TRUE;
					strText = strText.Mid(4);
				}
				else
				{
					str1 = strText.Left(index);
					if (str1.GetLength() <= 0)
					{
						if (strText.GetLength() != 0)
						{
							str1 = strText[0];
							index = 1;
							n -= 1;
						}
					}
					strText = strText.Mid(index);
				}
			}
			else
			{
				//str1 = strText;
				strText = _T("");
			}
		}

		// update font

		if (hOldFont)
			SelectObject(m_hMemDC, hOldFont);
		if (hNewFont)
			DeleteObject(hNewFont);

		TRACE(_T("creating new font:  lf.lfFaceName=%s lf.lfHeight=%d\n"), m_CurrentFontInfo.lf.lfFaceName, m_CurrentFontInfo.lf.lfHeight);
		hNewFont = CreateFontIndirect(&m_CurrentFontInfo.lf);
		_ASSERTE(hNewFont);

		hOldFont = (HFONT) SelectObject(m_hMemDC, hNewFont);

		::SetTextColor(m_hMemDC, m_CurrentFontInfo.crText);

		if (GetWindowLong(m_hWnd, GWL_EXSTYLE) & WS_EX_TRANSPARENT)
			::SetBkMode(m_hMemDC, TRANSPARENT);
		else
			::SetBkColor(m_hMemDC, m_CurrentFontInfo.crBackground);

		GetTextMetrics(m_hMemDC, &tm);
		//int nBaselineAdjust = 1;//tm.tmAscent / 2;
		TRACE(_T("lfHeight=%d  tmAscent=%d  tmDescent=%d\n"), m_CurrentFontInfo.lf.lfHeight, tm.tmAscent, tm.tmDescent);

		// nBaselineAdjust is positive for subscript, negative for superscript
		//rect.top    += m_CurrentFontInfo.nBaseLineAdjust;
		//rect.bottom += m_CurrentFontInfo.nBaseLineAdjust;
		m_yStart    += m_CurrentFontInfo.nBaseLineAdjust;

#if 0  // -----------------------------------------------------------
		if (m_bSubscript)
		{
			rect.top += nBaselineAdjust;
			rect.bottom += nBaselineAdjust;
			m_yStart += nBaselineAdjust;
		}
		if (m_bSuperscript)
		{
			TRACE(_T("processing sup\n"));
			rect.top -= nBaselineAdjust;
			rect.bottom -= nBaselineAdjust;
			m_yStart -= nBaselineAdjust;
		}
#endif // -----------------------------------------------------------
		int saved_left = rect.left;
		if (m_bCenter)
		{
			TRACE(_T("centering <%s>\n"), str1);
			SIZE size;
			GetTextExtentPoint32(m_hMemDC, str1, (int)_tcslen(str1), &size);
			CXRect rectClient;
			::GetClientRect(m_hWnd, &rectClient);
			int w = rectClient.Width();
			nInitialXOffset = (w - size.cx) / 2;	//+++2.4
		}

		BOOL bCharEntityFound = FALSE;
		if (nInitialXOffset > 0)		//+++2.4
			rect.left = 0;				// remove left margin after first draw
		nInitialXOffset = FormatText(m_hMemDC, str1, &rect, nInitialXOffset, &bCharEntityFound);
		TRACE(_T("nInitialXOffset=%d  m_yStart=%d ~~~~~ \n"), nInitialXOffset, m_yStart);
		rect.left = saved_left;
		if ((str1.GetLength() > 1) && (str1[0] == _T('\r')) && (str1[1] == _T('\n')))
		{
			nInitialXOffset = nNextInitialXOffset;
			nNextInitialXOffset = 0;
		}

		if (bCharEntityFound)
			nInitialXOffset += 3;

		// leave extra room for italic
		if (m_CurrentFontInfo.lf.lfItalic && (nInitialXOffset != 0))	//+++2.0.1
			nInitialXOffset += 3;

		// leave extra room for super and subscripts
		if (m_CurrentFontInfo.nBaseLineAdjust)
			nInitialXOffset += 2;

		//rect.top    -= m_CurrentFontInfo.nBaseLineAdjust;
		//rect.bottom -= m_CurrentFontInfo.nBaseLineAdjust;
		m_yStart    -= m_CurrentFontInfo.nBaseLineAdjust;

#if 0  // -----------------------------------------------------------
		if (m_bSubscript)
		{
			rect.left += 1;
			rect.top -= nBaselineAdjust;
			rect.bottom -= nBaselineAdjust;
			m_yStart -= nBaselineAdjust;
		}
		if (m_bSuperscript)
		{
			rect.left += 1;
			rect.top += nBaselineAdjust;
			rect.bottom += nBaselineAdjust;
			m_yStart += nBaselineAdjust;
		}
#endif // -----------------------------------------------------------

		if (m_bInAnchor)
		{
			TRACE(_T("in anchor\n"));
			SIZE size;
			GetTextExtentPoint32(m_hMemDC, str1, str1.GetLength(), &size);

			CXRect rectDraw;
			rectDraw.top    = rect.top;	//+++2.0
			rectDraw.bottom = rectDraw.top + size.cy;
			rectDraw.left   = nInitialXOffset - size.cx;
			rectDraw.right  = nInitialXOffset;

			XHTMLCTRL_ANCHOR_INFO info;
			info.rect = rectDraw;
			_tcsncpy(info.szUrl, strAnchorText, _countof(info.szUrl));
			info.szUrl[_countof(info.szUrl)-1] = 0;
			m_AnchorInfo.Push(info);

			TRACE(_T("added anchor:  <%s>\n"), strAnchorText);
			TRACERECT(rectDraw);

			if (m_bToolTip)
			{
				CWTLString strTooltip = _T("");
				if (!strAnchorTitle.IsEmpty())
				{
					// use title if it was specified
					strTooltip = strAnchorTitle;
				}
				else
				{
					// no title - display url if this is not an app: command
					if (_tcsnicmp(strAnchorText, _T("APP:"), 4) != 0)
					{
						strTooltip = strAnchorText;
					}
				}
				if (!strTooltip.IsEmpty())
				{
					TRACE(_T("adding tool %d <%s>\n"), nToolId, strTooltip);
#ifdef XHTMLCTRL_USE_MFC
					m_ToolTip.AddTool(&m_CWnd, strTooltip, &rectDraw, nToolId++);
#else
					m_ToolTip.AddTool(m_hWnd, strTooltip, &rectDraw, nToolId++);
#endif //XHTMLCTRL_USE_MFC
				}
			}
		}  // if (m_bInAnchor)

		// draw horizontal rule 
		if (m_bHorizontalRule)
		{
			int nPenWidth = m_nHorizontalRuleSize;
			HPEN hPen = CreatePen(PS_SOLID, nPenWidth, m_CurrentFontInfo.crText);
			_ASSERTE(hPen);

			if (hPen)
			{
				HPEN hOldPen = (HPEN) SelectObject(m_hMemDC, hPen);

				::MoveToEx(m_hMemDC, rect.left-m_nLeftMargin, rect.top, NULL);
				::LineTo(m_hMemDC, rect.right+m_nRightMargin, rect.top);

				if (hOldPen)
					SelectObject(m_hMemDC, hOldPen);

				DeleteObject(hPen);
			}

			m_yStart += nPenWidth;
			rect.top += nPenWidth;
			rect.bottom += nPenWidth;
			nInitialXOffset = 0;

			m_bHorizontalRule--;
		}

		if (!m_bGeneratedText)
			n -= str1.GetLength();
		m_bGeneratedText = FALSE;
	}

	_ASSERTE(m_FontInfo.IsEmpty());

	// end double buffering - limit output to client rect
	BitBlt(hDC, 0, 0, nRectWidth, nRectHeight, m_hMemDC, 0, 0, SRCCOPY);

	// clean up font
	if (hOldFont)
		SelectObject(m_hMemDC, hOldFont);
	if (hNewFont)
		DeleteObject(hNewFont);

	// Do not call CStatic::OnPaint() for painting messages
}

//=============================================================================
BOOL CXHTMLCtrl::IsBlank(LPCTSTR lpszText)
//=============================================================================
{
	TCHAR c = 0;
	while ((c = *lpszText++) != _T('\0'))
		if (c != _T(' ') && c != _T('\t'))
			return FALSE;
	return TRUE;
}

//=============================================================================
int CXHTMLCtrl::FormatText(HDC hdc, 
						   LPCTSTR lpszText, 
						   RECT * pRect, 
						   int nInitialXOffset,
						   BOOL *pbCharEntityFound)
//=============================================================================
{
	TRACE(_T("in CXHTMLCtrl::FormatText:  nInitialXOffset=%d  <%s>\n"), 
		nInitialXOffset, lpszText);
	TRACERECT(*pRect);

	*pbCharEntityFound = FALSE;

	int		xStart, nWord, xLast;
	int		yStart = m_yStart;// + m_CurrentFontInfo.nBaseLineAdjust;
	TCHAR	*pText = (TCHAR *) lpszText;
	SIZE	size;

	xLast = 0;
	xStart = 0;

	if (pRect->top >= (pRect->bottom-1))
		return 0;

	// set initial size
	const TCHAR * szTest = _T("abcdefgABCDEFG");
	GetTextExtentPoint32(hdc, szTest, (int)_tcslen(szTest), &size);

	// prepare for next line - clear out the error term
	SetTextJustification(hdc, 0, 0);

	CWTLString strOut = _T("");

	BOOL bReturnSeen = FALSE;
	BOOL bCharEntityFound = FALSE;

	TEXTMETRIC tm = { 0 };
	::GetTextMetrics(hdc, &tm);

	int saved_right = pRect->right;
	pRect->right -= m_sizeRightIndent.cx; 

	int xNext = nInitialXOffset;
	if (xNext == 0)
		xNext = m_sizeLeftIndent.cx + m_nLeftMargin;	//+++2.4

	do									// for each text line
	{
		nWord = 0;						// initialize number of spaces in line

		// skip to first non-space in line
		while (/**pText != _T('\0') && */*pText == _T(' '))
		{
			if (xNext > m_sizeLeftIndent.cx)		//+++2.0.1
				strOut += *pText;
			pText++;
		}

		for (;;)							// process each word
		{
			TCHAR *saved_pText = pText;
			CWTLString strWord = GetNextWord(&pText, &bReturnSeen, &bCharEntityFound);

			if (bCharEntityFound)
				*pbCharEntityFound = TRUE;

			CWTLString strTrial = strOut + strWord;

			// after each word, calculate extents
			nWord++;
			GetTextExtentPoint32(hdc, strTrial, strTrial.GetLength(), &size);

			BOOL bOverflow = (size.cx >= (pRect->right - xNext - 2));	
											// don't get too close to margin,
											// in case of italic text

			// suppress overflow if sub or superscript
			if (m_CurrentFontInfo.nBaseLineAdjust)
				bOverflow = FALSE;

			if (bOverflow)
			{
				if (strOut.IsEmpty())
				{
					bOverflow = FALSE;
					strOut = strWord;

					// FOLLOWING CHANGE SUGGESTED BY DAVID PRITCHARD
					if (xNext > 0) 
					{
						yStart += size.cy;
						m_sizeLeftIndent.cy -= size.cy;
						if (m_sizeLeftIndent.cy < 0)
							m_sizeLeftIndent.cx = 0;
						xStart = m_sizeLeftIndent.cx + m_nLeftMargin;	//+++2.4
						xNext = m_sizeLeftIndent.cx + m_nLeftMargin;	//+++2.4
						m_sizeRightIndent.cy -= size.cy;
						if (m_sizeRightIndent.cy < 0)
						{
							m_sizeRightIndent.cx = 0;
							pRect->right = saved_right;
						}
					}
					// --------  END CHANGE  --------
				}
			}
			else
			{
				strOut += strWord;
			}

			if (bReturnSeen || bOverflow || (*pText == _T('\0')))
			{
				if (strOut.IsEmpty())
					break;

				if (bOverflow)
					pText = saved_pText;
				nWord--;               // discount last space at end of line

				// if end of text and no space characters, set pEnd to end

				GetTextExtentPoint32(hdc, strOut, strOut.GetLength(), &size);

				xStart = xNext;		//+++2.4
				if (xStart == 0)	//+++2.4
					xStart = pRect->left;
				xLast = xStart + size.cx;
				if (xStart == 0)	//+++2.4
				{
					xStart += m_nLeftMargin;	
					xLast += m_nLeftMargin;
				}

				// display the text

				if ((yStart <= (pRect->bottom-size.cy)))
				{
					if (!IsBlank(strOut))	//+++2.0
					{
						TRACE(_T("TextOut:  xStart=%d  <%s>\n"), xStart, strOut);
						TextOut(hdc, xStart, yStart, strOut, strOut.GetLength());
					}
					TRACE(_T("pText=<%s>\n"), pText);
					while (*pText == _T(' '))
						pText++;

					if (*pText || bReturnSeen)
					{
						yStart += size.cy;
						xLast = 0;
						m_sizeLeftIndent.cy -= size.cy;
						if (m_sizeLeftIndent.cy < 0)
							m_sizeLeftIndent.cx = 0;
						m_sizeRightIndent.cy -= size.cy;
						if (m_sizeRightIndent.cy < 0)
						{
							m_sizeRightIndent.cx = 0;
							pRect->right = saved_right;
						}
					}
				}
				xNext = m_sizeLeftIndent.cx + m_nLeftMargin;	//+++2.4

				// prepare for next line - clear out the error term
				SetTextJustification(hdc, 0, 0);

				strOut.Empty();
			}
			else	// new word will fit
			{

			}
			xStart = 0;	//+++2.4
		}

		nWord--;               // discount last space at end of line

		// prepare for next line - clear out the error term
		SetTextJustification(hdc, 0, 0);

		strOut.Empty();

	} while (*pText && (yStart < pRect->bottom));

	pRect->right = saved_right;
	//pRect->left -= m_sizIndent.cx;

	if (yStart > (pRect->bottom-size.cy))
		pRect->top = pRect->bottom;
	else
		pRect->top = yStart;

	m_yStart = yStart;// - m_CurrentFontInfo.nBaseLineAdjust;

	return xLast;
}

//=============================================================================
CWTLString CXHTMLCtrl::GetNextWord(TCHAR **ppText, 
									 BOOL *pbReturnSeen, 
									 BOOL *pbCharEntityFound)
//=============================================================================
{
	CWTLString strWord;
	strWord = _T("");
	TCHAR *pText = *ppText;

	BOOL bWordBreak = FALSE;

	*pbReturnSeen = FALSE;
	*pbCharEntityFound = FALSE;

	// skip to next word

	for(;;)
	{
		if (*pText == _T('\0'))
			break;

		// skip \r
		if (*pText == _T('\r'))
			pText++;

		// \n = new line
		if (*pText == _T('\n'))
		{
			strWord += _T(' ');
			pText++;
			*pbReturnSeen = TRUE;
			break;
		}

		TCHAR c = *pText;

		// process character entities
		if (c == _T('\001'))
		{
			c = *++pText;
			c = GetCharEntity(c);
			*pbCharEntityFound = TRUE;
		}

		strWord += c;

		if (*pText == _T(' ') || *pText == _T(',') || *pText == _T('\\') || *pText == _T('/') || *pText == _T('.'))
			bWordBreak = TRUE;

		pText++;

		if (bWordBreak)
			break;

	}

	*ppText = pText;

	return strWord;
}

//=============================================================================
BOOL CXHTMLCtrl::GotoURL(LPCTSTR url, int showcmd)
//=============================================================================
{
	TRACE(_T("in CXHTMLCtrl::GotoURL  <%s>\n"), url);

	BOOL bRet = FALSE;

	_ASSERTE(url);
	_ASSERTE(url[0] != _T('\0'));

	// check if this is "app:" protocol	
	int nAppSize = 0;
	if (_tcsnicmp(url, _T("APP:"), 4) == 0)
		nAppSize = 4;
	else if (_tcsnicmp(url, _T("\"APP:"), 5) == 0)
		nAppSize = 5;
	if (nAppSize)
	{
		bRet = ProcessAppCommand(&url[nAppSize]);
	}
	else
	{
		// not "app" - assume http: or mailto:

		// first try ShellExecute()
		int result = (int)::ShellExecute(NULL, _T("open"), url, NULL, NULL, 
						showcmd);

		if (result <= 32) 
		{
			TRACE(_T("ERROR - ShellExecute failed for <%s>\n"), url);
		}

		bRet = result > 32;
	}

	return bRet;
}

//=============================================================================
CXHTMLCtrl& CXHTMLCtrl::SetAppCommands(XHTMLCTRL_APP_COMMAND * paAppCommands, 
									   int nAppCommands)
//=============================================================================
{
	if (m_paAppCommands)
		delete [] m_paAppCommands;
	m_paAppCommands = NULL;

	m_nAppCommands = 0;

	if (paAppCommands && (nAppCommands > 0))
	{
		m_paAppCommands = new XHTMLCTRL_APP_COMMAND[nAppCommands];
		_ASSERTE(m_paAppCommands);
		memcpy(m_paAppCommands, paAppCommands, 
			sizeof(XHTMLCTRL_APP_COMMAND) * nAppCommands);
		m_nAppCommands = nAppCommands;
	}
	return *this;
}

//=============================================================================
BOOL CXHTMLCtrl::ProcessAppCommand(LPCTSTR lpszCommand)
//=============================================================================
{
	TRACE(_T("in CXHTMLCtrl::ProcessAppCommand:  %s\n"), lpszCommand);

	BOOL bRet = FALSE;

	CWTLString strCommand(lpszCommand);
	if (strCommand[0] == _T('"'))
		strCommand = strCommand.Mid(1);
	if (strCommand[strCommand.GetLength()-1] == _T('"'))
		strCommand = strCommand.Left(strCommand.GetLength()-1);

	if ((m_nAppCommands > 0) && (m_paAppCommands != NULL))
	{
		for (int i = 0; i < m_nAppCommands; i++)
		{
			if (_tcsicmp(m_paAppCommands[i].pszCommand, strCommand) == 0)
			{
				TRACE(_T("found app command %s\n"), strCommand);
				if (m_paAppCommands[i].hWnd &&
					::IsWindow(m_paAppCommands[i].hWnd))
				{
					::SendMessage(m_paAppCommands[i].hWnd, 
								  m_paAppCommands[i].uMessage, 
								  m_paAppCommands[i].wParam,
								  0);
					bRet = TRUE;
					break;
				}
			}
		}
	}

	return bRet;
}

//=============================================================================
LONG CXHTMLCtrl::GetRegKey(HKEY key, LPCTSTR subkey, LPTSTR retdata)
//=============================================================================
{
	HKEY hkey;
	LONG retval = RegOpenKeyEx(key, subkey, 0, KEY_QUERY_VALUE, &hkey);
	
	*retdata = 0;

	if (retval == ERROR_SUCCESS) 
	{
		long datasize = MAX_PATH;
		TCHAR data[MAX_PATH];
		retval = RegQueryValue(hkey, NULL, data, &datasize);
		if (retval == ERROR_SUCCESS) 
		{
			lstrcpy(retdata, data);
			RegCloseKey(hkey);
		}
	}
	
	return retval;
}

//=============================================================================
CXHTMLCtrl& CXHTMLCtrl::SetWindowText(LPCTSTR lpszString)
//=============================================================================
{
	m_strText = lpszString;
	Reset();
	RedrawWindow();
	return *this;
}

//=============================================================================
void CXHTMLCtrl::CleanupImageWindows()
//=============================================================================
{
	TRACE(_T("in CXHTMLCtrl::CleanupImageWindows: %d\n"), m_nImages);
	if (m_pImageInfo)
	{
		for (int i = 0; i < m_nImages; i++)
		{
			if (IsWindow(m_pImageInfo[i].hwnd))
			{
				TRACE(_T("destroying window 0x%X\n"), m_pImageInfo[i].hwnd);
				::DestroyWindow(m_pImageInfo[i].hwnd);
				m_pImageInfo[i].hwnd = 0;
			}
		}
		delete [] m_pImageInfo;
	}
	m_pImageInfo = 0;
	m_nImages = 0;
}

//=============================================================================
int CXHTMLCtrl::CountImageTags(LPCTSTR lpszString)
//=============================================================================
{
	int n = 0;
	TCHAR *cp = (TCHAR *) lpszString;

	while (_tcslen(cp) > 4)
	{
		TCHAR *cp1 = _tcsstr(cp, _T("<img"));
		if (!cp1)
		{
			cp1 = _tcsstr(cp, _T("<IMG"));
		}
		if (cp1)
		{
			n++;
			cp = cp1 + 4;
		}
		else
		{
			break;
		}
	}
	TRACE(_T("%d image tags found\n"), n);

	return n;
}

//=============================================================================
int CXHTMLCtrl::AddImageWindow(HWND hwnd, UINT nToolId, RECT& rect)
//=============================================================================
{
	int rc = -1;

	if (m_pImageInfo)
	{
		for (int i = 0; i < m_nImages; i++)
		{
			if (m_pImageInfo[i].hwnd == 0)
			{
				// we found an open slot
				m_pImageInfo[i].hwnd = hwnd;
				m_pImageInfo[i].id   = nToolId;
				m_pImageInfo[i].rect = rect;
				rc = i;
				break;
			}
		}
	}

	return rc;
}

//=============================================================================
int CXHTMLCtrl::FindActiveImageWindow(POINT point)
//=============================================================================
{
	int rc = -1;

	if (m_pImageInfo)
	{
		for (int i = 0; i < m_nImages; i++)
		{
			if (m_pImageInfo[i].hwnd != 0)
			{
				if (PtInRect(&m_pImageInfo[i].rect, point))
				{
					rc = i;
					break;
				}
			}
		}
	}

	return rc;
}

//=============================================================================
void CXHTMLCtrl::GetWindowRect(LPRECT lpRect)
//=============================================================================
{
	::GetWindowRect(m_hWnd, lpRect);
}

//=============================================================================
void CXHTMLCtrl::MoveWindow(LPCRECT lpRect, BOOL bRepaint /*= TRUE*/)
//=============================================================================
{
	::MoveWindow(m_hWnd, lpRect->left, lpRect->top, 
		lpRect->right-lpRect->left, lpRect->bottom-lpRect->top,
		bRepaint);
}

//=============================================================================
BOOL CXHTMLCtrl::ModifyStyleEx(DWORD dwRemove, DWORD dwAdd, UINT nFlags /*= 0*/)
//=============================================================================
{
	BOOL rc = FALSE;
	_ASSERTE(IsWindow(m_hWnd));
	if (IsWindow(m_hWnd))
	{
		LONG_PTR dwStyle = ::GetWindowLongPtr(m_hWnd, GWL_EXSTYLE);
		LONG_PTR dwNewStyle = (dwStyle & ~dwRemove) | dwAdd;
		if (dwStyle != dwNewStyle)
		{
			::SetWindowLongPtr(m_hWnd, GWL_EXSTYLE, dwNewStyle);
			if (nFlags != 0)
			{
				::SetWindowPos(m_hWnd, NULL, 0, 0, 0, 0,
					SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE | nFlags);
			}
			rc = TRUE;
		}
	}
	return rc;
}

//=============================================================================
BOOL CXHTMLCtrl::RedrawWindow(LPCRECT lpRectUpdate /*= NULL*/, 
								HRGN hrgnUpdate /*= NULL*/, 
								UINT flags /*= RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE*/)
//=============================================================================
{
	BOOL rc = FALSE;
	_ASSERTE(IsWindow(m_hWnd)); 
	if (IsWindow(m_hWnd))
		rc = ::RedrawWindow(m_hWnd, lpRectUpdate, hrgnUpdate, flags);
	return rc;
}

//=============================================================================
BOOL CXHTMLCtrl::UpdateWindow()
//=============================================================================
{
	BOOL rc = FALSE;
	_ASSERTE(IsWindow(m_hWnd)); 
	if (IsWindow(m_hWnd))
		rc = ::UpdateWindow(m_hWnd);
	return rc;
}

//=============================================================================
BOOL CXHTMLCtrl::ShowWindow(int nCmdShow)
//=============================================================================
{
	BOOL rc = FALSE;
	_ASSERTE(IsWindow(m_hWnd)); 
	if (IsWindow(m_hWnd))
		rc = ::ShowWindow(m_hWnd, nCmdShow);
	return rc;
}

//=============================================================================
BOOL CXHTMLCtrl::Invalidate(BOOL bErase /*= TRUE*/)
//=============================================================================
{
	BOOL rc = FALSE;
	_ASSERTE(IsWindow(m_hWnd)); 
	if (IsWindow(m_hWnd))
		rc = ::InvalidateRect(m_hWnd, NULL, bErase);
	return rc;
}

//=============================================================================
void CXHTMLCtrl::ClientToScreen(RECT *pRect)
//=============================================================================
{
	int w = pRect->right - pRect->left;
	int h = pRect->bottom - pRect->top;
	POINT point;
	point.x = pRect->left;
	point.y = pRect->top;
	::ClientToScreen(m_hWnd, &point);
	pRect->left = point.x;
	pRect->top = point.y;
	pRect->right = pRect->left + w;
	pRect->bottom = pRect->top + h;
}

//=============================================================================
UINT CXHTMLCtrl::SetTimer(UINT nIDEvent, 
							UINT nElapse, 
							TIMERPROC lpTimerFunc)
//=============================================================================
{
	return (UINT) ::SetTimer(m_hWnd, nIDEvent, nElapse, lpTimerFunc);
}

//=============================================================================
BOOL CXHTMLCtrl::KillTimer(UINT nIDEvent)
//=============================================================================
{
	return ::KillTimer(m_hWnd, nIDEvent);
}

//=============================================================================
void CXHTMLCtrl::SetHyperlinkCursor()
//=============================================================================
{
	int n = m_AnchorInfo.SizeUsed();
	//TRACE(_T("in CXHTMLCtrl::SetHyperlinkCursor:  n=%d\n"), n);

	if (n && IsWindowVisible(m_hWnd))
	{
		POINT point;
		if (GetCursorPos(&point))
		{
			::ScreenToClient(m_hWnd, &point);
			m_bOnHyperlink = FALSE;

			for (int i = 0; i < n; i++)
			{
				CXRect rect = m_AnchorInfo[i].rect;

				if (rect.PtInRect(point))		 // Cursor is currently over control
				{
					TRACE(_T("~~~~~ over hyperlink\n"));
					if (m_hLinkCursor)
					{
						HCURSOR hPrevCursor = ::SetCursor(m_hLinkCursor);
						if (m_hPrevCursor == NULL)
							m_hPrevCursor = hPrevCursor;
						m_bOnHyperlink = TRUE;
						break;
					}
				}
			}

			if (!m_bOnHyperlink)
			{
				if (m_hPrevCursor)
					::SetCursor(m_hPrevCursor);
				m_hPrevCursor = NULL;
			}
		}
	}
	else
	{
		// no hyperlinks, or window is hidden
	}
}

//=============================================================================
void CXHTMLCtrl::DisplayImageTooltip(POINT point)
//=============================================================================
{
	if (m_bToolTip && ::IsWindow(m_ToolTip.m_hWnd))
	{
		TRACEPOINT(point);

		int index = FindActiveImageWindow(point);
		if (index >= 0)
		{
			// cursor is over an image
			if ((m_nImageTooltipActive < 0) && (m_pImageInfo[index].id != 0))
			{
				// no image tooltip active, set hover timer
				TRACE(_T("in image rect\n"));
				SetTimer(TIMER_HOVER, m_nInitialDelay, NULL);
			}
		}
		else
		{
			// cursor is not over an image
			if (m_nImageTooltipActive >= 0)
			{
				// hide tooltip
				TRACE(_T("not in rect\n"));
				TOOLINFO ti = { 0 };
				ti.cbSize   = sizeof(ti);
				ti.hwnd     = m_hWnd;
				ti.uId      = m_pImageInfo[m_nImageTooltipActive].id;
				if (ti.uId != 0)
					::SendMessage(m_ToolTip.m_hWnd, TTM_TRACKACTIVATE, FALSE, (LPARAM)&ti);
			}
			m_nImageTooltipActive = -1;
		}
	}
}

//=============================================================================
void CXHTMLCtrl::OnTimer(UINT nIDEvent) 
//=============================================================================
{
	if (nIDEvent == TIMER_MOUSEMOVE)			// mouse move timer
	{
		CXPoint point;
		if (GetCursorPos(&point))
		{
			// if the cursor position has changed...
			if ((point != m_prevPoint) && IsWindow(m_hWnd))
			{
				m_prevPoint = point;

				CXRect rect;
				::GetWindowRect(m_hWnd, &rect);

				if (rect.PtInRect(point))
				{
					SetHyperlinkCursor();
					::ScreenToClient(m_hWnd, &point);
					DisplayImageTooltip(point);
				}
				else
				{
					KillTimer(nIDEvent);
				}
			}
		}
	}
	else if (nIDEvent == TIMER_HOVER)			// hover timer
	{
		TRACE(_T("hover timer\n"));

		KillTimer(nIDEvent);

		POINT point;
		if (::GetCursorPos(&point))
		{
			POINT pt = point;
			::ScreenToClient(m_hWnd, &point);

			m_nImageTooltipActive = FindActiveImageWindow(point);

			if (m_nImageTooltipActive >= 0)
			{
				if (IsWindow(m_ToolTip.m_hWnd))
				{
					if (m_pImageInfo[m_nImageTooltipActive].id != 0)
					{
						pt.y += m_sizeCursor.cy + 1;
						::SendMessage(m_ToolTip.m_hWnd, TTM_TRACKPOSITION, 0, (LPARAM)MAKELPARAM(pt.x, pt.y));

						TOOLINFO ti = { 0 };
						ti.cbSize = sizeof(ti);
						ti.uFlags = TTF_TRACK | TTF_ABSOLUTE | TTF_TRANSPARENT;
						ti.hwnd   = m_hWnd;
						ti.uId    = m_pImageInfo[m_nImageTooltipActive].id;
						if (ti.uId != 0)
							::SendMessage(m_ToolTip.m_hWnd, TTM_TRACKACTIVATE, TRUE, (LPARAM)&ti);
						TRACE(_T("starting TRACKACTIVATE\n"));

						SetTimer(TIMER_OUT_OF_RECT, 50, NULL);		// out-of-rect timer
						SetTimer(TIMER_AUTOPOP, m_nAutoPop, NULL);	// autopop timer
					}
				}
			}
		}
	}
	else if (nIDEvent == TIMER_OUT_OF_RECT)		// check if mouse no longer in image rect
	{
		POINT point;
		if (::GetCursorPos(&point))
		{
			::ScreenToClient(m_hWnd, &point);

			int index = FindActiveImageWindow(point);

			if (index < 0)
			{
				if (m_nImageTooltipActive >= 0)
				{
					if (IsWindow(m_ToolTip.m_hWnd))
					{
						TOOLINFO ti = { 0 };
						ti.cbSize = sizeof(ti);
						ti.hwnd   = m_hWnd;
						ti.uId    = m_pImageInfo[m_nImageTooltipActive].id;
						if (ti.uId != 0)
							::SendMessage(m_ToolTip.m_hWnd, TTM_TRACKACTIVATE, FALSE, (LPARAM)&ti);
					}
				}
				m_nImageTooltipActive = -1;
				KillTimer(nIDEvent);
				KillTimer(TIMER_AUTOPOP);
			}
		}
		else
		{
			m_nImageTooltipActive = -1;
			KillTimer(nIDEvent);
		}
	}
	else if (nIDEvent == TIMER_AUTOPOP)		// autopop timer
	{
		POINT point;
		if (::GetCursorPos(&point))
		{
			::ScreenToClient(m_hWnd, &point);

			int index = FindActiveImageWindow(point);

			if (index < 0)
			{
				if (m_nImageTooltipActive >= 0)
				{
					if (IsWindow(m_ToolTip.m_hWnd))
					{
						TOOLINFO ti = { 0 };
						ti.cbSize = sizeof(ti);
						ti.hwnd   = m_hWnd;
						ti.uId    = m_pImageInfo[m_nImageTooltipActive].id;
						if (ti.uId != 0)
							::SendMessage(m_ToolTip.m_hWnd, TTM_TRACKACTIVATE, FALSE, (LPARAM)&ti);
					}
				}
				m_nImageTooltipActive = -1;
			}
		}
		m_ToolTip.Pop();
		KillTimer(nIDEvent);
		KillTimer(TIMER_OUT_OF_RECT);
	}
}

//=============================================================================
//
// GetCursorOffset()
//
// Purpose:     GetCursorOffset() returns the location of the non-transparent 
//              pixel in the cursor closest to the bottom right corner of 
//              the cursor
//
// Parameters:  hWnd - HWND of window containing cursor
//
// Returns:     SIZE - coordinates of last pixel location
//
SIZE CXHTMLCtrl::GetCursorOffset(HWND hWnd)
{
	SIZE sz = { 32, 32 };

	// get info for current cursor
	ICONINFO info;
	HICON hIcon = (HICON) ::GetCursor();
	if (hIcon == NULL)
	{
		hIcon = (HICON)::LoadCursor(NULL, IDC_ARROW);
	}

	_VERIFY(::GetIconInfo(hIcon, &info));

	if (info.hbmMask)
	{
		BITMAP bmInfo;
		::GetObject(info.hbmMask, sizeof(BITMAP), &bmInfo);
		sz.cx = bmInfo.bmWidth;
		sz.cy = bmInfo.bmHeight;

		CXDC dcWindow;
		HDC hDC = ::GetDC(hWnd);
		dcWindow.Attach(hDC);
		CXDC dc;
		dc.CreateCompatibleDC(&dcWindow);
		HBITMAP oldbmp = (HBITMAP) dc.SelectObject(info.hbmMask);
		int row = 0, col = 0;
		BOOL bFinished = FALSE;

		// start at bottom and search up
		for (row = bmInfo.bmHeight - 1; row >= 0; row--)
		{
			COLORREF first = dc.GetPixel(bmInfo.bmWidth - 1, row);

			// start at right and search to left
			for (col = bmInfo.bmWidth - 1; col >= 0; col--)
			{
				COLORREF cr = dc.GetPixel(col, row);
				if (cr != first)
				{
					// color has changed
					TRACE(_T("row=%d  col=%d  cr=%X  first=%X\n"), row, col, cr, first);
					bFinished = TRUE;
					break;
				}
			}
			if (bFinished)
				break;
		}
		if (bFinished)
		{
			sz.cx = col;
			sz.cy = row;
		}

		dc.SelectObject(oldbmp);

		// must delete bitmaps returned by GetIconInfo
		::DeleteObject(info.hbmMask);
		if (info.hbmColor)
			::DeleteObject(info.hbmColor);

		::ReleaseDC(hWnd, hDC);
	}

	TRACE(_T("sz.cx=%d  sz.cy=%d\n"), sz.cx, sz.cy);
	return sz;
}

//=============================================================================
void CXHTMLCtrl::GetSafeFont(LOGFONT& lf)
//=============================================================================
{
	int rc = 0;

	HFONT hFont = (HFONT) ::SendMessage(m_hParent, WM_GETFONT, 0, 0);
	if (hFont)
	{
		rc = ::GetObject(hFont, sizeof(lf), &lf);
	}

	if (!rc)
	{
		struct OLD_NONCLIENTMETRICS
		{
			UINT    cbSize;
			int     iBorderWidth;
			int     iScrollWidth;
			int     iScrollHeight;
			int     iCaptionWidth;
			int     iCaptionHeight;
			LOGFONT lfCaptionFont;
			int     iSmCaptionWidth;
			int     iSmCaptionHeight;
			LOGFONT lfSmCaptionFont;
			int     iMenuWidth;
			int     iMenuHeight;
			LOGFONT lfMenuFont;
			LOGFONT lfStatusFont;
			LOGFONT lfMessageFont;
		};

		// get the system window message font

		const UINT cbProperSize = sizeof(OLD_NONCLIENTMETRICS);

		NONCLIENTMETRICS ncm;
		ncm.cbSize = cbProperSize;

#ifdef _DEBUG
		BOOL ok = 
#endif
			::SystemParametersInfo(SPI_GETNONCLIENTMETRICS, cbProperSize, &ncm, 0);
		_ASSERTE(ok);

		TRACE(_T("message font=<%s>\n"), ncm.lfMessageFont.lfFaceName);
		lf = ncm.lfMessageFont;
	}
}

//=============================================================================
LOGFONT& CXHTMLCtrl::GetLogFont(int index /*= -1*/)
//=============================================================================
{
	if ((index >= 0) && (index < 6))
		return m_HeaderFontInfo[index].lf;
	else
		return m_CurrentFontInfo.lf;
}

//=============================================================================
CXHTMLCtrl& CXHTMLCtrl::SetLogFont(const LOGFONT& lf, int index /*= -1*/)
//=============================================================================
{
	if ((index >= 0) && (index < 6))
		m_HeaderFontInfo[index].lf = lf;
	else
		m_CurrentFontInfo.lf = lf;
	return *this;
}

//=============================================================================
// Returns point size corresponding to height
int CXHTMLCtrl::GetFontPointSize(int nHeight)
//=============================================================================
{
	HDC hdc = ::CreateDC(_T("DISPLAY"), NULL, NULL, NULL);
	_ASSERTE(hdc);
	int cyPixelsPerInch = ::GetDeviceCaps(hdc, LOGPIXELSY);
	::DeleteDC(hdc);

	int nPointSize = MulDiv(nHeight, 72, cyPixelsPerInch);
	if (nPointSize < 0)
		nPointSize = -nPointSize;

	return nPointSize;
}

//=============================================================================
// Returns height corresponding to point size
int CXHTMLCtrl::GetFontHeight(int nPointSize)
//=============================================================================
{
	HDC hdc = ::CreateDC(_T("DISPLAY"), NULL, NULL, NULL);
	_ASSERTE(hdc);
	int cyPixelsPerInch = ::GetDeviceCaps(hdc, LOGPIXELSY);
	::DeleteDC(hdc);

	int nHeight = -MulDiv(nPointSize, cyPixelsPerInch, 72);

	return nHeight;
}

//=============================================================================
CXHTMLCtrl& CXHTMLCtrl::SetFontHeight(int nHeight, int index /*= -1*/)
//=============================================================================
{
	if ((index >= 0) && (index < 6))
		m_HeaderFontInfo[index].lf.lfHeight = nHeight;
	else
		m_CurrentFontInfo.lf.lfHeight = nHeight;
	return *this;
}

//=============================================================================
CXHTMLCtrl& CXHTMLCtrl::SetFontPointSize(int nPointSize, int index /*= -1*/)
//=============================================================================
{
	int nHeight = GetFontHeight(nPointSize);
	return SetFontHeight(nHeight, index);
}

//=============================================================================
COLORREF CXHTMLCtrl::GetBkColor(int index /*= -1*/) const 
//=============================================================================
{ 
	if ((index >= 0) && (index < 6))
		return m_HeaderFontInfo[index].crBackground;
	else
		return m_CurrentFontInfo.crBackground; 
}

//=============================================================================
CXHTMLCtrl& CXHTMLCtrl::SetBkColor(COLORREF rgb, int index /*= -1*/) 
//=============================================================================
{
	if (rgb & 0x80000000)
		rgb = ::GetSysColor(rgb & 0x7FFFFFFF);
	COLORREF oldrgb = 0;
	if ((index >= 0) && (index < 6))
	{
		oldrgb = m_HeaderFontInfo[index].crBackground;
		m_HeaderFontInfo[index].crBackground = rgb;
	}
	else
	{
		oldrgb = m_CurrentFontInfo.crBackground;
		m_CurrentFontInfo.crBackground = rgb;
	}
	return *this;
}

//=============================================================================
CXHTMLCtrl& CXHTMLCtrl::SetBkColor(LPCTSTR lpszColor, int index /*= -1*/) 
//=============================================================================
{
	_ASSERTE(lpszColor);
	if (lpszColor)
	{
		CXNamedColors nc(lpszColor);
		SetBkColor(nc.GetRGB(), index);
	}
	return *this;
}

//=============================================================================
CXHTMLCtrl& CXHTMLCtrl::SetAllBkColor(COLORREF rgb) 
//=============================================================================
{
	if (rgb & 0x80000000)
		rgb = ::GetSysColor(rgb & 0x7FFFFFFF);
	for (int index = 0; index < 6; index++)
		m_HeaderFontInfo[index].crBackground = rgb;
	m_CurrentFontInfo.crBackground = rgb;
	return *this;
}

//=============================================================================
CXHTMLCtrl& CXHTMLCtrl::SetAllBkColor(LPCTSTR lpszColor) 
//=============================================================================
{
	_ASSERTE(lpszColor);
	if (lpszColor)
	{
		CXNamedColors nc(lpszColor);
		SetAllBkColor(nc.GetRGB());
	}
	return *this;
}

//=============================================================================
COLORREF CXHTMLCtrl::GetTextColor(int index /*= -1*/) const 
//=============================================================================
{ 
	if ((index >= 0) && (index < 6))
		return m_HeaderFontInfo[index].crText;
	else
		return m_CurrentFontInfo.crText; 
}

//=============================================================================
CXHTMLCtrl& CXHTMLCtrl::SetTextColor(COLORREF rgb, int index /*= -1*/)
//=============================================================================
{
	if (rgb & 0x80000000)
		rgb = ::GetSysColor(rgb & 0x7FFFFFFF);
	COLORREF oldrgb = 0;
	if ((index >= 0) && (index < 6))
	{
		oldrgb = m_HeaderFontInfo[index].crText;
		m_HeaderFontInfo[index].crText = rgb;
	}
	else
	{
		oldrgb = m_CurrentFontInfo.crText;
		m_CurrentFontInfo.crText = rgb;
	}
	return *this;
}

//=============================================================================
CXHTMLCtrl& CXHTMLCtrl::SetTextColor(LPCTSTR lpszColor, int index /*= -1*/) 
//=============================================================================
{
	_ASSERTE(lpszColor);
	if (lpszColor)
	{
		CXNamedColors nc(lpszColor);
		SetTextColor(nc.GetRGB(), index);
	}
	return *this;
}

//=============================================================================
CXHTMLCtrl& CXHTMLCtrl::SetAllTextColor(COLORREF rgb) 
//=============================================================================
{
	if (rgb & 0x80000000)
		rgb = ::GetSysColor(rgb & 0x7FFFFFFF);
	for (int index = 0; index < 6; index++)
		m_HeaderFontInfo[index].crText = rgb;
	m_CurrentFontInfo.crText = rgb;
	return *this;
}

//=============================================================================
CXHTMLCtrl& CXHTMLCtrl::SetAllTextColor(LPCTSTR lpszColor) 
//=============================================================================
{
	_ASSERTE(lpszColor);
	if (lpszColor)
	{
		CXNamedColors nc(lpszColor);
		SetAllTextColor(nc.GetRGB());
	}
	return *this;
}
}