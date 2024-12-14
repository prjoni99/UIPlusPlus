#include "StdAfx.h"
#include "UserInfoCheck.h"

namespace UIpp {
	bool CUserInfoCheck::Create(CRect rect, CWnd* pParentWnd, UINT& uID, LONG lineHeight, LONG spacing, CFont* pFont)
	{
		ASSERT(pParentWnd);

		CUserInputBase::Create(rect, pParentWnd, uID, lineHeight, spacing, pFont);

		SetStatus(m_status);
		m_statusIndicator.CreateTooltip(m_statusTooltipText);

		return true;
	}
}