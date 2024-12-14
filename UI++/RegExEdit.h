#pragma once

#include <regex>
#include "FTW\FTWControl.h"

// CRegExEdit

namespace FTW
{

	class CRegExEdit : public CEdit, public CFTWControl
	{
		DECLARE_DYNAMIC(CRegExEdit)

	public:
		//CRegExEdit();
		CRegExEdit(LPCTSTR regEx = NULL, bool caseSensitive = true, bool emptyAllowed = false);
		virtual ~CRegExEdit();

	protected:
		DECLARE_MESSAGE_MAP()

	public:
		afx_msg void OnEnChange();
		afx_msg void OnEnKillfocus();
		afx_msg void OnEnSetfocus();
		afx_msg HBRUSH CtlColor(CDC* pDC, UINT /*nCtlColor*/);

		bool ValidateInput(void);
		void SetRegEx(LPCTSTR regEx, bool caseSensitive = true, bool emptyAllowed = false);
		void SetEditFont(LPCTSTR fontFace, LONG fontHeight = 0);

		void CreateToolTip(CWnd *pParent, LPCTSTR pszText, LPCTSTR pTitle);
		void SetToolTipText(LPCTSTR pszText);
		void SetToolTipTitle(const int iIconType, LPCTSTR pszTitle);
		void DisplayToolTip(const bool bDisplay, const int iIconType = TTI_ERROR);

		const CString GetErrorMessage(void) const {
			return m_creationErrorMessage;
		};

		const int ErrorState(void) const { return m_creationErrorCode; }

	protected:
		//bool m_validInput;
		std::wregex m_regex;
		int m_creationErrorCode;
		CString m_creationErrorMessage;

		//bool m_emptyAllowed;
		//bool m_hasFocus;
		//bool m_initialFocus;

		//CBrush m_focusBrush;
		//CBrush m_noFocusBrush;
		//CBrush m_background;
		//CBrush m_noFocusErrorBrush;

		CFont* m_pFont;

		CToolTipCtrl m_ToolTip;
		CString m_toolTipText;
		CString m_toolTipTitle;

		//void DrawEditFrame(void);


	};
}