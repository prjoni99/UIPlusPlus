#pragma once

#include "UserInputBase.h"

namespace UIpp {

	class CUserInputText :
		public CUserInputBase
	{
	public:
		CUserInputText(PCTSTR question, PCTSTR valueName, PCTSTR fontFace, PCTSTR hint = nullptr, PCTSTR prompt = nullptr,
			PCTSTR regEx = 0, bool isRequired = true, PCTSTR defaultInput = nullptr,
			bool isPassword = false, PCTSTR forceCase = nullptr, bool hScroll = false, bool isReadOnly = false)
			: CUserInputBase(question, valueName, fontFace),
			m_prompt(prompt),
			m_hint(hint),
			m_regEx(regEx),
			m_defaultInput(defaultInput),
			m_pEdit(nullptr),
			m_password(isPassword),
			m_forceCase(forceCase),
			m_isRequired(isRequired),
			m_hScroll(hScroll),
			m_isReadOnly(isReadOnly)
		{
			if (m_isReadOnly)
				m_isRequired = false;
		};

		~CUserInputText(void)
		{
			if (m_pEdit)
				delete m_pEdit;
		}

		bool Create(CRect rect, CWnd* pParentWnd, UINT& uID, LONG lineHeight, LONG spacing = 0, CFont* pFont = 0);

		void ShowInput(bool show = true)
		{
			CUserInputBase::ShowInput(show);

			m_pEdit->ShowWindow(show ? SW_SHOW : SW_HIDE);
		}

		int MoveItem(int y, int h)
		{
			y = CUserInputBase::MoveInput(y, h);

			CRect editPos;
			m_pEdit->GetWindowRect(&editPos);
			editPos.MoveToY(y);

			y += h;

			return y;
		}

		void Disable(bool disable = true)
		{
			if (m_pEdit)
				m_pEdit->EnableWindow(!disable);
		}

		bool IsInputValid();

		PCTSTR GetValue(bool alternate = false)
		{
			if (m_pEdit)
			{
				m_pEdit->GetWindowText(m_value);
				return (PCTSTR)m_value;
			}
			else return _T("");
		}

		void ClearInput(void)
		{
			m_pEdit->SetWindowText(L"");
			m_status = StatusType::NotChecked;
		};

		virtual const CString GetIdentifier(void) const {
			return m_regEx;
		};

		virtual const CWnd* GetControl(void) const { return m_pEdit; };

		const CString GetErrorMessage(void) const
		{
			if (m_pEdit)
			{
				CString error;

				return m_pEdit->GetErrorMessage();
			}

			return _T("");
		};

		const int ErrorState(void) const
		{
			if (m_pEdit)
				return m_pEdit->ErrorState();

			return 0;
		};

		InputType GetType(void) { return CUserInputBase::InputType::TextInput; };

		const int GetInputHeight(void) {
			return 2 * m_lineHeight;
		};

		bool HideValue(void) { return m_password; }

	protected:
		CString m_prompt;
		CString m_hint;
		CString m_regEx;
		CString m_defaultInput;
		CString m_forceCase;
		FTW::CRegExEdit* m_pEdit;

		CString m_value;

		bool m_password;
		bool m_isRequired;
		bool m_isReadOnly;
		bool m_hScroll;
	};

}