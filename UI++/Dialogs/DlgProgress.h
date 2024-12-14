#pragma once
#include "afxcmn.h"
#include "afxwin.h"
#include "Constants.h"

#include "HansDietrich\XColorStatic.h"

typedef UINT(*ProgressFunction)(LPVOID);

#define WM_UPDATEPROGRESSBAR (WM_USER + 100)
#define WM_CLOSEPROGRESSBAR (WM_USER + 101)

// CUIProgressDlg dialog

class CDlgUIProgress : public CDialog
{
	DECLARE_DYNAMIC(CDlgUIProgress)

public:
	CDlgUIProgress(int maxRange = 100, PCTSTR title = 0, PCTSTR fontFace = DEFAULT_FONTFACE, CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgUIProgress();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_UIPROGRESSDLG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
	CProgressCtrl m_progress;
	HansDietrich::CXColorStatic m_progressMessage;
	HansDietrich::CXColorStatic m_progressTitle;
	CString m_titleText;
	CString m_fontFaceName;
	CBrush m_brush;

	int m_maxRange;

	//afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	virtual BOOL OnInitDialog();

	afx_msg LRESULT OnCloseProgressBar(WPARAM, LPARAM);
	afx_msg LRESULT OnUpdateProgressBar(WPARAM wParam, LPARAM lParam);
public:
	//virtual INT_PTR DoModal();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnIdcancel();
};
