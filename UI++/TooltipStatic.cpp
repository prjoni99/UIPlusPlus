// TooltipStatic.cpp : implementation file
//

#include "stdafx.h"
#include "UI++.h"
#include "TooltipStatic.h"


// CTooltipStatic
namespace FTW
{
	IMPLEMENT_DYNAMIC(CTooltipStatic, CStatic)

		CTooltipStatic::CTooltipStatic()
	{

	}

	CTooltipStatic::~CTooltipStatic()
	{
	}


	BEGIN_MESSAGE_MAP(CTooltipStatic, CStatic)
	END_MESSAGE_MAP()



	// CTooltipStatic message handlers




	void CTooltipStatic::PreSubclassWindow()
	{
		DWORD dwStyle = GetStyle();
		SetWindowLong(GetSafeHwnd(), GWL_STYLE, dwStyle | SS_NOTIFY);

		CStatic::PreSubclassWindow();
	}

	void CTooltipStatic::CreateTooltip(PCTSTR text)
	{
		if (text != nullptr && _tcslen(text) > 0)
		{
			m_tooltip.Create(this, TTS_ALWAYSTIP | TTS_BALLOON | TTS_NOPREFIX);
			CRect oRect;
			GetClientRect(&oRect);
			m_tooltip.AddTool(this, text, oRect, 1001);
			m_tooltip.ShowWindow(SW_HIDE);
			m_tooltipCreated = true;
		}
	}


	BOOL CTooltipStatic::PreTranslateMessage(MSG* pMsg)
	{
		if(m_tooltipCreated)
			m_tooltip.RelayEvent(pMsg);

		return CStatic::PreTranslateMessage(pMsg);
	}
}
