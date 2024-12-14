#include "stdafx.h"
#include "TSVar.h"
#include "FTW\Utils.h"
#include "resource.h"
#include <iostream>
#include <fstream>

#define MAX_TSVAR_STRING_LENGTH 256

CTSEnv::CTSEnv() : m_pNonTSVars (0)
{

	//dbgFile.open("x:\\ui++tsvardbg.log");
	//dbgFile << "Starting UI++ TSVar logging\n";
	//dbgFile.flush();

	try
	{
		//Try to retrieve an instance of the Microsoft.SMS.TSEnvironment class
		HRESULT hr = m_tsEnv.CreateInstance (TSEnvironmentLib::CLSID_TSEnvClass);

		if (hr == S_OK)
		{
			TRACE("INFO: Created Microsoft.SMS.TSEnvironment\n");

			//Get the log file location for UI; throws an exception if we're not in a TS
			_bstr_t b = m_tsEnv->GetValue (_T ("_SMSTSLogPath"));

			m_tsLogPath = (PCTSTR) b;

			m_inTS = true;
		}
		else
		{
			TRACE("INFO: Unable to create Microsoft.SMS.TSEnvironment\n");

			m_tsLogPath.GetEnvironmentVariable (_T ("TEMP"));
			m_inTS = false;

			m_pNonTSVars = new CMapStringToString();
		}

		m_rxValidWritableTSVariableName.assign(_T("^[a-zA-Z][\\w-]{0,254}$"));
		m_rxValidTSVariableName.assign(_T("^(_\\w[\\w-]{0,253}|[a-zA-Z][\\w-]{0,254})$")); 

		TRACE1("INFO: Using TS log path: %s\n", m_tsLogPath);
	}
	catch (_com_error ce)
	{
		TRACE("INFO: Unable to create Microsoft.SMS.TSEnvironment\n");
	}

}

CTSEnv::~CTSEnv()
{
	TRACE("INFO: CTSVarMap destructor\n");

	if (m_pNonTSVars)
		delete m_pNonTSVars;

	//dbgFile << "Destroyed object\n";
	//dbgFile.flush();
	//dbgFile.close();
}

const BOOL CTSEnv::Exists(PCTSTR pValueName)
{
	ASSERT(pValueName);

	if(!pValueName || _tcsnlen(pValueName, MAX_TSVAR_STRING_LENGTH) == 0)
		return FALSE;

	//CSingleLock singleLock(&m_cs);
	
	BOOL returnValue = FALSE;

	//if (singleLock.Lock())
	if(m_cs.Lock())
	{
		if (!m_inTS)
		{
			CString val;

			ASSERT(m_pNonTSVars);

			if (m_pNonTSVars != nullptr)
			{
				returnValue = (pValueName && m_pNonTSVars->Lookup(pValueName, val));
			}
			else
			{
				return FALSE;
			}
		}
		else
		{
			_bstr_t b = m_tsEnv->GetValue(pValueName);

			returnValue = (b.length() > 0);
		}

		m_cs.Unlock();
	}

	return returnValue;
}

const CString CTSEnv::Get(PCTSTR pValueName)
{
	CString val = _T("");
	bool valFound = false;

	ASSERT(pValueName);

	if(!pValueName || _tcsnlen(pValueName, MAX_TSVAR_STRING_LENGTH) == 0)
		return val;

	//CSingleLock singleLock(&m_cs);

	if (m_cs.Lock())
	{
		if (!m_inTS)
		{

			ASSERT(m_pNonTSVars);

			if (m_pNonTSVars != nullptr && pValueName && m_pNonTSVars->Lookup(pValueName, val))
				valFound = true;
			else
				valFound = false;
		}
		else
		{
			_bstr_t b = m_tsEnv->GetValue(pValueName);

			if (b.length() > 0)
			{
				val = b.GetBSTR();
				valFound = true;
			}
		}

		m_cs.Unlock();
	}

	if (valFound)
		return val;
	else
		return _T("");
}

BOOL CTSEnv::Get(PCTSTR pValueName, CString& value)
{
	//CSingleLock singleLock(&m_cs);

	BOOL returnValue = FALSE;

	if (m_cs.Lock())
	{
		if (!m_inTS)
		{
			ASSERT(m_pNonTSVars);

			if (m_pNonTSVars != nullptr && pValueName)
				returnValue = m_pNonTSVars->Lookup(pValueName, value);
			else
				returnValue = FALSE;
		}
		else
		{
			_bstr_t b = m_tsEnv->GetValue(pValueName);

			if (b.length() > 0)
			{
				value = b.GetBSTR();
				returnValue = TRUE;
			}
		}

		m_cs.Unlock();
	}

	return returnValue;
}

void CTSEnv::Set(FTWCMLOG::ICMLogPtr pLog, PCTSTR pValueName, PCTSTR pNewValue, bool writeToLog)
{
	ASSERT (pValueName && pNewValue);

	std::wstring ws (pValueName);
	
	if (pValueName && _tcsnlen(pValueName, MAX_TSVAR_STRING_LENGTH) > 0 && std::regex_match(ws.begin(), ws.end(), m_rxValidWritableTSVariableName))
	{
		CString key, value;

		//CSingleLock singleLock(&m_cs);

		if (m_cs.Lock())
		{
			if (!m_inTS)
			{
				ASSERT(m_pNonTSVars);

				m_pNonTSVars->SetAt(pValueName, pNewValue);

			}
			else
			{
				key = pValueName;
				value = pNewValue;
				m_tsEnv->PutValue(key.AllocSysString(), value.AllocSysString());
			}

			//if(pLog != nullptr)
			//	pLog->LogMsg(FTW::CCMLog::Info, GetCurrentThreadId(), __TFILE__, __LINE__,
			//		IDS_LOGMSG_SETTSVARIABLE, pValueName, pNewValue);
			//else

			if (writeToLog)
				pLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info,
					AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
					FTW::FormatResourceString(IDS_LOGMSG_SETTSVARIABLE, pValueName, pNewValue));
			else
				pLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info,
					AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
					FTW::FormatResourceString(IDS_LOGMSG_SETTSVARIABLE, pValueName, L"******"));


			m_cs.Unlock();
		}
	}
	else
	{
		//if (pLog != nullptr)
		//	pLog->LogMsg(FTW::CCMLog::Info, GetCurrentThreadId(), __TFILE__, __LINE__,
		//		IDS_LOGERROR_SETTSVARIABLE, pValueName, pNewValue);
		//else
		//	FTW::CCMLog::Instance().LogMsg (FTW::CCMLog::Info, AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
		//	IDS_LOGERROR_SETTSVARIABLE, pValueName, pNewValue);

		if (writeToLog)
			pLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info,
				AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
				FTW::FormatResourceString(IDS_LOGERROR_SETTSVARIABLE, pValueName, pNewValue));
		else
			pLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info,
				AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
				FTW::FormatResourceString(IDS_LOGERROR_SETTSVARIABLE, pValueName, L"******"));

	}
}

void CTSEnv::Set(FTWCMLOG::ICMLogPtr pLog, PCTSTR pValueName, const unsigned long newValue, bool writeToLog)
{
	ASSERT (pValueName);

	CString temp;

	temp.Format(_T("%u"), newValue);

	Set (pLog, pValueName, temp, writeToLog);

}

CString CTSEnv::VariableSubstitute(PCTSTR pString)
{
	ASSERT(pString);
	
	if (!pString || _tcsnlen(pString, MAX_TSVAR_STRING_LENGTH) == 0)
		return _T("");

	CString tempString(pString), outString, token, variableValue, variable;
	std::wstring ws;
	PTSTR pTmp = nullptr;
	DWORD bufferLength = 0;

	bufferLength = ExpandEnvironmentStrings((PCTSTR)pString, pTmp, bufferLength);

	if (bufferLength > 0)
	{
		pTmp = new TCHAR[bufferLength + 2];

		bufferLength = ExpandEnvironmentStrings((PCTSTR)pString, pTmp, bufferLength);

		if (bufferLength > 0)
			tempString = pTmp;

		delete[] pTmp;
	}
	
	outString = tempString;
	
	int pos = tempString.Find(_T("%"), 0);

	while (pos != -1)
	{
		token = tempString.Tokenize (_T("%"), pos);
		ws = token.GetString();

		if (pos != -1 && pos <= tempString.GetLength() && std::regex_match(ws.begin(), ws.end(), m_rxValidTSVariableName))
		{

			variable.Format(_T("%%%s%%"), token.GetString());

			if (Get(token, variableValue))
				outString.Replace(variable, variableValue);
			else
				outString.Replace(variable, _T(""));
		}
	}

	return outString;
}

CString& CTSEnv::VariableSubstitute(CString& inString)
{
	inString = VariableSubstitute((PCTSTR)inString);

	return inString;
}

void CTSEnv::DumpToFile(FTWCMLOG::ICMLogPtr pLog, PCTSTR pFilename) const
{
	std::wofstream tsDumpFile;

	CString logPath = pFilename;

	if (pFilename == nullptr)
	{
		CString time = CTime::GetCurrentTime().Format(_T("%c"));
		time.Replace(_T(":"), _T("-"));
		time.Replace(_T("/"), _T("-"));

		logPath.Format(_T("%s\\UI++ Variable Dump %s.txt"), pLog->Path().c_str(), (PCTSTR)time);
	}

	pLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info,
		AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
		FTW::FormatResourceString(IDS_LOGMSG_TSVARDUMP, logPath));

	tsDumpFile.open(logPath, std::ios::out | std::ios::trunc);

	if (!tsDumpFile.is_open())
		return;

	if (!m_inTS && m_pNonTSVars != NULL)
	{
		if (m_pNonTSVars->IsEmpty())
		{
			tsDumpFile.close();
			return;
		}

		CMapStringToString::CPair* pCurrent = m_pNonTSVars->PGetFirstAssoc();

		while (pCurrent != NULL)
		{
			tsDumpFile << (PCTSTR)pCurrent->key << _T(":\t") << (PCTSTR)pCurrent->value << _T("\n\r");

			pCurrent = m_pNonTSVars->PGetNextAssoc(pCurrent);
		}
	}
	else if (m_inTS)
	{
		_variant_t vars = m_tsEnv->GetVariables();

		CComSafeArray<VARIANT> varArray(*(vars.parray));

		CString name, value;

		for (ULONG count = 0; count < varArray.GetCount(); count++)
		{
			_variant_t v = varArray.GetAt(count);
			name = v.bstrVal;
			value = (LPCTSTR)m_tsEnv->GetValue(v.bstrVal);

			tsDumpFile << (PCTSTR)name << _T(":\t") << (PCTSTR)value << _T("\n\r");

		}
	}

	tsDumpFile.close();
}

void CTSEnv::SaveToFile(FTWCMLOG::ICMLogPtr pLog, PCTSTR pFilename) const
{
	if (pFilename == nullptr)
		return;

	CFileException e;

	CFile saveFile;
	PTSTR filePath = nullptr;
	DWORD bufferLength = 0;

	if (!m_inTS && m_pNonTSVars != NULL && !m_pNonTSVars->IsEmpty())
	{
		
		bufferLength = ExpandEnvironmentStrings(pFilename, filePath, bufferLength);

		filePath = new TCHAR[bufferLength + 2];

		bufferLength = ExpandEnvironmentStrings(pFilename, filePath, bufferLength);

		if (bufferLength > 0)
		{
			pLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info,
				AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
				FTW::FormatResourceString(IDS_LOGMSG_SAVINGVARIABLES, filePath));

			if (saveFile.Open(filePath, CFile::modeCreate | CFile::modeWrite | CFile::shareExclusive, &e))
			{
				CArchive archive(&saveFile, CArchive::store);
				CMapStringToString tempVariables;

				CMapStringToString::CPair* pCurrent = m_pNonTSVars->PGetFirstAssoc();

				while (pCurrent != NULL)
				{
					if (pCurrent->key[0] != 'X' && pCurrent->key[0] != '_')
					{
						tempVariables.SetAt(pCurrent->key, pCurrent->value);

						pLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info,
							AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
							FTW::FormatResourceString(IDS_LOGMSG_SAVINGVARIABLES_ADDING, pCurrent->key, pCurrent->value));
					}
					else
						pLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info,
							AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
							FTW::FormatResourceString(IDS_LOGMSG_SAVINGVARIABLES_SKIPPING, pCurrent->key));

					pCurrent = m_pNonTSVars->PGetNextAssoc(pCurrent);
				}

				try
				{
					tempVariables.Serialize(archive);

					pLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info,
						AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
						FTW::FormatResourceString(IDS_LOGMSG_SAVINGVARIABLES_SUCCESS));
				}
				catch (CException* pEx)
				{
					TCHAR pErrorMsg[MAX_STRING_LENGTH];
					pEx->GetErrorMessage(pErrorMsg, (UINT)MAX_STRING_LENGTH, NULL);

					pLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Error,
						AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
						FTW::FormatResourceString(IDS_LOGERROR_SAVINGVARIABLES, pErrorMsg));

					pEx->Delete();
				}

			}
			else
				pLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Error,
					AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
					FTW::FormatResourceString(IDS_LOGERROR_SAVINGVARIABLES, FTW::FormatHRString(GetLastError()).c_str()));
		}
		else
			pLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Error,
				AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
				FTW::FormatResourceString(IDS_LOGERROR_SAVINGVARIABLES, FTW::FormatHRString(GetLastError()).c_str()));
	}
	else
		pLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Warning,
			AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
			FTW::FormatResourceString(IDS_LOGMSG_NOTSAVINGVARIABLES));
}

void CTSEnv::LoadFromFile(FTWCMLOG::ICMLogPtr pLog, PCTSTR pFilename)
{
	std::ofstream dbgFile;

	dbgFile.open("x:\\ui++tsvardbg.log");
	dbgFile << "Starting UI++ TSVar logging\n";
	dbgFile.flush();
	
	if (pFilename == nullptr)
		return;

	dbgFile << "Filename: " << CT2A(pFilename) << "\n";
	dbgFile.flush();

	CFileException e;

	CFile loadFile;
	PTSTR filePath = nullptr;
	DWORD bufferLength = 0;

	dbgFile << "Expanding environment variables\n";
	dbgFile.flush();
	
	bufferLength = ExpandEnvironmentStrings(pFilename, filePath, bufferLength);

	filePath = new TCHAR[bufferLength + 2];

	bufferLength = ExpandEnvironmentStrings(pFilename, filePath, bufferLength);

	if (bufferLength > 0)
	{
		pLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info,
			AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
			FTW::FormatResourceString(IDS_LOGMSG_LOADINGVARIABLES, filePath));

		dbgFile << "Loading file: " << CT2A(filePath) << "\n";
		dbgFile.flush();

		if (loadFile.Open(filePath, CFile::modeRead | CFile::shareExclusive, &e))
		{
			dbgFile << "Loaded file\n";
			dbgFile.flush();

			CArchive archive(&loadFile, CArchive::load);
			CMapStringToString tempVariables;

			try
			{
				dbgFile << "Serializing data\n";
				dbgFile.flush();

				tempVariables.Serialize(archive);

				dbgFile << "Serialized data, count: " << tempVariables.GetCount() << "\n";
				dbgFile.flush();
			}
			catch (CException* pEx)
			{
				TCHAR pErrorMsg[MAX_STRING_LENGTH];
				pEx->GetErrorMessage(pErrorMsg, (UINT)MAX_STRING_LENGTH, NULL);

				pLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Error,
					AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
					FTW::FormatResourceString(IDS_LOGERROR_LOADINGVARIABLES, pErrorMsg));

				pEx->Delete();
			}

			dbgFile << "Converting data\n";
			dbgFile.flush();

			CMapStringToString::CPair* pCurrent = tempVariables.PGetFirstAssoc();

			while (pCurrent != NULL)
			{
				if (!m_inTS && m_pNonTSVars != NULL && pCurrent->key[0] != 'X' && pCurrent->key[0] != '_')
				{
					m_pNonTSVars->SetAt(pCurrent->key, pCurrent->value);

					pLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info,
						AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
						FTW::FormatResourceString(IDS_LOGMSG_LOADINGVARIABLES_LOADING, pCurrent->key, pCurrent->value));
				}
				else if (m_inTS && m_tsEnv != nullptr)
				{
					m_tsEnv->PutValue(pCurrent->key.AllocSysString(), pCurrent->value.AllocSysString());

					pLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info,
						AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
						FTW::FormatResourceString(IDS_LOGMSG_LOADINGVARIABLES_LOADING, pCurrent->key, pCurrent->value));
				}

				pCurrent = tempVariables.PGetNextAssoc(pCurrent);
			}

			dbgFile << "Done loading data\n";
			dbgFile.flush();
		}
		else
			pLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Error,
				AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
				FTW::FormatResourceString(IDS_LOGERROR_LOADINGVARIABLES, FTW::FormatHRString(GetLastError()).c_str()));
	}
	else
		pLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Error,
			AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
			FTW::FormatResourceString(IDS_LOGERROR_LOADINGVARIABLES, FTW::FormatHRString(GetLastError()).c_str()));

	dbgFile.close();
}