// FTWTreeCtrl.cpp : implementation file
//

#include "stdafx.h"
//#include "UI++.h"
#include "FTWTreeCtrl.h"
#include "..\resource.h"
#include <algorithm>

namespace FTW
{
	// CFTWTreeCtrl

	IMPLEMENT_DYNAMIC(CFTWTreeCtrl, CTreeCtrl)

	CFTWTreeCtrl::CFTWTreeCtrl()
	{

	}

	CFTWTreeCtrl::~CFTWTreeCtrl()
	{
	}


	BEGIN_MESSAGE_MAP(CFTWTreeCtrl, CTreeCtrl)
		ON_WM_LBUTTONDOWN()
		ON_WM_LBUTTONDBLCLK()
		ON_WM_KEYDOWN()
	END_MESSAGE_MAP()

	// CFTWTreeCtrl message handlers

	void CFTWTreeCtrl::PreSubclassWindow()
	{
		DWORD dwStyle = GetStyle();

		ModifyStyle(TVS_DISABLEDRAGDROP, TVS_CHECKBOXES | TVS_HASBUTTONS | TVS_HASLINES);

		CTreeCtrl::PreSubclassWindow();
	}

	void CFTWTreeCtrl::Init(UINT id)
	{
		CBitmap bmpTree;

		bmpTree.LoadBitmap(IDB_TREECHECKBOXIMAGES);

		// Initialize the image list for the checkboxes
		m_checkboxImageList.Create(16, 16, ILC_COLOR32, 5, 5);
		m_checkboxImageList.Add(&bmpTree, 0x0000FFFF);

		SetImageList(&m_checkboxImageList, TVSIL_STATE);

		//Initialize the image list for the item icons
		if (id)
		{
			CBitmap bmpIcons;
			bmpIcons.LoadBitmap(id);

			m_itemImageList.Create(18, 18, ILC_COLOR32, 5, 5);
			m_itemImageList.Add(&bmpIcons, 0x0000FFFF);

			SetImageList(&m_itemImageList, TVSIL_NORMAL);

		}
	}

	void CFTWTreeCtrl::OnLButtonDown(UINT nFlags, CPoint point)
	{
		UINT flags = 0;
		HTREEITEM hItem = HitTest(point, &flags);

		if (flags & TVHT_ONITEMSTATEICON)
			ItemClicked(hItem);

		else
			CTreeCtrl::OnLButtonDown(nFlags, point);
	}

	void CFTWTreeCtrl::OnLButtonDblClk(UINT nFlags, CPoint point)
	{
		UINT flags = 0;
		HTREEITEM hItem = HitTest(point, &flags);

		if (!(flags & TVHT_ONITEMSTATEICON))
			CTreeCtrl::OnLButtonDblClk(nFlags, point);
	}

	void CFTWTreeCtrl::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
	{
		if (nChar == VK_SPACE)
			ItemClicked(GetSelectedItem());

		else
			CTreeCtrl::OnKeyDown(nChar, nRepCnt, nFlags);
	}

	void CFTWTreeCtrl::ExpandAll(void)
	{
		HTREEITEM hRoot = GetRootItem();

		while (hRoot)
		{
			ExpandItemandChildren(hRoot, TVE_EXPAND);

			hRoot = GetNextSiblingItem(hRoot);
		}
	}

	void CFTWTreeCtrl::ExpandItemandChildren(HTREEITEM hItem, UINT code)
	{
		if (hItem)
		{
			Expand(hItem, code);

			HTREEITEM hChild = GetChildItem(hItem);

			while (hChild)
			{
				if (ItemHasChildren(hItem))
					ExpandItemandChildren(hChild, code);

				hChild = GetNextSiblingItem(hChild);
			}
		}
	}

	void CFTWTreeCtrl::ItemClicked(HTREEITEM hItem)
	{
		CFTWTreeItem* pTI = (CFTWTreeItem*)GetItemData(hItem);

		VERIFY(pTI);

		if (pTI != nullptr && !pTI->IsDisabled())
		{
			if (pTI->Type() == CFTWTreeItem::ItemType::GROUP)
				UpdateChildrenState(static_cast<CFTWTreeGroupItem*>(pTI), pTI->IsCheckedIfToggled());
			else
				pTI->ItemClicked();

			RefreshItemState(pTI);

		}
	}

	void CFTWTreeCtrl::RefreshItemState(CFTWTreeItem* pItem, FTW::FTWTreeItemSet* pItemIncludedChain)
	{
		if (!pItem)
			return;

		HTREEITEM hItem = pItem->GetTreeHandle();

		CheckStateImageType image = CSIT_NONE;
		int icon;

		if (pItem->Type() == CFTWTreeItem::ItemType::GROUP)
			icon = TREEICON_GROUP;
		else
			icon = TREEICON_APP;

		if (pItem->IsRequired())
		{
			image = CSIT_CHECKED_DISABLED;

			if (pItem->Type() == CFTWTreeItem::ItemType::GROUP)
				icon = TREEICON_GROUPLOCK;
			else
				icon = TREEICON_APPLOCK;
		}
		else if (pItem->IsIncluded())
		{
			image = CSIT_CHECKED_DISABLED;

			if (pItem->Type() == CFTWTreeItem::ItemType::GROUP)
				icon = TREEICON_GROUPLOCK;
			else
				icon = TREEICON_APPLINK;
		}
		else if (pItem->IsTriState())
		{
			image = CSIT_TRISTATE;

		}
		else if (pItem->IsChecked())
		{
			image = CSIT_CHECKED;
		}
		else
		{
			image = CSIT_UNCHECKED;
		}

		if (hItem)
		{
			SetItemState(hItem, INDEXTOSTATEIMAGEMASK(image), TVIS_STATEIMAGEMASK);
			SetItemImage(hItem, icon, icon);
		}

		if (RefreshParentItemState(pItem->GetParentItem()))
			RefreshItemState(pItem->GetParentItem());

		std::for_each(pItem->m_includeTreeItems.begin(), pItem->m_includeTreeItems.end(), [&](CFTWTreeItem* pIncludedItem)
		{
			if (pItemIncludedChain == nullptr || (pItemIncludedChain->find(pIncludedItem) == pItemIncludedChain->end()))
			{
				VERIFY(pIncludedItem);

				pIncludedItem->ItemIncluded(pItem->IsChecked(), pItem);

				FTW::FTWTreeItemSet itemIncludeChain;
				if (pItemIncludedChain != nullptr)
					itemIncludeChain = *pItemIncludedChain;

				itemIncludeChain.insert(pItem);

				RefreshItemState(pIncludedItem, &itemIncludeChain);
			}
		});
	}

	//void CFTWTreeCtrl::RefreshItemState(HTREEITEM hItem, TreeItemSet* pItemIncludedChain)
	//{
	//	VERIFY(hItem);
	//
	//	CTreeItem* pTI = (CTreeItem*)GetItemData(hItem);
	//
	//	VERIFY(pTI);
	//
	//	CheckStateImageType image = CSIT_NONE;
	//
	//	if (pTI->IsRequired())
	//		image = CSIT_CHECKED_DISABLED;
	//	else if (pTI->IsIncluded())
	//		image = CSIT_CHECKED_DISABLED;
	//	else if (pTI->IsTriState())
	//		image = CSIT_TRISTATE;
	//	else if (pTI->IsChecked())
	//		image = CSIT_CHECKED;
	//	else
	//		image = CSIT_UNCHECKED;
	//
	//	SetItemState(hItem, INDEXTOSTATEIMAGEMASK(image), TVIS_STATEIMAGEMASK);
	//
	//	if (HTREEITEM hParent = GetParentItem(hItem))
	//	{
	//		RefreshParentItemState(hParent);
	//		RefreshItemState(hParent);
	//	}
	//
	//	TreeItemSet* const pItemList = pTI->GetIncludeItemList();
	//
	//	std::for_each(pItemList->begin(), pItemList->end(), [&](CTreeItem* pIncludedItem)
	//	{
	//		if (pItemIncludedChain == nullptr || (pItemIncludedChain->find(pIncludedItem) == pItemIncludedChain->end()))
	//		{
	//			//CTreeItem* pIncludedItem = (CTreeItem*)GetItemData(hIncludedItem);
	//
	//			VERIFY(pIncludedItem);
	//
	//			pIncludedItem->ItemIncluded(pTI->IsChecked(), pTI);
	//
	//			TreeItemSet itemIncludeChain;
	//			if (pItemIncludedChain != nullptr)
	//				itemIncludeChain = *pItemIncludedChain;
	//
	//			itemIncludeChain.insert(pTI);
	//
	//			RefreshItemState(pIncludedItem->GetTreeHandle(), &itemIncludeChain);
	//		}
	//	});
	//}

	void CFTWTreeCtrl::UpdateChildrenState(CFTWTreeGroupItem* pItem, bool parentDesiredCheckState)
	{
		if (!pItem)
			return;

		std::for_each(pItem->m_childItems.begin(), pItem->m_childItems.end(), [&](CFTWTreeItem* pChildItem)
		{
			VERIFY(pChildItem);

			if (pChildItem != nullptr && !pChildItem->IsDisabled())
			{
				if (pChildItem->Type() == CFTWTreeItem::ItemType::GROUP)
					UpdateChildrenState(static_cast<CFTWTreeGroupItem*>(pChildItem), parentDesiredCheckState);
				else
					pChildItem->ParentItemClicked(parentDesiredCheckState);

				RefreshItemState(pChildItem);
			}

		});

		//RefreshParentItemState(hItem);
	}

	bool CFTWTreeCtrl::RefreshParentItemState(CFTWTreeGroupItem* pItem)
	{
		if (!pItem)
			return false;

		WORD parentCheckState = ITEMSTATE_UNDEFINED;
		bool tristate = false;

		std::for_each(pItem->m_childItems.begin(), pItem->m_childItems.end(), [&](CFTWTreeItem* pChildItem)
		{
			if (!pChildItem->IsHidden())
			{
				if (pChildItem->IsTriState() || (parentCheckState != ITEMSTATE_UNDEFINED && ((parentCheckState == ITEMSTATE_CHECKED) != pChildItem->IsChecked())))
				{
					pItem->SetTriState(true);
					tristate = true;
				}
				else if (parentCheckState == ITEMSTATE_UNDEFINED)
				{
					if (pChildItem->IsChecked())
						parentCheckState = ITEMSTATE_CHECKED;
					else
						parentCheckState = 0;
				}
			}
		});

		if (!tristate)
			pItem->SetChecked((parentCheckState & ITEMSTATE_CHECKED) == ITEMSTATE_CHECKED);

		if (RefreshParentItemState(pItem->GetParentItem()))
			RefreshItemState(pItem->GetParentItem());

		return true;
	}

	//void CFTWTreeCtrl::RefreshParentItemState(HTREEITEM hItem)
	//{
	//	VERIFY(hItem);
	//
	//	CTreeItem* pTI = (CTreeItem*)GetItemData(hItem);
	//
	//	VERIFY(hItem);
	//
	//	HTREEITEM hChildItem = GetChildItem(hItem);
	//
	//	WORD parentCheckState = ITEMSTATE_UNDEFINED;
	//	bool tristate = false;
	//
	//	while (hChildItem != NULL)
	//	{
	//		CTreeItem* pChildItem = (CTreeItem*)GetItemData(hChildItem);
	//
	//		VERIFY(pChildItem);
	//
	//		if (pChildItem->IsTriState() || (parentCheckState != ITEMSTATE_UNDEFINED && ((parentCheckState == ITEMSTATE_CHECKED) != pChildItem->IsChecked())))
	//		{
	//			pTI->SetTriState(true);
	//			tristate = true;
	//			break;
	//		}
	//		else if (parentCheckState == ITEMSTATE_UNDEFINED)
	//		{
	//			if (pChildItem->IsChecked())
	//				parentCheckState = ITEMSTATE_CHECKED;
	//			else
	//				parentCheckState = 0;
	//		}
	//
	//		hChildItem = GetNextSiblingItem(hChildItem);
	//	}
	//
	//	if (!tristate)
	//		pTI->SetChecked((parentCheckState & ITEMSTATE_CHECKED) == ITEMSTATE_CHECKED);
	//
	//	if (HTREEITEM hParent = GetParentItem(hItem))
	//	{
	//		RefreshParentItemState(hParent);
	//		RefreshItemState(hParent);
	//	}
	//}

	// CTreeItem

	HTREEITEM CFTWTreeItem::GetParentTreeHandle(void)
	{
		if (m_pParentItem)
			return m_pParentItem->GetTreeHandle();
		else return TVI_ROOT;
	};

	bool CFTWTreeItem::ItemClicked(void)
	{
		if (!IsDisabled())
		{
			ToggleChecked();

			if (IsChecked())
				SetManuallyChecked();
			else
				SetManuallyChecked(false);

			return true;
		}

		return false;
	}

	void CFTWTreeItem::ParentItemClicked(bool parentChecked)
	{
		if (!IsDisabled())
		{
			SetChecked(parentChecked);
			//SetManuallyChecked(parentChecked);
		}
	}

	bool CFTWTreeItem::IsCheckedIfToggled(void)
	{
		return (IsTriState() || !IsChecked());
	}

	void CFTWTreeItem::ItemIncluded(bool included, CFTWTreeItem* pTreeItem)
	{
		if (included)
		{
			m_includedByTreeItems.insert(pTreeItem);

			SetChecked(true);
			SetDisabled(true);
		}
		else
		{
			auto it = m_includedByTreeItems.find(pTreeItem);

			if (it != m_includedByTreeItems.end())
				m_includedByTreeItems.erase(it);

			if (m_includedByTreeItems.size() == 0)
			{
				if (!WasManuallyChecked())
					SetChecked(false);

				SetDisabled(false);
			}

		}
	}

	void CFTWTreeItem::SetChecked(bool checked)
	{
		if (!IsDisabled())
			m_itemState = ((checked ? (m_itemState | ITEMSTATE_CHECKED) : (m_itemState & ~ITEMSTATE_CHECKED)) & ~ITEMSTATE_TRISTATE);
		else
		{
			TRACE("Trying to check disabled item!");
		}
	}
}