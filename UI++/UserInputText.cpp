#include "StdAfx.h"
#include "UserInputText.h"

namespace UIpp {

	bool CUserInputText::Create(CRect rect, CWnd* pParentWnd, UINT& uID, LONG lineHeight, LONG spacing, CFont* pFont)
	{
		ASSERT(pParentWnd);

		CUserInputBase::Create(rect, pParentWnd, uID, lineHeight, spacing, pFont);

		DWORD style = /*WS_VISIBLE |*/ WS_CHILD | WS_TABSTOP;

		if (m_hScroll)
			style |= ES_AUTOHSCROLL;

		if (m_forceCase == VALUE_UPPER)
			style |= ES_UPPERCASE;
		else if (m_forceCase == VALUE_LOWER)
			style |= ES_LOWERCASE;

		if (m_password)
			style |= ES_PASSWORD;

		rect.MoveToY(rect.top + m_lineHeight);
		rect.bottom += m_lineHeight; 
		
		m_pEdit = new FTW::CRegExEdit(m_regEx, true, !m_isRequired);
		m_pEdit->Create(style, rect, pParentWnd, ++uID);

		if (m_prompt.GetLength())
			m_pEdit->SetCueBanner(m_prompt);

		if (m_pFontx && m_pFontx->m_hObject)
			m_pEdit->SetFont(m_pFontx);

		m_pEdit->CreateToolTip(pParentWnd, m_hint, m_prompt);

		m_pEdit->SetWindowText(m_defaultInput);

		if (m_hScroll)
			m_pEdit->SetLimitText(256);

		if (m_isReadOnly)
			Disable();

		return true;
	}

	bool CUserInputText::IsInputValid()
	{
		bool valid = false;
		StatusType status = m_status;

		if (m_pEdit)
		{
			valid = m_pEdit->IsInputValid();

			if (!valid)
				status = StatusType::Attention;
			else if (m_status == StatusType::Attention)
				status = StatusType::NotChecked;

			CUserInputBase::SetStatus(status);
		}

		return valid && status != StatusType::Failed;
	}
}