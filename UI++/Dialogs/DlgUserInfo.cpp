// InfoDlg.cpp : implementation file
//

#include "stdafx.h"
#include "UI++.h"
#include "DlgUserInfo.h"
#include "afxdialogex.h"

//#define TIMERID 4181972

// CInfoDlg dialog

IMPLEMENT_DYNAMIC(CDlgUserInfo, CDlgBase)

//CInfoDlg::CInfoDlg(bool showBanner, bool showCancel, CWnd* pParent /*=NULL*/)
//: CUIDlg(CInfoDlg::IDD, showCancel, pParent), m_showBanner(showBanner)
//{
//	m_dlgIconID = IDI_INFO;
//}

//CDlgUserInfo::CDlgUserInfo(DialogVisibilityFlags flags, FTWCMLOG::ICMLogPtr pLog,
//	PCTSTR brandingImagePath, PCTSTR infoImagePath, CWnd* pParent /*=NULL*/)
//	: CDlgBase((brandingImagePath == nullptr || _tcslen(brandingImagePath) == 0 ? CDlgUserInfo::IDD : CDlgUserInfo::IDD_BANNER), flags, pLog, pParent)
//	, m_brandingImagePath(brandingImagePath), m_brandingImageLoaded(false)
//	, m_infoImagePath(infoImagePath)//, m_toInfo(toInfo)
//{
//	m_dlgIconID = IDI_DLGINFO;
//
//	if((flags & SHOW_OK) == 0)
//		m_dlgIconID = IDI_DLGSTOP;
//
//	if (LoadUIImage(m_brandingImagePath, m_brandingImage,
//		IDS_LOGMSG_BRANDIMAGE_LOADED, IDS_LOGERROR_BRANDIMAGE_LOADFAIL) == ERROR_SUCCESS)
//	{
//		m_dlgIconID = 0;
//		m_brandingImageLoaded = true;
//	}
//
//	LoadUIImage(m_infoImagePath, m_infoImage,
//		IDS_LOGMSG_INFOIMAGE_LOADED, IDS_LOGERROR_INFOIMAGE_LOADFAIL);
//}

CDlgUserInfo::CDlgUserInfo(PCTSTR dlgTitle, 
	const DialogVisibilityFlags flags,
	const UIpp::ActionData& data,
	PCTSTR infoText,
	PCTSTR brandingImagePath, 
	PCTSTR infoImagePath,
	CWnd* pParent)
	: CDlgBase(dlgTitle,
		(brandingImagePath == nullptr || _tcslen(brandingImagePath) == 0 ? CDlgUserInfo::IDD : CDlgUserInfo::IDD_BANNER), 
		flags, data, pParent),
	m_dlgInfoText(infoText),
	m_brandingImagePath(brandingImagePath), 
	m_brandingImageLoaded(false),
	m_infoImagePath(infoImagePath)
{
	if (LoadUIImage(m_brandingImagePath, m_brandingImage,
		IDS_LOGMSG_BRANDIMAGE_LOADED, IDS_LOGERROR_BRANDIMAGE_LOADFAIL) == ERROR_SUCCESS)
	{
		m_brandingImageLoaded = true;
	}

	LoadUIImage(m_infoImagePath, m_infoImage,
		IDS_LOGMSG_INFOIMAGE_LOADED, IDS_LOGERROR_INFOIMAGE_LOADFAIL);
}

CDlgUserInfo::~CDlgUserInfo()
{
}

void CDlgUserInfo::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	CDlgBase::DoDataExchange(pDX);
	
	if(m_brandingImageLoaded)
		DDX_Control(pDX, IDC_BANNER, m_dlgBrandingBanner);
}

BEGIN_MESSAGE_MAP(CDlgUserInfo, CDlgBase)
	//ON_COMMAND(IDCANCEL, &CDlgUserInfo::OnIdcancel)
END_MESSAGE_MAP()

// CInfoDlg message handlers

BOOL CDlgUserInfo::OnInitDialog()
{
	CDlgBase::OnInitDialog();

	//CRect r (0, 0, 319, 40);
	//MapDialogRect(&r);
	//65x479

	CRect rectFrame;

	GetDlgItem(IDC_INFOTEXT)->GetWindowRect(&rectFrame);
    ScreenToClient(&rectFrame);
    
	GetDlgItem(IDC_INFOTEXT)->ShowWindow(SW_HIDE);
    
	VERIFY(m_dlgInfo.Create(AfxGetInstanceHandle(), AfxGetResourceHandle(), WS_CHILD | WS_VISIBLE, 
        rectFrame, m_hWnd, 9001));
	
	LOGFONT lf;

	lf = m_dlgInfo.GetLogFont();

	_tcscpy_s(lf.lfFaceName, sizeof(lf.lfFaceName) / sizeof(TCHAR)-1, m_actionData.globalDialogTraits.fontFace);
	lf.lfHeight = 0 - ScaleY(12);
	lf.lfWeight = 400;

	m_dlgInfo.SetLogFont(lf);
	m_dlgInfo.SetWindowText(m_dlgInfoText);

	if (!m_brandingImage.IsNull())
	{
		m_dlgBrandingBanner.SetBitmap(HBITMAP(m_brandingImage));
	}

	if (!m_infoImage.IsNull())
	{
		CPoint center = rectFrame.CenterPoint();

		CRect infoImgRect(center.x - (m_infoImage.GetWidth() / 2) - 15, (rectFrame.bottom - m_infoImage.GetHeight()) - 15,
			center.x + (m_infoImage.GetWidth() / 2) - 15, rectFrame.bottom - 15);

		m_dlgInfoBanner.Create(_T(""), WS_VISIBLE | WS_CHILD | SS_BITMAP | SS_CENTERIMAGE | WS_EX_TRANSPARENT, infoImgRect, this);
		m_dlgInfoBanner.SetBitmap(HBITMAP(m_infoImage));
	}

	if(!m_toInfo.AllowPreempt())
		m_next.EnableWindow(false);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

