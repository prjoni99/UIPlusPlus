#pragma once

#include <string>
#include <mutex>
#include <filesystem>

#if defined(FTWCMLOG_EXPORTS) // inside DLL
#   define FTWCMLOGAPI   __declspec(dllexport)
#else // outside DLL
#   define FTWCMLOGAPI   __declspec(dllimport)
#endif  // FTWCMLOGAPI

namespace FTWCMLOG
{
	struct ICMLog
	{
		virtual bool IsValid() = 0;
		virtual void Release() = 0;

		enum class MsgType { Info = 1, Warning, Error };

		virtual void WriteMsg(MsgType msgType, const long threadID, const std::wstring& filename, const int lineNum, const std::wstring& msg) = 0;

		virtual const std::wstring Path(void) const = 0;
		virtual const std::wstring Filename(void) const = 0;
		//virtual const std::wstring GetPathandFilename(void) const = 0;
	};

	typedef std::shared_ptr<ICMLog> ICMLogPtr;
	typedef ICMLog* CMLOGHANDLE;

	extern "C" FTWCMLOGAPI CMLOGHANDLE APIENTRY GetCMLog(const wchar_t* pComponentName, const wchar_t* pPath);

	typedef CMLOGHANDLE(__cdecl *GETCMLOG)(const wchar_t* pComponentName, const wchar_t* pPath);

	class CCMLog : public ICMLog
	{
	public:
		CCMLog(const std::wstring& componentName = L"", const std::wstring& path = L"");
		~CCMLog();

		bool IsValid() { return true; };
		void Release() { delete this; }

		CCMLog(const CCMLog&) = delete;

		//void WriteMsg(MsgType msgType, const long threadID, const std::wstring& filename, const int lineNum, const DWORD msgID, ...);
		void WriteMsg(MsgType msgType, const long threadID, const std::wstring& filename, const int lineNum, const std::wstring& msg);

		inline const std::wstring Path(void) const { return m_logPath.parent_path(); };
		inline const std::wstring Filename(void) const { return m_logPath.filename(); };
		//const std::wstring GetPathandFilename(void) const { return (JoinPath(m_path, m_filename)); };

	protected:

		void GetTimeZoneInfo(void);
		void OpenLog(const std::wstring& componentName, const std::wstring& location);
		//std::wstring JoinPath(const std::wstring part1, const std::wstring part2) const;
		//std::wstring StripPath(const std::wstring path) const;

		std::wstring Now(const time_t now, short option) const;

		std::wstring m_componentName;
		//std::wstring m_path;
		//std::wstring m_filename;

		std::filesystem::path m_logPath;

		long m_timeZoneOffset = 0;

		std::mutex criticalSection;
	};

}