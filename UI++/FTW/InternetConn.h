#pragma once

#include <afxinet.h>

class CInternetConn
{
public:
	CInternetConn(void)
	{
		m_pSharedInternetSession = 0;

		if (!m_pSharedInternetSession)
			m_pSharedInternetSession = new CInternetSession;

	}
	~CInternetConn(void)
	{
		if (m_pSharedInternetSession)
		{
			m_pSharedInternetSession->Close();
			delete m_pSharedInternetSession;
			m_pSharedInternetSession = 0;
		}
	}
	bool DownloadFile(PCTSTR url, PCTSTR filename, DWORD& httpStatusCode)
	{
		CStdioFile* pFile = 0;
		DWORD flags = INTERNET_FLAG_DONT_CACHE | INTERNET_FLAG_RELOAD | INTERNET_FLAG_TRANSFER_ASCII;

		if (!m_pSharedInternetSession)
			return false;

		m_pSharedInternetSession->SetOption(INTERNET_OPTION_CONNECT_TIMEOUT, 20000);

		pFile = m_pSharedInternetSession->OpenURL(url, 1, flags);

		if (!pFile)
			return false;

		((CHttpFile*)pFile)->QueryInfoStatusCode(httpStatusCode);

		if (httpStatusCode != 200)
			return false;

		char buff[1025];
		UINT len = 0;
		FILE* pLocalFile = 0;

		errno_t e = fopen_s(&pLocalFile, CT2A(filename), "w");

		if (e != 0)
		{
			unsigned long g;
			_get_doserrno(&g);
			throw FTW::FTWErrorCodeException(g);
		}

		while ((len = pFile->Read(buff, 1024)))
		{
			buff[len] = 0;
			//fileContent += buff;

			fwrite(buff, 1, len, pLocalFile);
		}

		fclose(pLocalFile);

		pFile->Close();
		delete pFile;

		return true;
	}

protected:
	CInternetSession* m_pSharedInternetSession;
};