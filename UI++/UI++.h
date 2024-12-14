
// UI++.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols
#include "pugi\pugixml.hpp"
#include "Software.h"
#include "FTWCMLog.h"

#include <vector>

typedef std::vector<HINSTANCE> DLLVector;
typedef std::vector<HINSTANCE>::iterator DLLVectorIterator;


// CUIppApp:
// See UI++.cpp for the implementation of this class
//

class CUIppApp : public CWinApp
{
public:
	CUIppApp();
	~CUIppApp();

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
protected:
	//std::ofstream dbgFile;
	
	void Process(FTWCMLOG::ICMLogPtr pLog, bool isInTaskSequence, bool isInWinPE);
	void ProcessCommandLine(FTWCMLOG::ICMLogPtr pLog, CString& configFilename, CString& configFilenameFallback, int& downloadRetry, bool& disableVarEditor);
	void LoadActionLibraries(FTWCMLOG::ICMLogPtr pLog, pugi::xml_node& libraryNode);
	bool LoadConfig(FTWCMLOG::ICMLogPtr pLog, pugi::xml_document& configDoc, PCTSTR pFilename, int retry = 0, PCTSTR pAltFilename = nullptr);
	void GetSoftware(FTWCMLOG::ICMLogPtr pLog, pugi::xml_node& softwareNode, UIpp::SoftwareMap& software);
	//FTW::ComInit m_com;

	INT_PTR m_returnCode;
	DLLVector m_loadedDlls;

public:
	virtual int ExitInstance();
};

extern CUIppApp theApp;
