#include "stdafx.h"
#include "UserInputChoice.h"

namespace UIpp {

	bool CUserInputChoice::Create(CRect rect, CWnd* pParentWnd, UINT& uID, LONG lineHeight, LONG spacing, CFont* pFont)
	{
		ASSERT(pParentWnd);

		//m_lineHeight = lineHeight;
		//rect.bottom += m_lineHeight;

		CString defaultString;

		CUserInputBase::Create(rect, pParentWnd, uID, lineHeight, spacing, pFont);

		//	m_pCombo = new CComboBox ();
		m_pCombo = new CodeProject::CComboCompletion();

		DWORD style = /*WS_VISIBLE |*/ WS_CHILD | WS_TABSTOP | WS_VSCROLL;// | CBS_OWNERDRAWFIXED | CBS_HASSTRINGS;// | CBS_SORT;

		if (!m_autoComplete)
			style |= CBS_DROPDOWNLIST;
		else
			style |= CBS_DROPDOWN;

		rect.MoveToY(rect.top + m_lineHeight);
		rect.bottom += m_lineHeight;

		m_pCombo->Create(style, rect, pParentWnd, ++uID);

		m_pCombo->SetMinVisibleItems(m_comboDropBoxSize);

		if (m_pFontx && m_pFontx->m_hObject)
			m_pCombo->SetFont(m_pFontx);

		if (!m_sortChoices)
		{
			for (COptionValueListIterator i = m_choices.Begin(); i != m_choices.End(); i++)
			{
				//m_pCombo->InsertString(-1, *i);
				m_pCombo->InsertItem({ *i, L"", L"", 0 });

				if (m_default.GetLength() > 0 && (m_default == *i || m_default == m_choices.Find(*i, 1) || m_default == m_choices.Find(*i, 2)))
					defaultString = *i;
			}
		}
		else
		{
			for (COptionValueMapIterator i = m_choices.BeginSorted(); i != m_choices.EndSorted(); i++)
			{
				//m_pCombo->InsertString(-1, i->first);
				m_pCombo->InsertItem({ i->first, L"", L"", 0 });

				if (m_default.GetLength() > 0 && (m_default == i->second.value1 || m_default == i->second.value2 || m_default == i->first))
					defaultString = i->first;
			}
		}

		if (defaultString.GetLength() > 0)
		{
			m_pCombo->SelectString(-1, defaultString);
		}

		return true;
	}

	bool CUserInputChoice::IsInputValid()
	{
		bool valid = false;

		if (m_pCombo)
		{
			int currentSelection = m_pCombo->GetCurSel();

			CString str;
			m_pCombo->GetWindowText(str);

			if (currentSelection != CB_ERR
				|| (currentSelection == CB_ERR && str.GetLength() > 0 && m_pCombo->FindStringExact(-1, str) != CB_ERR)
				|| (m_noSelectionIsValid && str.GetLength() == 0))
				valid = true;

			//if (!m_noSelectionValid && currentSelection == CB_ERR)
			//	valid = false;

			CUserInputBase::SetStatus(valid ? StatusType::NotChecked : StatusType::Attention);
		}

		return valid;
	}

	PCTSTR CUserInputChoice::GetValue(bool alternate)
	{
		CString temp;
		m_value = _T("");

		if (m_pCombo)
		{
			if (m_pCombo->GetCurSel() != CB_ERR)
				m_pCombo->GetLBText(m_pCombo->GetCurSel(), temp);
			else
				m_pCombo->GetWindowText(temp);

			if (temp.GetLength() > 0)
			{
				m_value = m_choices.Find(temp, alternate ? 2 : 1);

				//				if (!alternate)
				//					m_value = m_choices.find(temp)->second.value1;
				//				else
				//					m_value = m_choices.find(temp)->second.value2;
			}
		}

		return m_value;
	}

}