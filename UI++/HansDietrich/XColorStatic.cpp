// XColorStatic.cpp  Version 2.1
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
#include "XColorStatic.h"
#include "FontSize.h"

#pragma warning(disable : 4996)	// disable bogus deprecation warning

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

namespace HansDietrich {


//=============================================================================
BEGIN_MESSAGE_MAP(CXColorStatic, CStatic)
//=============================================================================
	//{{AFX_MSG_MAP(CXColorStatic)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//=============================================================================
CXColorStatic::CXColorStatic(UINT n_Justify /*= DT_LEFT*/)
//=============================================================================
{
	m_rgbText       = GetSysColor(COLOR_BTNTEXT);
	m_rgbBackground = GetSysColor(COLOR_BTNFACE);
	m_nJustify		= n_Justify;
	m_pBrush        = new CBrush(m_rgbBackground);
	m_bBold         = FALSE;
	m_hIcon         = NULL;
	m_strText       = _T("");
	m_nXMargin = m_nYMargin = 0;
	HFONT hFont = (HFONT)::GetStockObject(DEFAULT_GUI_FONT);
	CFont *pFont = CFont::FromHandle(hFont);
	pFont->GetLogFont(&m_lf);
	m_font.CreateFontIndirect(&m_lf);
}

//=============================================================================
CXColorStatic::~CXColorStatic()
//=============================================================================
{
	//TRACE(_T("in CXColorStatic::~CXColorStatic\n"));

	if (m_font.GetSafeHandle())
		m_font.DeleteObject();

	if (m_pBrush)
	{
		m_pBrush->DeleteObject();
		delete m_pBrush;
	}
	m_pBrush = NULL;
}

//=============================================================================
void CXColorStatic::PreSubclassWindow() 
//=============================================================================
{
	//TRACE(_T("in CXColorStatic::PreSubclassWindow\n"));
	
	if (!m_font.GetSafeHandle())
	{
		// get current font
		CFont* pFont = GetSafeFont();
		ASSERT(pFont);
		ASSERT(pFont->GetSafeHandle());
		pFont->GetLogFont(&m_lf);
		m_font.CreateFontIndirect(&m_lf);
	}

	DWORD dwStyle = GetStyle();
	SetWindowLong(GetSafeHwnd(), GWL_STYLE, dwStyle | SS_NOTIFY | m_nJustify);

	GetWindowText(m_strText);
	CStatic::SetWindowText(_T(""));
}

BOOL CXColorStatic::PreTranslateMessage(MSG* pMsg)
{
	if (m_tooltipCreated)
		m_tooltip.RelayEvent(pMsg);

	return CStatic::PreTranslateMessage(pMsg);
}

//=============================================================================
void CXColorStatic::OnPaint() 
//=============================================================================
{
	CPaintDC dc(this); // device context for painting
	
	dc.SaveDC();

	CRect rect;
	GetClientRect(rect); 

	dc.SelectObject(m_pBrush);
	dc.SetTextColor(m_rgbText);
	if (GetExStyle() & WS_EX_TRANSPARENT)	//+++2.1
	{
		dc.SetBkMode(TRANSPARENT);
	}
	else
	{
		dc.SetBkColor(m_rgbBackground);
		dc.SetBkMode(OPAQUE);
		dc.FillSolidRect(&rect, m_rgbBackground);
	}

	// cannot have both an icon and text

	if (m_hIcon)
	{
		int nIconX = ::GetSystemMetrics(SM_CXICON);
		int nIconY = ::GetSystemMetrics(SM_CYICON);

		rect.left = rect.left + (rect.Width() - nIconX) / 2;
		rect.top = rect.top + (rect.Height() - nIconY) / 2;

		dc.DrawIcon(rect.left, rect.top, m_hIcon);
	}
	else
	{
		dc.SelectObject(&m_font);

		UINT nFormat = 0;
		DWORD dwStyle = GetStyle();

		// set DrawText format from static style settings
		if (dwStyle & SS_CENTER)
			nFormat |= DT_CENTER;
		else if (dwStyle & SS_LEFT)
			nFormat |= DT_LEFT;
		else if (dwStyle & SS_RIGHT)
			nFormat |= DT_RIGHT;

		if (dwStyle & SS_CENTERIMAGE)	// vertical centering ==> single line only
			nFormat |= DT_VCENTER | DT_SINGLELINE;
		else
			nFormat |= DT_WORDBREAK;

		if (dwStyle & SS_NOPREFIX)
			nFormat |= DT_NOPREFIX;

		if (m_strText.Find(_T('\t')) != -1)
			nFormat |= DT_EXPANDTABS;

		rect.left  += m_nXMargin;
		rect.right -= m_nXMargin;		//+++2.1
		rect.top   += m_nYMargin;
		dc.DrawText(m_strText, rect, nFormat);

		m_extent = dc.GetTextExtent(m_strText);
	}
	
	dc.RestoreDC(-1);
}

//=============================================================================
BOOL CXColorStatic::OnEraseBkgnd(CDC* pDC) 
//=============================================================================
{
	if (!(GetExStyle() & WS_EX_TRANSPARENT))	//+++2.1
	{
		CRect rect;
		GetClientRect(rect); 
		pDC->FillRect(&rect, m_pBrush);
	}

	return TRUE; //CStatic::OnEraseBkgnd(pDC);
}

//=============================================================================
CXColorStatic& CXColorStatic::SetText(LPCTSTR lpszString, BOOL bDynamicFontSize)
//=============================================================================
{
	m_strText = lpszString;

	if (bDynamicFontSize)
	{
		BOOL textWidthOK = FALSE;
		int defaultFontSize = m_lf.lfHeight;
		SetFont(m_lf.lfFaceName, GetFontPointSize(defaultFontSize), FALSE);

		CPaintDC dc(this); // device context for painting

		dc.SaveDC();

		CRect rect;
		GetClientRect(rect);

		while (!textWidthOK)
		{
			dc.SelectObject(&m_font);

			DWORD dwStyle = GetStyle();

			int width = dc.GetTextExtent(m_strText).cx;

			if (!(dwStyle & SS_CENTERIMAGE))
				width /= 2;

			if (width > (rect.Width() - (m_nXMargin * 2)))
			{
				int fontSize = GetFontPointSize(m_lf.lfHeight);

				if (fontSize > 8)
					SetFont(m_lf.lfFaceName, fontSize - 1, FALSE);
				else
					textWidthOK = TRUE;
			}
			else
				textWidthOK = TRUE;
		}

		dc.RestoreDC(-1);

		m_lf.lfHeight = defaultFontSize;
	}

	Invalidate();
	return *this;
}

//=============================================================================
CXColorStatic& CXColorStatic::SetFont(LOGFONT *pLogFont, BOOL bRedraw /*= TRUE*/)
//=============================================================================
{
	ASSERT(pLogFont);
	if (!pLogFont)
		return *this;

	m_lf = *pLogFont;

	if (m_font.GetSafeHandle())
		m_font.DeleteObject();

	VERIFY(m_font.CreateFontIndirect(&m_lf));

	if (bRedraw)
		RedrawWindow();

	return *this;
}

//=============================================================================
CXColorStatic& CXColorStatic::SetFont(LPCTSTR lpszFaceName, 
									  int nPointSize, 
									  BOOL bRedraw /*= TRUE*/)
//=============================================================================
{
	// null face name is ok - we will use current font

	if (lpszFaceName != NULL)
		_tcsncpy(m_lf.lfFaceName, lpszFaceName, sizeof(m_lf.lfFaceName)/sizeof(TCHAR)-1);
	m_lf.lfFaceName[sizeof(m_lf.lfFaceName)/sizeof(TCHAR)-1] = 0;

	m_lf.lfHeight = GetFontHeight(nPointSize);

	SetFont(&m_lf, bRedraw);

	return *this;
}

//=============================================================================
CXColorStatic& CXColorStatic::SetFont(CFont *pFont, BOOL bRedraw /*= TRUE*/)
//=============================================================================
{
	ASSERT(pFont);
	if (!pFont)
		return *this;

	pFont->GetLogFont(&m_lf);
	SetFont(&m_lf, bRedraw);

	return *this;
}

//=============================================================================
CXColorStatic& CXColorStatic::SetTextColor(COLORREF rgb, BOOL bRedraw /*= TRUE*/) 
//=============================================================================
{ 
	m_rgbText = rgb; 
	if (bRedraw)
		RedrawWindow();
	return *this;
}

//=============================================================================
CXColorStatic& CXColorStatic::SetBold(BOOL bBold, BOOL bRedraw /*= TRUE*/)
//=============================================================================
{ 
	m_bBold = bBold;

	m_lf.lfWeight = bBold ? FW_BOLD : FW_NORMAL;

	SetFont(&m_lf, bRedraw);

	return *this;
}

//=============================================================================
CXColorStatic& CXColorStatic::SetBackgroundColor(COLORREF rgb, BOOL bRedraw /*= TRUE*/) 
//=============================================================================
{ 
	m_rgbBackground = rgb; 
	if (m_pBrush)
	{
		m_pBrush->DeleteObject();
		delete m_pBrush;
	}
	m_pBrush = new CBrush(m_rgbBackground);
	if (bRedraw)
		RedrawWindow();

	return *this;
}

//=============================================================================
CXColorStatic& CXColorStatic::SetIcon(HICON hIcon, BOOL bRedraw /*= TRUE*/)
//=============================================================================
{
	ASSERT(hIcon);

	m_hIcon = hIcon;
	if (bRedraw)
		RedrawWindow();

	return *this;
}

//=============================================================================
CFont * CXColorStatic::GetSafeFont()
//=============================================================================
{
	// get current font
	CFont *pFont = CWnd::GetFont();

	if (pFont == 0)
	{
		// try to get parent font
		CWnd *pParent = GetParent();
		if (pParent && IsWindow(pParent->m_hWnd))
			pFont = pParent->GetFont();

		if (pFont == 0)
		{
			// no font, so get a system font
			HFONT hFont = (HFONT)::GetStockObject(DEFAULT_GUI_FONT);
			if (hFont == 0)
				hFont = (HFONT)::GetStockObject(SYSTEM_FONT);
			if (hFont == 0)
				hFont = (HFONT)::GetStockObject(ANSI_VAR_FONT);
			if (hFont)
				pFont = CFont::FromHandle(hFont);
		}
	}

	return pFont;
}

CXColorStatic& CXColorStatic::CreateTooltip(PCTSTR text)
{
	if (text != nullptr && _tcslen(text) > 0)
	{
		m_tooltip.Create(this, TTS_ALWAYSTIP | TTS_BALLOON | TTS_NOPREFIX);
		CRect oRect;
		GetClientRect(&oRect);
		m_tooltip.AddTool(this, text, oRect, 1001);
		m_tooltip.ShowWindow(SW_HIDE);
		m_tooltipCreated = true;
	}
	
	return *this;

}

}