// UIProgressDlg.cpp : implementation file
//

#include "stdafx.h"
#include "UI++.h"
#include "DlgProgress.h"
#include "afxdialogex.h"
#include "HansDietrich\XNamedColors.h"

// CUIProgressDlg dialog

IMPLEMENT_DYNAMIC(CDlgUIProgress, CDialog)

CDlgUIProgress::CDlgUIProgress(int maxRange, PCTSTR title, PCTSTR fontFace, CWnd* pParent /*=NULL*/)
	: CDialog(IDD_UIPROGRESSDLG, pParent),
	m_maxRange(maxRange), m_titleText(title), m_fontFaceName(fontFace)
{

}

CDlgUIProgress::~CDlgUIProgress()
{
}

void CDlgUIProgress::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PROGRESS1, m_progress);
	DDX_Control(pDX, IDC_PROGRESSMESSAGE, m_progressMessage);
	DDX_Control(pDX, IDC_PROGRESSTITLE, m_progressTitle);
}


BEGIN_MESSAGE_MAP(CDlgUIProgress, CDialog)
	ON_WM_ERASEBKGND()
	ON_MESSAGE(WM_UPDATEPROGRESSBAR, OnUpdateProgressBar)
	ON_MESSAGE(WM_CLOSEPROGRESSBAR, OnCloseProgressBar)
	ON_WM_CTLCOLOR()
	ON_COMMAND(IDCANCEL, &CDlgUIProgress::OnIdcancel)
	ON_COMMAND(IDOK, &CDlgUIProgress::OnIdcancel)
END_MESSAGE_MAP()


// CUIProgressDlg message handlers


//BOOL CUIProgressDlg::OnEraseBkgnd(CDC* pDC)
//{
//	CRect rc;
//	GetClientRect(&rc);
//
//	pDC->FillSolidRect(rc, RGB(255, 255, 255));
//
//	return TRUE;
//
//	//return CDialog::OnEraseBkgnd(pDC);
//}


BOOL CDlgUIProgress::OnInitDialog()
{
	m_brush.CreateSolidBrush(RGB(255, 255, 255));

	CDialog::OnInitDialog();

	m_progressMessage.SetBackgroundColor(HansDietrich::colorWhite);
	m_progressMessage.SetFont(m_fontFaceName, 14, FALSE);

	m_progressTitle.SetBackgroundColor(HansDietrich::colorWhite);
	m_progressTitle.SetFont(m_fontFaceName, 16, FALSE);
	m_progressTitle.SetText(m_titleText);

	if (m_maxRange > 0)
	{
		m_progress.SetRange(0, m_maxRange);
		m_progress.SetPos(0);
	}
	else
	{
		LONG_PTR progressStyle = GetWindowLongPtr(m_progress.GetSafeHwnd(), GWL_STYLE);
		SetWindowLongPtr(m_progress.GetSafeHwnd(), GWL_STYLE, progressStyle | PBS_MARQUEE | PBS_SMOOTH);
		m_progress.SetRange(0, 100);
		m_progress.SetPos(0);
		m_progress.SetMarquee(TRUE, 25);
	}

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}


//INT_PTR CUIProgressDlg::DoModal()
//{
//	if (m_pFunc == nullptr || m_pFuncData == nullptr)
//		return IDABORT;
//
//	return CDialog::DoModal();
//}

afx_msg LRESULT CDlgUIProgress::OnUpdateProgressBar(WPARAM wParam, LPARAM lParam)
{
	TRACE1("Setting progress postion: %d\n", (int)wParam);
	m_progress.SetPos((int)wParam);
	
	if (lParam != 0)
	{
		CString* pText = (CString*)lParam;
		CString progressText(*pText);

		progressText.Append(_T(" ..."));

		if (progressText.GetLength() > 0)
		{
			TRACE1("Progress Message: %s\n", progressText);
			m_progressMessage.SetText(progressText);
		}
	}
	
	return 0;
}

LRESULT CDlgUIProgress::OnCloseProgressBar(WPARAM, LPARAM)
{
	//EndDialog(ERROR_SUCCESS);
	EndModalLoop(ERROR_SUCCESS);
	this->DestroyWindow();
	return 0;
}



HBRUSH CDlgUIProgress::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	//HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	// TODO:  Change any attributes of the DC here

	// TODO:  Return a different brush if the default is not desired
	//return hbr;

	return m_brush;
}


void CDlgUIProgress::OnIdcancel()
{
	// TODO: Add your command handler code here
}
