#pragma once

#include "HansDietrich\XColorStatic.h"
#include "DlgData.h"
#include "Constants.h"

// CDlgUserInfoFullScreen dialog

class CDlgUserInfoFullScreen : public CDialog
{
	DECLARE_DYNAMIC(CDlgUserInfoFullScreen)

public:
	CDlgUserInfoFullScreen(COLORREF bkColor, FTWCMLOG::ICMLogPtr pLog, PCTSTR brandingImagePath = nullptr, PCTSTR fontFace = DEFAULT_FONTFACE, CWnd* pParent = nullptr);   // standard constructor
	virtual ~CDlgUserInfoFullScreen();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_INFOFULLSCREEN };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	HACCEL m_hAccelTable;
	CBrush m_brBgColor;
	COLORREF m_backgroundColor;

	CString m_dlgInfoText;
	COLORREF m_dlgInfoTextColor;
	HansDietrich::CXColorStatic m_dlgInfo;

	CProgressCtrl m_dlgProgress;

	CImage m_brandingImage;
	CString m_brandingImagePath;
	CStatic m_dlgBrandingImage;
	CString m_fontFaceName;

	bool m_brandingImageLoaded = false;

	FTWCMLOG::ICMLogPtr m_pCMLog;

	int m_xScale;
	int m_yScale;
	inline int ScaleX(int x) { return MulDiv(x, m_xScale, 96); };
	inline int ScaleY(int y) { return MulDiv(y, m_yScale, 96); };

	HRESULT LoadUIImage(const CString imagePath, CImage& image, DWORD successMsg, DWORD failMsg);
	bool LoadBrandingImage(CRect desktopRect, CRect& brandingImageRect);
	CRect GetBrandingImageRect(CRect desktop, int imageWidth, int imageHeight);
	void ResizeBrandingImage(int newWidth, int newHeight, CImage& original);

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnClosefullscreen();
	afx_msg void OnIdcancel();
	afx_msg void OnIdok();
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	void SetupDialog(PCTSTR dlgTitle, const FTW::DialogTraits & traits);
	void SetText(PCTSTR dlgTitle, PCTSTR dlgInfo, COLORREF textColor);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);

};
