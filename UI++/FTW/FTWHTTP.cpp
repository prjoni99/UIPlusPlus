#include "stdafx.h"
#include "FTWHTTP.h"
#include "FTWError.h"
#include <sstream>
#include <iostream>

stringmap merge(stringmap a, stringmap b) 
{
	stringmap temp(a); temp.insert(b.begin(), b.end()); return temp;
}

namespace FTW
{

	CHTTPOperation::CHTTPOperation()
		: m_error(0)
	{
		CURLcode res = curl_global_init(CURL_GLOBAL_ALL);

		if (res)
			throw HTTPCURLException(res);

	}

	CHTTPOperation::~CHTTPOperation()
	{
		curl_global_cleanup();
	}

	size_t CHTTPOperation::GetData(void *ptr, size_t size, size_t nmemb, void *stream)
	{
		std::string data((const char*)ptr, (size_t)size * nmemb);
		*((std::stringstream*)stream) << data << std::endl;
		return size * nmemb;
	}

	size_t CHTTPOperation::WriteToFile(void *ptr, size_t size, size_t nmemb, void *data)
	{
		if (data != nullptr)
		{
			FILE* stream = reinterpret_cast<FILE*>(data);
			size_t written = fwrite((FILE*)ptr, size, nmemb, stream);

			if (ferror(stream))
				return 0;

			return written;
		}

		return size * nmemb;
	}

	size_t CHTTPOperation::ProcessHeader(char *buffer, size_t size, size_t nitems, void *userdata)
	{
		std::string data((const char*)buffer, (size_t)size * nitems);

		// Make sure sure the userdata pointer is pointing to something valid (hopefully)
		if (userdata == nullptr)
			return (size_t)size * nitems;

		// userdata should contain a pointer to a string
		//CHTTPFile& theFile = *reinterpret_cast<CHTTPFile*>(userdata);

		// userdata should contain a pointer to a HeaderInfo struct
		CHTTPOperation::HeaderInfo& hInfo = *reinterpret_cast<CHTTPOperation::HeaderInfo*>(userdata);

		// Look for the HTTP header containg the filename
		if (data.find("Content-Disposition: attachment; filename=", 0) == 0)
		{
			data = trim(data);

			// Find the filename token in the header
			size_t filenameStart = data.rfind("filename=") + 9;
			bool quoted = false;

			// If the first character after the filename token is a quote, we'll need to remove it 
			//  and remove the quote at the end also
			if (data.at(filenameStart) == '\"')
			{
				filenameStart++;
				quoted = true;
			}

			size_t filenameEnd = data.find(";", filenameStart);

			if (filenameEnd == std::string::npos)
				filenameEnd = data.length();

			size_t filenameLength = filenameEnd - filenameStart;

			if (quoted)
				filenameLength--;

			// Grab the full filename while removing quotes
			hInfo.targetFilename = data.substr(filenameStart, filenameLength);
		}
		else if (data.find("Content-Type:", 0) == 0)
		{
			data = trim(data);

			hInfo.mimetype = data.substr(14);
		}
		else if (data.find("HTTP/1.1", 0) == 0)
		{
			data = trim(data);

			hInfo.httpstatuscode = data.substr(9, 3);
			hInfo.httpstatusmsg = data.substr(13);
		}

		return (size_t)size * nitems;
	}

	std::string CHTTPOperation::GetFilenameFromUrl(const std::string& url)
	{
		std::string filename;

		size_t lastSlash = url.rfind("/");
		size_t queryParams = url.rfind("?");

		if (lastSlash && queryParams == std::string::npos)
			filename = url.substr(lastSlash + 1, url.length() - lastSlash - 1);
		else if (lastSlash)
			filename = url.substr(lastSlash + 1, queryParams - lastSlash - 1);
		else
			filename = url;

		return filename;

		//if (targetFilename.length() > 0)
		//{
		//	size_t extensionStart = targetFilename.rfind(".");

		//	if (extensionStart != std::string::npos)
		//	{
		//		extension = targetFilename.substr(extensionStart + 1, targetFilename.length() - extensionStart - 1);
		//		targetFilename = targetFilename.substr(0, extensionStart);
		//	}
		//}
	}

	std::string CHTTPOperation::Get(const std::string& url)
	{
		int headercount = 0;

		void* curl = curl_easy_init();

		if (curl == NULL)
			throw HTTPCURLException(CURLE_FAILED_INIT);

		else
		{
			curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
			curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, ProcessHeader);

			curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
			curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1); //Prevent "longjmp causes uninitialized stack frame" bug
			curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "deflate");

			std::stringstream out;

			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, GetData);
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, &out);

			/* Perform the request, res will get the return code */
			CURLcode res = curl_easy_perform(curl);

			/* Check for errors */
			if (res != CURLE_OK) {
				fprintf(stderr, "curl_easy_perform() failed: %s\n",
					curl_easy_strerror(res));
			}

			curl_easy_cleanup(curl);

			return out.str();
		}

		return "";

	}

	bool CHTTPOperation::DownloadFile(const char* url, CHTTPFile* pFile, bool moveAndRename, const char* pDestination)
	{
		std::string _url(url);

		void* curl = curl_easy_init();

		if (curl == NULL)
			throw HTTPCURLException(CURLE_FAILED_INIT);

		if (pFile && curl && _url.length() > 0)
		{
			pFile->Open();

			HeaderInfo hInfo;

			curl_easy_setopt(curl, CURLOPT_URL, _url.c_str());
			curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
			curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1); //Prevent "longjmp causes uninitialized stack frame" bug
			curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "deflate");

			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteToFile);
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, pFile->GetFILE());

			curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, ProcessHeader);
			curl_easy_setopt(curl, CURLOPT_HEADERDATA, &hInfo);

			CURLcode res = curl_easy_perform(curl);

			pFile->Close();
			curl_easy_cleanup(curl);

			/* Check for errors */
			if (res != CURLE_OK)
				throw HTTPCURLException(res);

			if (hInfo.httpstatuscode != "200")
				throw HTTPStatusException(hInfo.httpstatuscode, hInfo.httpstatusmsg);

			if (hInfo.targetFilename == "")
				hInfo.targetFilename = CHTTPOperation::GetFilenameFromUrl(_url);

			if (hInfo.targetFilename.rfind(".") == std::string::npos)
			{
				hInfo.targetFilename.append(".");
				hInfo.targetFilename.append(CHTTPFile::GetExtensionFromMimeType(hInfo.mimetype));
			}

			if (moveAndRename)
				pFile->MoveandRenameFile(hInfo.targetFilename, pDestination);

			return true;
		}

		return false;
	}

	bool CHTTPOperation::DownloadFile(PCTSTR url, CHTTPFile* pFile, bool moveAndRename, PCTSTR destination)
	{
		USES_CONVERSION;

		return DownloadFile(T2CA(url), pFile, moveAndRename, (T2CA(destination)));
	}

	const std::string CHTTPFile::GetFullFilename(void)
	{
		return folder + "\\" + filename;
	}

	void CHTTPFile::GetFullFilename(CString& fname)
	{
		USES_CONVERSION;

		fname = A2CT(std::string(folder + "\\" + filename).c_str());
	}

	CHTTPFile::CHTTPFile()
	{

	}

	stringmap CHTTPFile::mimemap = {
		{ "image/jpeg","jpg" },
		{ "image/bmp","bmp" },
		{ "image/gif","gif" },
		{ "image/x-icon","ico" },
		{ "image/png","png" },
		{ "application/json","js" },
		{ "text/xml","xml" } };

	void CHTTPFile::Open(void)
	{
		if (fp == nullptr)
		{
			char name[L_tmpnam_s];
			tmpnam_s(name);
			std::string tempFullname = name;

			errno_t error = fopen_s(&fp, tempFullname.c_str(), "wb");

			if (error != 0)
			{
				fp = nullptr;
				throw HTTPFileException(error);
			}

			size_t lastSlash = tempFullname.rfind('\\');

			if (lastSlash != std::string::npos)
			{
				folder = tempFullname.substr(0, lastSlash);
				filename = tempFullname.substr(lastSlash + 1, std::string::npos);
			}
			else
			{
				Close();
				throw HTTPFileException(1);
			}
		}
	}

	CHTTPFile::~CHTTPFile()
	{
		Close();
	}

	void CHTTPFile::Close(void)
	{
		if (fp != nullptr)
		{
			fclose(fp);
			fp = nullptr;
		}
	}

	const std::string& CHTTPFile::GetFilename(void)
	{
		return filename;
	}

	const std::string & CHTTPFile::GetFolder(void)
	{
		return folder;
	}

	const std::string CHTTPFile::GetExtensionFromMimeType(std::string mimetype)
	{
		if (!mimetype.empty())
		{
			auto ext = mimemap.find(mimetype);

			if (ext != mimemap.end())
				return ext->second;
		}

		return "";
	}

	bool CHTTPFile::MoveandRenameFile(std::string newFilename, std::string newFolder)
	{
		struct _stat buf;
		std::string dest;

		if (newFolder.empty())
			dest = folder;
		else
			dest = newFolder;

		if (_stat(newFilename.c_str(), &buf) == 0)
		{
			dest.append("\\");
			dest.append(newFilename);

			if (rename(GetFullFilename().c_str(), dest.c_str()) == 0)
			{
				folder = newFolder;
				filename = newFilename;
			}
			else
				throw HTTPFileException(errno);

			return true;
		}

		return false;
	}

	std::string trim(const std::string &s)
	{
		std::string::const_iterator it = s.begin();
		while (it != s.end() && isspace(*it))
			it++;

		std::string::const_reverse_iterator rit = s.rbegin();
		while (rit.base() != it && isspace(*rit))
			rit++;

		return std::string(it, rit.base());
	}

	bool CHTTPOperation::PostJsonData(const char* url, const char* data, std::string& result)
	{
		struct curl_slist *headers = NULL;
		//headers = curl_slist_append(headers, "Accept: application/json");
		headers = curl_slist_append(headers, "Content-Type: application/json");
		//headers = curl_slist_append(headers, "charsets: utf-8");

		bool postResult = PostData(url, headers, data, result);

		curl_slist_free_all(headers);

		return postResult;
	}

	bool CHTTPOperation::PostJsonData(PCTSTR url, PCTSTR data, CString& result)
	{
		USES_CONVERSION;

		std::string res;

		bool returnValue = PostJsonData(T2CA(url), T2CA(data), res);

		result = res.c_str();

		return returnValue;
	}

	bool CHTTPOperation::PostData(const char* url, struct curl_slist* headers, const char* data, std::string& result)
	{
		std::string _url(url);

		void* curl = curl_easy_init();

		if (curl == NULL)
			throw HTTPCURLException(CURLE_FAILED_INIT);

		if (curl && _url.length() > 0)
		{
			HeaderInfo hInfo;
			ContentinMemory content;

			curl_easy_setopt(curl, CURLOPT_URL, _url.c_str());
			curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
			curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);

			curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, ProcessHeader);
			curl_easy_setopt(curl, CURLOPT_HEADERDATA, &hInfo);

			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteToMemory);
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, &content);

			CURLcode res = curl_easy_perform(curl);

			curl_easy_cleanup(curl);

			/* Check for errors */
			if (res == CURLE_WRITE_ERROR)
			{
				throw FTW::FTWErrorCodeException(ERROR_OUTOFMEMORY);
			}
			else if (res != CURLE_OK)
				throw HTTPCURLException(res);

			result = content.memory;

			return true;
		}

		return false;
	}

	size_t CHTTPOperation::WriteToMemory(void *ptr, size_t size, size_t nmemb, void *data)
	{
		size_t realsize = size * nmemb;

		CHTTPOperation::ContentinMemory* mem = reinterpret_cast<CHTTPOperation::ContentinMemory*>(data);

		mem->memory = (char*)realloc(mem->memory, mem->size + realsize + 1);

		if (mem->memory == NULL)
		{
			/* out of memory! */
			return 0;
		}

		memcpy(&(mem->memory[mem->size]), ptr, realsize);
		mem->size += realsize;
		mem->memory[mem->size] = 0;

		return realsize;
	}

}