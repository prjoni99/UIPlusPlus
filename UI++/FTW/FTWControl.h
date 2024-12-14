#pragma once

class CFTWControl
{
public:
	CFTWControl(CEdit* pEdit, bool emptyAllowed = false);
	~CFTWControl();

	bool IsInputValid(void) const { return m_validInput; }
	bool ValidateInput(void);

	static UINT UWM_VALUE_CHANGED;

protected:
	bool m_validInput;
	bool m_emptyAllowed;
	bool m_hasFocus;
	bool m_initialFocus;

	CBrush m_focusBrush;
	CBrush m_noFocusBrush;
	CBrush m_background;
	CBrush m_noFocusErrorBrush;

	CEdit* m_pEdit;

	//CFont* m_pFont;

	//CToolTipCtrl m_ToolTip;
	//CString m_toolTipText;
	//CString m_toolTipTitle;

	void DrawEditFrame();
	void DrawEditFrame(bool hasFocus);
	
	afx_msg void OnEnKillfocus();
	afx_msg void OnEnSetfocus();
	afx_msg void OnEnChange();
	afx_msg HBRUSH CtlColor(CDC* pDC, UINT /*nCtlColor*/);
};

