#include "stdafx.h"
#include "FTWControl.h"
#include "Constants.h"

UINT CFTWControl::UWM_VALUE_CHANGED = ::RegisterWindowMessage(_T("UWM_VALUE_CHANGED-{BEEB966C-71A6-44BB-88AE-AA84F8968B90}"));

CFTWControl::CFTWControl(CEdit* pEdit, bool emptyAllowed)
	: m_pEdit(pEdit)
	, m_validInput(emptyAllowed)
	, m_emptyAllowed(emptyAllowed)
	, m_hasFocus(false)
	, m_initialFocus(true)
{
	m_focusBrush.CreateSolidBrush(MBLUE);
	m_noFocusBrush.CreateSolidBrush(GetSysColor(COLOR_INACTIVECAPTION));
	m_background.CreateSolidBrush(RGB(255, 255, 255));
	m_noFocusErrorBrush.CreateSolidBrush(MRED);
}


CFTWControl::~CFTWControl()
{
}

void CFTWControl::DrawEditFrame()
{
	ASSERT(m_pEdit);

	CRect clientRect;

	if (m_pEdit)
	{
		CDC* pDC = m_pEdit->GetDC();
		m_pEdit->GetClientRect(&clientRect);

		clientRect.InflateRect(1, 1);

		if (pDC)
		{
			if (m_hasFocus)
				pDC->FrameRect(clientRect, &m_focusBrush);
			else if (m_validInput)
				pDC->FrameRect(clientRect, &m_noFocusBrush);
			else
				pDC->FrameRect(clientRect, &m_noFocusErrorBrush);
		}
	}
}

void CFTWControl::DrawEditFrame(bool hasFocus)
{
	m_hasFocus = hasFocus;
	DrawEditFrame();
}

void CFTWControl::OnEnKillfocus()
{
	DrawEditFrame(false);
}


void CFTWControl::OnEnSetfocus()
{
	DrawEditFrame(true);
}


HBRUSH CFTWControl::CtlColor(CDC* pDC, UINT /*nCtlColor*/)
{
	DrawEditFrame();
	return NULL;
}

void CFTWControl::OnEnChange()
{
	if (ValidateInput())
	{
		m_pEdit->InvalidateRect(NULL);
		//DisplayToolTip(!m_validInput);
	}

	m_pEdit->GetParent()->SendMessage(CFTWControl::UWM_VALUE_CHANGED, (WPARAM)(CWnd*)m_pEdit);
}

bool CFTWControl::ValidateInput(void)
{
	CString input;
	
	bool validityChange = false;

	m_pEdit->GetWindowText(input);

	bool isValid = (input.GetLength() > 0 || m_emptyAllowed);

	//if (input.GetLength() == 0)
	//	isValid = m_emptyAllowed;

	validityChange = isValid != m_validInput;

	m_validInput = isValid;

	return validityChange;
}