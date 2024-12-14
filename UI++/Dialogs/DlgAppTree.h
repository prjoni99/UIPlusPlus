#pragma once

#include "DlgBase.h"
#include "afxcmn.h"
#include "Software.h"
#include "FTW\FTWTreeCtrl.h"

// CAppTreeDlg dialog

class CDlgAppTree : public CDlgBase
{
	DECLARE_DYNAMIC(CDlgAppTree)

public:
//	CDlgAppTree(FTWCMLOG::ICMLogPtr pLog, CWnd* pParent = NULL);   // standard constructor
	CDlgAppTree(PCTSTR dlgTitle, 
		const DialogVisibilityFlags flags,
		const UIpp::ActionData& data,
		FTW::FTWTreeItemList* pItems, 
		CDlgBase::DlgSize size,
		bool expanded = false, 
		CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgAppTree();

// Dialog Data
	enum { IDD = IDD_APPTREEDLG };
	enum { IDDTALL = IDD_APPTREEDLGTALL };
	enum { IDDEXTRATALL = IDD_APPTREEDLGEXTRATALL };

protected:
	void Dummy(void) {};

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	afx_msg void OnBnClickedNextaction();
	afx_msg void OnCheckNotify(NMHDR* pNMHDR, LRESULT* pResult);

	DECLARE_MESSAGE_MAP()

public:
	virtual BOOL OnInitDialog();

protected:
	FTW::CFTWTreeCtrl m_appTree;
	FTW::FTWTreeItemList* m_pAppTreeItems;

	int m_initialized;
	bool m_treeExpandedByDefault;

	void ConvertIDStringList(CString idList, FTW::FTWTreeItemSet* pItemList);

};
