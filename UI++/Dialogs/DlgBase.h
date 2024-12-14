#pragma once

#include "DlgData.h"
#include "Actions\IAction.h"
#include "CodeProject\KCSideBannerWnd\KCSideBannerWnd.h"
#include "HansDietrich\XButtonXP.h"
#include "HansDietrich\XColorStatic.h"
#include "HansDietrich\XNamedColors.h"
#include "CLdap.h"
#include "FTWCMLog.h"
#include "Constants.h"
#include "resource.h"
#include <deque>

#define WM_NCUAHDRAWCAPTION 174
#define WM_NCUAHDRAWFRAME 175

#define BANNER_OFFSET 10
#define ICON_OFFSET 10
#define ICON_SIZE 48

#define BUTTONAREA_HEIGHT 66

#define BUTTONOFFSET 40

#define RESID_TO_STRING(Variable) (void(Variable),L#Variable)

constexpr UINT UI_BACK = 35;
constexpr UINT UI_RETRY = 37;

// CUIDlg Parent dialog Class

class CDlgBase : public CDialog
{

DECLARE_DYNAMIC(CDlgBase)

public:
	typedef std::deque<HansDietrich::CXButtonXP*> ButtonQueue;

	class CDlgTimeoutInfo
	{
	public:
		CDlgTimeoutInfo(int to = 0, PCTSTR pMsg = _T(""), PCTSTR pAction = VALUE_CONTINUE) :
			m_timeout(to), m_countdown(to), m_msg(pMsg), m_action(pAction)
		{
		}

		CDlgTimeoutInfo(const CDlgTimeoutInfo& ti) :
			m_timeout(ti.m_timeout), m_countdown(ti.m_countdown), m_msg(ti.m_msg), m_action(ti.m_action)
		{
		}

		inline int GetCountdown(bool decrement = true) { return (decrement ? --m_countdown : m_countdown); }
		inline bool CountdownElapsed() { return m_countdown == 0; }
		inline void SetAction(PCTSTR pNewAction) { m_action = pNewAction; }
		inline bool AllowPreempt(void) { return m_action != VALUE_CONTINUENOPREMPT; }
		inline const CString& GetAction() { return m_action; }
		inline bool CountdownLessThanThreshold(int threshold) { return ((m_countdown * 100 / m_timeout) < threshold); }
		inline void CancelTimer() { m_countdown = 0; }
		inline DWORD GetActionCode()
		{
			if (m_action == VALUE_CANCEL)
				return ERROR_CANCELLED;
			else if (m_action == VALUE_CONTINUE)
				return ERROR_SUCCESS;
			else if (m_action == VALUE_CONTINUEONWARNING)
				return ERROR_SUCCESS;
			else
			{
				PTSTR pStop;

				return (DWORD) _tcstol(m_action, &pStop, 10);
			}
		}
	private:
		int m_countdown;
		int m_timeout;
		CString m_msg;
		CString m_action;
	}; 
	
	enum class MsgType { Info = 1, Warning, Error };
	enum class DlgSize { Regular = 1, Tall, ExtraTall };

	typedef short DialogVisibilityFlags;

	const static DialogVisibilityFlags SHOW_CANCEL = 1;
	const static DialogVisibilityFlags SHOW_BACK = 2;
	const static DialogVisibilityFlags SHOW_OK = 4;
	const static DialogVisibilityFlags SHOW_REFRESH = 8;
	const static DialogVisibilityFlags SHOW_TITLE_CENTER = 16;
	const static DialogVisibilityFlags SHOW_RESTART = 32;
	const static DialogVisibilityFlags NO_DEFAULT = 64;

	static DialogVisibilityFlags BuildDialogVisibilityFlags(
		bool cancel = false, 
		bool back = false, 
		bool ok = true, 
		bool centerTitle = false,
		bool noDefault = false)
	{
		return ((cancel ? SHOW_CANCEL : 0) |
				(back ? SHOW_BACK : 0) |
				(ok ? SHOW_OK : 0) |
				(centerTitle ? SHOW_TITLE_CENTER : 0) |
				(noDefault ? NO_DEFAULT : 0));
	}
	
	CDlgBase(PCTSTR dlgTitle, 
		UINT id,
		const DialogVisibilityFlags flags, 
		const UIpp::ActionData& data,
		CWnd* pParent);

	virtual ~CDlgBase()
	{
		m_font.DeleteObject();
	};

	virtual BOOL OnInitDialog();
	void SetTimeoutInfo(const CDlgTimeoutInfo& toInfo = CDlgBase::CDlgTimeoutInfo(0)) { m_toInfo = toInfo; };
	void ShowRestartButton(void) { m_visibilityFlags = m_visibilityFlags | SHOW_RESTART; };

	virtual afx_msg void OnBnClickedShowTSVarDLG();
	virtual afx_msg void OnBnClickedDumpTSVar();
	virtual afx_msg void OnBnClickedNextaction() { EndDialog(ERROR_SUCCESS); }
	virtual afx_msg void OnBnClickedCanceled() { EndDialog(ERROR_CANCELLED); }
	virtual afx_msg void OnBnClickedBack() { EndDialog(UI_BACK); }
	virtual afx_msg void OnTimer(UINT_PTR nIDEvent);

	virtual BOOL PreTranslateMessage(MSG* pMsg)
	{
		if (m_hAccelTable && ::TranslateAccelerator(m_hWnd, m_hAccelTable, pMsg)) 
			return(TRUE);

		//m_buttonToolTips.RelayEvent(pMsg);

		return CDialog::PreTranslateMessage(pMsg);
	};
protected:
	void DoDataExchange(CDataExchange* pDX);

	DECLARE_MESSAGE_MAP()

	void OnPaint();
	void OnNcPaint();
	void OnSize(UINT nType, int cx, int cy);
	BOOL OnEraseBkgnd(CDC * pDC);
	HBRUSH OnCtlColor(CDC * pDC, CWnd * pWnd, UINT nCtlColor);

	HRESULT LoadUIImage(const CString imagePath, CImage& image, DWORD successMsg, DWORD failMsg);

	afx_msg LRESULT OnNCUAHDrawCaption(WPARAM wParam, LPARAM lParam) { return 0; }
 	afx_msg LRESULT OnNCUAHDrawFrame(WPARAM wParam, LPARAM lParam) { return 0; }
	afx_msg BOOL OnNcActivate(BOOL bActive) { return TRUE; }
	afx_msg void OnIdcancel() {};
	afx_msg void OnIdok() { if (!FTW::CheckForFlag(m_visibilityFlags, NO_DEFAULT)) CDialog::OnOK(); };

	inline int ScaleX(int x) { return MulDiv(x, m_actionData.globalDialogTraits.screenScale.x, 96); };
	inline int ScaleY(int y) { return MulDiv(y, m_actionData.globalDialogTraits.screenScale.y, 96); };

	void MessageInit(void);

	virtual void MoveButtons();
	void MoveButton(int buttonPos, int buttonCount, HansDietrich::CXButtonXP& button);
	void HideBackButton(void) { m_visibilityFlags = m_visibilityFlags & ~SHOW_BACK; };

	void _SetMessage(PCTSTR msg, CDlgBase::MsgType type = CDlgBase::MsgType::Info)
	{
		m_userMsg.SetText(msg, TRUE);

		if (type == CDlgBase::MsgType::Warning)
			m_userMsg.SetTextColor(HansDietrich::colorDarkGoldenRod);
		else if (type == CDlgBase::MsgType::Error)
			m_userMsg.SetTextColor(HansDietrich::colorRed);
		else
			m_userMsg.SetTextColor(HansDietrich::colorBlack);

	}

#define SETMESSAGE(msgID, pMsg, type) _SetMessage(msgID, L#msgID, pMsg, type)

	void _SetMessage(DWORD msgID, PCTSTR pMsgID, PCTSTR pMsg = 0, CDlgBase::MsgType type = CDlgBase::MsgType::Info)
	{
		CString displayMsg;

		if (m_actionData.messages)
		{
			CString lookupId(pMsgID);
			lookupId.Replace(_T("IDS_MSG"), _T(""));

			displayMsg = m_actionData.messages.find_child_by_attribute(XML_ELEMENT_MESSAGE, XML_ATTRIBUTE_ID, lookupId).child_value();
		}

		if (displayMsg.IsEmpty())
		{
			if (pMsg && _tcslen(pMsg) > 0)
				displayMsg = FTW::FormatResourceString(msgID, pMsg).c_str();
			else
				displayMsg = FTW::FormatResourceString(msgID).c_str();
		}
			
		_SetMessage(displayMsg, type);
	}

	void UpdateTimerMessage()
	{
		m_timeoutMsg.SetText(FTW::FormatSeconds(m_toInfo.GetCountdown()).c_str());

		if(m_toInfo.CountdownLessThanThreshold(25))
			m_timeoutMsg.SetTextColor(HansDietrich::colorRed);
		else if (m_toInfo.CountdownLessThanThreshold(10))
			m_timeoutMsg.SetTextColor(HansDietrich::colorDarkGoldenRod);
	}
	void ClearTimer()
	{
		m_timeoutMsg.SetText(L"");
		KillTimer(m_timerID);
		m_toInfo.CancelTimer();
	}
	
	CodeProject::CKCSideBannerWnd m_banner;
	CString m_dlgTitleText;

	CFont m_font;
	CRect m_controlAreaRect;

	HansDietrich::CXColorStatic m_userMsg;
	HansDietrich::CXColorStatic m_dlgTitle;
	HansDietrich::CXColorStatic m_timeoutMsg;

	bool m_enableNext;
	HansDietrich::CXButtonXP m_next;
	HansDietrich::CXButtonXP m_cancel;
	HansDietrich::CXButtonXP m_back;
	ButtonQueue m_buttonQueue;
	CToolTipCtrl m_buttonToolTips;

	DialogVisibilityFlags m_visibilityFlags;
	const UIpp::ActionData& m_actionData;

	UINT_PTR m_timerID = 0;
	CDlgTimeoutInfo m_toInfo;

	HACCEL m_hAccelTable;

	CDlgBase::DlgSize m_dlgSize;

	virtual void Dummy(void) = 0;
};
