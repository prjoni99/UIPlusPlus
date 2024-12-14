// UI++REST.h : main header file for the UI++REST DLL
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

class CImage; 
#include "resource.h"		// main symbols
#include "Actions\Action.h"

// CUIRESTApp
// See UI++REST.cpp for the implementation of this class
//

//class CUIRESTApp : public CWinApp
//{
//public:
//	CUIRESTApp();
//
//// Overrides
//public:
//	virtual BOOL InitInstance();
//
//	DECLARE_MESSAGE_MAP()
//};

const _TCHAR* const	XML_ACTION_TYPE_RESTCALL =		(_T("REST"));
const _TCHAR* const	XML_ACTION_TYPE_TOJSON =		(_T("ToJSON"));
const _TCHAR* const	XML_ACTION_TYPE_FROMJSON =		(_T("FromJSON"));

class CRESTCallAction : public UIpp::CAction
{
public:
	CRESTCallAction(const UIpp::ActionData& data) : CAction(data) { m_actionTypeName = XML_ACTION_TYPE_RESTCALL; };
	~CRESTCallAction(void) {};

	INT_PTR Go(void);
	bool IsGUIAction(void) { return false; };

};

class CToJSONAction : public UIpp::CAction
{
public:
	CToJSONAction(const UIpp::ActionData& data) : CAction(data) { m_actionTypeName = XML_ACTION_TYPE_TOJSON; };
	~CToJSONAction(void) {};

	INT_PTR Go(void);
	bool IsGUIAction(void) { return false; };

};

class CFromJSONAction : public UIpp::CAction
{
public:
	CFromJSONAction(const UIpp::ActionData& data) : CAction(data) { m_actionTypeName = XML_ACTION_TYPE_FROMJSON; };
	~CFromJSONAction(void) {};

	INT_PTR Go(void);
	bool IsGUIAction(void) { return false; };

};