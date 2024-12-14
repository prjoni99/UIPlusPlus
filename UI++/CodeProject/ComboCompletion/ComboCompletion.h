#if !defined(AFX_ComboCompletion_H__115F422E_5CD5_11D1_ABBA_00A02__INCLUDED_)
#define AFX_ComboCompletion_H__115F422E_5CD5_11D1_ABBA_00A02__INCLUDED_

#pragma once


#include <unordered_map>

// ComboCompletion.h : header file
//
// Copyright (c) Chris Maunder 1997.
// Please feel free to use and distribute.

/////////////////////////////////////////////////////////////////////////////
// CComboCompletion window

struct ComboItem
{
	ComboItem(PCTSTR d, PCTSTR v1, PCTSTR v2, COLORREF c)
		: display(d), value1(v1), value2(v2), color(c)
	{};

	CString display;
	CString value1;
	CString value2;
	COLORREF color;
};

typedef std::unordered_map<int, ComboItem> COMBOITEMS, *PCOMBOITEMS;

namespace CodeProject
{

	class CComboCompletion : public CComboBox
	{
		// Construction
	public:
		CComboCompletion();

		// Attributes
	public:

		// Operations
	public:

		// Overrides
		// ClassWizard generated virtual function overrides
		//{{AFX_VIRTUAL(CComboCompletion)
	public:
		//virtual void PreSubclassWindow();
		virtual BOOL PreTranslateMessage(MSG* pMsg);
		//}}AFX_VIRTUAL

		// Implementation
	public:
		virtual ~CComboCompletion();

		BOOL m_bAutoComplete;

		// Generated message map functions
	protected:
		void RecalcDropWidth();
		void CreateTooltip();

		//CXTipListBox	m_listbox;
		HWND			m_hWndToolTip;
		TOOLINFO		m_ToolInfo;
		bool			m_toolTipActive;
		COMBOITEMS		m_items;
		int				m_itemCount = 0;

		//{{AFX_MSG(CComboCompletion)
		afx_msg void OnEditUpdate();
//		afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
//		afx_msg void OnDestroy();
		afx_msg void OnMouseMove(UINT nFlags, CPoint point);
		afx_msg void OnTimer(UINT_PTR nIDEvent);
		//}}AFX_MSG

		DECLARE_MESSAGE_MAP()
	public:
		afx_msg void OnCbnDropdown();

		void InsertItem(const ComboItem& ci);
		virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
		virtual void MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct);
		virtual int CompareItem(LPCOMPAREITEMSTRUCT lpCompareItemStruct);
	};
}
/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately 
// before the previous line.

#endif 
