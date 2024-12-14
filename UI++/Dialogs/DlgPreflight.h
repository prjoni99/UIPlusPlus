#pragma once

#include "DlgUserInput.h"
#include "winldap.h"

// CUserAuthDlg dialog

class CDlgPreflight : public CDlgUserInput
{
	DECLARE_DYNAMIC(CDlgPreflight)

public:
	//CDlgPreflight(short flags, int maxRetries = 0, CWnd* pParent = NULL);   // standard constructor
	CDlgPreflight(PCTSTR dlgTitle, 
		const DialogVisibilityFlags flags,
		const UIpp::ActionData& data, 
		CDlgBase::DlgSize size, 
		int maxRetries = 0,
		CWnd* pParent = NULL);   // standard constructor

	virtual ~CDlgPreflight();

// Dialog Data
	enum { IDD = IDD_INPUTDLG };
	enum { IDDTALL = IDD_INPUTDLGTALL};

	virtual BOOL OnInitDialog();
	
	void ChecksWithWarnings(bool warnings) { m_checkWarnings = warnings; };
	bool AddCheck(PCTSTR infoText, PCTSTR fontFace, UIpp::CUserInputBase::StatusType chkStat, PCTSTR descriptionText = nullptr, PCTSTR statusTooltipText = nullptr);
	void ShowRefreshButton(bool show) { if (show) m_visibilityFlags |= SHOW_REFRESH; else m_visibilityFlags = m_visibilityFlags & ~SHOW_REFRESH; };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	afx_msg void OnBnClickedRetryaction();
	afx_msg void OnBnClickedNextaction();

	DECLARE_MESSAGE_MAP()

	void Dummy(void) {};

	void MoveButtons();

	HansDietrich::CXButtonXP m_refresh;
	int m_refreshButtonPos = -1;
	//bool m_allChecksPassed;
	bool m_moveButtons = false;
	bool m_checkWarnings;
};
