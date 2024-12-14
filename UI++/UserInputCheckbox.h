#pragma once

#include "UserInputBase.h"

namespace UIpp {

	class CUserInputCheckbox :
		public CUserInputBase
	{
	public:
		CUserInputCheckbox(PCTSTR question, PCTSTR valueName, PCTSTR fontFace, PCTSTR checkedVal, PCTSTR uncheckVal, PCTSTR defaultVal)
			: CUserInputBase(question, valueName, fontFace, nullptr, 1),
			m_checkedValue(checkedVal),
			m_uncheckedValue(uncheckVal),
			m_defaultValue(defaultVal),
			m_pCheckBox(nullptr)
		{};

		~CUserInputCheckbox(void)
		{
			if (m_pCheckBox != nullptr)
				delete m_pCheckBox;
		}

		bool Create(CRect rect, CWnd* pParentWnd, UINT& uID, LONG lineHeight, LONG spacing = 0, CFont* pFont = 0);

		void ShowInput(bool show = true)
		{
			CUserInputBase::ShowInput(show);
			m_pCheckBox->ShowWindow(show ? SW_SHOW : SW_HIDE);
		}

		int MoveItem(int y, int h)
		{
			y = CUserInputBase::MoveInput(y, h);
			return y;
		}

		void Disable(bool disable)
		{
			if (m_pCheckBox != nullptr)
				m_pCheckBox->EnableWindow(!disable);
		}

		bool IsInputValid()
		{
			return true;
		}

		PCTSTR GetValue(bool alternate);

		void ClearInput(void)
		{
			//m_pEdit->SetWindowText(L"");
		};

		InputType GetType(void) { return CUserInputBase::InputType::CheckboxInput; };

	protected:
		CString m_checkedValue;
		CString m_uncheckedValue;
		CString m_defaultValue;

		CString m_value;

		CButton* m_pCheckBox;
	};

}