#include "StdAfx.h"
#include "ActionHelper.h"
#include "TSVar.h"
#include "resource.h"

#define MAX_EVAL_STRING_LENGTH 2048

namespace UIpp
{
	bool CActionHelper::GetWMIPropertyFromFirstInstance(FTWCMLOG::ICMLogPtr pLog, FTW::CWMIAccess& wmi, PCTSTR wmiClassName, PCTSTR wmiKey, PCTSTR wmiPropertyName, CString& value)
	{
		return _GetWMIProperty(pLog, wmi, wmiClassName, wmiPropertyName, value, wmiKey);
	}

	bool CActionHelper::GetWMIPropertyFromAllInstances(FTWCMLOG::ICMLogPtr pLog, FTW::CWMIAccess& wmi, PCTSTR wmiClassName, PCTSTR wmiPropertyName, CString& value)
	{
		return _GetWMIProperty(pLog, wmi, wmiClassName, wmiPropertyName, value, nullptr, true);
	}

	bool CActionHelper::QueryWMI(FTWCMLOG::ICMLogPtr pLog, FTW::CWMIAccess& wmi, PCTSTR wmiPropertyName, PCTSTR pQuery, CString& value)
	{
		try
		{
			return wmi.GetPropertyFromQuery(pQuery, wmiPropertyName, value);
		}
		catch (FTW::FTWException& i)
		{
			pLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Error,
				AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
				FTW::FormatResourceString(IDS_LOGERROR_WMIPROPERTYQUERY, wmiPropertyName, pQuery, i.Message()));
		}

		return false;
	}

	bool CActionHelper::_GetWMIProperty(FTWCMLOG::ICMLogPtr pLog, FTW::CWMIAccess& wmi, PCTSTR wmiClassName, PCTSTR wmiPropertyName, CString& value, PCTSTR wmiKey, bool returnAllValues)
	{
		try
		{
			wmi.Open(wmiClassName);

			try
			{
				if (!returnAllValues)
					return wmi.GetPropertyFromSingleInstance(wmiPropertyName, value, wmiKey);
				else
					return wmi.GetPropertyFromAllInstances(wmiPropertyName, value);

			}
			catch (FTW::FTWException& i)
			{
				pLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Error,
					AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
					FTW::FormatResourceString(IDS_LOGERROR_WMIPROPERTYOPEN, wmiPropertyName, wmiClassName, i.Message()));
			}

			wmi.Close();

		}
		catch (FTW::FTWException& i)
		{
			pLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Error,
				AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
				FTW::FormatResourceString(IDS_LOGERROR_WMICLASSOPEN, wmiClassName, i.Message()));
		}
		catch (...)
		{
			pLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Error,
				AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
				FTW::FormatResourceString(IDS_LOGERROR_WMICLASSOPEN2, wmiClassName));
		}

		return false;
	}

	bool CActionHelper::EvalCondition(FTWCMLOG::ICMLogPtr pLog, PCTSTR pCondition, FTW::CScriptHost* const scriptHost, PCTSTR pType, PCTSTR pName)
	{
		_variant_t result;

		if (!pCondition || _tcsnlen(pCondition, MAX_EVAL_STRING_LENGTH) == 0)
			return true;

		CString condition = CTSEnv::Instance().VariableSubstitute(pCondition);

		pLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info,
			AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
			FTW::FormatResourceString(IDS_LOGMSG_EVALCONDITION, pType, pName, pCondition, condition));

		if (SUCCEEDED(scriptHost->Eval(condition, &result)))
		{
			if ((result.vt == VT_BOOL || result.vt == VT_I2) && result.boolVal == VARIANT_TRUE)
			{
				pLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info,
					AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
					FTW::FormatResourceString(IDS_LOGMSG_CONDITIONTRUE));

				return true;
			}
			else
			{
				pLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info,
					AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
					FTW::FormatResourceString(IDS_LOGMSG_CONDITIONFALSE));
			}

		}
		else
		{
			pLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Warning,
				AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
				FTW::FormatResourceString(IDS_LOGMSG_CONDITIONERROR));
		}

		return false;
	}

	void CActionHelper::UICopyFiles(FTWCMLOG::ICMLogPtr pLog, PCTSTR sourceFilePattern, PCTSTR sourcePath, PCTSTR destPath)
	{
		WIN32_FIND_DATA findFileData;
		HANDLE hFind;

		CString searchPath = FTW::JoinPath(sourcePath, sourceFilePattern).c_str();
		CString sFile = _T(""), dFile = _T("");

		hFind = FindFirstFile(searchPath, &findFileData);

		if (hFind != INVALID_HANDLE_VALUE)
		{
			do
			{
				sFile = FTW::JoinPath(sourcePath, findFileData.cFileName).c_str();
				dFile = FTW::JoinPath(destPath, findFileData.cFileName).c_str();

				if (!sFile.IsEmpty() && !dFile.IsEmpty())
				{
					if (CopyFile(sFile, dFile, FALSE))
					{
						pLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info,
							AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
							FTW::FormatResourceString(IDS_LOGMSG_COPIEDFILE, sFile, dFile));
					}
					else
					{
						DWORD dwErr = GetLastError();

						pLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Error,
							AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
							FTW::FormatResourceString(IDS_LOGERROR_COPYFILE, sFile, dFile, dwErr, FTW::FormatHRString(HRESULT_FROM_WIN32(dwErr)).c_str()));
					}
				}

			} while (FindNextFile(hFind, &findFileData));


			FindClose(hFind);
		}

	}
}