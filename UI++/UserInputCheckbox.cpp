#include "stdafx.h"
#include "UserInputCheckbox.h"

namespace UIpp {

	bool CUserInputCheckbox::Create(CRect rect, CWnd* pParentWnd, UINT& uID, LONG lineHeight, LONG spacing, CFont* pFont)
	{
		ASSERT(pParentWnd);

		m_lineHeight = lineHeight;
		rect.bottom += m_lineHeight;

		DWORD style = /*WS_VISIBLE |*/ WS_CHILD | WS_TABSTOP | BS_AUTOCHECKBOX;
		m_pFontx = pFont;

		CRect checkboxRect(rect);
		checkboxRect.right = checkboxRect.left + 13;
		checkboxRect.top = checkboxRect.top + 5;
		checkboxRect.bottom = checkboxRect.bottom - 6;

		m_pCheckBox = new CButton();
		m_pCheckBox->Create(_T(""), style, checkboxRect, pParentWnd, ++uID);

		if (m_defaultValue == m_checkedValue)
			m_pCheckBox->SetCheck(BST_CHECKED);

		rect.left += 25;

		CreateTextControl(rect, pParentWnd);

		return true;
	}

	PCTSTR CUserInputCheckbox::GetValue(bool alternate)
	{
		if (m_pCheckBox)
		{
			if (m_pCheckBox->GetCheck() == BST_CHECKED)
				return (PCTSTR)m_checkedValue;
			else if (m_pCheckBox->GetCheck() == BST_UNCHECKED)
				return (PCTSTR)m_uncheckedValue;
		}

		return _T("");
	}

}