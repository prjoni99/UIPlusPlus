#pragma once

#include <memory>
#include <map>
#include <unordered_map>
//#include <functional>
#include "fnv1a.hpp"

//template < >
//struct std::hash<CString>
//{
//	size_t operator()(const CString& s) const
//	{
//		return std::hash_value<std::string>{}((PCTSTR)s);
//	}
//};

namespace std {
	template <>
	struct hash<CString>
	{   // hash functor for CString
		size_t operator()(const CString& _Keyval) const
		{   // hash _Keyval to size_t value by pseudorandomizing transform
			return (fnv1a_hash_bytes((const unsigned char*)(LPCWSTR)_Keyval, _Keyval.GetLength() * sizeof(wchar_t)));
		}
	};

	template <>
	struct hash<CStringA>
	{   // hash functor for CStringA
		size_t operator()(const CStringA& _Keyval) const
		{   // hash _Keyval to size_t value by pseudorandomizing transform
			return (fnv1a_hash_bytes((const unsigned char*)(LPCSTR)_Keyval, _Keyval.GetLength() * sizeof(char)));
		}
	};
}

namespace UIpp
{
	class CSoftware
	{
	public:
		CSoftware(PCTSTR id, PCTSTR label, PCTSTR info, PCTSTR includeIds, PCTSTR excludeIds, int index)
			: m_label(label), m_info(info), m_id(id), m_includeIds(includeIds), m_excludeIds(excludeIds), m_index(index) {};
		virtual ~CSoftware() {};

		CSoftware(CSoftware& soft)
		{
			*this = soft;
		}

		CSoftware& operator=(const CSoftware& soft)
		{
			m_label = soft.m_label;
			m_info = soft.m_info;
			m_id = soft.m_id;
			m_includeIds = soft.m_includeIds;
			m_excludeIds = soft.m_excludeIds;
			m_index = soft.m_index;

			return *this;
		};

		CSoftware(CSoftware&& soft) noexcept
		{
			*this = std::move(soft);
		}

		CSoftware& operator=(CSoftware&& soft) noexcept
		{
			m_label = std::move(soft.m_label);
			m_info = std::move(soft.m_info);
			m_id = std::move(soft.m_id);
			m_includeIds = std::move(soft.m_includeIds);
			m_excludeIds = std::move(soft.m_excludeIds);
			m_index = std::move(soft.m_index);

			return *this;
		};


		PCTSTR GetLabel(void) const { return m_label; }
		PCTSTR GetInfo(void) const { return m_info; }
		PCTSTR GetID(void) const { return m_id; }
		int GetIndex(void) const { return m_index; }

		virtual CString GetVariableValue(void) const = 0;
		CString GetIncludes(void) const { return m_includeIds; }
		CString GetExcludes(void) const { return m_excludeIds; }

		enum class SoftwareType { APPLICATION = 1, PACKAGE };

		virtual SoftwareType Type(void) const = 0;

	protected:

		CString m_label;
		CString m_info;
		CString m_id;
		CString m_includeIds;
		CString m_excludeIds;
		int m_index;
	};

	//class CSoftwareIndexCompare
	//{
	//public:
	//	bool operator()(CSoftware* pS1, CSoftware* pS2)
	//	{
	//		return (pS1->GetIndex() < pS2->GetIndex());
	//	}
	//};
	//
	//class CSoftwareIDCompare
	//{
	//public:
	//	bool operator()(CSoftware* pS1, CSoftware* pS2)
	//	{
	//		return (_tcscmp(pS1->GetID(), pS2->GetID()) < 0);
	//	}
	//};

	//class CSoftwareIDCompare
	//{
	//public:
	//	bool operator()(CSoftware* pS1, CSoftware* pS2)
	//	{
	//		return (_tcscmp(pS1->GetID(), pS2->GetID()) < 0);
	//	}
	//};

	class CApplication : public CSoftware
	{
	public:
		CApplication(PCTSTR id, PCTSTR label, PCTSTR info, PCTSTR appName, PCTSTR includeIds, PCTSTR excludeIds, int index)
			: CSoftware(id, label, info, includeIds, excludeIds, index), m_appName(appName) {};
		~CApplication() {};

		CApplication(CApplication& app)
			: CSoftware(app.m_id, app.m_label, app.m_info, app.m_includeIds, app.m_excludeIds, app.m_index), m_appName(app.m_appName)
		{};

		CApplication& operator=(CApplication& app)
		{
			CSoftware::operator=(app);

			m_appName = std::move(app.m_appName);
		};

		CApplication(CApplication&& app) noexcept
			: CSoftware(app.m_id, app.m_label, app.m_info, app.m_includeIds, app.m_excludeIds, app.m_index)
		{
			m_appName = std::move(app.m_appName);
		};

		CApplication& operator=(CApplication&& app) noexcept
		{
			CSoftware::operator=(app);

			m_appName = std::move(app.m_appName);
		};


		CString GetVariableValue(void) const
		{
			return m_appName;
		}

		SoftwareType Type(void) const { return SoftwareType::APPLICATION; }

	protected:

		CString m_appName;
	};

	class CPackage : public CSoftware
	{
	public:
		CPackage(PCTSTR id, PCTSTR label, PCTSTR info, PCTSTR pkgID, PCTSTR progName, PCTSTR includeIds, PCTSTR excludeIds, int index)
			: CSoftware(id, label, info, includeIds, excludeIds, index), m_pkgID(pkgID), m_programName(progName) {};
		~CPackage() {};

		CPackage(CPackage& pkg)
			: CSoftware(pkg.m_id, pkg.m_label, pkg.m_info, pkg.m_includeIds, pkg.m_excludeIds, pkg.m_index), m_pkgID(pkg.m_pkgID), m_programName(pkg.m_programName)
		{};

		CPackage& operator=(CPackage& pkg)
		{
			CSoftware::operator=(pkg);

			m_pkgID = pkg.m_pkgID;
			m_programName = pkg.m_programName;
		};

		CPackage(CPackage&& pkg) noexcept
			: CSoftware(pkg.m_id, pkg.m_label, pkg.m_info, pkg.m_includeIds, pkg.m_excludeIds, pkg.m_index)
		{
			m_pkgID = std::move(pkg.m_pkgID);
			m_programName = std::move(pkg.m_programName);
		};

		CPackage& operator=(CPackage&& pkg) noexcept
		{
			CSoftware::operator=(pkg);

			m_pkgID = std::move(pkg.m_pkgID);
			m_programName = std::move(pkg.m_programName);
		};

		CString GetVariableValue(void) const
		{
			CString tmp;
			tmp.Format(_T("%s:%s"), m_pkgID.GetString(), m_programName.GetString());

			return tmp;
		}

		SoftwareType Type(void) const { return SoftwareType::PACKAGE; }

	protected:

		CString m_pkgID;
		CString m_programName;
	};

	typedef std::unique_ptr<CSoftware> CSoftwarePtr;

	typedef std::unordered_map<CString, CSoftwarePtr> SoftwareMap;
	typedef std::map<int, CSoftware*> OrderedSoftwareMap;
}



//typedef std::unique_ptr<CTreeItem> CTreeItemPtr;
//typedef std::vector<CTreeItemPtr> TreeItemList;

//typedef std::unordered_map<CString, CSoftware*> SoftwareMap;
//typedef std::map<int, const CSoftware* const> OrderedSoftwareMap;
//typedef std::forward_list<CTreeItem*> TreeItemList;
//typedef std::unordered_set<CString> StringSet;

//struct TreeItemID : public std::binary_function < CTreeItem*, PCTSTR, bool >
//{
//	bool operator () (const CTreeItem* ti, PCTSTR id) const
//	{
//		return (_tcscmp(ti->GetID(), id) == 0);
//	}
//};

//bool operator==(const CTreeItem& TI, PCTSTR id) {
//	return (_tcscmp(TI.GetID(), id) == 0);
//};