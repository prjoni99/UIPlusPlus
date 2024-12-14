
// UI++.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "UI++.h"
#include "Dialogs\DlgUserInfo.h"
#include "CodeProject\CmdLine\CmdLine.h"
#include "CodeProject\regkey.h"
#include "FTW\FTWHTTP.h"
#include "FTW\ComInit.h"
#include "FTW\TSProgress.h"
#include "Actions\Actions.h"
#include "ScriptHost.h"
#include "TSVar.h"
#include "version.h"

#include <stack>

extern HMODULE hMod;

using namespace UIpp;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CUIppApp

BEGIN_MESSAGE_MAP(CUIppApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()

// CUIppApp construction

CUIppApp::CUIppApp() : m_returnCode(0)
{
	TRACE("Starting UI++\n");
	//dbgFile.open("x:\\ui++dbg.log");
	//dbgFile << "Starting UI++\n";
	//dbgFile.flush();

	// TODO: add construction code here,
	// Place all significant initialization in InitInstance

}

CUIppApp::~CUIppApp()
{
	//dbgFile.close();
	
	std::for_each(m_loadedDlls.begin(), m_loadedDlls.end(), [](HINSTANCE& hDll)
	{
		if (hDll != NULL)
			FreeLibrary(hDll);
	});
}


// The one and only CUIppApp object

CUIppApp theApp;


// CUIppApp initialization

BOOL CUIppApp::InitInstance()
{
	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.

	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	 //Set this to include all the common control classes you want to use
	 //in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();

	//AfxEnableControlContainer();

	HRESULT hr = FTW::ComInit::Instance().InitSecurity();

	bool inWinPE = false;
	bool inTaskSequence = false;


	if (hr == S_OK)
	{
		hMod = AfxGetInstanceHandle();

		CString ver (STRPRODUCTVER);
		ver.Replace(_T(", "), _T("."));

		//FTW::CCMLog::Instance().OpenLog (_T("UI++"), CTSEnv::Instance().GetLogPath());

		FTWCMLOG::ICMLogPtr pCMLog(FTWCMLOG::GetCMLog(L"UI++", CTSEnv::Instance().GetLogPath()), std::mem_fn(&FTWCMLOG::ICMLog::Release));

		pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info,
			AfxGetThread()->m_nThreadID, __TFILE__, __LINE__, 
			FTW::FormatResourceString(IDS_LOGMSG_START));

		pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info,
			AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
			FTW::FormatResourceString(IDS_LOGMSG_VERSION, ver));

		if (CTSEnv::Instance().InTS())
		{
			pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info,
				AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
				FTW::FormatResourceString(IDS_LOGMSG_INTS));

			inTaskSequence = true;
		}
		else
		{
			pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info,
				AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
				FTW::FormatResourceString(IDS_LOGMSG_NOTINTS));
 		}

		CodeProject::RegKey hklm(HKEY_LOCAL_MACHINE);

		if (hklm.Open(_T("System\\CurrentControlSet\\Control\\MiniNT"), KEY_READ) == ERROR_SUCCESS)
		{
			pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info,
				AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
				FTW::FormatResourceString(IDS_LOGMSG_INWINPE));

			inWinPE = true;

			hklm.Close();
		}
		else
		{
			pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info,
				AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
				FTW::FormatResourceString(IDS_LOGMSG_NOTINWINPE));
		}
		
		//if (CTime::GetCurrentTime().GetYear() > 2014)
		//	FTW::CCMLog::Instance().LogMsg (FTW::CCMLog::Info, 
		//		AfxGetThread()->m_nThreadID, 
		//		__TFILE__, 
		//		__LINE__, 
		//		IDS_LOGMSG_EXPIRED);
		//else
			
		Process(pCMLog, inTaskSequence, inWinPE);

 		return TRUE;
	}
	else
	{
		FTWCMLOG::CMLOGHANDLE cmLog = FTWCMLOG::GetCMLog(L"UI++2", L"");
		
		cmLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Error,
			AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
			FTW::FormatResourceString(IDS_LOGERROR_COM));
 	}

	//CInfoDlg dlg;
	//m_pMainWnd = &dlg;
	//INT_PTR nResponse = dlg.DoModal();
	//if (nResponse == IDOK)
	//{
	//	// TODO: Place code here to handle when the dialog is
	//	//  dismissed with OK
	//}
	//else if (nResponse == IDCANCEL)
	//{
	//	// TODO: Place code here to handle when the dialog is
	//	//  dismissed with Cancel
	//}

	return TRUE;
}

void CUIppApp::ProcessCommandLine(FTWCMLOG::ICMLogPtr pLog, CString& configFilename, CString& configFilenameFallback, int& downloadRetry, bool& disableVarEditor)
{
	CString commandlineOption;

	//FTW::CCMLog::Instance().LogMsg(FTW::CCMLog::Info,
	//	AfxGetThread()->m_nThreadID, __TFILE__, __LINE__, IDS_LOGMSG_PARAMETERS);

	CodeProject::CCmdLine cmdLine(::GetCommandLine());
	if(commandlineOption.LoadString(IDS_COMMANDLINE_CONFIG) && (cmdLine.HasKey(commandlineOption)))
	{
		cmdLine.Lookup(commandlineOption, configFilename);

		pLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info,
			AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
			FTW::FormatResourceString(IDS_LOGMSG_PARAMETERSTRINGVALUE, commandlineOption, configFilename));
	}

	if(commandlineOption.LoadString(IDS_COMMANDLINE_CONFIGFALLBACK) && (cmdLine.HasKey(commandlineOption)))
	{
		cmdLine.Lookup(commandlineOption, configFilenameFallback);

		pLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info,
			AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
			FTW::FormatResourceString(IDS_LOGMSG_PARAMETERSTRINGVALUE, commandlineOption, configFilenameFallback));
	}

	if(commandlineOption.LoadString(IDS_COMMANDLINE_CONFIGRETRY) && (cmdLine.HasKey(commandlineOption)))
	{
		CString configRetry;

		cmdLine.Lookup(commandlineOption, configRetry);
		downloadRetry = _ttoi(configRetry);

		pLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info,
			AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
			FTW::FormatResourceString(IDS_LOGMSG_PARAMETERINTVALUE, commandlineOption, configRetry, downloadRetry));
	}

	if(commandlineOption.LoadString(IDS_COMMANDLINE_DISABLE_TSVAREDITOR))
		disableVarEditor = (bool)!cmdLine.HasKey(commandlineOption);

}

void CUIppApp::Process(FTWCMLOG::ICMLogPtr pLog, bool isInTaskSequence, bool isInWinPE)
{
	CString configFilename = DEFAULT_CONFIG_FILENAME;
	CString configFilenameFallback = DEFAULT_CONFIG_FILENAME;
	int downloadRetry = 0;
	bool disableTSVarEditor = false;

	pugi::xml_document config;

	ProcessCommandLine(pLog, configFilename, configFilenameFallback, downloadRetry, disableTSVarEditor);
	
	if (LoadConfig(pLog, config, configFilename, downloadRetry, configFilenameFallback))
	{
		CString title = GetXMLAttribute(config.child(XML_ELEMENT_ROOT), XML_ATTRIBUTE_TITLE, XML_ATTRIBUTE_TITLE_DEF);
		CString subtitle = GetXMLAttribute(config.child(XML_ELEMENT_ROOT), XML_ATTRIBUTE_SUBTITLE);
		CString icon = GetXMLAttribute(config.child(XML_ELEMENT_ROOT), XML_ATTRIBUTE_ICON);
		CString font = GetXMLAttribute(config.child(XML_ELEMENT_ROOT), XML_ATTRIBUTE_FONT, DEFAULT_FONTFACE);
		COLORREF color = FTW::HexToCOLORREF(GetXMLAttribute(config.child(XML_ELEMENT_ROOT), XML_ATTRIBUTE_COLOR, XML_ATTRIBUTE_ACCENTCOLOR_DEF));
		bool showIcons = FTW::IsTrue(GetXMLAttribute(config.child(XML_ELEMENT_ROOT), XML_ATTRIBUTE_DIALOGICONS, XML_ACTION_TRUE));
		bool showSidebar = FTW::IsTrue(GetXMLAttribute(config.child(XML_ELEMENT_ROOT), XML_ATTRIBUTE_DIALOGSIDEBAR, XML_ACTION_TRUE));
		COLORREF textcolor = FTW::HexToCOLORREF(GetXMLAttribute(config.child(XML_ELEMENT_ROOT), XML_ATTRIBUTE_SIDEBARTEXTCOLOR, XML_ATTRIBUTE_SIDEBARTEXTCOLOR_DEF));
		bool flatDialogs = FTW::IsTrue(GetXMLAttribute(config.child(XML_ELEMENT_ROOT), XML_ATTRIBUTE_FLATDIALOGS, XML_ACTION_FALSE));
		bool alwaysOnTop = FTW::IsTrue(GetXMLAttribute(config.child(XML_ELEMENT_ROOT), XML_ATTRIBUTE_ALWAYSONTOP, XML_ACTION_TRUE));
		bool allowBack = false;
		bool allowRefresh = false;
		HICON hIcon = 0;
		int xDPI = 96, yDPI = 96;

		if (icon)
		{
			hIcon = (HICON) LoadImage(AfxGetInstanceHandle(), icon, IMAGE_ICON, 32, 32, LR_LOADFROMFILE);
 		}

		HDC hDC = GetDC(NULL);

		if (hDC)
		{
			xDPI = GetDeviceCaps(hDC, LOGPIXELSX);
			yDPI = GetDeviceCaps(hDC, LOGPIXELSY);
		}

		try
		{
			CString actionType, actionName, actionCondition;
			FTW::CScriptHost activeScriptHost;
			FTWLDAP::ILdapPtr pLdap(FTWLDAP::GetLdap(pLog), std::mem_fn(&FTWLDAP::ILdap::Release));
			std::stack<pugi::xml_node> lastGUIAction;
			SoftwareMap definedSoftware;
			
			bool isAction = false, isGroup = false;
			const _TCHAR* elementType = nullptr;
			const _TCHAR* elementName = nullptr;

			pLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info,
				AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
				FTW::FormatResourceString(IDS_LOGMSG_LOADINGVBSCRIPT, FTW::FormatResourceString(IDS_MSGSUCCESS).c_str(), _T("")));

			pLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info,
				AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
				FTW::FormatResourceString(IDS_LOGMSG_BEGINPARSING));

			pugi::xml_node softwareNode = config.child(XML_ELEMENT_ROOT).child(XML_ELEMENT_SOFTWARE);

			//LoadActionLibraries(pLog, config.child(XML_ELEMENT_ROOT).child(XML_ELEMENT_LIBRARIES));
			GetSoftware(pLog, softwareNode, definedSoftware);

			pugi::xml_node xmlActions = config.child(XML_ELEMENT_ROOT).child(XML_ELEMENT_ACTIONS);
			pugi::xml_node xmlCurrentActionorGroup = xmlActions.first_child();

			ActionData actionData(&xmlCurrentActionorGroup, 
				&activeScriptHost,
				isInTaskSequence, 
				isInWinPE,
				&definedSoftware,
				title, 
				subtitle, 
				font, 
				hIcon, 
				color,
				textcolor,
				pLog, 
				&(CTSEnv::Instance()),
				FTW::Scale(xDPI, yDPI),
				pLdap,
				config.child(XML_ELEMENT_ROOT).child(XML_ELEMENT_MESSAGES),
				FTW::DialogTraits::BuildDialogTraitFlags(showIcons, 
					flatDialogs, 
					alwaysOnTop,
					disableTSVarEditor, 
					showSidebar, 
					allowBack, 
					allowRefresh));

			while(xmlCurrentActionorGroup)
			{
				isAction = (_tcsicmp(xmlCurrentActionorGroup.name(), XML_ELEMENT_ACTION) == 0);
				isGroup = (_tcsicmp(xmlCurrentActionorGroup.name(), XML_ELEMENT_ACTIONGROUP) == 0);
				
				if (isAction || isGroup)
				{
					actionName = xmlCurrentActionorGroup.attribute(XML_ATTRIBUTE_NAME).value();
					actionCondition = xmlCurrentActionorGroup.attribute(XML_ATTRIBUTE_CONDITION).value();

					if (isGroup)
					{
						pLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info,
							AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
							FTW::FormatResourceString(IDS_LOGMSG_FOUNDACTIONGROUP, actionName));

						elementType = XML_ELEMENT_ACTIONGROUP;
						elementName = XML_ACTION_BLANK;
					}
					else if (isAction)
					{
						actionType = xmlCurrentActionorGroup.attribute(XML_ATTRIBUTE_TYPE).value();

						pLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info,
							AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
							FTW::FormatResourceString(IDS_LOGMSG_FOUNDACTION, actionType, actionName));

						elementType = actionType;
						elementName = XML_ELEMENT_ACTION;
					}

					if (CActionHelper::EvalCondition(pLog, actionCondition, &activeScriptHost, elementType, elementName))
					{
						if (isGroup)
						{
							if (xmlCurrentActionorGroup.first_child())
							{
								xmlCurrentActionorGroup = xmlCurrentActionorGroup.first_child();
								continue;
							}
						}
						else if (isAction)
						{
							TRACE1("New Action: <%s>\n", actionType);

							allowRefresh = (_tcsicmp(xmlCurrentActionorGroup.parent().name(), XML_ELEMENT_ACTIONGROUP) == 0 &&
								xmlCurrentActionorGroup.parent().first_child() != xmlCurrentActionorGroup);
							allowBack = (!lastGUIAction.empty());

							actionData.globalDialogTraits.AllowBack(allowBack);
							actionData.globalDialogTraits.AllowRefresh(allowRefresh);

							try
							{
								actionData.pActionNode = &xmlCurrentActionorGroup;

								std::unique_ptr<IAction> action(CActionFactory::Instance().Create(actionType.GetString(), actionData));

								pLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info,
									AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
									FTW::FormatResourceString(IDS_LOGMSG_INITIATEACTION, actionType, actionName));

								if (action->IsGUIAction() && CTSEnv::Instance().InTS())
									CTSProgress::Instance().Close();

								m_returnCode = action->Go();

								if (m_returnCode == UI_BACK /*&& !lastGUIAction.empty()*/)
								{
									if (allowBack)
									{
										if(xmlCurrentActionorGroup == lastGUIAction.top())
											lastGUIAction.pop();

										xmlCurrentActionorGroup = lastGUIAction.top();
										lastGUIAction.pop();

										pLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info,
											AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
											FTW::FormatResourceString(IDS_LOGMSG_BACKACTION));

										continue;
									}
								}
								else if (m_returnCode == UI_RETRY)
								{ 
									if (allowRefresh)
									{
										xmlCurrentActionorGroup = xmlCurrentActionorGroup.parent();

										pLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info,
											AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
											FTW::FormatResourceString(IDS_LOGMSG_REFRESHACTION, xmlCurrentActionorGroup.attribute(XML_ATTRIBUTE_NAME).value()));

										continue;
									}
								}
								else if (m_returnCode != ERROR_SUCCESS)
								{
									pLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info,
										AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
										FTW::FormatResourceString(IDS_LOGMSG_CANCELACTION));

									break;
								}
								else if (action->IsGUIAction())
								{
									pLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info,
										AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
										FTW::FormatResourceString(IDS_LOGMSG_OKACTION));

									lastGUIAction.push(xmlCurrentActionorGroup);

								}
								else
								{
								}
							}
							catch (std::runtime_error&)
							{
								pLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Warning,
									AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
									FTW::FormatResourceString(IDS_LOGMSG_UNKNOWNACTION, actionType));
							}
							catch (FTW::FTWException& q)
							{
								pLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Error,
									AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
									FTW::FormatResourceString(IDS_LOGERROR_ACTIONEXECUTION, q.Message(), q.DataMessage()));
							}
						}
					}
					else
					{
						if(isGroup)
							pLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info,
								AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
								FTW::FormatResourceString(IDS_LOGMSG_SKIPPINGACTIONGROUP, actionName));
						else if (isAction)
							pLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info,
								AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
								FTW::FormatResourceString(IDS_LOGMSG_SKIPPINGACTION, actionType, actionName));
					}
				}

				if (xmlCurrentActionorGroup.next_sibling())
					xmlCurrentActionorGroup = xmlCurrentActionorGroup.next_sibling();
				else if (_tcsicmp(xmlCurrentActionorGroup.parent().name(), XML_ELEMENT_ACTIONGROUP) == 0)
				{
					actionName = xmlCurrentActionorGroup.parent().attribute(XML_ATTRIBUTE_NAME).value();

					pLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info,
						AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
						FTW::FormatResourceString(IDS_LOGMSG_FOUNDACTIONGROUPEND, actionName));
					
					xmlCurrentActionorGroup = xmlCurrentActionorGroup.parent().next_sibling();
				}
				else
					xmlCurrentActionorGroup = pugi::xml_node();
			}
		}
		catch (FTW::FTWException& q)
		{
			pLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Error,
				AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
				FTW::FormatResourceString(IDS_LOGMSG_LOADINGVBSCRIPT, FTW::FormatResourceString(IDS_MSGSUCCESS + (int)q.GetClassification()), q.Message()));
		}
		catch (...)
		{
			pLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Error,
				AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
				FTW::FormatResourceString(IDS_LOGERROR_UNKNOWN));
		}

		//definedSoftware.clear();

		if (hIcon)
			DestroyIcon(hIcon);

	}
	
	pLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info,
		AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
		FTW::FormatResourceString(IDS_LOGMSG_FINISH));
}

void CUIppApp::LoadActionLibraries(FTWCMLOG::ICMLogPtr pLog, pugi::xml_node& libraryNode)
{
	//for (pugi::xml_node xmllibrary = libraryNode.first_child(); xmllibrary; xmllibrary = xmllibrary.next_sibling())
	//{
	//	CString n = xmllibrary.name();

	//	if (_tcsicmp(xmllibrary.name(), XML_ELEMENT_ACTIONLIBRARY) != 0)
	//		continue;

	//	CString actionLibraryName = GetXMLAttribute(&xmllibrary, XML_ATTRIBUTE_NAME);

	//	if (actionLibraryName.GetLength() != 0)
	//	{
	//		CString dllName;
	//		dllName.Format(_T("UI++%s.dll"), actionLibraryName);
	//		HINSTANCE hGetProcIDDLL = LoadLibrary(dllName);

	//		if (!hGetProcIDDLL)
	//		{
	//			pLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Error,
	//				AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
	//				FTW::FormatResourceString(IDS_LOGMSG_LOADINGACTIONDLL, dllName, FTW::FormatHRString(HRESULT_FROM_WIN32(GetLastError())).c_str()));
	//		}
	//		else
	//		{
	//			pLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info,
	//				AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
	//				FTW::FormatResourceString(IDS_LOGMSG_LOADINGACTIONDLL, dllName, FTW::FormatResourceString(IDS_MSGSUCCESS).c_str()));

	//			RegisterActionFunction funci = (RegisterActionFunction)GetProcAddress(hGetProcIDDLL, "RegisterAction");
	//			if (!funci)
	//			{
	//				TRACE0("Failed to find function!");
	//				pLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Error,
	//					AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
	//					FTW::FormatResourceString(IDS_LOGMSG_NOACTIONTOREGISTER));

	//				FreeLibrary(hGetProcIDDLL);
	//			}
	//			else
	//			{
	//				funci(&(ActionFactory::Instance()), pLog);
	//				m_loadedDlls.push_back(hGetProcIDDLL);
	//			}
	//		}
	//	}
	//}
}

void CUIppApp::GetSoftware(FTWCMLOG::ICMLogPtr pLog, pugi::xml_node& softwareNode, UIpp::SoftwareMap& software)
{
	CString id, label, info, appName, pkgID, progName, includeIds, excludeIds;
	int orderIndex = 0;

	if (softwareNode == NULL)
		return;

	pLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info,
		AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
		FTW::FormatResourceString(IDS_LOGMSG_ELEMENTFOUND, softwareNode.name()));

	for (pugi::xml_node xmlsoftware = softwareNode.first_child(); xmlsoftware; xmlsoftware = xmlsoftware.next_sibling(), orderIndex++)
	{
		CString n = xmlsoftware.name();

		if (_tcsicmp(xmlsoftware.name(), XML_ACTION_APPTREE_APPLICATION) != 0 && _tcsicmp(xmlsoftware.name(), XML_ACTION_APPTREE_PACKAGE) != 0)
			continue;

		id = GetXMLAttribute(&xmlsoftware, XML_ATTRIBUTE_ID);

		if (id.GetLength() == 0)
		{
			pLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Warning,
				AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
				FTW::FormatResourceString(IDS_LOGMSG_SOFTWAREFOUNDNOID, xmlsoftware.name()));

			continue;
		}

		label = GetXMLAttribute(&xmlsoftware, XML_ATTRIBUTE_LABEL, XML_ATTRIBUTE_LABEL_DEF);

		pLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info,
			AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
			FTW::FormatResourceString(IDS_LOGMSG_SOFTWAREFOUND, xmlsoftware.name(), id, label));

		info = GetXMLAttribute(&xmlsoftware, XML_ATTRIBUTE_SOFTWAREINFO);
		includeIds = GetXMLAttribute(&xmlsoftware, XML_ATTRIBUTE_INCLUDEID);
		excludeIds = GetXMLAttribute(&xmlsoftware, XML_ATTRIBUTE_EXCLUDEID);

		if (_tcsicmp(xmlsoftware.name(), XML_ACTION_APPTREE_APPLICATION) == 0)
		{
			appName = GetXMLAttribute(&xmlsoftware, XML_ATTRIBUTE_APPNAME);

			software.insert(SoftwareMap::value_type(id, CSoftwarePtr(new CApplication(id, label, info, appName, includeIds, excludeIds, orderIndex))));

			pLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info,
				AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
				FTW::FormatResourceString(IDS_LOGMSG_ADDAPPLICATION));
		}
		else if (_tcsicmp(xmlsoftware.name(), XML_ACTION_APPTREE_PACKAGE) == 0)
		{
			pkgID = GetXMLAttribute(&xmlsoftware, XML_ATTRIBUTE_PKGID);
			progName = GetXMLAttribute(&xmlsoftware, XML_ATTRIBUTE_PROGNAME);

			software.insert(SoftwareMap::value_type(id, CSoftwarePtr(new CPackage(id, label, info, pkgID, progName, includeIds, excludeIds, orderIndex))));

			pLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info,
				AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
				FTW::FormatResourceString(IDS_LOGMSG_ADDPACKAGE));
		}
	}
}

bool CUIppApp::LoadConfig(FTWCMLOG::ICMLogPtr pLog, pugi::xml_document& configDoc, PCTSTR pFilename, int retry, PCTSTR pAltFilename)
{
	//USES_CONVERSION;
	
	pugi::xml_parse_result result;
	CString configFilename = pFilename;

	if (_tcsncmp(pFilename, _T("http://"), 7) == 0 || _tcsncmp(pFilename, _T("https://"), 8) == 0)
	{
		CString resultMsg;
		FTWCMLOG::CCMLog::MsgType resultType = FTWCMLOG::CCMLog::MsgType::Info;

		FTW::CHTTPFile fileDownload;
		FTW::CHTTPOperation dl;

		int downloadAttempt = 0;

		do
		{
			try
			{
				dl.DownloadFile(pFilename, &fileDownload, false);

				fileDownload.GetFullFilename(configFilename);

				if (!resultMsg.LoadString(IDS_MSGSUCCESS))
					resultMsg = _T("");

				resultType = FTWCMLOG::CCMLog::MsgType::Info;
			}
			catch (FTW::HTTPCURLException& e)
			{
				resultType = FTWCMLOG::CCMLog::MsgType::Error;
				resultMsg = e.what();
				m_returnCode = e.code();
			}
			catch (FTW::HTTPStatusException& e)
			{
				resultType = FTWCMLOG::CCMLog::MsgType::Error;
				resultMsg = e.what();
				m_returnCode = atoi(e.code());
			}

			pLog->WriteMsg(resultType,
				AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
				FTW::FormatResourceString(IDS_LOGMSG_DOWNLOADCONFIGFILE, pFilename, resultMsg));

			if (resultType != FTWCMLOG::CCMLog::MsgType::Info)
			{
				if ((retry - downloadAttempt) > 0)
				{
					pLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info,
						AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
						FTW::FormatResourceString(IDS_LOGMSG_DOWNLOADCONFIGFILE_RETRY, (retry - downloadAttempt)));

					Sleep(5000);
				}
				else
					pLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Error,
						AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
						FTW::FormatResourceString(IDS_LOGERROR_DOWNLOADCONFIGFILE_FAIL));
			}

		} while (++downloadAttempt <= retry && resultType != FTWCMLOG::CCMLog::MsgType::Info);

		if (resultType != FTWCMLOG::CCMLog::MsgType::Info)
		{
			if (pAltFilename != nullptr)
			{
				configFilename = pAltFilename;
				
				pLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info,
					AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
					FTW::FormatResourceString(IDS_LOGMSG_CONFIGFILE_FALLBACK, configFilename));
			}
			
			else
				return false;
		}
	}

	result = configDoc.load_file(configFilename);

	switch (result.status)
	{
		case pugi::xml_parse_status::status_ok:
			m_returnCode = ERROR_SUCCESS;
			break;
		case pugi::xml_parse_status::status_file_not_found:
			m_returnCode = ERROR_FILE_NOT_FOUND;
			break;
		case pugi::xml_parse_status::status_io_error:
			m_returnCode = ERROR_INVALID_FUNCTION;
			break;
		case pugi::xml_parse_status::status_out_of_memory:
			m_returnCode = ERROR_OUTOFMEMORY;
			break;
		case pugi::xml_parse_status::status_internal_error:
			m_returnCode = ERROR_INVALID_FUNCTION;
			break;
		case pugi::xml_parse_status::status_unrecognized_tag:
		case pugi::xml_parse_status::status_bad_pi:
		case pugi::xml_parse_status::status_bad_comment:
		case pugi::xml_parse_status::status_bad_cdata:
		case pugi::xml_parse_status::status_bad_doctype: 
		case pugi::xml_parse_status::status_bad_pcdata:
		case pugi::xml_parse_status::status_bad_start_element:  
		case pugi::xml_parse_status::status_bad_attribute:
		case pugi::xml_parse_status::status_bad_end_element:
		case pugi::xml_parse_status::status_end_element_mismatch:
		default:
			m_returnCode = ERROR_INVALID_DATA;
			break;
	}

	//CString msg = A2T(result.description());

	if (result.offset != 0)
	{
		std::ifstream configFile;

		configFile.open(configFilename);
		
		CString errorLocation;
		errorLocation.Format(_T("%ld"), (int)result.offset);
		
		if (configFile.is_open())
		{
			char buffer[32];

			errorLocation += _T(" [...");

			configFile.seekg((int)result.offset);

			configFile.get(buffer, 31);

			errorLocation += buffer;

			errorLocation += _T(" ]");

			configFile.close();
		}

		pLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info,
			AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
			FTW::FormatResourceString(IDS_LOGERROR_CONFIGFILE, configFilename, CString(result.description()), errorLocation));
	}
	else
	{
		pLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info,
			AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
			FTW::FormatResourceString(IDS_LOGMSG_CONFIGFILE, configFilename, CString(result.description())));
	}

	return result;
}

int CUIppApp::ExitInstance()
{
	ControlBarCleanUp();

	int returnCode = CWinApp::ExitInstance();

	if (m_returnCode == 0)
		return returnCode;
	else return (int)m_returnCode;
}
