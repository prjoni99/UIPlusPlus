// UserAuthDlg.cpp : implementation file
//

#include "stdafx.h"
#include "UI++.h"
#include "DlgPreflight.h"
#include "UserInfoCheck.h"
#include "afxdialogex.h"
#include "TSvar.h"

//#include "activeds.h"

// CPreflightDlg dialog

IMPLEMENT_DYNAMIC(CDlgPreflight, CDlgBase)

CDlgPreflight::CDlgPreflight(PCTSTR dlgTitle, 
	const DialogVisibilityFlags flags,
	const UIpp::ActionData& data, 
	CDlgBase::DlgSize size, 
	int maxRetries,
	CWnd* pParent)   // standard constructor
: CDlgUserInput(dlgTitle, flags, data, size, pParent), 
	m_checkWarnings(false)//, m_allChecksPassed(false)
{

}

CDlgPreflight::~CDlgPreflight()
{
}

void CDlgPreflight::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	CDlgBase::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_REFRESH, m_refresh);
}


BEGIN_MESSAGE_MAP(CDlgPreflight, CDlgUserInput)
	ON_BN_CLICKED(IDC_REFRESH, OnBnClickedRetryaction)
	//ON_REGISTERED_MESSAGE(FTW::CRegExEdit::UWM_VALUE_CHANGED, OnInputChange)
	//ON_CONTROL_RANGE(CBN_SELCHANGE, IDC_INPUTCONTROLS, IDC_INPUTCONTROLS + 10, &CDlgUserInput::OnComboChange)
END_MESSAGE_MAP()


// CUserAuthDlg message handlers

BOOL CDlgPreflight::OnInitDialog()
{
	CDlgUserInput::OnInitDialog();

	if (!m_allValid)
	{
		//m_allChecksPassed = false;
		
		SETMESSAGE(IDS_MSGPREFLIGHTFAILED, nullptr, CDlgBase::MsgType::Error);
		
		if (!FTW::CheckForFlag(m_visibilityFlags, SHOW_CANCEL))
		{
			m_next.SetIcon(IDI_BUTTONCANCEL, IDI_BUTTONCANCELGREY, HansDietrich::CXButtonXP::LEFT).EnableTheming(TRUE).SetDrawToolbar(TRUE);
			m_next.EnableWindow(true);
			m_toInfo.SetAction(VALUE_CANCEL);
		}
	}
	else
	{
		//m_allChecksPassed = true;
		
		if (!m_checkWarnings)
			SETMESSAGE(IDS_MSGPREFLIGHTPASSED, nullptr, CDlgBase::MsgType::Info);
		else
			SETMESSAGE(IDS_MSGPREFLIGHTPASSEDWITHWARNING, nullptr, CDlgBase::MsgType::Warning);
	}

	if (FTW::CheckForFlag(m_visibilityFlags, CDlgBase::SHOW_REFRESH) && (!m_allValid || m_checkWarnings))
	{
		m_refresh.SetIcon(IDI_BUTTONREFRESH, IDI_BUTTONREFRESHGREY, HansDietrich::CXButtonXP::LEFT).SetDrawToolbar(TRUE);
		m_refresh.ShowWindow(SW_SHOW);
		m_buttonQueue.push_front(&m_refresh);
	}

	m_moveButtons = true;
	MoveButtons();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgPreflight::MoveButtons()
{
	if (m_moveButtons)
		CDlgBase::MoveButtons();
}

void CDlgPreflight::OnBnClickedNextaction()
{
	//UIpp::CUserInput* pPasswordUI;

	if (m_allValid)
		EndDialog(ERROR_SUCCESS);
	else
		EndDialog(ERROR_NOT_READY);
}

void CDlgPreflight::OnBnClickedRetryaction()
{
	EndDialog(UI_RETRY);
}

bool CDlgPreflight::AddCheck(PCTSTR infoText, PCTSTR fontFace, UIpp::CUserInputBase::StatusType chkStat, PCTSTR descriptionText, PCTSTR statusTooltipText)
{
	UIpp::CUserInputBase* ui = new UIpp::CUserInfoCheck(infoText, fontFace, chkStat, descriptionText, statusTooltipText);

	//m_allPrompts.AddTail(ui);
	m_allInputs.push_back(ui);

	return true;
}

