#pragma once

#include <string>
#include <unordered_map>
#include "curl\include\curl\curl.h"
#include <stdio.h>

typedef std::unordered_map <std::string, std::string> stringmap;

namespace FTW {
	std::string trim(const std::string &s);

	class CHTTPFile
	{
		friend class CHTTPOperation;

	public:
		CHTTPFile();
		void Open(void);
		~CHTTPFile();

		void Close(void);

		const std::string& GetFilename(void);
		const std::string GetFullFilename(void);
		void GetFullFilename(CString& fname);

		const std::string& GetFolder(void);

		const FILE* GetFILE(void) { return fp; };

		static const std::string GetExtensionFromMimeType(std::string mimetype);

	private:
		FILE * fp = nullptr;

		std::string folder;
		std::string filename;

		//	std::string tempFullname;

		bool MoveandRenameFile(std::string newFilename, std::string newFolder);

		static stringmap mimemap;
	};

	//class CHTTPOperation
	//{
	//public:
	//	CHTTPOperation();
	//	~CHTTPOperation();
	//
	//protected:
	//	void* m_curl;
	//	errno_t m_error;
	//	virtual void Abstract() = 0;
	//};

	class CHTTPOperation //: public CHTTPOperation
	{
	public:
		CHTTPOperation();
		virtual ~CHTTPOperation();

		bool DownloadFile(const char* url, CHTTPFile* pFile, bool moveAndRename = false, const char* pDestination = nullptr);
		bool DownloadFile(PCTSTR url, CHTTPFile* pFile, bool moveAndRename = false, PCTSTR destination = nullptr);
		std::string Get(const std::string& url);

		static bool PostJsonData(const char* url, const char* data, std::string& result);
		static bool PostJsonData(PCTSTR url, PCTSTR data, CString& result);


	private:
		static size_t WriteToMemory(void *ptr, size_t size, size_t nmemb, void *data);
		static size_t WriteToFile(void *ptr, size_t size, size_t nmemb, void *data);
		static std::string GetFilenameFromUrl(const std::string& url);
		static size_t ProcessHeader(char *buffer, size_t size, size_t nitems, void *userdata);
		static size_t GetData(void *ptr, size_t size, size_t nmemb, void *stream);

		static bool PostData(const char* url, struct curl_slist* headers, const char* data, std::string& result);

		struct HeaderInfo
		{
			std::string mimetype;
			std::string targetFilename;
			std::string httpstatuscode;
			std::string httpstatusmsg;
		};

		struct ContentinMemory
		{
			ContentinMemory()
			{
				memory = (char*)malloc(1);
				size = 0;
			}
			~ContentinMemory()
			{
				free(memory);
			}

			char *memory;
			size_t size;
		};

		//void* m_curl;
		errno_t m_error;

		//void Abstract() {};
	};

	//class CHTTPPost : public CHTTPOperation
	//{
	//public:
	//	CHTTPPost() {};
	//	virtual ~CHTTPPost() {};
	//
	//	bool PostData(const char* url, const char* data);
	//
	//private:
	//	void Abstract() {};
	//};

	class HTTPCURLException : public std::exception
	{
	public:
		HTTPCURLException(CURLcode code) : m_code(code) { };
		~HTTPCURLException() {};

		const char* what() const throw() { return curl_easy_strerror(m_code); };
		const CURLcode code(void) const { return m_code; };

	protected:
		CURLcode m_code;
	};

	class HTTPStatusException : public std::exception
	{
	public:
		HTTPStatusException(std::string statuscode, std::string statusmsg)
			: m_statuscode(statuscode), m_statusmsg(statusmsg), m_statusmsgandcode(statuscode + " " + statusmsg) { };
		~HTTPStatusException() {};

		const char* what() const throw() { return m_statusmsgandcode.c_str(); };
		const char* code(void) const { return m_statuscode.c_str(); };
		const char* msg() const throw() { return m_statusmsg.c_str(); };

	protected:
		std::string m_statuscode;
		std::string m_statusmsg;
		std::string m_statusmsgandcode;

	};

	class HTTPFileException : public std::exception
	{
	public:
		HTTPFileException(errno_t err) : m_errno(err)
		{
			char msg[256] = { '\0' };

			strerror_s(msg, 255, m_errno);

			m_msg = msg;
		};
		~HTTPFileException() {};

		const char* what() const throw() { return m_msg.c_str(); };
		const errno_t err(void) const { return m_errno; };

	protected:
		errno_t m_errno;
		std::string m_msg = "";
	};

}