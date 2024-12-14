// ComboCompletion.cpp : implementation file
//
// Copyright (c) Chris Maunder 1997.
// Please feel free to use and distribute.

#include "stdafx.h"
#include "ComboCompletion.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

namespace CodeProject
{

	/////////////////////////////////////////////////////////////////////////////
	// CComboCompletion

	CComboCompletion::CComboCompletion()
	{
		m_bAutoComplete = TRUE;
		m_hWndToolTip = NULL;
		m_toolTipActive = false;
	}

	CComboCompletion::~CComboCompletion()
	{
	}


	BEGIN_MESSAGE_MAP(CComboCompletion, CComboBox)
		//{{AFX_MSG_MAP(CComboCompletion)
		ON_CONTROL_REFLECT(CBN_EDITUPDATE, OnEditUpdate)
		//		ON_WM_CTLCOLOR()
		//		ON_WM_DESTROY()
		ON_WM_MOUSEMOVE()
		//}}AFX_MSG_MAP
		ON_CONTROL_REFLECT(CBN_DROPDOWN, &CComboCompletion::OnCbnDropdown)
		ON_WM_TIMER()
	END_MESSAGE_MAP()

	/////////////////////////////////////////////////////////////////////////////
	// CComboCompletion message handlers

	BOOL CComboCompletion::PreTranslateMessage(MSG* pMsg)
	{
		// Need to check for backspace/delete. These will modify the text in
		// the edit box, causing the auto complete to just add back the text
		// the user has just tried to delete. 

		if (pMsg->message == WM_KEYDOWN)
		{
			m_bAutoComplete = TRUE;

			int nVirtKey = (int)pMsg->wParam;
			if (nVirtKey == VK_DELETE || nVirtKey == VK_BACK)
				m_bAutoComplete = FALSE;
		}

		return CComboBox::PreTranslateMessage(pMsg);
	}

	void CComboCompletion::RecalcDropWidth()
	{
		// Reset the dropped width
		int nNumEntries = GetCount();
		int nWidth = 0;
		CString str;

		CClientDC dc(this);
		int nSave = dc.SaveDC();
		dc.SelectObject(GetFont());

		int nScrollWidth = ::GetSystemMetrics(SM_CXVSCROLL);
		for (int i = 0; i < nNumEntries; i++)
		{
			GetLBText(i, str);
			int nLength = dc.GetTextExtent(str).cx + nScrollWidth;
			nWidth = max(nWidth, nLength);
		}

		// Add margin space to the calculations
		nWidth += dc.GetTextExtent(_T("0")).cx;

		dc.RestoreDC(nSave);
		SetDroppedWidth(nWidth);
	}

	void CComboCompletion::CreateTooltip()
	{
		if (m_hWndToolTip != NULL)
			return;

		// create tooltip
		m_hWndToolTip = ::CreateWindowEx(WS_EX_TOPMOST,
			TOOLTIPS_CLASS,
			NULL,
			TTS_NOPREFIX | TTS_ALWAYSTIP,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			m_hWnd,
			NULL,
			NULL,
			NULL);
		ASSERT(m_hWndToolTip);

		// initialize toolinfo struct
		memset(&m_ToolInfo, 0, sizeof(m_ToolInfo));
		m_ToolInfo.cbSize = sizeof(m_ToolInfo);
		m_ToolInfo.uFlags = TTF_TRACK | TTF_TRANSPARENT;
		m_ToolInfo.hwnd = m_hWnd;

		// add combo box
		::SendMessage(m_hWndToolTip, TTM_SETMAXTIPWIDTH, 0, SHRT_MAX);
		::SendMessage(m_hWndToolTip, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&m_ToolInfo);
		::SendMessage(m_hWndToolTip, TTM_SETTIPBKCOLOR, ::GetSysColor(COLOR_HIGHLIGHT), 0);
		::SendMessage(m_hWndToolTip, TTM_SETTIPTEXTCOLOR, ::GetSysColor(COLOR_HIGHLIGHTTEXT), 0);

		// reduce top & bottom margins
		CRect rectMargins(0, -1, 0, -1);
		::SendMessage(m_hWndToolTip, TTM_SETMARGIN, 0, (LPARAM)&rectMargins);

		// set font
		CFont *pFont = GetFont();
		::SendMessage(m_hWndToolTip, WM_SETFONT, (WPARAM)(HFONT)*pFont, FALSE);
	}

	void CComboCompletion::OnEditUpdate()
	{
		// if we are not to auto update the text, get outta here
		if (!m_bAutoComplete)
			return;

		// Get the text in the edit box
		CString str;
		GetWindowText(str);
		int nLength = str.GetLength();

		// Currently selected range
		DWORD dwCurSel = GetEditSel();
		WORD dStart = LOWORD(dwCurSel);
		WORD dEnd = HIWORD(dwCurSel);

		// Search for, and select in, and string in the combo box that is prefixed
		// by the text in the edit box
		if (SelectString(-1, str) == CB_ERR)
		{
			SetWindowText(str);            // No text selected, so restore what 
										   // was there before
			if (dwCurSel != CB_ERR)
				SetEditSel(dStart, dEnd);    //restore cursor postion
		}

		// Set the text selection as the additional text that we have added
		if (dEnd < nLength && dwCurSel != CB_ERR)
			SetEditSel(dStart, dEnd);
		else
			SetEditSel(nLength, -1);
	}

	void CComboCompletion::OnCbnDropdown()
	{
		RecalcDropWidth();
	}

	//void CComboCompletion::PreSubclassWindow()
	//{
	//	TRACE(_T("in CComboCompletion::PreSubclassWindow\n"));

	//	CComboBox::PreSubclassWindow();


	//}

	///////////////////////////////////////////////////////////////////////////////
	// OnCtlColor
	//HBRUSH CXTipComboBox::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
	//{
	//	if (nCtlColor == CTLCOLOR_LISTBOX)
	//	{
	//		if (m_listbox.GetSafeHwnd() == NULL)
	//		{
	//			TRACE(_T("subclassing listbox\n"));
	//			m_listbox.SubclassWindow(pWnd->GetSafeHwnd());
	//		}
	//	}
	//	HBRUSH hbr = CComboBox::OnCtlColor(pDC, pWnd, nCtlColor);
	//	return hbr;
	//}

	///////////////////////////////////////////////////////////////////////////////
	// OnDestroy
	//void CComboCompletion::OnDestroy()
	//{
	//	if (m_listbox.GetSafeHwnd() != NULL)
	//		m_listbox.UnsubclassWindow();

	//	CComboBox::OnDestroy();
	//}

	///////////////////////////////////////////////////////////////////////////////
	// OnMouseMove
	void CComboCompletion::OnMouseMove(UINT nFlags, CPoint point)
	{
		CRect rectClient;
		GetClientRect(&rectClient);
		int nComboButtonWidth = ::GetSystemMetrics(SM_CXHTHUMB) + 2;
		rectClient.right = rectClient.right - nComboButtonWidth;

		if (rectClient.PtInRect(point))
		{
			CreateTooltip();

			TRACE(_T("in ccombo\n"));
			ClientToScreen(&rectClient);

			CString strText = _T("");
			GetWindowText(strText);
			m_ToolInfo.lpszText = (LPTSTR)(LPCTSTR)strText;

			HDC hDC = ::GetDC(m_hWnd);
			ASSERT(hDC);

			CFont *pFont = GetFont();
			HFONT hOldFont = (HFONT) ::SelectObject(hDC, (HFONT)*pFont);

			SIZE size;
			::GetTextExtentPoint32(hDC, strText, strText.GetLength(), &size);
			::SelectObject(hDC, hOldFont);
			::ReleaseDC(m_hWnd, hDC);

			if (size.cx > (rectClient.Width() - 6))
			{
				if (!m_toolTipActive)
				{
					rectClient.left += 1;
					rectClient.top += 3;

					COLORREF rgbText = ::GetSysColor(COLOR_WINDOWTEXT);
					COLORREF rgbBackground = ::GetSysColor(COLOR_WINDOW);

					CWnd *pWnd = GetFocus();
					if (pWnd)
					{
						if (pWnd->m_hWnd == m_hWnd)
						{
							rgbText = ::GetSysColor(COLOR_HIGHLIGHTTEXT);
							rgbBackground = ::GetSysColor(COLOR_HIGHLIGHT);
						}
					}

					::SendMessage(m_hWndToolTip, TTM_SETTIPBKCOLOR, rgbBackground, 0);
					::SendMessage(m_hWndToolTip, TTM_SETTIPTEXTCOLOR, rgbText, 0);
					::SendMessage(m_hWndToolTip, TTM_UPDATETIPTEXT, 0, (LPARAM)&m_ToolInfo);
					::SendMessage(m_hWndToolTip, TTM_TRACKPOSITION, 0,
						(LPARAM)MAKELONG(rectClient.left, rectClient.top));
					::SendMessage(m_hWndToolTip, TTM_TRACKACTIVATE, TRUE, (LPARAM)(LPTOOLINFO)&m_ToolInfo);
				}

				TRACE(_T("setting timer\n"));
				SetTimer(1, 80, NULL);	// set timer for out-of-rect detection
				m_toolTipActive = true;
			}
			else	// text fits inside client rect
			{
				::SendMessage(m_hWndToolTip, TTM_TRACKACTIVATE, FALSE, (LPARAM)(LPTOOLINFO)&m_ToolInfo);
			}
		}
		else	// not inside client rect
		{
			::SendMessage(m_hWndToolTip, TTM_TRACKACTIVATE, FALSE, (LPARAM)(LPTOOLINFO)&m_ToolInfo);
		}

		CComboBox::OnMouseMove(nFlags, point);
	}

	///////////////////////////////////////////////////////////////////////////////
	// OnTimer
	void CComboCompletion::OnTimer(UINT_PTR nIDEvent)
	{
		CPoint point;
		::GetCursorPos(&point);
		ScreenToClient(&point);

		CRect rectClient;
		GetClientRect(&rectClient);
		int nComboButtonWidth = ::GetSystemMetrics(SM_CXHTHUMB) + 2;

		rectClient.right = rectClient.right - nComboButtonWidth;

		if (!rectClient.PtInRect(point))
		{
			TRACE(_T("killing timer\n"));
			KillTimer(nIDEvent);

			::SendMessage(m_hWndToolTip, TTM_TRACKACTIVATE, FALSE, (LPARAM)(LPTOOLINFO)&m_ToolInfo);

			m_toolTipActive = false;
		}
	}

	void CComboCompletion::InsertItem(const ComboItem& ci)
	{
		m_items.insert({ m_itemCount, ci });
		int index = AddString(ci.display);

		if (index == CB_ERR || index == CB_ERRSPACE)
		{
			throw;
		}
		else
		{
			SetItemData(index, m_itemCount++);
		}
	}

	void CComboCompletion::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
	{
		ASSERT(lpDrawItemStruct->CtlType == ODT_COMBOBOX);

		UINT itemToDraw = lpDrawItemStruct->itemID;

		if (itemToDraw == CB_ERR)
			itemToDraw = GetCurSel();

		CString str;
		//GetLBText(lpDrawItemStruct->itemID, str);
		//LPCTSTR lpszText = (LPCTSTR)lpDrawItemStruct->itemData;

		CDC dc;
		dc.Attach(lpDrawItemStruct->hDC);

		// Save these value to restore them when done drawing.
		COLORREF crOldTextColor = dc.GetTextColor();
		COLORREF crOldBkColor = dc.GetBkColor();

		// If this item is selected, set the background color 
		// and the text color to appropriate values. Erase
		// the rect by filling it with the background color.
		if ((lpDrawItemStruct->itemAction | ODA_SELECT) &&
			(lpDrawItemStruct->itemState  & ODS_SELECTED))
		{
			dc.SetTextColor(::GetSysColor(COLOR_HIGHLIGHTTEXT));
			dc.SetBkColor(::GetSysColor(COLOR_HIGHLIGHT));
			dc.FillSolidRect(&lpDrawItemStruct->rcItem, ::GetSysColor(COLOR_HIGHLIGHT));
		}
		else
			dc.FillSolidRect(&lpDrawItemStruct->rcItem, crOldBkColor);

		CRect rect(lpDrawItemStruct->rcItem);
		rect.DeflateRect(5, 0);

		auto item = m_items.find(lpDrawItemStruct->itemID);

		if (item != m_items.end())
		{
			str = L" >> ";
			str += item->second.display;
		}

		// Draw the text.
		dc.DrawText(
			str,
			-1,
			&rect,
			DT_LEFT | DT_SINGLELINE);// | DT_VCENTER);

		// Reset the background color and the text color back to their
		// original values.
		dc.SetTextColor(crOldTextColor);
		dc.SetBkColor(crOldBkColor);

		dc.Detach();
	}

	void CComboCompletion::MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct)
	{
		if (lpMeasureItemStruct->itemID != CB_ERR)
		{
			CString str = L"abcdefghijklmnopqrstuvwxyz";

			CSize   sz;
			CDC*    pDC = GetDC();

			sz = pDC->GetTextExtent(str);

			ReleaseDC(pDC);

			lpMeasureItemStruct->itemHeight = sz.cy;
		}
	}

	int CComboCompletion::CompareItem(LPCOMPAREITEMSTRUCT lpCompareItemStruct)
	{

		// return -1 = item 1 sorts before item 2
		// return 0 = item 1 and item 2 sort the same
		// return 1 = item 1 sorts after item 2

		CString str1, str2;

		auto item1 = m_items.find(lpCompareItemStruct->itemData1);

		if (item1 != m_items.end())
			str1 = item1->second.display;

		TRACE(L"%d\n", lpCompareItemStruct->itemID2);

		auto item2 = m_items.find(m_itemCount);//lpCompareItemStruct->itemData2);

		if (item2 != m_items.end())
			str2 = item2->second.display;

		int compare = wcscmp(str1, str2);

		return compare;
	}
}