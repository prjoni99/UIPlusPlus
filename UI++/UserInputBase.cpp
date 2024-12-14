#pragma once

#include "StdAfx.h"
#include "resource.h"
#include "UserInputBase.h"

namespace UIpp {

bool CUserInputBase::Create(CRect rect, CWnd* pParentWnd, UINT& uID, LONG lineHeight, LONG spacing, CFont* pFont)
{
	ASSERT(pParentWnd);

	m_lineHeight = lineHeight;
	rect.bottom += m_lineHeight;
	
	CreateTextControl(rect, pParentWnd);

	if (m_numberofTextLines == 2)
	{
		rect.MoveToY(rect.top + m_lineHeight);
	}

	CRect statusRect = GetStatusRect(rect, spacing);
	VERIFY(m_statusIndicator.Create(_T(""), WS_CHILD | SS_ICON | SS_NOTIFY, statusRect, pParentWnd, ++uID));
	m_hasStatusIndicator = true;

	m_pFontx = pFont;

	return true;
}

CUserInputBase::StatusType CUserInputBase::SetStatus(StatusType newStatus)
{
	m_status = newStatus;
	UINT icon = NULL;

	if (m_status == StatusType::Attention)
		icon = IDI_INPUTATTENTION;
	else if (m_status == StatusType::Warning)
		icon = IDI_STATUSWARN;
	else if (m_status == StatusType::Failed)
		icon = IDI_STATUSFAILED;
	else if (m_status == StatusType::Success)
		icon = IDI_STATUSPASSED;

	if (m_status != StatusType::NotChecked && icon != NULL)
	{
		m_statusIcon = (HICON)LoadImage(AfxGetInstanceHandle(), MAKEINTRESOURCE(icon), IMAGE_ICON, 24, 24, LR_SHARED);
		m_statusIndicator.SetIcon(m_statusIcon);

		m_statusIndicator.ShowWindow(m_inputVisible ? SW_SHOW : SW_HIDE);
	}
	else
		m_statusIndicator.ShowWindow(SW_HIDE);

	return m_status;
}

void CUserInputBase::CreateTextControl(CRect r, CWnd* pW, PCTSTR tooltipText)
{
	int fontSize = 12;

	if (tooltipText != nullptr && _tcslen(tooltipText) > 0)
	{
		CRect r2(r);

		r2.right = r2.left + 16;
		r2.top += 4;
		r2.bottom = r2.top + 16;

		VERIFY(m_textInfo.Create(_T(""), WS_CHILD | SS_ICON | SS_NOTIFY /*| WS_VISIBLE*/, r2, pW, 15178));
		m_textInfoIcon = (HICON)LoadImage(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_INFOTIP), IMAGE_ICON, 16, 16, LR_SHARED);
		m_textInfo.SetIcon(m_textInfoIcon);
		m_textInfo.CreateTooltip(tooltipText);
		m_hasInfoIconTootip = true;

		r.left += 24;
	}

	m_textControl.Create(_T(""), /*WS_VISIBLE |*/ WS_CHILD | SS_LEFT, r, pW);
	m_textControl.SetBackgroundColor(RGB(255, 255, 255));

	//if (_tcslen(m_text) > (size_t)(m_numberofLines * 65))
	//	fontSize = 6;
	//else if (_tcslen(m_text) > (size_t)(m_numberofLines * 55))
	//	fontSize = 8;
	//else if (_tcslen(m_text) > (size_t)(m_numberofLines * 45))
	//	fontSize = 10;

	m_textControl.SetFont(m_fontFaceName, fontSize, FALSE);
	m_textControl.SetText(m_text, TRUE);
	m_textControl.SetTextColor(m_textColor);

}

void CUserInputBase::ShowInput(bool show)
{
	m_inputVisible = show;

	if(m_hasInfoIconTootip)
		m_textInfo.ShowWindow(show ? SW_SHOW : SW_HIDE);

	m_textControl.ShowWindow(show ? SW_SHOW : SW_HIDE);
	
	if(m_hasStatusIndicator)
		m_statusIndicator.ShowWindow(show && m_status != StatusType::NotChecked ? SW_SHOW : SW_HIDE);
}

int CUserInputBase::MoveInput(int y, int h)
{
	CRect itemPos;

	if (m_hasInfoIconTootip)
	{
		m_textInfo.GetWindowRect(&itemPos);
		itemPos.MoveToY(y + 4);
		m_textInfo.MoveWindow(itemPos);
	}

	m_textControl.GetWindowRect(&itemPos);
	itemPos.MoveToY(y);
	m_textControl.MoveWindow(itemPos);

	if (m_numberofTextLines == 2)
	{
		y += h;
	}

	if (m_hasStatusIndicator)
	{
		m_statusIndicator.GetWindowRect(&itemPos);
		itemPos.MoveToY(y);
		m_statusIndicator.MoveWindow(itemPos);
	}

	return y;
}


//bool CUserInputBrowse::Create(CRect& rect, CWnd* pParentWnd, UINT& uID, LONG height, LONG spacing, CFont* pFont)
//{
//	ASSERT(pParentWnd);
//
//	CString defaultString;
//
//	CUserInputBase::Create(rect, pParentWnd, uID, height, spacing);
//	m_pFontx = pFont;
//
//	m_pBrowseEdit = new CFTWBrowseEdit();
//	ASSERT(m_pBrowseEdit);
//
//	DWORD style = WS_VISIBLE | WS_CHILD | WS_TABSTOP;
//
//	bool b = m_pBrowseEdit->Create(style, rect, pParentWnd, ++uID);
//
//	m_pBrowseEdit->EnableBrowseButton();
//
//	if (m_pFontx && m_pFontx->m_hObject)
//		m_pBrowseEdit->SetFont(m_pFontx);
//
//	return true;
//}
}