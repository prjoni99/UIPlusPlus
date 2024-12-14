// Utils.cpp
// 
// Copyright (c) Jason Sandys, 2009
//
// License: This code is released according to the 
// Microsoft Public License (Ms-PL) as documented at 
// http://UIappchooser.codeplex.com/license
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Utils.h"
#include "Constants.h"
//#include "InternetConn.h"

#include <regex>

namespace FTW
{

	PCTSTR BoolString(bool value)
	{
		if (value)
			return TRUE_STRING;
		else
			return FALSE_STRING;
	}

	PCTSTR BoolString(VARIANT_BOOL value)
	{
		if (value == VARIANT_TRUE)
			return TRUE_STRING;
		else
			return FALSE_STRING;
	}

	PCTSTR BoolString(BOOL value)
	{
		if (value == TRUE)
			return TRUE_STRING;
		else
			return FALSE_STRING;
	}

CString FormatHRString(HRESULT hr)
{
	CString strMsg = _T (""); //Final resting place for the string returned

	HINSTANCE hInst = NULL;
	LPTSTR lpMsg = NULL; //Pointer to the final string, space is allocated by the function called
	DWORD nFlags = FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM;

	DWORD nSize = FormatMessage (nFlags, NULL, hr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &lpMsg, 0, NULL);

	if (nSize > 0)
		strMsg = (LPTSTR)lpMsg;
	else
	{
		hInst = LoadLibrary (_T("C:\\Windows\\System32\\wbem\\wmiutils.dll"));

		if (hInst != NULL)
		{
			nFlags = FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_IGNORE_INSERTS;
			nSize = FormatMessage (nFlags, hInst, hr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &lpMsg, 0, NULL);

			if(nSize > 0)
				strMsg = (LPTSTR)lpMsg;

			FreeLibrary(hInst);
		}

	}

	if (lpMsg != 0)
		LocalFree (lpMsg);

	return strMsg;

}

CString FormatResourceString(DWORD dwStrID, ...)
{
	CString strMsg = _T (""); //Final resting place for the string returned

	if (dwStrID > 0)
	{
		LPTSTR lpMsg; //Pointer to the final string, space is allocated by the function called
		DWORD nFlags = FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_STRING;
		LPVOID lpSource = new _TCHAR[MAX_STRING_LENGTH]; //Pointer to a string to load the resource string

		//Load the string from the resources based upon the ID passed in
		LoadString(AfxGetInstanceHandle (), dwStrID, (LPTSTR)lpSource, MAX_STRING_LENGTH);

		va_list args;
		va_start(args, dwStrID);

		//Format the resource string replacing substitution paramaters with the variable arg list passed in
		//Allocats memory for lpMsg automatically
		DWORD nSize = FormatMessage (nFlags, lpSource, dwStrID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR) &lpMsg, 0, &args);
		
		va_end(args);

		if (nSize > 0)
			strMsg = (LPTSTR)lpMsg;

		if (lpSource != 0)
			delete lpSource;

		if (lpMsg != 0)
			LocalFree (lpMsg);
	}

	return strMsg;
};

CString FormatResourceString(DWORD dwStrID, va_list* args)
{
	CString strMsg = _T (""); //Final resting place for the string returned

	if (dwStrID > 0)
	{
		LPTSTR lpMsg; //Pointer to the final string, space is allocated by the function called
		DWORD nFlags = FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_STRING;
		LPVOID lpSource = new _TCHAR[MAX_STRING_LENGTH]; //Pointer to a string to load the resource string

		//Load the string from the resources based upon the ID passed in
		LoadString(AfxGetInstanceHandle (), dwStrID, (LPTSTR)lpSource, MAX_STRING_LENGTH);

		//Format the resource string replacing substitution paramaters with the variable arg list passed in
		//Allocats memory for lpMsg automatically
		DWORD nSize = FormatMessage (nFlags, lpSource, dwStrID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR) &lpMsg, 0, args);
		
		if (nSize > 0)
			strMsg = (LPTSTR)lpMsg;

		if (lpSource != 0)
			delete lpSource;

		if (lpMsg != 0)
			LocalFree (lpMsg);
	}

	return strMsg;
};

bool IsTrue(PCTSTR value)
{
	if (!value || _tcsnlen(value, MAX_STRING_LENGTH) == 0)
		return false;

	PCTSTR value2 = nullptr;
	bool notValue = false;

	value2 = value;

	if (_tcsnlen(value, MAX_STRING_LENGTH) > 1 && _tcsncicmp(value, _T("!"), 1) == 0)
	{
		notValue = true;
		value2++;
	}

	if (_tcsnlen(value2, MAX_STRING_LENGTH) == 1)
	{
		if (_tcsncicmp(value2, XML_ACTION_YES, 1) == 0 || _tcsncicmp(value2, XML_ACTION_TRUE, 1) == 0 || _tcsncicmp(value2, _T("1"), 1) == 0)
			return true ^ notValue;

		if (_tcsncicmp(value2, XML_ACTION_NO, 1) == 0 || _tcsncicmp(value2, XML_ACTION_FALSE, 1) == 0 || _tcsncicmp(value2, _T("0"), 1) == 0)
			return false ^ notValue;
	}

	if (_tcsicmp(value2, XML_ACTION_YES) == 0 || _tcsicmp(value2, XML_ACTION_TRUE) == 0)
		return true ^ notValue;

	if (_tcsicmp(value2, XML_ACTION_NO) == 0 || _tcsicmp(value2, XML_ACTION_FALSE) == 0)
		return false ^ notValue;

	return false;
}

bool FileExists(PCTSTR filename)
{
	if (filename && _tcsnlen(filename, MAX_STRING_LENGTH) > 0)
	{
		WIN32_FIND_DATA findFileData;
		HANDLE find;

		find = FindFirstFile(filename, &findFileData);

		if (find != INVALID_HANDLE_VALUE)
		{
			FindClose(find);
			return true;
		}

	}

	return false;
}

COLORREF HexToCOLORREF(PCTSTR hexColor, const COLORREF defaultColor)
{
	ASSERT(hexColor);
	int index = 0;

	PTSTR pStop = 0;

	int value;
	BYTE r, g, b;

	if (_tcslen(hexColor) != 7 && _tcslen(hexColor) != 6)
		return defaultColor;

	if (_tcslen(hexColor) == 7 && hexColor[0] == L'#')
		index = 1;

	value = _tcstol(hexColor + index, &pStop, 16);

	r = (value & 0XFF0000) >> 16;
	g = (value & 0x00FF00) >> 8;
	b = (value & 0X0000FF);

	return RGB(r, g, b);
}

BOOL Is64BitOS()
{
	BOOL bIs64Bit = FALSE;

#if defined(_WIN64)

	bIs64Bit = TRUE;  // 64-bit programs run only on Win64

#elif defined(_WIN32)

	// Note that 32-bit programs run on both 32-bit and 64-bit Windows

	typedef BOOL(WINAPI *LPFNISWOW64PROCESS) (HANDLE, PBOOL);
	LPFNISWOW64PROCESS pfnIsWow64Process = (LPFNISWOW64PROCESS)GetProcAddress(GetModuleHandle(_T("kernel32")), "IsWow64Process");

	if (pfnIsWow64Process)
		pfnIsWow64Process(GetCurrentProcess(), &bIs64Bit);

#endif

	return bIs64Bit;
}

// Read a single digit from the version. 
std::wistream& operator >> (std::wistream& str, Version::VersionDigit& digit)
{
	str.get();
	str >> digit.value;
	return str;
}

bool VersionCheck(PCTSTR versionA, PCTSTR versionB, PCTSTR versionOpr)
{
	if (versionA == nullptr || versionB == nullptr || _tcslen(versionA) == 0 || _tcslen(versionB) == 0)
		return false;

	Version A(versionA);
	Version B(versionB);

	if (_tcsncicmp(versionOpr, XML_ACTION_OPR_EQUAL, 3) == 0)
		return (A == B);
	if (_tcsncicmp(versionOpr, XML_ACTION_OPR_NE, 3) == 0)
		return !(A == B);
	if (_tcsncicmp(versionOpr, XML_ACTION_OPR_GT, 3) == 0)
		return !(A < B || A == B);
	if (_tcsncicmp(versionOpr, XML_ACTION_OPR_LT, 3) == 0)
		return (A < B);
	if (_tcsncicmp(versionOpr, XML_ACTION_OPR_GTE, 3) == 0)
		return !(A < B);
	if (_tcsncicmp(versionOpr, XML_ACTION_OPR_LTE, 3) == 0)
		return (A < B || A == B);
	else if (_tcsncicmp(versionOpr, XML_ACTION_OPR_RE, 3) == 0)
	{
		std::wregex regex;

		regex.assign(versionA, std::regex_constants::icase);

		return (std::regex_match(versionB, regex));
	}

	return (A == B);
}

CString FormatSeconds(int totalSeconds)
{
	if (totalSeconds > 0)
	{
		int hours = totalSeconds / 3600;
		int minutes = (totalSeconds % 3600) / 60;
		int seconds = (totalSeconds % 3600) % 60;

		CString value;
		value.Format(_T("%02d:%02d:%02d"), hours, minutes, seconds);

		return value;
	}
	else
	{
		return _T("00:00:00");
	}
}

CString JoinPath(PCTSTR part1, PCTSTR part2)
{
	CString tmp = part1;

	if (tmp.GetAt(tmp.GetLength() - 1) != _T('\\'))
		tmp += _T("\\");

	tmp += part2;

	return tmp;
}

//DWORD DownloadFileFromURL(CString& destinationPathandName, PCTSTR pURL, PCTSTR pDestination, bool binary)
//{
//	DWORD httpStatusReturn = 0;
//
//	if (_tcsncmp(pURL, _T("http://"), 7) == 0)
//	{
//	}
//
//	return httpStatusReturn;
//}


}
