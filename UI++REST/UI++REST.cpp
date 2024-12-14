// UI++REST.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include "UI++REST.h"
#include "TSVar.h"
#include "FTWError.h"
#include "HTTP.h"
#include "RestConstants.h"
#include "rapidjson/document.h"
#include "rapidjson/error/en.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//
//TODO: If this DLL is dynamically linked against the MFC DLLs,
//		any functions exported from this DLL which call into
//		MFC must have the AFX_MANAGE_STATE macro added at the
//		very beginning of the function.
//
//		For example:
//
//		extern "C" BOOL PASCAL EXPORT ExportedFunction()
//		{
//			AFX_MANAGE_STATE(AfxGetStaticModuleState());
//			// normal function body here
//		}
//
//		It is very important that this macro appear in each
//		function, prior to any calls into MFC.  This means that
//		it must appear as the first statement within the
//		function, even before any object variable declarations
//		as their constructors may generate calls into the MFC
//		DLL.
//
//		Please see MFC Technical Notes 33 and 58 for additional
//		details.
//

// CUIRESTApp

//BEGIN_MESSAGE_MAP(CUIRESTApp, CWinApp)
//END_MESSAGE_MAP()
//
//
//// CUIRESTApp construction
//
//CUIRESTApp::CUIRESTApp()
//{
//	// TODO: add construction code here,
//	// Place all significant initialization in InitInstance
//}
//
//
//// The one and only CUIRESTApp object
//
//CUIRESTApp theApp;
//
//
//// CUIRESTApp initialization
//
//BOOL CUIRESTApp::InitInstance()
//{
//	CWinApp::InitInstance();
//
//	return TRUE;
//}

//INT_PTR CRESTCallAction::Go(void)
//{
//	return INT_PTR();
//}

bool __declspec(dllexport) __stdcall RegisterAction(UIpp::typeActionFactory* pActiontypes, FTW::CCMLog* pCMLog)
{
	pActiontypes->registerBuilder(XML_ACTION_TYPE_RESTCALL, CodeProject::TypeID<CRESTCallAction>());

	pCMLog->LogMsg(FTW::CCMLog::Info,
		GetCurrentThreadId(), __TFILE__, __LINE__,
		IDS_LOGMSG_ACTIONREGISTERED, XML_ACTION_TYPE_RESTCALL);

	pActiontypes->registerBuilder(XML_ACTION_TYPE_FROMJSON, CodeProject::TypeID<CFromJSONAction>());

	pCMLog->LogMsg(FTW::CCMLog::Info,
		GetCurrentThreadId(), __TFILE__, __LINE__,
		IDS_LOGMSG_ACTIONREGISTERED, XML_ACTION_TYPE_FROMJSON);

	pActiontypes->registerBuilder(XML_ACTION_TYPE_TOJSON, CodeProject::TypeID<CToJSONAction>());

	pCMLog->LogMsg(FTW::CCMLog::Info,
		GetCurrentThreadId(), __TFILE__, __LINE__,
		IDS_LOGMSG_ACTIONREGISTERED, XML_ACTION_TYPE_TOJSON);

	return true;
}

INT_PTR CRESTCallAction::Go(void)
{
	CString url = UIpp::GetXMLAttribute(m_actionData.pActionNode, XML_ATTRIBUTE_URL, nullptr, m_actionData.pTSEnv);

	if (url.GetLength() == 0 || (url.Left(7).MakeLower() != _T("http://") && url.Left(8).MakeLower() != _T("https://")))
	{
		m_actionData.pCMLog->LogMsg(FTW::CCMLog::Warning, GetCurrentThreadId(), __TFILE__, __LINE__,
			IDS_LOGMSG_NOVALIDURL);
	}
	else
	{
		CString variable = UIpp::GetXMLAttribute(m_actionData.pActionNode, XML_ATTRIBUTE_VARIABLE, XML_ATTRIBUTE_VARIABLE_RESTRESULT_DEF, m_actionData.pTSEnv);
		CString json = UIpp::GetXMLAttribute(m_actionData.pActionNode, XML_ATTRIBUTE_JSON, nullptr, m_actionData.pTSEnv);

		if (!json.IsEmpty())
		{
			m_actionData.pCMLog->LogMsg(FTW::CCMLog::Info, GetCurrentThreadId(), __TFILE__, __LINE__,
				IDS_LOGMSG_RESTCALL, url, json);

			try
			{
				CString postResult;

				if (FTW::CHTTPOperation::PostJsonData((PCTSTR)url, (PCTSTR)json, postResult))
				{
					m_actionData.pCMLog->LogMsg(FTW::CCMLog::Info, GetCurrentThreadId(), __TFILE__, __LINE__,
						IDS_LOGMSG_RESTSUCCESS);

					m_actionData.pTSEnv->Set(variable, postResult, m_actionData.pCMLog);
				}
			}
			catch (FTW::HTTPCURLException& e)
			{
				m_actionData.pCMLog->LogMsg(FTW::CCMLog::Error, GetCurrentThreadId(), __TFILE__, __LINE__,
					IDS_LOGERROR_RESTFAIL, CString(e.what()));
			}
			catch (FTW::FTWErrorCodeException& e)
			{
				m_actionData.pCMLog->LogMsg(FTW::CCMLog::Error, GetCurrentThreadId(), __TFILE__, __LINE__,
					IDS_LOGERROR_RESTFAIL, CString(e.Message()));
			}
		}
		else
			m_actionData.pCMLog->LogMsg(FTW::CCMLog::Error, GetCurrentThreadId(), __TFILE__, __LINE__,
				IDS_LOGERROR_RESTNOPARAMS);
	}

	//		CHTTPOperation::PostJsonData("https://kndkpwv331.execute-api.us-east-2.amazonaws.com/prod/dataDemoFunction",
	//		"{\"device\":\"ui++\",\"lat\":\"31.06\",\"lon\":\"22.22\"}");

	return ERROR_SUCCESS;
}

INT_PTR CToJSONAction::Go(void)
{
	CString json = _T("{");
	CString attr, value;
	pugi::xml_node attrNode;
	bool dontEval;
	size_t attributeCount = 0;

	CString variable = UIpp::GetXMLAttribute(m_actionData.pActionNode, XML_ATTRIBUTE_VARIABLE, XML_ATTRIBUTE_VARIABLE_TOJSON_DEF, m_actionData.pTSEnv);
	pugi::xpath_node_set jsonAttributes = m_actionData.pActionNode->select_nodes(XML_ACTION_REST_JSONATTRIBUTE);

	for (size_t nodeIndex = 0; nodeIndex < jsonAttributes.size(); nodeIndex++)
	{
		attrNode = jsonAttributes[nodeIndex].node();

		attr = UIpp::GetXMLAttribute(&attrNode, XML_ATTRIBUTE_NAME, nullptr, m_actionData.pTSEnv);
		value = m_actionData.pTSEnv->VariableSubstitute(attrNode.child_value());
		dontEval = FTW::IsTrue(UIpp::GetXMLAttribute(&attrNode, XML_ATTRIBUTE_DONTEVAL, XML_ACTION_FALSE, m_actionData.pTSEnv));

		_variant_t r;

		if (!dontEval && SUCCEEDED(m_actionData.pScriptHost->Eval(value, &r)) && r.vt > 0 && ((_bstr_t)r).length() > 0)
		{
			value = ((_bstr_t)r).GetBSTR();
		}

		if (attr.GetLength() > 0)
		{
			if (attributeCount)
				json += _T(",");

			json += _T("\"");
			json += attr;
			json += _T("\":\"");

			if (value.GetLength() == 0)
				value = " ";

			json += value;
			json += _T("\"");

			attributeCount++;
		}
	}

	json += _T("}");
	
	m_actionData.pTSEnv->Set(variable, json, m_actionData.pCMLog);
	
	return ERROR_SUCCESS;
}

INT_PTR CFromJSONAction::Go(void)
{
	using namespace rapidjson;

	USES_CONVERSION;

	CString json = m_actionData.pTSEnv->VariableSubstitute(m_actionData.pActionNode->child_value());

	if (json.GetLength() > 0)
	{
		GenericDocument<UTF16<>> d;
		d.Parse<kParseNumbersAsStringsFlag>(json);

		if (d.HasParseError())
		{
			m_actionData.pCMLog->LogMsg(FTW::CCMLog::Warning, GetCurrentThreadId(), __TFILE__, __LINE__,
				IDS_LOGERROR_JSONPARSE, A2CT(GetParseError_En(d.GetParseError())));

		}
		else
		{
			CString name, value;

			for (auto itr = d.MemberBegin(); itr != d.MemberEnd(); ++itr)
			{
				name = m_actionData.pTSEnv->VariableSubstitute(itr->name.GetString());

				if (itr->value.GetType() == kStringType)
				{
					value = m_actionData.pTSEnv->VariableSubstitute(itr->value.GetString());

					m_actionData.pCMLog->LogMsg(FTW::CCMLog::Info, GetCurrentThreadId(), __TFILE__, __LINE__,
						IDS_LOGMSG_FOUNDJSONVALUE, name, value);

					m_actionData.pTSEnv->Set(name, value, m_actionData.pCMLog);
				}
				else
					m_actionData.pCMLog->LogMsg(FTW::CCMLog::Warning, GetCurrentThreadId(), __TFILE__, __LINE__,
						IDS_LOGMSG_FOUNDNONSTRINGJSONVALUE, name);
			}
		}
	}

	return ERROR_SUCCESS;
}