#pragma once

#import "..\UI++\FTW\TSCore.dll" named_guids

#include "FTWCMLog.h"
#include "atlsafe.h"
#include "afxmt.h"
#include <regex>
#include <iostream>
#include <fstream>


class CTSEnv
{
	friend class CTSVarGridCtrl;
public:
	CTSEnv();
	~CTSEnv();

	inline static CTSEnv& Instance()
	{
		static CTSEnv _TSVarMap;
		return _TSVarMap;
	}

	inline PCTSTR GetLogPath(void) { return m_tsLogPath; };
	inline bool InTS(void) { return m_inTS; };

	const CString Get(PCTSTR pValueName);
	BOOL Get(PCTSTR pValueName, CString& value);
	const BOOL Exists(PCTSTR pValueName);

	inline const CString operator [] (PCTSTR pValueName) { return Get(pValueName); }

	void Set(FTWCMLOG::ICMLogPtr pLog, PCTSTR pValueName, PCTSTR pNewValue, bool writeToLog = true);
	void Set(FTWCMLOG::ICMLogPtr pLog, PCTSTR pValueName, const unsigned long newValue, bool writeToLog = true);

	CString VariableSubstitute(PCTSTR pString);
	CString& VariableSubstitute(CString& string);

	void DumpToFile(FTWCMLOG::ICMLogPtr pLog, PCTSTR pFilename = nullptr) const;
	void SaveToFile(FTWCMLOG::ICMLogPtr pLog, PCTSTR pFilename = nullptr) const;
	void LoadFromFile(FTWCMLOG::ICMLogPtr pLog, PCTSTR pFilename = nullptr);

	//TSVARMAP varMap;

private:
	CTSEnv operator=(CTSEnv const&);
	CTSEnv (CTSEnv const&);

	TSEnvironmentLib::ITSEnvClassPtr m_tsEnv;

	CMapStringToString* m_pNonTSVars;

	CString m_tsLogPath;
	bool m_inTS;

	std::wregex m_rxValidWritableTSVariableName;
	std::wregex m_rxValidTSVariableName;

	CCriticalSection m_cs;

	//std::ofstream dbgFile;

};

				//_variant_t vars = m_tsEnv->GetVariables();

				//CComSafeArray<VARIANT> varArray(*(vars.parray));

				//TRACE1("INFO: Found variables: %d\n", varArray.GetCount());

				//for (ULONG count = 0; count < varArray.GetCount(); count++)
				//{
				//	_variant_t v = varArray.GetAt(count);
				//	CString name (v.bstrVal);
				//	CString value = (LPCTSTR) m_tsEnv->GetValue(v.bstrVal);

				//	TRACE2("	INFO: %s -> %s\n", name, value );
				//}

				//TRACE("INFO: Loaded TS Environment\n");
