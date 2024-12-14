#pragma once

#include "UserInputInfo.h"

namespace UIpp {
	class CUserInfoCheck :
		public CUserInputInfo
	{
	public:

		CUserInfoCheck(PCTSTR text, PCTSTR fontFace, CUserInputBase::StatusType stat = CUserInputBase::StatusType::Failed, PCTSTR descriptionText = nullptr, PCTSTR statusTootipText = nullptr)
			: CUserInputInfo(text, fontFace), m_descriptionText(descriptionText), m_statusTooltipText(statusTootipText)
		{
			m_status = stat;
		};

		~CUserInfoCheck(void)
		{

		}

		bool Create(CRect rect, CWnd* pParentWnd, UINT& uID, LONG lineHeight, LONG spacing = 0, CFont* pFont = 0);

		void ShowInput(bool show = true)
		{
			m_inputVisible = show;
			CUserInputBase::ShowInput(show);
		}

		int MoveItem(int y, int h)
		{
			y = CUserInputBase::MoveInput(y, h);
			return y;
		}

		virtual bool IsInputValid()
		{
			return m_status != CUserInputBase::StatusType::Failed;
		}

		virtual bool IsInputStatusWarning()
		{
			return m_status == CUserInputBase::StatusType::Warning;
		}

		InputType GetType(void) { return CUserInputBase::InputType::CheckInfo; };

	protected:
		CString m_descriptionText;
		CString m_statusTooltipText;
	};
}