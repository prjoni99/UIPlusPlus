#pragma once

#include <memory>
#include <vector>
#include <unordered_set>

// CFTWTreeCtrl

namespace FTW
{

	const int TREEICON_GROUP = 0;
	const int TREEICON_GROUPLOCK = 1;
	const int TREEICON_APP = 2;
	const int TREEICON_APPLOCK = 3;
	const int TREEICON_APPLINK = 4;
	const int TREEICON_APPEXCLUDE = 5;

	enum CheckStateImageType { CSIT_NONE = 0, CSIT_UNCHECKED, CSIT_CHECKED, CSIT_TRISTATE, CSIT_UNCHECKED_DISABLED, CSIT_CHECKED_DISABLED, CSIT_TRISTATE_DISABLED };

	const WORD ITEMSTATE_CHECKED = 0x0001;
	const WORD ITEMSTATE_DISABLED = 0x0002;
	const WORD ITEMSTATE_TRISTATE = 0x0004;
	const WORD ITEMSTATE_MANUALLYCHECKED = 0x0100;
	const WORD ITEMSTATE_REQUIRED = 0x0010;
	const WORD ITEMSTATE_DEFAULT = 0x0020;
	const WORD ITEMSTATE_HIDDEN = 0x0040;
	const WORD ITEMSTATE_UNDEFINED = 0xFFFF;

	class CFTWTreeItem;
	class CFTWTreeGroupItem;
	class CFTWTreeLeafItem;

	typedef std::unique_ptr<CFTWTreeItem> CFTWTreeItemPtr;
	typedef std::vector<CFTWTreeItemPtr> FTWTreeItemList;

	//typedef std::vector<HTREEITEM> TreeItemHandleList;
	typedef std::unordered_set<CFTWTreeItem*> FTWTreeItemSet;


	class CFTWTreeItem
	{
		friend class CFTWTreeCtrl;

	public:

		CFTWTreeItem(PCTSTR label, PCTSTR id, CFTWTreeGroupItem* pParentItem = 0, bool defaultItem = false, bool requiredItem = false, bool hiddenItem = false, CString includes = _T(""))
			: m_itemLabel(label), m_itemID(id), m_pParentItem(pParentItem), m_itemState(0), m_includeStringList(includes), m_hItem(NULL)
		{
			if (requiredItem)
				m_itemState = m_itemState | ITEMSTATE_REQUIRED | ITEMSTATE_CHECKED | ITEMSTATE_DISABLED;
			else if (defaultItem)
				m_itemState = m_itemState | ITEMSTATE_DEFAULT | ITEMSTATE_CHECKED | ITEMSTATE_MANUALLYCHECKED;

			if (hiddenItem)
				m_itemState = m_itemState | ITEMSTATE_HIDDEN;
		};

		virtual ~CFTWTreeItem() {};

		enum class ItemType { GROUP = 1, LEAF };

		virtual ItemType Type(void) const = 0;

		inline PCTSTR GetLabel(void) { return m_itemLabel; };
		inline PCTSTR GetID(void) { return m_itemID; };

		inline void SetTreeHandle(HTREEITEM hItem) { m_hItem = hItem; };
		inline HTREEITEM GetTreeHandle(void) { return m_hItem; };

		HTREEITEM GetParentTreeHandle(void);
		CFTWTreeGroupItem* GetParentItem(void) { return m_pParentItem; };

		WORD GetState(void) { return m_itemState; };

		inline bool IsChecked(void) { return ((m_itemState & ITEMSTATE_CHECKED) == ITEMSTATE_CHECKED); }
		inline bool IsRequired(void) { return ((m_itemState & ITEMSTATE_REQUIRED) == ITEMSTATE_REQUIRED); }
		inline bool IsDefault(void) { return ((m_itemState & ITEMSTATE_DEFAULT) == ITEMSTATE_DEFAULT); }
		inline bool IsHidden(void) { return ((m_itemState & ITEMSTATE_HIDDEN) == ITEMSTATE_HIDDEN); }
		inline bool IsDisabled(void) { return ((m_itemState & ITEMSTATE_DISABLED) == ITEMSTATE_DISABLED); }
		inline bool IsTriState(void) { return ((m_itemState & ITEMSTATE_TRISTATE) == ITEMSTATE_TRISTATE); }
		inline bool IsIncluded(void) { return m_includedByTreeItems.size() > 0; }
		inline bool WasManuallyChecked(void) { return ((m_itemState & ITEMSTATE_MANUALLYCHECKED) == ITEMSTATE_MANUALLYCHECKED); }

		bool ItemClicked(void);
		void ParentItemClicked(bool parentChecked);

		bool IsCheckedIfToggled(void);

		//	inline void SetChecked(bool checked = true) { m_itemState = ((checked ? (m_itemState | ITEMSTATE_CHECKED) : (m_itemState & ~ITEMSTATE_CHECKED)) & ~ITEMSTATE_TRISTATE); }
		void SetChecked(bool checked = true);
		inline void SetTriState(bool tristate = true) { m_itemState = ((tristate ? (m_itemState | ITEMSTATE_TRISTATE) : (m_itemState & ~ITEMSTATE_TRISTATE)) | ITEMSTATE_CHECKED); }

		void ItemIncluded(bool included, CFTWTreeItem* pTreeItem);

		PCTSTR GetIncludeStringList(void) { return m_includeStringList; };
		void SetIncludeItemList(FTWTreeItemSet& treeItems) { m_includeTreeItems = treeItems; };
		FTWTreeItemSet* const GetIncludeItemList(void) { return &m_includeTreeItems; };

	protected:

		inline bool ToggleChecked(void) { m_itemState = m_itemState ^ ITEMSTATE_CHECKED; return IsChecked(); }

		inline void SetManuallyChecked(bool mChecked = true) { m_itemState = (mChecked ? m_itemState | ITEMSTATE_MANUALLYCHECKED : m_itemState & ~ITEMSTATE_MANUALLYCHECKED); }
		inline void SetDisabled(bool disable = true) { m_itemState = (disable ? (m_itemState | ITEMSTATE_DISABLED) : (m_itemState & ~ITEMSTATE_DISABLED)); }

		HTREEITEM m_hItem = NULL;

		CString m_itemLabel;
		CString m_itemID;
		CString m_includeStringList;

		//int m_includedCount;
		FTWTreeItemSet m_includedByTreeItems;
		FTWTreeItemSet m_includeTreeItems;

		CFTWTreeGroupItem* m_pParentItem;

		WORD m_itemState;

	};


	class CFTWTreeGroupItem : public CFTWTreeItem
	{
		friend class CFTWTreeCtrl;

	public:

		CFTWTreeGroupItem(PCTSTR label, PCTSTR id, CFTWTreeGroupItem* pParentItem = 0, bool defaultItem = false, bool requiredItem = false)
			: CFTWTreeItem(label, id, pParentItem, defaultItem, requiredItem) {};

		~CFTWTreeGroupItem() {};

		ItemType Type(void) const { return CFTWTreeItem::ItemType::GROUP; };

		void AddChildItem(CFTWTreeItem* pChild) { if (pChild) m_childItems.insert(pChild); };

	protected:

		FTWTreeItemSet m_childItems;
	};

	class CFTWTreeLeafItem : public CFTWTreeItem
	{
		friend class CFTWTreeCtrl;

	public:

		CFTWTreeLeafItem(PCTSTR label, PCTSTR id, CFTWTreeGroupItem* pParentItem = 0, bool defaultItem = false, bool requiredItem = false, bool hiddenItem = false, CString includes = _T(""))
			: CFTWTreeItem(label, id, pParentItem, defaultItem, requiredItem, hiddenItem, includes) {};

		~CFTWTreeLeafItem() {};

		ItemType Type(void) const { return ItemType::LEAF; };


	protected:

	};

	class CFTWTreeCtrl : public CTreeCtrl
	{
		DECLARE_DYNAMIC(CFTWTreeCtrl)

	public:
		CFTWTreeCtrl();
		virtual ~CFTWTreeCtrl();

		//Initialize the tree including checkbox imagelist and the icon image list
		void Init(UINT id = 0);

		afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
		afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
		afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);

		void ExpandAll(void);
		void ExpandItemandChildren(HTREEITEM hItem, UINT code = TVE_EXPAND);

		void ItemClicked(HTREEITEM hItem);

		void RefreshItemState(CFTWTreeItem* pItem, FTWTreeItemSet* pItemIncludedChain = 0);
		//void RefreshItemState(HTREEITEM hItem, TreeItemSet* pItemIncludedChain = 0);

	protected:
		DECLARE_MESSAGE_MAP()

		virtual void PreSubclassWindow();

		void UpdateChildrenState(CFTWTreeGroupItem* pItem, bool parentDesiredCheckState);
		//void UpdateChildrenState(HTREEITEM hItem, bool parentDesiredCheckState);
		bool RefreshParentItemState(CFTWTreeGroupItem* pItem);
		//void RefreshParentItemState(HTREEITEM hItem);

		CImageList m_checkboxImageList;
		CImageList m_itemImageList;

	};
}