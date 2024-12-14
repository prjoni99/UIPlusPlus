#pragma once

#include "winldap.h"
#include <memory>
#include <atlstr.h>
#include "FTWCMLog.h"

#if defined(FTWLDAPLIBRARY_EXPORT) // inside DLL
#   define FTWLDAPAPI   __declspec(dllexport)
#else // outside DLL
#   define FTWLDAPAPI   __declspec(dllimport)
#endif  // FTWLDAPLIBRARY_EXPORT

namespace FTWLDAP
{
	struct ILdap
	{
		enum class LDAPObjectCategory { Person = 0, Group, Computer };

		virtual void Authenticate(PTSTR pUsername, PTSTR pPassword, PTSTR pDomain, PCTSTR pDomainController, bool doNotFallback) = 0;

		virtual ULONG GetRootNamingContext(CString& namingContext) = 0;
		virtual ULONG GetGroupMembership(PCTSTR pUsername, TCHAR**& ppAttributeVal) = 0;
		virtual CString ExtractGroupNamesFromAttributeList(TCHAR**& ppAttributeVal) = 0;

		virtual ULONG GetAttributeValue(PCTSTR pSearchBase, ULONG scope, PCTSTR searchFilter, PCTSTR attributeName, CString& attributeValue) = 0;
		virtual ULONG GetAttributeValue(PCTSTR pSearchBase, ULONG scope, PCTSTR searchFilter, PCTSTR attributeName, TCHAR **& ppAttributeVal) = 0;
		virtual ULONG GetAttributeValueFromObject(PCTSTR pAttributeName, PCTSTR pObjectName, LDAPObjectCategory objCategory, CString& value) = 0;
		virtual bool IsValueinAttributeList(TCHAR**& ppAttributeList, PCTSTR pValue) = 0;
		
		virtual bool IsValid() = 0; 
		
		virtual void Release() = 0;
	};

	typedef std::shared_ptr<ILdap> ILdapPtr;
	typedef ILdap* LDAPHANDLE;

	extern "C" FTWLDAPAPI LDAPHANDLE APIENTRY GetLdap(FTWCMLOG::ICMLogPtr pLog);

	class CLdap : public ILdap
	{
	public:
		CLdap(FTWCMLOG::ICMLogPtr pLog);
		~CLdap();

		void Authenticate(PTSTR pUsername, PTSTR pPassword, PTSTR pDomain, PCTSTR pDomainController, bool doNotFallback = false);

		ULONG GetRootNamingContext(CString& namingContext);
		ULONG GetGroupMembership(PCTSTR pUsername, TCHAR**& ppAttributeVal);
		CString ExtractGroupNamesFromAttributeList(TCHAR**& ppAttributeVal);

		ULONG GetAttributeValue(PCTSTR pSearchBase, ULONG scope, PCTSTR searchFilter, PCTSTR attributeName, CString& attributeValue);
		ULONG GetAttributeValue(PCTSTR pSearchBase, ULONG scope, PCTSTR searchFilter, PCTSTR attributeName, TCHAR **& ppAttributeVal);
		ULONG GetAttributeValueFromObject(PCTSTR pAttributeName, PCTSTR pObjectName, LDAPObjectCategory objCategory, CString& value);
		bool IsValueinAttributeList(TCHAR**& ppAttributeList, PCTSTR pValue);

		bool IsValid() { return m_ldap != nullptr; };

		void Release() { delete this; }

		CLdap(const CLdap&) = delete;

		operator const LDAP*() { return m_ldap; }

	protected:
		ULONG _GetAttributeValue(PCTSTR pSearchBase, ULONG scope, PCTSTR searchFilter, PCTSTR attributeName, CString& attributeValue, TCHAR **& ppAttributeVal);
		ULONG _GetAttributeValue(PCTSTR pSearchBase, ULONG scope, PCTSTR searchFilter, PCTSTR attributeName, CString& attributeValue);

		void Unbind()
		{
			if (m_ldap != nullptr)
			{
				ldap_unbind(m_ldap);
				m_ldap = nullptr;
			}
		}

		bool FindClosetDomainController(PTSTR pDomain);

		ULONG InitandConnect();

		static PCTSTR LDAPCategoryString(LDAPObjectCategory cat)
		{
			static PCTSTR person = _T("Person");
			static PCTSTR group = _T("Group");
			static PCTSTR computer = _T("Computer");
			static PCTSTR empty = _T("");

			if (cat == ILdap::LDAPObjectCategory::Person)
				return person;
			else if (cat == ILdap::LDAPObjectCategory::Group)
				return group;
			else if (cat == ILdap::LDAPObjectCategory::Computer)
				return computer;
			else return empty;
		}

		LDAP * m_ldap = nullptr;
		FTWCMLOG::ICMLogPtr m_pCMLog;

		CString m_rootLdapPath = L"";
		CString m_domain = L"";
		CString m_domainController = L"";

		UINT m_authResult = LDAP_SUCCESS;
	};

}