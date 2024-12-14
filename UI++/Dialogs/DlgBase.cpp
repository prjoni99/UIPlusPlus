// UIDlg.cpp : implementation file
//

#include "stdafx.h"
#include "UI++.h"
#include "DlgBase.h"
#include "afxdialogex.h"
#include "DlgTSVar.h"
#include "TSVar.h"
#include "FTW\FTWHTTP.h"
#include "Constants.h"
#include <filesystem>

// CUIDlg dialog

#define TIMERID 4181972

IMPLEMENT_DYNAMIC(CDlgBase, CDialog)

CDlgBase::CDlgBase(PCTSTR dlgTitle, 
	UINT id,
	const DialogVisibilityFlags flags, 
	const UIpp::ActionData& data,
	CWnd* pParent)
	: CDialog(id, pParent), 
	m_dlgTitleText(dlgTitle),
	m_visibilityFlags(flags),
	m_actionData(data),
	m_enableNext(false),
	m_dlgSize(CDlgBase::DlgSize::Regular), 
	m_dlgTitle(FTW::CheckForFlag(flags, SHOW_TITLE_CENTER) ? SS_CENTER : 0)
{
	m_hAccelTable = LoadAccelerators(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_ACCELERATOR_DIALOGS));
};

void CDlgBase::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TITLE, m_dlgTitle);
	DDX_Control(pDX, IDC_NEXT, m_next);
	DDX_Control(pDX, IDC_CANCEL, m_cancel);
	DDX_Control(pDX, IDC_BACK, m_back);
	DDX_Control(pDX, IDC_USERMSG, m_userMsg);
	DDX_Control(pDX, IDC_TIMEOUT, m_timeoutMsg);
}

BEGIN_MESSAGE_MAP(CDlgBase, CDialog)
	ON_BN_CLICKED(IDC_NEXT, OnBnClickedNextaction)
	ON_BN_CLICKED(IDC_CANCEL, OnBnClickedCanceled)
	ON_BN_CLICKED(IDC_BACK, OnBnClickedBack)
	ON_BN_CLICKED(ID_SHOWTSVARDLG, OnBnClickedShowTSVarDLG)
	ON_BN_CLICKED(ID_DUMPTSVAR, OnBnClickedDumpTSVar)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_NCACTIVATE()
	ON_WM_NCPAINT()
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_WM_CTLCOLOR()
	ON_MESSAGE(WM_NCUAHDRAWCAPTION, OnNCUAHDrawCaption)
    ON_MESSAGE(WM_NCUAHDRAWFRAME, OnNCUAHDrawFrame)
	ON_COMMAND(IDCANCEL, OnIdcancel)
	ON_COMMAND(IDOK, OnIdok)
	ON_WM_TIMER()
END_MESSAGE_MAP()

BOOL CDlgBase::OnInitDialog()
{
	CDialog::OnInitDialog();

	if (!FTW::CheckForFlag(m_actionData.globalDialogTraits.flags, FTW::DialogTraits::ALLOW_BACK))
		HideBackButton();

	if(FTW::CheckForFlag(m_actionData.globalDialogTraits.flags, FTW::DialogTraits::ALWAYS_ON_TOP))
		SetWindowPos(&CWnd::wndTopMost, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE);
	else
		SetWindowPos(&CWnd::wndNoTopMost, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE);
	
	if (FTW::CheckForFlag(m_actionData.globalDialogTraits.flags, FTW::DialogTraits::SHOW_SIDEBAR))
	{
		m_banner.Attach(this, KCSB_ATTACH_LEFT);

		m_banner.SetFillFlag(KCSB_FILL_FLAT);

		m_banner.SetColBkg(m_actionData.globalDialogTraits.accentColor);
		m_banner.SetColEdge(m_actionData.globalDialogTraits.accentColor);

		m_banner.SetEdgeOffset(CSize(0, BANNER_OFFSET));

		m_banner.SetSize(ScaleX(m_banner.GetSize()));

		if (m_font.CreatePointFont(240, m_actionData.globalDialogTraits.fontFace) != 0)
			m_banner.SetTitleFont(&m_font);

		else if (m_font.CreatePointFont(240, DEFAULT_FONTFACE) != 0)
			m_banner.SetTitleFont(&m_font);

		m_banner.SetColTxtTitle(m_actionData.globalDialogTraits.textColor);

		m_banner.SetTitle(m_actionData.globalDialogTraits.title);
	}

	m_dlgTitle.SetFont(m_actionData.globalDialogTraits.fontFace, 24, FALSE);
	m_dlgTitle.SetBold(TRUE, FALSE);
	m_dlgTitle.SetBackgroundColor(RGB(255, 255, 255));

	//#ifdef _DEBUG
	//		m_banner.SetCaption(m_sidebarSubtitleText);
	//		m_dlgTitleText.Insert(0, _T("BETA - "));
	//#endif

	m_dlgTitle.SetText(m_dlgTitleText, TRUE);

	MessageInit();

	if (m_actionData.globalDialogTraits.hIcon)
		m_banner.SetIcon(m_actionData.globalDialogTraits.hIcon, KCSB_ICON_VCENTER | KCSB_ICON_LEFT);

	// Removed 2.11.0.0
	//if (m_dlgIconID && m_showIcons)
	//{
	//	CRect iconRect;

	//	GetWindowRect(&iconRect);

	//	iconRect.right = iconRect.right - ScaleX(ICON_OFFSET);
	//	iconRect.left = iconRect.right - ICON_SIZE;
	//	iconRect.top = iconRect.top + ScaleY(ICON_OFFSET);
	//	iconRect.bottom = iconRect.top + ICON_SIZE;

	//	VERIFY(m_dlgIcon.Create(_T(""), (WS_CHILD | SS_ICON) & ~WS_TABSTOP, iconRect, this, IDC_ICONX));

	//	HICON hIcon = (HICON)LoadImage(AfxGetInstanceHandle(), MAKEINTRESOURCE(m_dlgIconID), IMAGE_ICON, 48, 48, LR_SHARED);

	//	m_dlgIcon.SetIcon(hIcon);

	//	m_dlgIcon.ShowWindow(SW_SHOW);
	//}

	if ((HWND)m_next != 0 && FTW::CheckForFlag(m_visibilityFlags, SHOW_OK))
	{
		m_next.SetIcon(IDI_BUTTONOK, IDI_BUTTONOKGREY, HansDietrich::CXButtonXP::LEFT).SetDrawToolbar(TRUE);
		m_next.ShowWindow(SW_SHOW);
		m_buttonQueue.push_front(&m_next);
	}
	else
		m_next.ShowWindow(SW_HIDE);

	if ((HWND)m_cancel != 0 && FTW::CheckForFlag(m_visibilityFlags, SHOW_RESTART))
	{
		m_cancel.SetIcon(IDI_BUTTONRESTART, IDI_BUTTONRESTART, HansDietrich::CXButtonXP::LEFT).SetDrawToolbar(TRUE);
		m_cancel.ShowWindow(SW_SHOW);
		m_buttonQueue.push_front(&m_cancel);
	}
	else if ((HWND)m_cancel != 0 && FTW::CheckForFlag(m_visibilityFlags, SHOW_CANCEL))
	{
		m_cancel.SetIcon(IDI_BUTTONCANCEL, IDI_BUTTONCANCELGREY, HansDietrich::CXButtonXP::LEFT).SetDrawToolbar(TRUE);
		m_cancel.ShowWindow(SW_SHOW);
		m_buttonQueue.push_front(&m_cancel);
	}

	else if ((HWND)m_cancel != 0)
		m_cancel.ShowWindow(SW_HIDE);

	if ((HWND)m_back != 0 && FTW::CheckForFlag(m_visibilityFlags, SHOW_BACK))
	{
		m_back.SetIcon(IDI_BUTTONBACK, IDI_BUTTONBACKGREY, HansDietrich::CXButtonXP::LEFT).SetDrawToolbar(TRUE);
		m_back.ShowWindow(SW_SHOW);
		m_buttonQueue.push_front(&m_back);
	}
	else if ((HWND)m_back)
		m_back.ShowWindow(SW_HIDE);

	this->MoveButtons();

	if (!m_toInfo.CountdownElapsed())
	{
		m_timerID = SetTimer(TIMERID, 1000, NULL);

		UpdateTimerMessage();
	}

	//m_buttonToolTips.Create(this, TTS_ALWAYSTIP | TTS_BALLOON | TTS_NOPREFIX);
	//::SendMessage(m_buttonToolTips.m_hWnd, TTM_SETMAXTIPWIDTH, 0, SHRT_MAX);

	//m_buttonToolTips.AddTool(GetDlgItem(IDC_NEXT), _T("Next"));
	//m_buttonToolTips.AddTool(GetDlgItem(IDC_CANCEL), _T("Cancel"));
	//m_buttonToolTips.AddTool(GetDlgItem(IDC_BACK), _T("Back"));

	if (FTW::CheckForFlag(m_visibilityFlags, NO_DEFAULT))
	{
		SetDefID(0);
		//return FALSE;
	}

	return TRUE;

}

void CDlgBase::MoveButtons()
{
	if (!m_buttonQueue.empty())
	{
		int position = 0;
		CRect buttonLocation;

		m_buttonQueue.back()->GetWindowRect(&buttonLocation);
		buttonLocation.MoveToX((int)(buttonLocation.left - (BUTTONOFFSET * (m_buttonQueue.size() - 1))));

		std::for_each(m_buttonQueue.begin(), m_buttonQueue.end(),
			[&](HansDietrich::CXButtonXP* pButton) {
			pButton->MoveWindow(buttonLocation, TRUE);
			buttonLocation.MoveToX(buttonLocation.left + (BUTTONOFFSET * 2));
		});

	}
}

void CDlgBase::MoveButton(int buttonPos, int buttonCount, HansDietrich::CXButtonXP& button)
{
	CRect buttonLocation;
	button.GetWindowRect(&buttonLocation);
	int base = BUTTONOFFSET * (buttonCount - 1);
	int adjust = BUTTONOFFSET * 2 * buttonPos;

	buttonLocation.MoveToX(buttonLocation.left - base);
	buttonLocation.MoveToX(buttonLocation.left + adjust);

	button.MoveWindow(buttonLocation);
}

void CDlgBase::MessageInit(void)
{
	m_userMsg.SetBackgroundColor(HansDietrich::colorWhite);
	m_userMsg.SetFont(m_actionData.globalDialogTraits.fontFace, 12, FALSE);
	m_userMsg.SetText(L"");

	m_timeoutMsg.SetBackgroundColor(GetSysColor(COLOR_BTNFACE));
	m_timeoutMsg.SetFont(m_actionData.globalDialogTraits.fontFace, 12, FALSE);
	m_timeoutMsg.SetText(L"");
}

afx_msg void CDlgBase::OnPaint()
{
	CDialog::OnPaint();

	//if (!m_flat)
	{
		CWindowDC dc(this); // Don't use CPaintDC
		CRgn rgn;
		//		CBrush FrameBrush(GetSysColor(COLOR_ACTIVECAPTION));
		CBrush FrameBrush(m_actionData.globalDialogTraits.accentColor);
		//CBrush FrameBrush(MBLUE);

		rgn.CreateRectRgn(0, 0, 0, 0);	// Cannot pass NULL region to GetWindowRgn()
		GetWindowRgn(rgn);
		dc.FrameRgn(&rgn, &FrameBrush, 1, 1);
	}
}

afx_msg void CDlgBase::OnNcPaint()
{
	CDialog::OnNcPaint();

	//if (m_flat)
		//CDialog::OnNcPaint();
		//return;

	//else
	//{
	//	// Do not call CDialog::OnNcPaint() for painting messages
	//	const MSG* pMsg = GetCurrentMessage();

	//	CRect rect;
	//	GetWindowRect(&rect);

	//	CRgn rgn;
	//	rgn.CreateRectRgn(rect.left, rect.top, rect.right, rect.bottom);

	//	if (pMsg->wParam != 1)
	//	{
	//		// just a part of the window's NC area needs to be drawn, so
	//		// we create a copy of the update region
	//		HRGN hRgn = (HRGN)pMsg->wParam;
	//		rgn.CopyRgn(CRgn::FromHandle(hRgn));

	//		// in this case the regions coordinates are screen coordinates,
	//		// so we need to move the region before we fill it
	//		rgn.OffsetRgn(-rect.left, -rect.top);
	//	}
	//	else
	//	{
	//		// the whole window needs to be drawn. In this case we don't need
	//		// to move the region since the window's region is relative to the window
	//		GetWindowRgn((HRGN)rgn.GetSafeHandle());
	//	}

	//	CWindowDC dc(this);
	//	CBrush brush;
	//	brush.CreateSolidBrush(GetSysColor(COLOR_ACTIVECAPTION));

	//	dc.FillRgn(&rgn, &brush);

	//	Invalidate();
	//	SendMessage(WM_PAINT);
	//}
}

afx_msg void CDlgBase::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	//if (!m_flat)
	if(!FTW::CheckForFlag(m_actionData.globalDialogTraits.flags, FTW::DialogTraits::FLAT))
	{
		if (cx > 0 && cy > 0)
		{
			// Here we calculate the resulting window size from the passed client size
			CRect rect(0, 0, cx, cy);
			CalcWindowRect(&rect);

			// Here we create the rounded region - you can adjust the 'rounding' using a different 'r'
			CRgn rgn;
			int r = 25;
			rgn.CreateRoundRectRgn(0, 0, rect.Width(), rect.Height(), r, r);
			SetWindowRgn((HRGN)rgn.Detach(), TRUE);
		}
	}
	else
	{
		if (cx > 0 && cy > 0)
		{
			// Here we calculate the resulting window size from the passed client size
			CRect rect(0, 0, cx, cy);
			CalcWindowRect(&rect);

			// Here we create the rounded region - you can adjust the 'rounding' using a different 'r'
			CRgn rgn;
			int r = 25;
			rgn.CreateRectRgn(0, 0, rect.Width(), rect.Height());
			//rgn.CreateRoundRectRgn(0, 0, rect.Width(), rect.Height(), r, r);
			SetWindowRgn((HRGN)rgn.Detach(), TRUE);
		}
	}
}

afx_msg BOOL CDlgBase::OnEraseBkgnd(CDC* pDC)
{
	CRect rc, top, bottom;
	GetClientRect(&rc);

	top = rc;
	top.DeflateRect(0, 0, 0, ScaleY(BUTTONAREA_HEIGHT));
	bottom = rc;
	bottom.DeflateRect(0, rc.Height() - ScaleY(BUTTONAREA_HEIGHT), 0, 0);

	pDC->FillSolidRect(top, RGB(255, 255, 255));
	pDC->FillSolidRect(bottom, GetSysColor(COLOR_BTNFACE));

	std::for_each(m_buttonQueue.begin(), m_buttonQueue.end(),
		[&](HansDietrich::CXButtonXP* pButton) {
		pButton->Invalidate();
	});

	m_controlAreaRect = top;
	m_controlAreaRect.DeflateRect(0, ScaleY(BUTTONAREA_HEIGHT), 0, 0);
	if (FTW::CheckForFlag(m_actionData.globalDialogTraits.flags, FTW::DialogTraits::SHOW_SIDEBAR))
	{
		m_controlAreaRect.DeflateRect(ScaleX(m_banner.GetSize()), 0, 0, 0);
	}

	return TRUE;
}

afx_msg HBRUSH CDlgBase::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	CBrush m_brBgColor;
	m_brBgColor.CreateSolidBrush(RGB(255, 255, 255));	// dialog background WHITE  

														//if (nCtlColor == CTLCOLOR_EDIT) 
														//{
														//pDC->SetTextColor(RGB(0, 0, 0));	// set the Text Color; black  
	pDC->SetBkMode(TRANSPARENT);		// Set the Background Mode to TRANSPARENT
	hbr = m_brBgColor;					// return handle to WHITE brush
										//}

	return hbr;
}

HRESULT CDlgBase::LoadUIImage(const CString imagePath, CImage& image, DWORD successMsg, DWORD failMsg)
{

	//if (imagePath.Left(7) == _T("http://"))
	//{
	//	CString tempLocation;
	//	FTW::DownloadFileFromURL(tempLocation, imagePath, nullptr, true);
	//}

	CString tmpInfoImagePath = imagePath;

	if (tmpInfoImagePath.Left(7) == _T("http://") || tmpInfoImagePath.Left(8) == _T("https://"))
	{
		USES_CONVERSION;

		FTW::CHTTPFile fileDownload;
		FTW::CHTTPOperation dl;
		try
		{
			dl.DownloadFile(T2CA(tmpInfoImagePath.GetBuffer()), &fileDownload, false);

			tmpInfoImagePath = A2CT(fileDownload.GetFullFilename().c_str());

			//m_pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info, AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
			//	IDS_LOGMSG_DOWNLOAD_FILE_SUCCESS, imagePath);
		}
		catch (FTW::HTTPCURLException& e)
		{
			tmpInfoImagePath = _T("");

			CString tmp(e.what());

			m_actionData.pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Error, 
				AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
				FTW::FormatResourceString(IDS_LOGMSG_DOWNLOAD_FILE_FAIL, imagePath, tmp));
		}
		catch (FTW::HTTPStatusException& e)
		{
			tmpInfoImagePath = _T("");

			CString tmp(e.what());

			m_actionData.pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Error,
				AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
				FTW::FormatResourceString(IDS_LOGMSG_DOWNLOAD_FILE_FAIL, imagePath, tmp));
		}
	}

	if (!tmpInfoImagePath.IsEmpty())
	{
		if (std::filesystem::exists((PCTSTR)tmpInfoImagePath))
			{
			HRESULT hr = image.Load(tmpInfoImagePath);

			if (hr != ERROR_SUCCESS)
			{
				m_actionData.pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info,
					AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
					FTW::FormatResourceString(failMsg, imagePath, FTW::FormatHRString(hr)));

				tmpInfoImagePath.Empty();
			}
			else
				m_actionData.pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info,
					AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
					FTW::FormatResourceString(successMsg, imagePath));

			return hr;

		}
		else
		{
			m_actionData.pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info,
				AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
				FTW::FormatResourceString(failMsg, imagePath, FTW::FormatHRString(2)));

			tmpInfoImagePath.Empty();

			return (HRESULT)2;
		}
	}
	
	return (HRESULT)-1;
}

afx_msg void CDlgBase::OnBnClickedShowTSVarDLG()
{
	CDlgTSVar tsdlg(m_actionData.pCMLog);

	//if (m_allowTSVarEditor)
	if(FTW::CheckForFlag(m_actionData.globalDialogTraits.flags, FTW::DialogTraits::ALLOW_VARIABLE_EDITOR))
		tsdlg.DoModal();
}

afx_msg void CDlgBase::OnBnClickedDumpTSVar()
{
	//if (m_allowTSVarEditor)
	if (FTW::CheckForFlag(m_actionData.globalDialogTraits.flags, FTW::DialogTraits::ALLOW_VARIABLE_EDITOR))
		CTSEnv::Instance().DumpToFile(m_actionData.pCMLog);
}

void CDlgBase::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent = m_timerID)
	{
		UpdateTimerMessage();

		if (m_toInfo.CountdownElapsed())
		{
			KillTimer(m_timerID);
			EndDialog(m_toInfo.GetActionCode());
		}
	}

	CDialog::OnTimer(nIDEvent);
}
