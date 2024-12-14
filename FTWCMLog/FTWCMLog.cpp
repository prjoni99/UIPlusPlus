#include "stdafx.h"
#include "FTWCMLog.h"
#include <time.h>
#include <iostream>
#include <fstream>
//#include <memory>

typedef std::basic_ofstream<wchar_t> ofwstream;

#define EXPORT comment(linker, "/EXPORT:" __FUNCTION__ "=" __FUNCDNAME__)

namespace FTWCMLOG
{
	//#if !defined(_WIN64)
	//	// This pragma is required only for 32-bit builds. In a 64-bit environment,
	//	// C functions are not decorated.
	//#pragma comment(linker, "/export:GetCMLog=_GetCMLog@0")
	//#endif  // _WIN64

	FTWCMLOGAPI CMLOGHANDLE APIENTRY GetCMLog(const wchar_t* pComponentName, const wchar_t* pPath)
	{
#if !defined(_WIN64)
		// This pragma is required only for 32-bit builds. In a 64-bit environment,
		// C functions are not decorated.
#pragma EXPORT
#endif  // _WIN64

		//static CCMLog _Log(pComponentName, pPath);
		//return &_Log;
		return new CCMLog(pComponentName, pPath);
	}

	CCMLog::CCMLog(const std::wstring& componentName, const std::wstring& path)
	{
		GetTimeZoneInfo();
		OpenLog(componentName, path);
	}

	CCMLog::~CCMLog() 
	{
		
	}

	void CCMLog::GetTimeZoneInfo(void)
	{
		int isDST;		//Indicates whether daylight savings is in effect
		long dstBias;	//The offset if daylight savings is in effect
		long tzOffset;

		_tzset();

		//Retrieve the timezone info
		_get_timezone(&tzOffset);
		_get_daylight(&isDST);
		_get_dstbias(&dstBias);

		m_timeZoneOffset = tzOffset;

		//Adjust the offset if daylight savings is in effect
		if (isDST)
			m_timeZoneOffset += dstBias;
	}

	void CCMLog::OpenLog(const std::wstring & componentName, const std::wstring & location)
	{
		m_componentName = !componentName.empty() ? componentName : L"GenericLog";

		if (!location.empty())
			m_logPath.assign(location);
		else
		{
			wchar_t* buffer = new wchar_t[1];
			DWORD bufferSize = 1;

			bufferSize = GetEnvironmentVariable(L"TEMP", buffer, bufferSize);
			
			if (bufferSize > 0)
			{
				delete[] buffer;

				size_t newBufferSize = bufferSize + 1;
				
				buffer = new wchar_t[newBufferSize];
				
				if (GetEnvironmentVariable(L"TEMP", buffer, bufferSize) == 0)
					throw;

				m_logPath.assign(buffer);
			}

			delete[] buffer;
		}
			   
		m_logPath /= (m_componentName + L".log");
		m_logPath.make_preferred();
	}

	void CCMLog::WriteMsg(MsgType msgType, const long threadID, const std::wstring & filename, const int lineNum, const std::wstring & msg)
	{
		if (m_logPath.empty())
			return;
		
		ofwstream log;
		time_t now;
		time(&now);

		//std::wstring location = GetPathandFilename();
		std::wstring location = m_logPath;

		criticalSection.lock();

		log.open(location, std::ios::out | std::ios::ate | std::ios::app);

		if (!log.is_open())
		{
			criticalSection.unlock();
			return;
		}

		std::wstring outmsg = msg;

		size_t pos = outmsg.find_first_of(L'\n', 0);

		while (pos != std::wstring::npos)
		{
			outmsg[pos] = L' ';
			pos = outmsg.find_first_of(L'\n', pos + 1);
		}

		log << L"<![LOG[" << outmsg << L"]LOG]!>";
		log << L"<time=\"" << Now(now, 0) << L".000+" << (m_timeZoneOffset / 60) << L"\"";
		log << L" date=\"" << Now(now, 1) << L"\"";
		log << L" component=\"" << m_componentName << L"\"";
		log << L" context=\"" << L"\"";
		log << L" type=\"" << (int)msgType << L"\"";
		log << L" thread=\"" << threadID << L"\"";
		log << L" file=\"" << std::filesystem::path(filename).filename() << L"\"";
		log << L">\n";

		log.flush();

		log.close();

		criticalSection.unlock();

	}

	//std::wstring CCMLog::JoinPath(const std::wstring part1, const std::wstring part2) const
	//{
	//	std::wstring tmp = part1;

	//	if (tmp.at(tmp.length() - 1) != L'\\')
	//		tmp += L'\\';

	//	tmp += part2;

	//	return tmp;
	//}

	//std::wstring CCMLog::StripPath(const std::wstring path) const
	//{
	//	std::wstring filename = path;

	//	size_t pos = filename.find_last_of(L'\\', std::wstring::npos);

	//	if (std::wstring::npos != pos)
	//		filename.erase(0, pos + 1);

	//	return filename;
	//}

	std::wstring CCMLog::Now(const time_t now, short option) const
	{
		wchar_t timeString[256] = L"";
		struct tm time;

		if (localtime_s(&time, &now) == 0)
		{
			if (option == 0)
				wcsftime(timeString, 256, L"%H:%M:%S", &time);
			else if (option == 1)
				wcsftime(timeString, 256, L"%m-%d-%Y", &time);
		}

		return timeString;
	}

}