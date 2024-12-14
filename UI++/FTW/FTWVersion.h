#pragma once

#include <iostream>
#include <sstream>
#include <vector>
#include <iterator>
#include <regex>

const wchar_t*	XML_ACTION_OPR_EQUAL =			L"eq";
const wchar_t* 	XML_ACTION_OPR_GT =				L"gt";
const wchar_t* 	XML_ACTION_OPR_GTE =			L"gte";
const wchar_t* 	XML_ACTION_OPR_LT =				L"lt";
const wchar_t* 	XML_ACTION_OPR_LTE =			L"lte";
const wchar_t* 	XML_ACTION_OPR_NE =				L"ne";
const wchar_t* 	XML_ACTION_OPR_RE =				L"re";

namespace FTW
{
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

	// Read a single digit from the version. 
	std::wistream& operator >> (std::wistream& str, Version::VersionDigit& digit)
	{
		str.get();
		str >> digit.value;
		return str;
	}

	bool VersionCheck(const wchar_t* versionA, const wchar_t* versionB, const wchar_t* versionOpr = L"eq")
	{
		if (versionA == nullptr || versionB == nullptr || wcslen(versionA) == 0 || wcslen(versionB) == 0)
			return false;

		Version A(versionA);
		Version B(versionB);

		if (_wcsnicmp(versionOpr, XML_ACTION_OPR_EQUAL, 3) == 0)
			return (A == B);
		if (_wcsnicmp(versionOpr, XML_ACTION_OPR_NE, 3) == 0)
			return !(A == B);
		if (_wcsnicmp(versionOpr, XML_ACTION_OPR_GT, 3) == 0)
			return !(A < B || A == B);
		if (_wcsnicmp(versionOpr, XML_ACTION_OPR_LT, 3) == 0)
			return (A < B);
		if (_wcsnicmp(versionOpr, XML_ACTION_OPR_GTE, 3) == 0)
			return !(A < B);
		if (_wcsnicmp(versionOpr, XML_ACTION_OPR_LTE, 3) == 0)
			return (A < B || A == B);
		else if (_wcsnicmp(versionOpr, XML_ACTION_OPR_RE, 3) == 0)
		{
			std::wregex regex;

			regex.assign(versionA, std::regex_constants::icase);

			return (std::regex_match(versionB, regex));
		}

		return (A == B);
	}
}