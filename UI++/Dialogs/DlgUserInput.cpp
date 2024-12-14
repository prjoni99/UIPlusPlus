// InputDlg.cpp : implementation file
//

#include "stdafx.h"
#include "UI++.h"
#include "DlgUserInput.h"
#include "afxdialogex.h"
#include "TSvar.h"
#include "UserInputText.h"
#include "UserInputCheckbox.h"
#include "UserInputChoice.h"
#include "UserInfoCheck.h"
#include "UserInputInfo.h"

constexpr auto LEFT_MARGIN = 25;
constexpr auto LEFT_SIDEBAR_MARGIN = 75;
constexpr auto RIGHT_MARGIN = 50;
constexpr auto INPUT_SPACING = 4;
constexpr auto INPUT_TOP = 60;
constexpr auto LINE_HEIGHT = 25;

// CInputDlg dialog

IMPLEMENT_DYNAMIC(CDlgUserInput, CDlgBase)

CDlgUserInput::CDlgUserInput(PCTSTR dlgTitle, 
	const DialogVisibilityFlags flags,
	const UIpp::ActionData& data, 
	UINT id, 
	CWnd* pParent /*=NULL*/)
	: CDlgBase(dlgTitle, id, flags, data, pParent)
	, m_inputID(IDC_INPUTCONTROLS)
	, m_isCreated(false)
{
	m_pInputFont = new CFont();
}

CDlgUserInput::CDlgUserInput(PCTSTR dlgTitle,
	const DialogVisibilityFlags flags,
	const UIpp::ActionData& data,
	CDlgBase::DlgSize size,
	CWnd* pParent)
	: CDlgBase(dlgTitle,
		((size == CDlgBase::DlgSize::Tall) ? CDlgUserInput::IDDTALL : CDlgUserInput::IDD),
		flags, data, pParent),
	m_inputID(IDC_INPUTCONTROLS), m_isCreated(false)
{
	m_pInputFont = new CFont();

	m_dlgSize = (size == CDlgBase::DlgSize::Tall ? CDlgBase::DlgSize::Tall : CDlgBase::DlgSize::Regular);
};

CDlgUserInput::~CDlgUserInput()
{
	for (auto pInput : m_allInputs)
	{
		delete pInput;
	}

	m_allInputs.clear();

	if( m_pInputFont->m_hObject )
		m_pInputFont->DeleteObject();

	if(m_pInputFont)
		delete m_pInputFont;
}

void CDlgUserInput::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	CDlgBase::DoDataExchange(pDX);
//	DDX_Control(pDX, IDC_SCROLLBAR1, m_inputScroll);
}


BEGIN_MESSAGE_MAP(CDlgUserInput, CDlgBase)
	ON_REGISTERED_MESSAGE(CFTWControl::UWM_VALUE_CHANGED, OnInputChange)
	ON_CONTROL_RANGE(CBN_SELCHANGE, IDC_INPUTCONTROLS, IDC_INPUTCONTROLS + 50, &CDlgUserInput::OnComboChange)
	ON_CONTROL_RANGE(CBN_EDITCHANGE, IDC_INPUTCONTROLS, IDC_INPUTCONTROLS + 50, &CDlgUserInput::OnComboChange)
//	ON_WM_VSCROLL()
END_MESSAGE_MAP()

// CInputDlg message handlers
BOOL CDlgUserInput::OnInitDialog()
{
	CDlgBase::OnInitDialog();

	m_allValid = true;
	bool anyWarnings = false;

	CreateInputFont();

	CWnd* pWndFocus = CreateInputs(anyWarnings);
	m_maxScollPos = ShowInputs();

	m_next.EnableWindow(m_allValid);

	if (!m_allValid || (anyWarnings && m_toInfo.GetAction() != VALUE_CONTINUEONWARNING))
		ClearTimer();

	if (pWndFocus != nullptr)
		GotoDlgCtrl(pWndFocus);
		//SendMessage(WM_NEXTDLGCTL, (WPARAM)wndFocus, TRUE);

	m_isCreated = true;
	m_itemCount = m_allInputs.size();

	//if (m_maxScollPos > 0)
	//{
	//	m_inputScroll.SetScrollRange(0, m_maxScollPos, false);
	//	m_inputScroll.SetScrollPos(0);
	//}

	//m_inputScroll.ShowWindow(SW_HIDE);

	return FALSE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

bool CDlgUserInput::AddTextInput(PCTSTR questionText, PCTSTR variable, PCTSTR fontFace, PCTSTR hintText, PCTSTR promptText, PCTSTR regex, bool isRequired, PCTSTR defaultInput, bool isPassword, PCTSTR forceCase, bool hScroll, bool isReadOnly)
{
	UIpp::CUserInputBase* ui = new UIpp::CUserInputText(questionText, variable, fontFace, hintText, promptText, regex, isRequired, defaultInput, isPassword, forceCase, hScroll, isReadOnly);

	//m_allPrompts.AddTail(ui);
	m_allInputs.push_back(ui);

	return true;
}

bool CDlgUserInput::AddCheckboxInput(PCTSTR questionText, PCTSTR variable, PCTSTR fontFace, PCTSTR checkVal, PCTSTR uncheckVal, PCTSTR defaultVal)
{
	UIpp::CUserInputBase* ui = new UIpp::CUserInputCheckbox(questionText, variable, fontFace, checkVal, uncheckVal, defaultVal);

	//m_allPrompts.AddTail(ui);
	m_allInputs.push_back(ui);
	
	return true;
}

bool CDlgUserInput::AddComboInput(PCTSTR questionText, PCTSTR variable, PCTSTR fontFace, PCTSTR altVariable, const UIpp::CUserInputChoiceOptions& choices, int choiceListSize, bool choiceSort = true, PCTSTR defaultOption, bool required, bool autoComplete)
{
	UIpp::CUserInputBase* ui = new UIpp::CUserInputChoice(questionText, variable, fontFace, altVariable, choices, choiceListSize, choiceSort, defaultOption, required, autoComplete);

	//m_allPrompts.AddTail(ui);
	m_allInputs.push_back(ui);

	return true;
}

bool CDlgUserInput::AddInfo(PCTSTR infoText, PCTSTR fontFace, int numLines, COLORREF textColor)
{
	UIpp::CUserInputBase* ui = new UIpp::CUserInputInfo(infoText, fontFace, numLines, textColor);

	//m_allPrompts.AddTail(ui);
	m_allInputs.push_back(ui);

	return true;
}

//bool CDlgUserInput::AddBrowseInput(PCTSTR questionText, PCTSTR variable)
//{
//	//UIpp::CUserInputBase* ui = new UIpp::CUserInputBrowse(questionText, variable);
//
//	//m_allPrompts.AddTail(ui);
//
//	//return true;
//}

LRESULT CDlgUserInput::OnInputChange(WPARAM wParam, LPARAM)
{
	if (!m_isCreated)
		return 0;
	
	m_allValid = true;

	for (auto pInput : m_allInputs)
	{
		ASSERT(pInput);

		if (pInput)
		{
			if (wParam && ((CWnd*)wParam == pInput->GetControl()))
			{
				pInput->SetStatus(UIpp::CUserInputBase::StatusType::NotChecked);
			}

			m_allValid &= pInput->IsInputValid();
		}
	}
	
	m_next.EnableWindow(m_allValid);

	_SetMessage(_T(""));

	return 0;
}

void CDlgUserInput::OnComboChange(UINT nID)
{
	OnInputChange(0,0);
}

void CDlgUserInput::OnBnClickedNextaction()
{
	//POSITION p = m_addedPrompts.GetHeadPosition();
	//UIpp::CUserInputBase* pui = nullptr;

	if (m_actionData.pLdap->IsValid() && !m_ADValidateVariable.IsEmpty())
	{
		BeginWaitCursor();

		for(auto pInput : m_allInputs)
		{
			ASSERT(pInput);

			if (m_ADValidateVariable == pInput->GetValueName() &&
				(pInput->GetStatus() == UIpp::CUserInputBase::StatusType::NotChecked || pInput->GetStatus() == UIpp::CUserInputBase::StatusType::Failed))
			{
				UIpp::CUserInputBase::StatusType newStatus = CheckADObject(pInput->GetValue(), m_adValidateType);
				pInput->SetStatus(newStatus);
				
				if (newStatus == UIpp::CUserInputBase::StatusType::Warning || newStatus == UIpp::CUserInputBase::StatusType::Failed)
				{
					EndWaitCursor();
					return;
				}
			}
		}

		EndWaitCursor();

	}

	for(auto pInput : m_allInputs)
	{
		ASSERT(pInput);

		if (_tcsnlen(pInput->GetValueName(), MAX_STRING_LENGTH) > 0)
			CTSEnv::Instance().Set(m_actionData.pCMLog, pInput->GetValueName(), pInput->GetValue(), !pInput->HideValue());
		
		if (pInput->GetType() == UIpp::CUserInputBase::InputType::ComboInput && _tcsnlen(pInput->GetValueName(true), MAX_STRING_LENGTH) > 0)
			CTSEnv::Instance().Set(m_actionData.pCMLog, pInput->GetValueName(true), pInput->GetValue(true));

	}

	EndDialog(0);
}

UIpp::CUserInputBase::StatusType CDlgUserInput::CheckADObject(PCTSTR pObjectName, byte type)
{
	CString objectName = pObjectName;
	CString dn;
	UIpp::CUserInputBase::StatusType status = UIpp::CUserInputBase::StatusType::NotChecked;

	if((type & AD_USER) != AD_USER)
		objectName.Append(_T("$"));

	ULONG result = m_actionData.pLdap->GetAttributeValueFromObject(_T("distinguishedName"), 
		objectName, 
		(type & AD_USER) == AD_USER ? FTWLDAP::ILdap::LDAPObjectCategory::Person : FTWLDAP::ILdap::LDAPObjectCategory::Computer,
		dn);

	if (result == ERROR_SUCCESS)
	{
		FTWCMLOG::CCMLog::MsgType logMsgType = FTWCMLOG::CCMLog::MsgType::Info;
		DWORD logMsgId = 0;

		if (dn.IsEmpty())
		{
			logMsgId = IDS_LOGMSG_AD_OBJECT_NOTFOUND;

			if ((type & AD_USER) == AD_USER)
			{
				if ((type & AD_WARNING) == AD_WARNING)
				{
					SETMESSAGE(IDS_MSGUSERDOESNOTEXISTWARNING, nullptr, CDlgBase::MsgType::Warning);
					logMsgType = FTWCMLOG::CCMLog::MsgType::Warning;
					status = UIpp::CUserInputBase::StatusType::Warning;
				}
				else
				{
					SETMESSAGE(IDS_MSGUSERDOESNOTEXIST, nullptr, CDlgBase::MsgType::Error);
					logMsgType = FTWCMLOG::CCMLog::MsgType::Error;
					status = UIpp::CUserInputBase::StatusType::Failed;
				}
			}
			else
			{
				logMsgType = FTWCMLOG::CCMLog::MsgType::Info;
				status = UIpp::CUserInputBase::StatusType::Success;
			}
		}
		else
		{
			logMsgId = IDS_LOGMSG_AD_OBJECT_FOUND;

			if ((type & AD_USER) == AD_USER)
			{
				logMsgType = FTWCMLOG::CCMLog::MsgType::Info;
				status = UIpp::CUserInputBase::StatusType::Success;
			}
			else
			{
				if ((type & AD_WARNING) == AD_WARNING)
				{
					SETMESSAGE(IDS_MSGCOMPUTEREXISTSWARNING, nullptr, CDlgBase::MsgType::Warning);
					logMsgType = FTWCMLOG::CCMLog::MsgType::Warning;
					status = UIpp::CUserInputBase::StatusType::Warning;
				}
				else
				{
					SETMESSAGE(IDS_MSGCOMPUTEREXISTS, nullptr, CDlgBase::MsgType::Error);
					logMsgType = FTWCMLOG::CCMLog::MsgType::Error;
					status = UIpp::CUserInputBase::StatusType::Failed;
				}
			}
		}

		m_actionData.pCMLog->WriteMsg(logMsgType,
			AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
			FTW::FormatResourceString(logMsgId, objectName, (type & AD_USER) == AD_USER ? VALUE_USER : VALUE_COMPUTER));

		if(logMsgType == FTWCMLOG::CCMLog::MsgType::Error)
			m_next.EnableWindow(false);

		//if (dn.IsEmpty())
		//{
		//	m_actionData.pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info,
		//		AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
		//		FTW::FormatResourceString(IDS_LOGMSG_AD_OBJECT_NOTFOUND, objectName, (type & AD_USER) == AD_USER ? VALUE_USER : VALUE_COMPUTER));

		//	status = UIpp::CUserInputBase::StatusType::Success;
		//}
		//else
		//{
		//	m_actionData.pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Warning,
		//		AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
		//		FTW::FormatResourceString(IDS_LOGMSG_AD_OBJECT_FOUND, objectName, (type & AD_USER) == AD_USER ? VALUE_USER : VALUE_COMPUTER));

		//	if ((type & AD_WARNING) == AD_WARNING)
		//	{
		//		SETMESSAGE(IDS_MSGCOMPUTEREXISTSWARNING, nullptr, CDlgBase::MsgType::Warning);
		//		status = UIpp::CUserInputBase::StatusType::Warning;
		//	}
		//	else
		//	{
		//		SETMESSAGE(IDS_MSGCOMPUTEREXISTS, nullptr, CDlgBase::MsgType::Warning);
		//		status = UIpp::CUserInputBase::StatusType::Failed;

		//		m_next.EnableWindow(false);
		//	}
		//}
	}
	else
	{
		m_actionData.pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Error,
			AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
			FTW::FormatResourceString(IDS_LOGERROR_CHECKING_AD_FOR_OBJECT, objectName, ldap_err2string(result), (type & AD_USER) == AD_USER ? VALUE_USER : VALUE_COMPUTER));
	}

	return status;
}

void CDlgUserInput::CreateInputFont()
{
	if( m_pInputFont->m_hObject )
		m_pInputFont->DeleteObject();

	LOGFONT  lgFont;

	_tcsncpy_s(lgFont.lfFaceName, sizeof(lgFont.lfFaceName)/sizeof(TCHAR)-1, m_actionData.globalDialogTraits.fontFace, _TRUNCATE);
	
	lgFont.lfHeight = 120;
	
	lgFont.lfCharSet = DEFAULT_CHARSET;
	lgFont.lfClipPrecision = 0;
	lgFont.lfEscapement = 0;
	lgFont.lfItalic = FALSE;
	lgFont.lfOrientation = 0;
	lgFont.lfOutPrecision = OUT_DEFAULT_PRECIS;
	lgFont.lfPitchAndFamily = DEFAULT_PITCH;
	lgFont.lfQuality = DEFAULT_QUALITY;
	lgFont.lfStrikeOut = FALSE;
	lgFont.lfUnderline = FALSE;
	lgFont.lfWidth = 0;
	lgFont.lfWeight = FW_NORMAL;

	m_pInputFont->CreatePointFontIndirect( &lgFont );
	//m_pInputFont->SetFont( m_pFontx );
}

CWnd* CDlgUserInput::CreateInputs(bool& warnings)
{
	CWnd* pWndFocus = nullptr;
	int inputY = ScaleY(INPUT_TOP);

	CRect clientRect;
	GetClientRect(&clientRect);
	int leftPromptBorder = clientRect.left + ScaleX(FTW::CheckForFlag(m_actionData.globalDialogTraits.flags, FTW::DialogTraits::SHOW_SIDEBAR) ? LEFT_SIDEBAR_MARGIN : LEFT_MARGIN);
	int rightPromptBorder = clientRect.right - ScaleX(RIGHT_MARGIN);
	int inputSpacing = ScaleY(INPUT_SPACING);
	int inputLineHeight = ScaleY(LINE_HEIGHT);

	for(auto pInput: m_allInputs)
	{
		ASSERT(pInput);

		CRect r(leftPromptBorder, inputY, rightPromptBorder, inputY);

		TRACE1("Y: %d\n", inputY);

		pInput->Create(r, this, m_inputID, inputLineHeight, inputSpacing, m_pInputFont);

		TRACE1("H: %d\n", pInput->GetInputHeight());

		if (pInput->ErrorState() != 0)
			m_actionData.pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Warning,
				AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
				FTW::FormatResourceString(IDS_LOGERROR_USERITEMCREATE, pInput->GetIdentifier(), pInput->GetErrorMessage()));

		if (pWndFocus == nullptr &&
			(pInput->GetType() == UIpp::CUserInputBase::InputType::TextInput
				|| pInput->GetType() == UIpp::CUserInputBase::InputType::CheckboxInput
				|| pInput->GetType() == UIpp::CUserInputBase::InputType::ComboInput))
			pWndFocus = GetDlgItem(m_inputID);

		m_inputID++;

		m_allValid &= pInput->IsInputValid();
		if (pInput->IsInputStatusWarning())
			warnings = true;

		inputY += (pInput->GetInputHeight() + inputSpacing);
	}

	return pWndFocus;
}

int CDlgUserInput::ShowInputs()
{
	CRect clientRect;
	GetClientRect(&clientRect);
	int inputY = ScaleY(INPUT_TOP);
	int inputSpacing = ScaleY(INPUT_SPACING);
	int inputBottom = clientRect.bottom - ScaleY(BUTTONAREA_HEIGHT);
	int notShownCount = 0;

	for (auto pInput : m_allInputs)
	{
		ASSERT(pInput);

		inputY += pInput->GetInputHeight();

		if (inputY < inputBottom)
		{
			pInput->ShowInput(true);
		}
		else
		{
			pInput->ShowInput(false);
			notShownCount++;
		}

		inputY += inputSpacing;

	}

	//RedrawWindow(m_controlAreaRect, 0, RDW_INVALIDATE | RDW_ERASE);

	return notShownCount;
}

void CDlgUserInput::ScrollInputs(int scrollPos)
{
	int inputCount = 0;
	CRect clientRect;
	GetClientRect(&clientRect);
	int inputY = ScaleY(INPUT_TOP);
	int inputSpacing = ScaleY(INPUT_SPACING);
	int inputBottom = clientRect.bottom - ScaleY(BUTTONAREA_HEIGHT);

	//HDWP hWindows = BeginDeferWindowPos(10);

	for (auto pInput : m_allInputs)
	{
		ASSERT(pInput);

		if (inputCount < scrollPos)
		{
			pInput->ShowInput(false);
		}
		else
		{
			inputY += pInput->GetInputHeight();

			if (inputY < inputBottom)
			{
				pInput->ShowInput(true);
				//pPrompt->MoveItem();
			}
			else
			{
				pInput->ShowInput(false);
			}

			inputY += inputSpacing;
		}

		inputCount++;
	}
}


void CDlgUserInput::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// TODO: Add your message handler code here and/or call default

	int currentPos = m_inputScroll.GetScrollPos();

	// Determine the new position of scroll box.
	switch (nSBCode)
	{
	case SB_TOP:
		currentPos = 0;
		break;

	case SB_BOTTOM:
		currentPos = m_inputScroll.GetScrollLimit();
		break;

	case SB_ENDSCROLL:   // End scroll.
		break;

	case SB_LINEUP:
		if (currentPos > 0)
			currentPos--;
		break;

	case SB_LINEDOWN:
		if (currentPos < m_inputScroll.GetScrollLimit())
			currentPos++;
		break;

	case SB_PAGEUP:
	{
		// Get the page size. 
		SCROLLINFO   info;
		m_inputScroll.GetScrollInfo(&info, SIF_ALL);

		if (currentPos > 0)
			currentPos = max(0, currentPos - (int)info.nPage);
	}
	break;

	case SB_PAGEDOWN:      // Scroll one page right
	{
		// Get the page size. 
		SCROLLINFO   info;
		m_inputScroll.GetScrollInfo(&info, SIF_ALL);

		if (currentPos < m_maxScollPos)
			currentPos = min(m_maxScollPos, currentPos + (int)info.nPage);
	}
	break;

	case SB_THUMBPOSITION: // Scroll to absolute position. nPos is the position
		currentPos = nPos;      // of the scroll box at the end of the drag operation.
		break;

	case SB_THUMBTRACK:   // Drag scroll box to specified position. nPos is the
		currentPos = nPos;     // position that the scroll box has been dragged to.
		break;
	}

	// Set the new position of the thumb (scroll box).
	m_inputScroll.SetScrollPos(currentPos);
	ScrollInputs(currentPos);

	//CDlgBase::OnVScroll(nSBCode, nPos, pScrollBar);
}
