// XColorStatic.h  Version 2.1
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

#ifndef XCOLORSTATIC_H
#define XCOLORSTATIC_H

namespace HansDietrich {

//=============================================================================
class CXColorStatic : public CStatic
//=============================================================================
{
// Construction
public:
	CXColorStatic(UINT n_Justify = DT_LEFT);
	virtual ~CXColorStatic();

// Attributes
public:
	CXColorStatic& SetBackgroundColor(COLORREF rgb, BOOL bRedraw = TRUE);
	CXColorStatic& SetBold(BOOL bBold, BOOL bRedraw = TRUE);
	CXColorStatic& SetFont(LPCTSTR lpszFaceName, int nPointSize, BOOL bRedraw = TRUE);
	CXColorStatic& SetFont(LOGFONT * pLogFont, BOOL bRedraw = TRUE);
	CXColorStatic& SetFont(CFont *pFont, BOOL bRedraw = TRUE);
	CXColorStatic& SetIcon(HICON hIcon, BOOL bRedraw = TRUE);
	CXColorStatic& SetMargins(int x, int y) 
	{ m_nXMargin = x; m_nYMargin = y; return *this; }
	CXColorStatic& SetTextColor(COLORREF rgb, BOOL bRedraw = TRUE);
	CXColorStatic& SetText(LPCTSTR lpszString, BOOL bDynamicFontSize = FALSE);

	CXColorStatic& CreateTooltip(PCTSTR text);

	const CSize GetExtent() { return m_extent; }

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CXColorStatic)
protected:
    virtual void PreSubclassWindow();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

// Implementation
protected:
	CString		m_strText;
	CFont		m_font;
	LOGFONT		m_lf;
	COLORREF	m_rgbText;
	COLORREF	m_rgbBackground;
	CBrush *	m_pBrush;
	BOOL		m_bBold;
	int			m_nXMargin, m_nYMargin;
	HICON		m_hIcon;
	CSize		m_extent;
	UINT		m_nJustify;

	CToolTipCtrl m_tooltip;
	bool m_tooltipCreated = false;

	CFont * GetSafeFont();

	// Generated message map functions
protected:
	//{{AFX_MSG(CXColorStatic)
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

}
/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif //XCOLORSTATIC_H
