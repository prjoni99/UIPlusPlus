// CDlgUserInfoFullScreen.cpp : implementation file
//

#include "stdafx.h"
#include "UI++.h"
#include "DlgUserInfoFullScreen.h"
#include "FTW\FTWHTTP.h"
#include <filesystem>
#include "afxdialogex.h"


// CDlgUserInfoFullScreen dialog

IMPLEMENT_DYNAMIC(CDlgUserInfoFullScreen, CDialog)

CDlgUserInfoFullScreen::CDlgUserInfoFullScreen(COLORREF bkColor, FTWCMLOG::ICMLogPtr pLog, PCTSTR brandingImagePath, PCTSTR fontFace, CWnd* pParent /*=nullptr*/)
	: m_dlgInfoTextColor(RGB(255,255,255)), m_xScale(1), m_yScale(1),
	CDialog(IDD_INFOFULLSCREEN, pParent), m_backgroundColor(bkColor), m_pCMLog(pLog), m_brandingImagePath (brandingImagePath), m_fontFaceName(fontFace)
{
	m_hAccelTable = LoadAccelerators(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_ACCELERATOR_USERINFOFULLSCREEN));
}

CDlgUserInfoFullScreen::~CDlgUserInfoFullScreen()
{
	if(m_brandingImageLoaded)
		m_brandingImage.Detach();
}

void CDlgUserInfoFullScreen::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_INFOTEXT, m_dlgInfo);
	DDX_Control(pDX, IDC_PROGRESS1, m_dlgProgress);
}


BEGIN_MESSAGE_MAP(CDlgUserInfoFullScreen, CDialog)
	ON_COMMAND(ID_CLOSEFULLSCREEN, &CDlgUserInfoFullScreen::OnClosefullscreen)
	ON_COMMAND(IDCANCEL, &CDlgUserInfoFullScreen::OnIdcancel)
	ON_WM_CTLCOLOR()
	ON_COMMAND(IDOK, &CDlgUserInfoFullScreen::OnIdok)
END_MESSAGE_MAP()


// CDlgUserInfoFullScreen message handlers


BOOL CDlgUserInfoFullScreen::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_brBgColor.CreateSolidBrush(m_backgroundColor);	// dialog background WHITE 
	
	CRect desktopRect, controlRect;

	GetDesktopWindow()->GetWindowRect(&desktopRect);

	MoveWindow(desktopRect, FALSE);

	m_dlgProgress.GetClientRect(&controlRect);
	controlRect.MoveToY((int)(desktopRect.Height() * 0.8));
	controlRect.left = (long)(desktopRect.Width() * 0.1);
	controlRect.right = (long)(desktopRect.Width() * 0.9);
	m_dlgProgress.MoveWindow(controlRect, FALSE);

	m_dlgInfo.GetClientRect(&controlRect);
	controlRect.MoveToY((int)((desktopRect.Height() * 0.8) - 72));
	controlRect.left = (long)(desktopRect.Width() * 0.1);
	controlRect.right = (long)(desktopRect.Width() * 0.9);
	m_dlgInfo.MoveWindow(controlRect, FALSE);

	m_dlgInfo.SetFont(m_fontFaceName, 24, FALSE);
	m_dlgInfo.SetBackgroundColor(m_backgroundColor);
	m_dlgInfo.SetTextColor(m_dlgInfoTextColor);
	m_dlgInfo.SetText(m_dlgInfoText);
	m_dlgInfo.SetBold(TRUE);

	CRect brandingImageRect;

	if (m_brandingImagePath.GetLength() > 0)
	{
		m_brandingImageLoaded = LoadBrandingImage(desktopRect, brandingImageRect);

		m_dlgBrandingImage.Create(_T(""), WS_VISIBLE | WS_CHILD | SS_BITMAP | SS_CENTERIMAGE | WS_EX_TRANSPARENT, brandingImageRect, this);
		m_dlgBrandingImage.SetBitmap(HBITMAP(m_brandingImage));
	}

	m_dlgProgress.SetMarquee(TRUE, 30);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}


void CDlgUserInfoFullScreen::OnClosefullscreen()
{
	EndDialog(ERROR_CANCELLED);
}


void CDlgUserInfoFullScreen::OnIdcancel()
{
	// TODO: Add your command handler code here
}

void CDlgUserInfoFullScreen::OnIdok()
{
	// TODO: Add your command handler code here
}


BOOL CDlgUserInfoFullScreen::PreTranslateMessage(MSG* pMsg)
{
	if (m_hAccelTable && ::TranslateAccelerator(m_hWnd, m_hAccelTable, pMsg))
		return(TRUE);

	//m_buttonToolTips.RelayEvent(pMsg);

	return CDialog::PreTranslateMessage(pMsg);
}

void CDlgUserInfoFullScreen::SetupDialog(PCTSTR dlgTitle, const FTW::DialogTraits & traits)
{
	//SetText(dlgTitle, dData.title, dData.subtitle);

	m_xScale = traits.screenScale.x;
	m_yScale = traits.screenScale.y;
}

void CDlgUserInfoFullScreen::SetText(PCTSTR dlgTitle, PCTSTR dlgInfo, COLORREF textColor)
{
	m_dlgInfoText = dlgInfo;
	m_dlgInfoTextColor = textColor;
}

HBRUSH CDlgUserInfoFullScreen::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	//HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	return m_brBgColor;
}

bool CDlgUserInfoFullScreen::LoadBrandingImage(CRect desktopRect, CRect& brandingImageRect)
{
	CImage tempImage;
	//m_brandingImage

	if (LoadUIImage(m_brandingImagePath, tempImage,
		IDS_LOGMSG_BRANDIMAGE_LOADED, IDS_LOGERROR_BRANDIMAGE_LOADFAIL) == ERROR_SUCCESS)
	{
		brandingImageRect = GetBrandingImageRect(desktopRect, tempImage.GetWidth(), tempImage.GetHeight());

		int maxWidth = (int)(desktopRect.Width() * 0.9);
		int maxHeight = (int)((desktopRect.Height() / 2) * 0.9);

		if (brandingImageRect.Width() > maxWidth)
		{
			double ratio = (double)((float)maxWidth / (float)brandingImageRect.Width());
			int newHeight = (int)(brandingImageRect.Height() * ratio);

			brandingImageRect = GetBrandingImageRect(desktopRect, maxWidth, newHeight);
		}

		if (brandingImageRect.Height() > maxHeight)
		{
			double ratio = (double)((float)maxHeight / (float)brandingImageRect.Height());
			int newWidth = (int)(brandingImageRect.Width() * ratio);

			brandingImageRect = GetBrandingImageRect(desktopRect, newWidth, maxHeight);
		}

		ResizeBrandingImage(brandingImageRect.Width(), brandingImageRect.Height(), tempImage);

		tempImage.Detach();

		return true;
	}

	return false;

}

HRESULT CDlgUserInfoFullScreen::LoadUIImage(const CString imagePath, CImage& image, DWORD successMsg, DWORD failMsg)
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

			m_pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Error,
				AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
				FTW::FormatResourceString(IDS_LOGMSG_DOWNLOAD_FILE_FAIL, imagePath, tmp));
		}
		catch (FTW::HTTPStatusException& e)
		{
			tmpInfoImagePath = _T("");

			CString tmp(e.what());

			m_pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Error,
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
				m_pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info,
					AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
					FTW::FormatResourceString(failMsg, imagePath, FTW::FormatHRString(hr)));

				tmpInfoImagePath.Empty();
			}
			else
				m_pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info,
					AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
					FTW::FormatResourceString(successMsg, imagePath));

			return hr;

		}
		else
		{
			m_pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info,
				AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
				FTW::FormatResourceString(failMsg, imagePath, FTW::FormatHRString(2)));

			tmpInfoImagePath.Empty();

			return (HRESULT)2;
		}
	}

	return (HRESULT)-1;
}

CRect CDlgUserInfoFullScreen::GetBrandingImageRect(CRect desktop, int imageWidth, int imageHeight)
{
	CPoint center = desktop.CenterPoint();
	
	int left = center.x - (imageWidth / 2);
	int right = left + imageWidth;
	int top = (int)(desktop.Height() * 0.1);
	int bottom = top + imageHeight;

	return CRect (left, top, right, bottom);
}

void CDlgUserInfoFullScreen::ResizeBrandingImage(int newWidth, int newHeight, CImage& original)
{
	//CDC *pScreenDC = GetDC();

	m_brandingImage.Create(newWidth, newHeight, 32);
	HDC imageDC = m_brandingImage.GetDC();

	SetStretchBltMode(imageDC, COLORONCOLOR);
	original.StretchBlt(imageDC, 0, 0, newWidth, newHeight, SRCCOPY);

	m_brandingImage.ReleaseDC();
}


