// ConfigMgrLogFile.cpp
// 
// Copyright (c) Jason Sandys, 2009
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "CMLog.h"
#include "Utils.h"

namespace FTW {

//************************************************************
/**
<summary>The default constructor. Opens a log file and 
determines the time zone offset.</summary>

<param name='lpszComponent'>(in)The name of the component to write
to the log file</param>
*/
//************************************************************
CCMLog::CCMLog()
	: m_isFileOpen (false), 
	  m_timeZoneOffset (0)
{
	int isDST;		//Indicates whether daylight savings is in effect
	long dstBias;	//The offset if daylight savings is in effect
	
	//dbgFile.open("x:\\ui++logdbg.log");
	//dbgFile << "Starting UI++ log logging\n";
	//dbgFile.flush();
	
	_tzset();

	//Retrieve the timezone info
	_get_timezone (&m_timeZoneOffset);
	_get_daylight (&isDST);
	_get_dstbias  (&dstBias);

	//Adjust the offset if daylight savings is in effect
	if (isDST)
		m_timeZoneOffset += dstBias;
}

CCMLog::~CCMLog(void)
{
	if (m_isFileOpen)
		Close ();

//	dbgFile.close();
}

void CCMLog::OpenLog(PCTSTR pComponentName, PCTSTR pLogLocation)
{
	CString path = _T("");
	CFileException e;

	m_componentName = pComponentName ? pComponentName : _T("ConfigMgrGeneric");

	if (pLogLocation && _tcsicmp(pLogLocation, _T("")) != 0)
		m_location = pLogLocation;

	else
	{
		m_location.GetEnvironmentVariable(_T("TEMP"));
	}

	//path.Format (_T ("%s\\%s.log"), m_location, m_componentName);

	m_filename = m_componentName;
	m_filename += _T(".log");

	//path = m_location;
	//path += _T("\\");
	//path += m_filename;

	path = JoinPath(m_location, m_filename);

		TRACE1("INFO: Opening log file: %s\n", path);

	//dbgFile << "Opening log: " << CT2A(path) << "\n";
	//dbgFile.flush();

	//Open the specified file for writing in text mode; create it if it doesn't already exist and don't truncate
	m_isFileOpen = (Open(path, CFile::modeCreate | CFile::modeWrite | CFile::typeText | CFile::modeNoTruncate | CFile::shareDenyWrite | CFile::osWriteThrough, &e) == TRUE);

	//dbgFile << "Log open operation complete.\n";
	//dbgFile.flush();

	if (m_isFileOpen)
	{
		//dbgFile << "Log opened.\n";
		//dbgFile.flush();

		TRACE("INFO: Log file open\n");

		//Go to the end of the log to begin writing
		try
		{
			SeekToEnd();
		}
		catch (CFileException* ee)
		{
			ee->Delete();
		}

	}
}

//************************************************************
/**
<summary>Prepares a message for the log file
</summary>

<param name='type'>(in)The type of message to log (Info, Warning,
or Error)</param>
<param name='nThreadID'>(in)The thread id of the thread initating
the log message</param>
<param name='lpszFile'>(in)The filename of the source code file
initiating the message</param>
<param name='nLineNum'>(in)The line number in the source code
initiaing the message</param>
<param name='dwStrID'>(in)The resource string ID of the message to
write</param>*/
//************************************************************
void CCMLog::LogMsg (CCMLog::MsgType msgType, const long threadID, PCTSTR pFileName, const int lineNum, const DWORD msgID, ...)
{
	if (m_isFileOpen)
	{
		va_list args;
		va_start(args, msgID);

		LogMsg (msgType, 
			    threadID, 
				pFileName, 
				lineNum, 
				FTW::FormatResourceString(msgID, &args));

		va_end(args);

	}
}

//************************************************************
/**
<summary>Write a ConfigMgr specific message to the log file
</summary>

<param name='type'>(in)The type of message to log (Info, Warning,
or Error)</param>
<param name='nThreadID'>(in)The thread id of the thread initating
the log message</param>
<param name='lpszFile'>(in)The filename of the source code file
initiating the message</param>
<param name='nLineNum'>(in)The line number in the source code
initiaing the message</param>
<param name='lpszMsg'>(in)The actual message to write to the log
</param>*/
//************************************************************
void CCMLog::LogMsg (CCMLog::MsgType msgType, const long threadID, PCTSTR pFileName, const int lineNum, PCTSTR pMsg)
{
	if (m_isFileOpen)
	{
		
		CString msg = pMsg;

		msg.Replace (_T('\n'), _T(' '));
		CTime time = CTime::GetCurrentTime();

		if (m_cs.Lock())
		{
			try
			{

				WriteString(_T("<![LOG["));
				WriteString(msg);
				WriteString(_T("]LOG]!>"));

				WriteString(_T("<time="));
				WriteString(time.Format(_T("\"%H:%M:%S.000")));
				msg.Format(_T("+%d\""), m_timeZoneOffset / 60);
				WriteString(msg);

				WriteString(_T(" date="));
				WriteString(time.Format(_T("\"%m-%d-%Y\"")));

				WriteString(_T(" component="));
				WriteString(_T("\""));
				WriteString(m_componentName);
				WriteString(_T("\""));

				WriteString(_T(" context="));
				WriteString(_T("\"\""));

				WriteString(_T(" type="));
				msg.Format(_T("\"%d\""), msgType);
				WriteString(msg);

				WriteString(_T(" thread="));
				msg.Format(_T("\"%d\""), threadID);
				WriteString(msg);

				WriteString(_T(" file="));
				msg = pFileName;
				int lastslash = msg.ReverseFind(_T('\\')) + 1;

				//Strip the path so that only the filename is written to the log
				msg.Format(_T("\"%s:%d\""), msg.Right(msg.GetLength() - lastslash), lineNum);
				WriteString(msg);

				WriteString(_T(">\n"));

				Flush();
			}	
			catch (...)
			{
			}

			m_cs.Unlock();
		}
	}

}
}