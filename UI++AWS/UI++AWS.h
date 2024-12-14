// UI++AWS.h : main header file for the UI++AWS DLL
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

class CImage;
#include "resource.h"		// main symbols
#include "Actions\Action.h"

// CUIAWSApp
// See UI++AWS.cpp for the implementation of this class
//

//class CUIAWSApp : public CWinApp
//{
//public:
//	CUIAWSApp();
//
//// Overrides
//public:
//	virtual BOOL InitInstance();
//
//	DECLARE_MESSAGE_MAP()
//};

const _TCHAR* const	XML_ACTION_TYPE_AWSUPLOADINFO = (_T("AWS"));

class CAWSUploadAction : public UIpp::CAction
{
public:
	CAWSUploadAction(const UIpp::ActionData& data) : CAction(data) { m_actionTypeName = XML_ACTION_TYPE_AWSUPLOADINFO; };
	~CAWSUploadAction(void) {};

	INT_PTR Go(void);
	bool IsGUIAction(void) { return false; };

};
