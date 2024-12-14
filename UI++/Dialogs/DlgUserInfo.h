#pragma once

#include "HansDietrich\XHTMLCtrl.h"

//#include "afxwin.h"
//#include "afxbutton.h"
#include "DlgBase.h"

// CInfoDlg dialog

class CDlgUserInfo : public CDlgBase
{
	DECLARE_DYNAMIC(CDlgUserInfo)

public:
	
	////CInfoDlg(bool showCancel = false, CWnd* pParent = NULL);   // standard constructor
	//CDlgUserInfo(DialogVisibilityFlags flags, FTWCMLOG::ICMLogPtr pLog, PCTSTR brandingImagePath = nullptr
	//	, PCTSTR infoImagePath = nullptr, CWnd* pParent = NULL);   // standard constructor

	CDlgUserInfo(PCTSTR dlgTitle, 
		const DialogVisibilityFlags flags,
		const UIpp::ActionData& data,
		PCTSTR infoText = _T(""),
		PCTSTR brandingImagePath = nullptr, 
		PCTSTR infoImagePath = nullptr,
		CWnd* pParent = NULL);

	virtual ~CDlgUserInfo();

// Dialog Data
	enum { IDD = IDD_INFODLG };
	enum { IDD_BANNER = IDD_INFOBANNERDLG };

	virtual BOOL OnInitDialog();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

	CString m_dlgInfoText;

	void Dummy(void) {};

	HansDietrich::CXHTMLCtrl m_dlgInfo;
	CStatic m_dlgBrandingBanner;
	CImage m_brandingImage;
	CString m_brandingImagePath;
	CStatic m_dlgInfoBanner;
	CImage m_infoImage;
	CString m_infoImagePath;

	bool m_brandingImageLoaded;
};
