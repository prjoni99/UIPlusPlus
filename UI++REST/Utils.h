// Utils.h
// 
// Copyright (c) Jason Sandys, 2009
//
// License: This code is released according to the 
// Microsoft Public License (Ms-PL) as documented at 
// http://UIappchooser.codeplex.com/license
//
// Some code in this file is based on a series of articles by David 
// Sackstein: http://blogs.microsoft.co.il/blogs/davids/archive/2008/12/20/msxml-in-c-but-as-elegant-as-c.aspx
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include <iostream>
#include <sstream>
#include <vector>
#include <iterator>


#define MAX_STRING_LENGTH 1024

namespace FTW
{

class ComInit
{
public:
	ComInit(bool init = true) : m_hrInit(CO_E_NOTINITIALIZED) { if (init) Init(); }
	~ComInit() 
	{ 
		if (m_hrInit == S_OK || m_hrInit == S_FALSE) 
		{ 
			::CoUninitialize(); 
//			TRACE("INFO: COM uninitizalized\n"); 
		}
	}
	
	static HRESULT InitSecurity(void) { return ::CoInitializeSecurity (NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_DEFAULT, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE, NULL); }

	static ComInit& Instance()
	{
		static ComInit _COM;
		return _COM;
	}

	HRESULT InitStatus(void) { return m_hrInit; }
	HRESULT Init(void) 
	{
		if (m_hrInit == CO_E_NOTINITIALIZED)
			m_hrInit = ::CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
		
		if (m_hrInit == S_OK)
//			TRACE("INFO: COM initizalized\n"); 
		
		return m_hrInit; 
	}

private:
	HRESULT m_hrInit;

};

inline bool CheckForFlag(const short flags, const short flag) {
	return ((flags & flag) == flag);
};

PCTSTR BoolString(bool value);

PCTSTR BoolString(VARIANT_BOOL value);

PCTSTR BoolString(BOOL value);

CString FormatHRString(HRESULT hr);
CString FormatResourceString(DWORD dwStrID, ...);
CString FormatResourceString(DWORD dwStrID, va_list* args);

//DWORD DownloadFileFromURL(CString& destinationPathandName, PCTSTR pURL, PCTSTR pDestination = nullptr, bool binary = false);

COLORREF HexToCOLORREF(PCTSTR hexColor, const COLORREF defaultColor = RGB(0, 0, 0));

BOOL Is64BitOS();

bool IsTrue(PCTSTR value);

bool FileExists(PCTSTR filename); 

CString JoinPath(PCTSTR part1, PCTSTR part2);

class Version
{
	// An internal utility structure just used to make the std::copy in the constructor easy to write.
	struct VersionDigit
	{
		int value;
		operator int() const { return value; }
	};
	friend std::wistream& operator >> (std::wistream& str, Version::VersionDigit& digit);

public:
	Version(std::wstring const& versionStr)
	{
		// To Make processing easier in VersionDigit prepend a '.'
		std::wstringstream versionStream(std::wstring(L".") + versionStr);

		// Copy all parts of the version number into the version Info vector.
		std::copy(std::istream_iterator<VersionDigit, wchar_t>(versionStream),
			std::istream_iterator<VersionDigit, wchar_t>(),
			std::back_inserter(versionInfo)
		);
	}

	// Test if two version numbers are the same. 
	bool operator<(Version const& rhs) const
	{
		return std::lexicographical_compare(versionInfo.begin(), versionInfo.end(), rhs.versionInfo.begin(), rhs.versionInfo.end());
	}

	bool operator==(Version const& rhs) const
	{
		return (!(std::lexicographical_compare(versionInfo.begin(), versionInfo.end(), rhs.versionInfo.begin(), rhs.versionInfo.end()) ||
			std::lexicographical_compare(rhs.versionInfo.begin(), rhs.versionInfo.end(), versionInfo.begin(), versionInfo.end())));
	}

private:
	std::vector<int>    versionInfo;
};

bool VersionCheck(PCTSTR versionA, PCTSTR versionB, PCTSTR versionOpr = L"eq");

CString FormatSeconds(int totalSeconds);
}

#define WIDEN2(x) L ## x
#define WIDEN(x) WIDEN2(x)
#define __WFILE__ WIDEN(__FILE__)
//wchar_t *pwsz = __WFILE__;

#ifdef _UNICODE
#define __TFILE__ __WFILE__
#else
#define __TFILE__ __FILE__
#endif