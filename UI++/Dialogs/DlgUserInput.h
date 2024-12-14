#pragma once

#include "DlgBase.h"
#include "RegExEdit.h"
#include "afxbutton.h"
#include "afxwin.h"
#include "UserInputBase.h"
#include "UserInputChoiceOptions.h"

#include <vector>

// CInputDlg dialog

constexpr auto AD_ERROR= 0x00;
constexpr auto AD_WARNING = 0x10;
constexpr auto AD_COMPUTER = 0x00;
constexpr auto AD_USER = 0x01;

class CDlgUserInput : public CDlgBase
{
	DECLARE_DYNAMIC(CDlgUserInput)

public:
//	CInputDlg(CUIDlg::ShowButtons showBtns, CWnd* pParent = NULL);   // standard constructor
	CDlgUserInput(PCTSTR dlgTitle, 
		const DialogVisibilityFlags flags,
		const UIpp::ActionData& data, 
		UINT id, 
		CWnd* pParent = NULL);   // standard constructor
	
	CDlgUserInput(PCTSTR dlgTitle, 
		const DialogVisibilityFlags flags,
		const UIpp::ActionData& data, 
		CDlgBase::DlgSize size, 
		CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgUserInput();

// Dialog Data
	enum { IDD = IDD_INPUTDLG };
	enum { IDDTALL = IDD_INPUTDLGTALL };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	//CStatic m_scrollBox;

	void Dummy(void) {};
	void CreateInputFont(void);
	CWnd* CreateInputs(bool& warnings);
	int ShowInputs(void);
	void ScrollInputs(int scollPos);

	std::vector<UIpp::CUserInputBase*> m_allInputs;

	int m_itemCount = 0;
	int m_maxScollPos = 0;
	const int MAXITEMS = 6;

	bool m_isCreated;
	bool m_allValid = true;
	UINT m_inputID; 
	CFont* m_pInputFont;

	CString m_ADValidateVariable;
	byte m_adValidateType = 0;

	afx_msg void OnBnClickedNextaction();
	afx_msg LRESULT OnInputChange(WPARAM wParam, LPARAM);
	afx_msg void OnComboChange(UINT nID);

	DECLARE_MESSAGE_MAP()

	CScrollBar m_inputScroll;

public:
	virtual BOOL OnInitDialog();

	bool AddTextInput(PCTSTR questionText, PCTSTR variable, PCTSTR fontFace, PCTSTR hint = nullptr, PCTSTR prompt = nullptr, PCTSTR regex = nullptr, bool isRequired = true, PCTSTR defaultInput = nullptr, bool isPassword = false, PCTSTR forceCase = nullptr, bool hScroll = false, bool isReadOnly = false);
	bool AddComboInput(PCTSTR questionText, PCTSTR variable, PCTSTR fontFace, PCTSTR altVariable, const UIpp::CUserInputChoiceOptions& choices, int choiceListSize, bool choiceSort, PCTSTR defaultOption = nullptr, bool required = false, bool autoComplete = false);
	bool AddCheckboxInput(PCTSTR questionText, PCTSTR variable, PCTSTR fontFace, PCTSTR checkVal, PCTSTR uncheckVal, PCTSTR defaultVal = _T(""));
	bool AddInfo(PCTSTR infoText, PCTSTR fontFace, int numLines = 1, COLORREF textColor = RGB(0, 0, 0));
	//bool AddBrowseInput(PCTSTR questionText, PCTSTR variable);

	void SetFieldToADValidate(CString field, CString type)
	{
		m_ADValidateVariable = field;

		if (_tcsicmp(type, VALUE_USER) == 0
			|| _tcsicmp(type, VALUE_USERWARNING) == 0)
		{
			m_adValidateType |= AD_USER;
		}

		if (_tcsicmp(type, VALUE_COMPUTERWARNING) == 0
			|| _tcsicmp(type, VALUE_USERWARNING) == 0)
		{
			m_adValidateType |= AD_WARNING;
		}

		TRACE1("AD Validate Type: %d\n", m_adValidateType);
	}

	UIpp::CUserInputBase::StatusType CheckADObject(PCTSTR pObjectName, byte type);
	
//	void SetText(PCTSTR dlgTitle, PCTSTR sidebarTitle, PCTSTR sidebarSubtitle);

	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
};
