#pragma once

//#include <XListCtrl.h>
#include "CodeProject\WndResizer\WndResizer.h"
#include "afxpropertygridctrl.h"
#include "resource.h"
#include "afxcmn.h"
#include "afxwin.h"

// CTSVarDlg dialog

class CTSVarGridCtrl : public CMFCPropertyGridCtrl
{
public:
	CTSVarGridCtrl();
	~CTSVarGridCtrl();

    void SetLeftColumnWidth(int cx = 0)
    {
        if (cx == 0)
		{
			CRect rect;
			GetWindowRect(&rect);
			m_nLeftColumnWidth = rect.Width() / 2;
		}
		else
			m_nLeftColumnWidth = cx;

		AdjustLayout();
    }

	void PopulateGrid(bool readOnly = true);

	virtual void OnPropertyChanged(CMFCPropertyGridProperty* pProp) const;

	static UINT UWM_GRID_UPDATED;

};


class CDlgTSVar : public CDialog
{
	DECLARE_DYNAMIC(CDlgTSVar)

public:
	CDlgTSVar(FTWCMLOG::ICMLogPtr pLog, CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgTSVar();

// Dialog Data
	enum { IDD = IDD_TSVARDLG };

protected:
//	CodeProject::CKCSideBannerWnd m_banner;
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	CodeProject::CWndResizer m_resizer;
	afx_msg LRESULT OnGridValueChange(WPARAM, LPARAM);

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();

	afx_msg void OnSelchangeVariabletab(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedOk();
	afx_msg void OnClickedGridreset();
	afx_msg void OnBnClickedCancel();protected:
	CButton m_OK;
	CButton m_resetAll;

protected:
	CTabCtrl m_varTabCtrl;

	CTSVarGridCtrl v_readonlyVarGrid;
	CTSVarGridCtrl v_editableVarGrid;

	void DeleteAllProperties(void);

	FTWCMLOG::ICMLogPtr m_pCMLog;
};
