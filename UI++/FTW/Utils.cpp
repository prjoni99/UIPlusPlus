#include <stdafx.h>
#include "Utils.h"


HMODULE hMod;

namespace FTW
{
	std::wstring FormatHRString(HRESULT hr)
	{
		std::wstring strMsg = L""; //Final resting place for the string returned

		HINSTANCE hInst = NULL;
		wchar_t* pMsg = NULL; //Pointer to the final string, space is allocated by the function called
		DWORD nFlags = FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM;

		DWORD nSize = FormatMessage(nFlags, NULL, hr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&pMsg, 0, NULL);

		if (nSize > 0)
			strMsg = (wchar_t*)pMsg;
		else
		{
			hInst = LoadLibrary(L"C:\\Windows\\System32\\wbem\\wmiutils.dll");

			if (hInst != NULL)
			{
				nFlags = FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_IGNORE_INSERTS;
				nSize = FormatMessage(nFlags, hInst, hr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&pMsg, 0, NULL);

				if (nSize > 0)
					strMsg = (wchar_t*)pMsg;

				FreeLibrary(hInst);
			}

		}

		if (pMsg != 0)
			LocalFree(pMsg);

		return strMsg;

	}

	std::wstring FormatResourceString(DWORD dwStrID, ...)
	{
		std::wstring strMsg = L""; //Final resting place for the string returned

		if (dwStrID > 0)
		{
			wchar_t* pMsg; //Pointer to the final string, space is allocated by the function called
			DWORD nFlags = FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_STRING;
			LPVOID pSource = new wchar_t[MAX_STRING_LENGTH]; //Pointer to a string to load the resource string

			//Load the string from the resources based upon the ID passed in
			LoadString(hMod, dwStrID, (wchar_t*)pSource, MAX_STRING_LENGTH); //GetModuleHandle(NULL)

			va_list args;
			va_start(args, dwStrID);

			//Format the resource string replacing substitution paramaters with the variable arg list passed in
			//Allocats memory for lpMsg automatically
			DWORD nSize = FormatMessage(nFlags, pSource, dwStrID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
				(wchar_t*)&pMsg, 0, &args);

			va_end(args);

			if (nSize > 0)
				strMsg = (wchar_t*)pMsg;

			if (pSource != 0)
				delete[] pSource;

			if (pMsg != 0)
				LocalFree(pMsg);
		}

		return strMsg;
	};

	std::wstring FormatResourceString(DWORD dwStrID, va_list* args)
	{
		std::wstring strMsg = L""; //Final resting place for the string returned

		if (dwStrID > 0)
		{
			LPTSTR pMsg; //Pointer to the final string, space is allocated by the function called
			DWORD nFlags = FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_STRING;
			LPVOID pSource = new wchar_t[MAX_STRING_LENGTH]; //Pointer to a string to load the resource string

			//Load the string from the resources based upon the ID passed in
			LoadString(hMod, dwStrID, (wchar_t*)pSource, MAX_STRING_LENGTH);

			//Format the resource string replacing substitution paramaters with the variable arg list passed in
			//Allocats memory for lpMsg automatically
			DWORD nSize = FormatMessage(nFlags, pSource, dwStrID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
				(LPTSTR)&pMsg, 0, args);

			if (nSize > 0)
				strMsg = (LPTSTR)pMsg;

			if (pSource != 0)
				delete[] pSource;

			if (pMsg != 0)
				LocalFree(pMsg);
		}

		return strMsg;
	};

	bool IsTrue(PCTSTR value)
	{
		if (!value || wcsnlen(value, MAX_STRING_LENGTH) == 0)
			return false;

		PCTSTR value2 = nullptr;
		bool notValue = false;

		value2 = value;

		if (wcsnlen(value, MAX_STRING_LENGTH) > 1 && _wcsnicmp(value, L"!", 1) == 0)
		{
			notValue = true;
			value2++;
		}

		if (wcsnlen(value2, MAX_STRING_LENGTH) == 1)
		{
			if (_wcsnicmp(value2, STRING_YES, 1) == 0 || _wcsnicmp(value2, STRING_TRUE, 1) == 0 || _wcsnicmp(value2, L"1", 1) == 0)
				return true ^ notValue;

			if (_wcsnicmp(value2, STRING_NO, 1) == 0 || _wcsnicmp(value2, STRING_FALSE, 1) == 0 || _wcsnicmp(value2, L"0", 1) == 0)
				return false ^ notValue;
		}

		if (_wcsicmp(value2, STRING_YES) == 0 || _wcsicmp(value2, STRING_TRUE) == 0)
			return true ^ notValue;

		if (_wcsicmp(value2, STRING_NO) == 0 || _wcsicmp(value2, STRING_FALSE) == 0)
			return false ^ notValue;

		return false;
	}

	bool FileExists(PCTSTR filename)
	{
		if (filename && wcsnlen(filename, MAX_STRING_LENGTH) > 0)
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
		int index = 0;

		PTSTR pStop = 0;

		int value;
		BYTE r, g, b;

		if (wcslen(hexColor) != 7 && wcslen(hexColor) != 6)
			return defaultColor;

		if (wcslen(hexColor) == 7 && hexColor[0] == L'#')
			index = 1;

		value = wcstol(hexColor + index, &pStop, 16);

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
		LPFNISWOW64PROCESS pfnIsWow64Process = (LPFNISWOW64PROCESS)GetProcAddress(GetModuleHandle(L"kernel32"), "IsWow64Process");

		if (pfnIsWow64Process)
			pfnIsWow64Process(GetCurrentProcess(), &bIs64Bit);

#endif

		return bIs64Bit;
	}

	std::wstring FormatSeconds(int totalSeconds)
	{
		if (totalSeconds > 0)
		{
			int hours = totalSeconds / 3600;
			int minutes = (totalSeconds % 3600) / 60;
			int seconds = (totalSeconds % 3600) % 60;

			std::wstringstream temp, hrBuffer, minBuffer, secBuffer;
			hrBuffer.fill('0');
			hrBuffer.width(2);
			minBuffer.fill('0');
			minBuffer.width(2);
			secBuffer.fill('0');
			secBuffer.width(2);

			hrBuffer << hours;
			minBuffer << minutes;
			secBuffer << seconds;
			temp << hrBuffer.str() << ':' << minBuffer.str() << ':' << secBuffer.str();
			//value.Format(_T("%02d:%02d:%02d"), hours, minutes, seconds);
			return temp.str();
		}
		else
		{
			return L"00:00:00";
		}
	}

	std::wstring JoinPath(const std::wstring part1, const std::wstring part2)
	{
		std::wstring tmp = part1;

		if (tmp.at(tmp.length() - 1) != L'\\')
			tmp += L'\\';

		tmp += part2;

		return tmp;
	}


}