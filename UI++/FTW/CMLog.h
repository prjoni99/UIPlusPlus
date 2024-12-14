// ConfigMgrLogFile.h
// 
// Copyright (c) Jason Sandys, 2009
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include <iostream>
#include <fstream>

#include "afxmt.h"

namespace FTW {

//************************************************************
/**
<summary>Specializes the CStdioFile class to create a
ConfigMgr specific log file</summary>
*/
//************************************************************
class CCMLog : protected CStdioFile
{
private:
	CCMLog();
	~CCMLog(void);
	CCMLog(CCMLog const&);
	CCMLog& operator=(CCMLog const&);

public:
	enum MsgType { Info = 1, Warning, Error };

	void LogMsg (CCMLog::MsgType msgType, const long threadID, PCTSTR pFileName, const int lineNum, const DWORD msgID, ...);
	void LogMsg (CCMLog::MsgType msgType, const long threadID, PCTSTR pFileName, const int lineNum, PCTSTR pMsg);
	void OpenLog (PCTSTR pComponentName = 0, PCTSTR pLogLocation = 0);

	static CCMLog& Instance()
	{
		static CCMLog _Log;
		return _Log;
	}

	PCTSTR GetLocation(void) const { return m_location; }
	PCTSTR GetLogFileName(void) const { return m_filename; }

protected:
	///<summary>The name of the component to write to the log</summary>
	CString m_componentName;
	///<summary>Sentinel to indicate if the file is open or not</summary>
	bool m_isFileOpen;
	///<summary>The timezone offset from UTC in minutes</summary>
	long m_timeZoneOffset;

	CString m_location;
	CString m_filename;

//	std::ofstream dbgFile;

	CCriticalSection m_cs;

	//void WriteMsg (CConfigMgrLogFile::MsgType type, long nThreadID, LPCTSTR lpszFile, int nLineNum, LPCTSTR lpszMsg);
};
}