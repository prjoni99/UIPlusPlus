#pragma once

#include "HansDietrich\XColorStatic.h"
#include "RegExEdit.h"
#include "CodeProject\ComboCompletion\ComboCompletion.h"
#include "FTW\FTWBrowseEdit.h"
#include "TooltipStatic.h"
#include "Constants.h"

#include <map>
#include <forward_list>

namespace UIpp
{

class CUserInputBase
{
public:
	CUserInputBase(PCTSTR question, PCTSTR valueName, PCTSTR fontFace, PCTSTR altValueName = 0, int numLines = 2)
		: m_text(question), m_valueName(valueName), m_altValueName(altValueName), m_fontFaceName(fontFace),
		m_pFontx(0), m_numberofTextLines(numLines), m_textColor(RGB(0, 0, 0)) {};

	CUserInputBase(PCTSTR question, PCTSTR fontFace, int numLines = 2)
		: m_text(question), m_valueName(_T("")), m_altValueName(_T("")), m_fontFaceName(fontFace),
		m_pFontx(0), m_numberofTextLines(numLines), m_textColor(RGB(0, 0, 0)) {};

	enum class InputType { TextInput = 1, CheckboxInput, ComboInput, InputInfo, CheckInfo, ComboTree };
	enum class StatusType { NotChecked = 0, Success, Warning, Failed, Attention };
	
	virtual ~CUserInputBase(void) {};
	virtual bool Create(CRect rect, CWnd* pParentWnd, UINT& uID, LONG lineHeight, LONG spacing = 0, CFont* pFont = 0);
	virtual void ShowInput(bool show = true);
	virtual int MoveInput(int y, int h);

	virtual bool IsInputValid() = 0;
	virtual bool IsInputStatusWarning() { return false; };
	virtual const CWnd* GetControl(void) const { return nullptr; };

	virtual void ClearInput(void) {};
	virtual void Disable(bool disable = true) {};
	virtual PCTSTR GetValue(bool alternate = false) { return _T(""); };

	virtual const CString GetErrorMessage(void) const {
		return _T("");
	};

	virtual const CString GetIdentifier(void) const {
		return m_text;
	};

	virtual const int ErrorState(void) const { return 0; }

	bool IsInputVisible(void) const { return m_inputVisible; }

	StatusType GetStatus(void) const { return m_status; }
	StatusType SetStatus(StatusType newStatus);

	PCTSTR GetValueName(bool alternate = false) const
	{
		if (!alternate)
			return (PCTSTR)m_valueName;
		else
			return (PCTSTR)m_altValueName;
	};

	int GetNumberofTextLines(void) 
	{
		return m_numberofTextLines;
	};

	virtual const int GetInputHeight(void) {
		return m_lineHeight;
	};

	//virtual HWND GetHWnd(void) { return m_textControl.GetSafeHwnd(); };
	virtual bool HideValue(void) { return false; }
	virtual InputType GetType(void) = 0;

protected:
	CRect GetStatusRect(CRect r, LONG s)
	{
		CRect r2;

		r2.left = r.right + s;
		r2.right = r2.left + 24;
		r2.top = r.top + ((r.bottom - r.top) / 2) - 12;
		r2.bottom = r2.top + 24;

		return r2;
	}

	void CreateTextControl(CRect r, CWnd* pW, PCTSTR tooltipText = nullptr);
	
	CString m_text;
	CString m_valueName;
	CString m_altValueName;

	CFont* m_pFontx;
	CString m_fontFaceName;

	HansDietrich::CXColorStatic m_textControl;
	FTW::CTooltipStatic m_statusIndicator;
	FTW::CTooltipStatic m_textInfo;
	HICON m_statusIcon = NULL;
	HICON m_textInfoIcon = NULL;

	bool m_hasInfoIconTootip = false;
	bool m_hasStatusIndicator = false;
	bool m_inputVisible = false;

	COLORREF m_textColor;

	int m_numberofTextLines;
	int m_lineHeight;
	StatusType m_status = StatusType::NotChecked;
};


//class CUserInputBrowse : public CUserInputBase
//{
//public:
//
//	CUserInputBrowse(PCTSTR question, PCTSTR value) : m_pBrowseEdit(nullptr), CUserInputBase (question, value) {};
//	~CUserInputBrowse() { if (m_pBrowseEdit) delete m_pBrowseEdit;  };
//
//	bool Create(CRect& rect, CWnd* pParentWnd, UINT& uID, LONG height = 0, LONG spacing = 0, CFont* pFont = 0);
//
//	bool IsInputValid()
//	{
//		if (m_pBrowseEdit)
//		{
//			return CUserInputBase::SetValidIndicator(m_pBrowseEdit->IsInputValid());
//		}
//
//		else return true;
//	}
//
//	ItemType GetType(void) { return CUserInputBase::ItemType::ComboTree; };
//
//protected:
//	CFTWBrowseEdit* m_pBrowseEdit;
//};
}