#pragma once

#include "UserInputBase.h"
#include "UserInputChoiceOptions.h"

namespace UIpp {

	class CUserInputChoice :
		public CUserInputBase
	{
	public:
		CUserInputChoice(PCTSTR question, PCTSTR valueName, PCTSTR fontFace, PCTSTR altValueName, const CUserInputChoiceOptions& choices,
			int comboDropBoxSize, bool sortChoices = true, PCTSTR defaultOption = nullptr, bool required = false, bool autoComplete = false)
			: CUserInputBase(question, valueName, fontFace, altValueName),
			m_choices(choices),
			m_default(defaultOption),
			m_noSelectionIsValid(false),
			m_autoComplete(autoComplete),
			m_sortChoices(sortChoices),
			m_comboDropBoxSize(comboDropBoxSize),
			m_pCombo(nullptr)
		{
			if ((!defaultOption || _tcslen(defaultOption) == 0))
			{
				m_noSelectionIsValid = !required;
			}

		};

		~CUserInputChoice(void)
		{
			if (m_pCombo)
				delete m_pCombo;
		}

		bool Create(CRect rect, CWnd* pParentWnd, UINT& uID, LONG lineHeight, LONG spacing = 0, CFont* pFont = 0);

		void ShowInput(bool show = true)
		{
			CUserInputBase::ShowInput(show);

			m_pCombo->ShowWindow(show ? SW_SHOW : SW_HIDE);
		}

		int MoveItem(int y, int h)
		{
			y = CUserInputBase::MoveInput(y, h);
			return y;
		}

		void Disable(bool disable = true)
		{
			if (m_pCombo)
				m_pCombo->EnableWindow(!disable);
		}

		bool IsInputValid();

		PCTSTR GetValue(bool alternate = false);

		InputType GetType(void) { return CUserInputBase::InputType::ComboInput; };

		const int GetInputHeight(void) {
			return 2 * m_lineHeight;
		};

	protected:
		CString m_value;
		CString m_default;
		bool m_noSelectionIsValid;
		bool m_autoComplete;
		bool m_sortChoices;
		int m_comboDropBoxSize;

		CodeProject::CComboCompletion* m_pCombo;

		CUserInputChoiceOptions m_choices;
	};
}