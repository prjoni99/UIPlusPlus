#pragma once

#include <string>
#include <sstream>

#define MAX_STRING_LENGTH 1024
const wchar_t* const	STRING_TRUE = L"True";
const wchar_t* const	STRING_FALSE = L"False";
const wchar_t* const	STRING_YES = L"Yes";
const wchar_t* const	STRING_NO = L"No";

namespace FTW
{

	inline bool CheckForFlag(const int flags, const int flag) 
	{
		return ((flags & flag) == flag);
	};

	inline const wchar_t* BoolString(bool value)
	{
		if (value)
			return STRING_TRUE;
		else
			return STRING_FALSE;
	}

	inline const wchar_t* BoolString(BOOL value)
	{
		if (value == TRUE)
			return STRING_TRUE;
		else
			return STRING_FALSE;
	}

	inline double round(double x, int n)
	{
		int d = 0;

		if ((x * pow(10, n + 1)) - (floor(x * pow(10, n))) > 4)
			d = 1;

		x = (floor(x * pow(10, n)) + d) / pow(10, n);

		return x;
	}
	
	std::wstring FormatHRString(HRESULT hr);
	std::wstring FormatResourceString(DWORD dwStrID, ...);
	std::wstring FormatResourceString(DWORD dwStrID, va_list* args);
	
	bool IsTrue(PCTSTR value);

	BOOL Is64BitOS();
	
	bool FileExists(PCTSTR filename);
	
	COLORREF HexToCOLORREF(PCTSTR hexColor, const COLORREF defaultColor = RGB(0, 0, 0));
	
	std::wstring FormatSeconds(int totalSeconds);
	
	std::wstring JoinPath(const std::wstring part1, const std::wstring part2);
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