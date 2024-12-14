#include "StdAfx.h"
#include "UserInputInfo.h"

namespace UIpp {
	bool CUserInputInfo::Create(CRect rect, CWnd* pParentWnd, UINT& uID, LONG lineHeight, LONG spacing, CFont* pFont)
	{
		ASSERT(pParentWnd);

		m_lineHeight = lineHeight;
		rect.bottom += (m_lineHeight * m_numberofTextLines);

		m_pFontx = pFont;
		CreateTextControl(rect, pParentWnd);

		return true;
	}

}