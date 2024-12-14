#pragma once


// CTooltipStatic
namespace FTW
{
	class CTooltipStatic : public CStatic
	{
		DECLARE_DYNAMIC(CTooltipStatic)

	public:
		CTooltipStatic();
		virtual ~CTooltipStatic();

		void CreateTooltip(PCTSTR text);

	protected:
		DECLARE_MESSAGE_MAP()
		virtual void PreSubclassWindow();
		virtual BOOL PreTranslateMessage(MSG* pMsg);

		CToolTipCtrl m_tooltip;

		bool m_tooltipCreated = false;

	};
}


