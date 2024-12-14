// RegExEdit.cpp : implementation file
//

#include "stdafx.h"
#include "UI++.h"
#include "RegExEdit.h"
#include "HansDietrich\FontSize.h"
//#include "Constants.h"

namespace FTW
{

	// CRegExEdit

	IMPLEMENT_DYNAMIC(CRegExEdit, CEdit)

		//CRegExEdit::CRegExEdit()
		//: m_validInput(true)
		//, m_emptyAllowed(false)
		//, m_hasFocus(false)
		//, m_pFont(NULL)
		//{
		//	m_regex.assign(_T("\\w*"));
		//	m_focusBrush.CreateSolidBrush(GetSysColor(COLOR_WINDOWTEXT));
		//	m_noFocusBrush.CreateSolidBrush(GetSysColor(COLOR_INACTIVECAPTION));
		//	m_background.CreateSolidBrush(RGB(255,255,255));
		//}

		CRegExEdit::CRegExEdit(LPCTSTR regEx, bool caseSensitive, bool emptyAllowed)
		: m_creationErrorMessage(_T(""))
		, m_creationErrorCode(0)
		, CFTWControl(this, emptyAllowed)
	{
		if (regEx && lstrlen(regEx) > 0)
			SetRegEx(regEx, caseSensitive, emptyAllowed);
		else
			SetRegEx(_T(".*"), caseSensitive, emptyAllowed);

		m_pFont = new CFont();
	};


	CRegExEdit::~CRegExEdit()
	{
		if (m_pFont && m_pFont->m_hObject)
			m_pFont->DeleteObject();

		if (m_pFont)
			delete m_pFont;

	}


	BEGIN_MESSAGE_MAP(CRegExEdit, CEdit)
		ON_CONTROL_REFLECT(EN_CHANGE, &CRegExEdit::OnEnChange)
		ON_CONTROL_REFLECT(EN_KILLFOCUS, &CRegExEdit::OnEnKillfocus)
		ON_CONTROL_REFLECT(EN_SETFOCUS, &CRegExEdit::OnEnSetfocus)
		ON_WM_CTLCOLOR_REFLECT()
	END_MESSAGE_MAP()

	// CRegExEdit message handlers

	void CRegExEdit::OnEnChange()
	{
		if (ValidateInput())
		{
			InvalidateRect(NULL);
			DisplayToolTip(!m_validInput);
		}

		GetParent()->SendMessage(CFTWControl::UWM_VALUE_CHANGED, (WPARAM)(CWnd*)this);
	}

	bool CRegExEdit::ValidateInput(void)
	{
		CString input;
		bool isValid = false;
		bool validityChange = false;

		GetWindowText(input);

		if (input.GetLength() == 0)
			isValid = m_emptyAllowed;
		else
			isValid = std::regex_match(input.GetString(), m_regex);

		validityChange = isValid != m_validInput;

		m_validInput = isValid;

		return validityChange;
	}

	void CRegExEdit::SetRegEx(LPCTSTR regEx, bool caseSensitive, bool emptyAllowed)
	{
		ASSERT(regEx);

		m_emptyAllowed = emptyAllowed;

		if (regEx)
		{
			try
			{
				if (!caseSensitive)
					m_regex.assign(regEx, std::regex_constants::icase);
				else
					m_regex.assign(regEx);
			}
			catch (std::regex_error e)
			{
				m_regex.assign(_T(".*"), std::regex_constants::icase);

				m_creationErrorMessage = CA2T(e.what());
				m_creationErrorCode = e.code();
			}
		}

	};

	void CRegExEdit::OnEnSetfocus()
	{
		DrawEditFrame(true);

		if (!m_validInput && !m_initialFocus)
			DisplayToolTip(!m_validInput);

		else if (m_initialFocus)
		{
			m_initialFocus = false;
			DisplayToolTip(true, TTI_INFO);
		}
	}

	void CRegExEdit::OnEnKillfocus()
	{
		DrawEditFrame(false);
		DisplayToolTip(false);
	}

	//void CRegExEdit::DrawEditFrame()
	//{
	//	CRect clientRect;
	//	CDC* pDC = this->GetDC();

	//	this->GetClientRect(&clientRect);

	//	clientRect.InflateRect(1, 1);

	//	if (m_hasFocus)
	//		pDC->FrameRect(clientRect, &m_focusBrush);
	//	else if (m_validInput)
	//		pDC->FrameRect(clientRect, &m_noFocusBrush);
	//	else
	//		pDC->FrameRect(clientRect, &m_noFocusErrorBrush);

	//}

	HBRUSH CRegExEdit::CtlColor(CDC* pDC, UINT /*nCtlColor*/)
	{
		DrawEditFrame();

		if (!m_validInput)
		{
			pDC->SetTextColor(RGB(255, 0, 0));
		}
		else
		{
			pDC->SetTextColor(GetSysColor(COLOR_WINDOWTEXT));
		}

		// TODO:  Return a non-NULL brush if the parent's handler should not be called
		return (HBRUSH)m_background.GetSafeHandle();
	}

	void CRegExEdit::SetEditFont(LPCTSTR fontFace, LONG fontHeight)
	{

		if (m_pFont->m_hObject)
			m_pFont->DeleteObject();

		LOGFONT  lgFont;

		_tcsncpy_s(lgFont.lfFaceName, sizeof(lgFont.lfFaceName) / sizeof(TCHAR) - 1, fontFace, _TRUNCATE);

		lgFont.lfHeight = fontHeight;

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

		m_pFont->CreatePointFontIndirect(&lgFont);
		SetFont(m_pFont);

	}

	void CRegExEdit::CreateToolTip(CWnd *pParent, LPCTSTR pText, LPCTSTR pTitle)
	{
		DWORD dwStyle = WS_POPUP | TTS_BALLOON | TTS_NOPREFIX;

		m_ToolTip.Create(pParent, dwStyle);
		m_ToolTip.AddTool(this, (LPTSTR)pText);

		m_toolTipText = pText;
		m_toolTipTitle = pTitle;
	}

	void CRegExEdit::SetToolTipText(LPCTSTR pszText)
	{
		m_ToolTip.UpdateTipText(pszText, this);
	}

	void CRegExEdit::SetToolTipTitle(const int iIconType, LPCTSTR pszTitle)
	{
		m_ToolTip.SetTitle(iIconType, pszTitle);
	}

	void CRegExEdit::DisplayToolTip(const bool bDisplay, const int iIconType /*= TTI_ERROR*/)
	{
		if (m_ToolTip.m_hWnd)
		{
			if (bDisplay)
				ShowBalloonTip(m_toolTipTitle, m_toolTipText, iIconType);
			else
				HideBalloonTip();
		}
	}
}