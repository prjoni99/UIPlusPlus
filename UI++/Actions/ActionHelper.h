#pragma once

#include "FTWCMLog.h"
#include "FTW\WMIAccess.h"
#include "ScriptHost.h"

namespace UIpp 
{
	class CActionHelper
	{
	public:
		static bool GetWMIPropertyFromFirstInstance(FTWCMLOG::ICMLogPtr pLog, FTW::CWMIAccess& wmi, PCTSTR wmiClassName, PCTSTR wmiKey, PCTSTR wmiPropertyName, CString& value);
		static bool GetWMIPropertyFromAllInstances(FTWCMLOG::ICMLogPtr pLog, FTW::CWMIAccess& wmi, PCTSTR wmiClassName, PCTSTR wmiPropertyName, CString& value);
		static bool QueryWMI(FTWCMLOG::ICMLogPtr pLog, FTW::CWMIAccess& wmi, PCTSTR wmiPropertyName, PCTSTR pQuery, CString& value);
		//bool GetWMIProperty(FTW::CWMIAccess& wmi, PCTSTR wmiClassName, PCTSTR wmiKey, PCTSTR wmiPropertyName, long& value);

		static bool EvalCondition(FTWCMLOG::ICMLogPtr pLog, PCTSTR pCondition, FTW::CScriptHost* const scriptHost, PCTSTR pType, PCTSTR pName);

		static void UICopyFiles(FTWCMLOG::ICMLogPtr pLog, PCTSTR sourceFilePattern, PCTSTR sourcePath, PCTSTR destPath);

	private:
		static bool _GetWMIProperty(FTWCMLOG::ICMLogPtr pLog, FTW::CWMIAccess& wmi, PCTSTR wmiClassName, PCTSTR wmiPropertyName, CString& value, PCTSTR wmiKey = nullptr, bool returnAllValues = false);

	};
}