// AppTreeDlg.cpp : implementation file
//

#include "stdafx.h"
#include "UI++.h"
#include "DlgAppTree.h"
#include "afxdialogex.h"

#include <algorithm>
#include <regex>

// CAppTreeDlg dialog

IMPLEMENT_DYNAMIC(CDlgAppTree, CDlgBase)

CDlgAppTree::CDlgAppTree(PCTSTR dlgTitle, 
	const DialogVisibilityFlags flags,
	const UIpp::ActionData& data,
	FTW::FTWTreeItemList* pItems,
	CDlgBase::DlgSize size,  
	bool expanded, 
	CWnd* pParent)
	: CDlgBase(dlgTitle,
		(size == CDlgBase::DlgSize::Tall ? CDlgAppTree::IDDTALL : (size == CDlgBase::DlgSize::ExtraTall ? CDlgAppTree::IDDEXTRATALL : CDlgAppTree::IDD)),
		flags, data, pParent),
	m_treeExpandedByDefault(expanded), 
	m_initialized(0), 
	m_pAppTreeItems(pItems)
{
}

CDlgAppTree::~CDlgAppTree()
{

}

void CDlgAppTree::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	CDlgBase::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_APPTREE, m_appTree);
}


BEGIN_MESSAGE_MAP(CDlgAppTree, CDlgBase)
	//ON_REGISTERED_MESSAGE(TVN_CHECK, OnCheckbox)
END_MESSAGE_MAP()

BOOL CDlgAppTree::OnInitDialog()
{
	CDlgBase::OnInitDialog();

	m_appTree.Init(IDB_TREEIMAGES);

	HTREEITEM hTreeItem = NULL;
	int icon = FTW::TREEICON_GROUP;

	ASSERT(m_pAppTreeItems);

	// First pass to populate tree
	std::for_each(m_pAppTreeItems->begin(), m_pAppTreeItems->end(),[&](FTW::CFTWTreeItemPtr& pTI)
	{
		if (!pTI->IsHidden())
		{
			int image = pTI->Type() == FTW::CFTWTreeItem::ItemType::GROUP ? FTW::TREEICON_GROUP : FTW::TREEICON_APP;

			HTREEITEM hItem = m_appTree.InsertItem(pTI->GetLabel(), image, image, pTI->GetParentTreeHandle());

			pTI->SetTreeHandle(hItem);

			m_appTree.SetItemData(hItem, (DWORD_PTR)pTI.get());
		}
		else
		{
			pTI->SetTreeHandle(nullptr);
		}
	});

	if (m_treeExpandedByDefault)
		m_appTree.ExpandAll();

	m_appTree.EnsureVisible(m_appTree.GetRootItem());

	m_initialized = 1;

	// Second pass to set checks for default items and disable required items
	std::for_each(m_pAppTreeItems->begin(), m_pAppTreeItems->end(), [&](FTW::CFTWTreeItemPtr& pTI)
	{
		FTW::FTWTreeItemSet itemList;

		ConvertIDStringList(pTI->GetIncludeStringList(), &itemList);

		pTI->SetIncludeItemList(itemList);

		m_appTree.RefreshItemState(pTI.get());

	});
	
	m_initialized = 2;

	GotoDlgCtrl(GetDlgItem(IDC_APPTREE));

	return FALSE;  // return TRUE unless you set the focus to a control
}

void CDlgAppTree::ConvertIDStringList(CString idList, FTW::FTWTreeItemSet* pItemList)
{
	int pos = 0;

	CString id = idList.Tokenize(_T(",;"), pos);

	id.Trim();

	while (id != _T(""))
	{
		auto it = std::find_if(m_pAppTreeItems->begin(), m_pAppTreeItems->end(), [&](FTW::CFTWTreeItemPtr& pTI)
		{
			return (id == pTI->GetID());
		});

		if (it != m_pAppTreeItems->end())
			pItemList->insert((*it).get());

		id = idList.Tokenize(_T(",;"), pos);
		id.Trim();
	}
}

// CAppTreeDlg message handlers

void CDlgAppTree::OnBnClickedNextaction()
{

	EndDialog(0);
}

void CDlgAppTree::OnCheckNotify(NMHDR* pNMHDR, LRESULT* pResult)
{
	*pResult = 0;

	
}
