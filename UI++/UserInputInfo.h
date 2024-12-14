#pragma once

#include "UserInputBase.h"

namespace UIpp {

	class CUserInputInfo :
		public CUserInputBase
	{
	public:
		CUserInputInfo(PCTSTR text, PCTSTR fontFace, int numLines = 1, COLORREF textColor = RGB(0, 0, 0))
			: CUserInputBase(text, fontFace, numLines)
		{
			m_textColor = textColor;
		};

		~CUserInputInfo(void)
		{

		}

		bool Create(CRect rect, CWnd* pParentWnd, UINT& uID, LONG lineHeight, LONG spacing, CFont* pFont = 0);

		void ShowInput(bool show = true)
		{
			CUserInputBase::ShowInput(show);
		}

		int MoveItem(int y, int h)
		{
			y = CUserInputBase::MoveInput(y, h);
			return y;
		}

		virtual bool IsInputValid()
		{
			return true;
		}

		InputType GetType(void) { return CUserInputBase::InputType::InputInfo; };

		const int GetInputHeight(void) {
			return m_lineHeight * m_numberofTextLines;
		};
	};
}