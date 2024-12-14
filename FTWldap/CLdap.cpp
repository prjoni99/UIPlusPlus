#include "stdafx.h"
#include "CLdap.h"
#include "dsgetdc.h"
#include "lm.h"
#include "Utils.h"
#include "resource.h"
#include <Rpc.h>
#include <cassert>

#define MAX_STRING_LENGTH 1024
#define EXPORT comment(linker, "/EXPORT:" __FUNCTION__ "=" __FUNCDNAME__)

#define WIDEN2(x) L ## x
#define WIDEN(x) WIDEN2(x)
#define __WFILE__ WIDEN(__FILE__)
//wchar_t *pwsz = __WFILE__;

#ifdef _UNICODE
#define __TFILE__ __WFILE__
#else
#define __TFILE__ __FILE__
#endif

namespace FTWLDAP
{
	//#if !defined(_WIN64)
	//	// This pragma is required only for 32-bit builds. In a 64-bit environment,
	//	// C functions are not decorated.
	//#pragma comment(linker, "/export:GetLdap=_GetLdap@0")
	//#endif  // _WIN64

	FTWLDAPAPI LDAPHANDLE APIENTRY GetLdap(FTWCMLOG::ICMLogPtr pLog)
	{
#if !defined(_WIN64)
		// This pragma is required only for 32-bit builds. In a 64-bit environment,
		// C functions are not decorated.
#pragma EXPORT
#endif  // _WIN64

		return new CLdap(pLog);
	}
	
	CLdap::CLdap(FTWCMLOG::ICMLogPtr pLog)
	: m_pCMLog(pLog), m_domainController(_T(""))
	{}

	CLdap::~CLdap()
	{
		Unbind();
	}

	bool CLdap::FindClosetDomainController(PTSTR pDomain)
	{
		m_pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info,
			GetCurrentThreadId(), __TFILE__, __LINE__,
			FTW::FormatResourceString(IDS_LOGMSG_LOOKINGFORDOMAINCONTROLLER));

		PDOMAIN_CONTROLLER_INFO dcInfo;

		DWORD retValue = DsGetDcName(nullptr, pDomain, nullptr, nullptr,
			DS_IS_DNS_NAME | DS_RETURN_DNS_NAME | DS_TRY_NEXTCLOSEST_SITE,
			&dcInfo);

		if (retValue == ERROR_SUCCESS)
		{
			m_domainController = dcInfo->DomainControllerName;

			if (m_domainController.Left(2) == _T("\\\\"))
				m_domainController.Delete(0, 2);

			m_pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info,
				GetCurrentThreadId(), __TFILE__, __LINE__,
				FTW::FormatResourceString(IDS_LOGMSG_FOUNDDOMAINCONTROLLER, dcInfo->ClientSiteName, m_domainController));

			NetApiBufferFree(dcInfo);

		}
		else
		{
			m_pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info,
				GetCurrentThreadId(), __TFILE__, __LINE__,
				FTW::FormatResourceString(IDS_LOGERROR_DIDNOTFINDDOMAINCONTROLLER));

			return false;
		}

		return true;
	}

	ULONG CLdap::InitandConnect()
	{
		m_ldap = ldap_init(m_domainController.GetBuffer(), LDAP_PORT);

		if (m_ldap == NULL)
		{
			throw LdapGetLastError();
		}

		unsigned ldapVersion = LDAP_VERSION3;
		ULONG result = ldap_set_option(m_ldap, LDAP_OPT_VERSION, &ldapVersion);

		assert(result == LDAP_SUCCESS);

		result = ldap_set_option(m_ldap, LDAP_OPT_ENCRYPT, LDAP_OPT_ON);

		assert(result == LDAP_SUCCESS);

		result = ldap_set_option(m_ldap, LDAP_OPT_SIGN, LDAP_OPT_ON);

		assert(result == LDAP_SUCCESS);

		return ldap_connect(m_ldap, NULL);
	}

	void CLdap::Authenticate(PTSTR pUsername, PTSTR pPassword, PTSTR pDomain, PCTSTR pDomainController, bool doNotFallback)
	{
		m_domain = pDomain;
		bool foundDomainController = false;

		Unbind();

		if (m_domainController.GetLength() > 0)
		{
			foundDomainController = true;
		}
		else if (pDomainController == nullptr)
		{
			foundDomainController = FindClosetDomainController(pDomain);
		}
		else
		{
			m_domainController = pDomainController;

			m_pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info,
				GetCurrentThreadId(), __TFILE__, __LINE__,
				FTW::FormatResourceString(IDS_LOGMSG_USINGDOMAINCONTROLLER, m_domainController));	
		}

		ULONG result = InitandConnect();

		if ((result == LDAP_SERVER_DOWN || result == LDAP_UNAVAILABLE) && doNotFallback == false && foundDomainController == false)
		{
			m_pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info,
				GetCurrentThreadId(), __TFILE__, __LINE__,
				FTW::FormatResourceString(IDS_LOGERROR_CANNOTUSEDDOMAINCONTROLLER, ldap_err2string(result)));

			FindClosetDomainController(pDomain);

			result = InitandConnect();
		}

		if (result == LDAP_SUCCESS)
		{
			SEC_WINNT_AUTH_IDENTITY auth;
			ZeroMemory(&auth, sizeof(auth));

			auth.User = reinterpret_cast<unsigned short*> (pUsername);
			auth.UserLength = (unsigned long)wcsnlen(pUsername, 36);
			auth.Domain = reinterpret_cast<unsigned short*> (pDomain);
			auth.DomainLength = (unsigned long)wcsnlen(pDomain, 36);
			auth.Password = reinterpret_cast<unsigned short*> (pPassword);
			auth.PasswordLength = (unsigned long)wcsnlen(pPassword, 36);
			auth.Flags = SEC_WINNT_AUTH_IDENTITY_UNICODE;

			result = ldap_bind_s(m_ldap, NULL, (TCHAR*)(&auth), LDAP_AUTH_NEGOTIATE);

			SecureZeroMemory(&auth, sizeof(auth));
		}

		m_authResult = result;

		if (result != LDAP_SUCCESS)
		{
			Unbind();
			throw result;
		}
	}
	
	ULONG CLdap::GetRootNamingContext(CString& namingContext)
	{
		ULONG returnvalue = LDAP_SUCCESS;

		assert(m_ldap);

		if (m_ldap != nullptr)
		{
			if (m_rootLdapPath.GetLength() == 0)
			{
				TCHAR base[] = L"";
				TCHAR filter[] = L"(objectClass=*)";
				TCHAR attribute[] = L"defaultNamingContext";

				returnvalue = _GetAttributeValue(base, LDAP_SCOPE_BASE, filter, attribute, m_rootLdapPath);
			}

			if (returnvalue == LDAP_SUCCESS)
				namingContext = m_rootLdapPath;
				//wcscpy_s(pNamingContext, namingContextSize, m_rootLdapPath.c_str());
		}

		return returnvalue;
	}

	ULONG CLdap::GetGroupMembership(PCTSTR pUsername, TCHAR**& ppAttributeVal)
	{
		TCHAR pSearchFilter[MAX_STRING_LENGTH];
		TCHAR attribute[] = L"memberOf";

		swprintf_s(pSearchFilter, MAX_STRING_LENGTH, L"(&(objectCategory=Person)(sAMAccountName=%s))", pUsername);

		ULONG result = GetAttributeValue(m_rootLdapPath.GetBuffer(), LDAP_SCOPE_SUBTREE, pSearchFilter, attribute, ppAttributeVal);

		m_rootLdapPath.ReleaseBuffer();

		return result;

	}

	CString CLdap::ExtractGroupNamesFromAttributeList(TCHAR**& ppAttributeVal)
	{
		CString groupNameList = _T("");
		
		if (ppAttributeVal != NULL)
		{
			int groupCount = ldap_count_values(ppAttributeVal);
			CString groupDN;
			CString groupName;
			TCHAR filter[] = _T("(objectClass=*)");
			TCHAR attribute[] = _T("sAMAccountName");

			for (int count = 0; count < groupCount; count++)
			{
				groupDN = ppAttributeVal[count];

				if (GetAttributeValue(groupDN.GetBuffer(), LDAP_SCOPE_BASE, filter, attribute, groupName) == LDAP_SUCCESS)
				{
					if (groupNameList.GetLength() > 0)
						groupNameList.AppendChar(',');

					groupNameList += groupName;
				}

				groupDN.ReleaseBuffer();
			}
		}

		return groupNameList;
	}

	ULONG CLdap::GetAttributeValue(PCTSTR pSearchBase, ULONG scope, PCTSTR searchFilter, PCTSTR attributeName, CString& attributeValue)
	{
		TCHAR** ppAttr = nullptr;

		assert(m_ldap);

		ULONG returnvalue = _GetAttributeValue(pSearchBase, scope, searchFilter, attributeName, attributeValue, ppAttr);

		if (ppAttr != nullptr)
		{
			ldap_value_free(ppAttr);
			ppAttr = nullptr;
		}

		return returnvalue;
	}

	ULONG CLdap::GetAttributeValue(PCTSTR pSearchBase, ULONG scope, PCTSTR searchFilter, PCTSTR attributeName, TCHAR**& ppAttributeVal)
	{
		CString attrVal;

		assert(m_ldap);

		return _GetAttributeValue(pSearchBase, scope, searchFilter, attributeName, attrVal, ppAttributeVal);

	}

	ULONG CLdap::GetAttributeValueFromObject(PCTSTR pAttributeName, PCTSTR pObjectName, LDAPObjectCategory objCategory, CString& value)
	{
		TCHAR pSearchFilter[MAX_STRING_LENGTH];

		if(objCategory == LDAPObjectCategory::Person)
			_stprintf_s(pSearchFilter, MAX_STRING_LENGTH, _T("(&(objectCategory=%s)(objectClass=user)(sAMAccountName=%s))"), LDAPCategoryString(objCategory), pObjectName);
		else
			_stprintf_s(pSearchFilter, MAX_STRING_LENGTH, _T("(&(objectCategory=%s)(sAMAccountName=%s))"), LDAPCategoryString(objCategory), pObjectName);

		ULONG returnValue = GetAttributeValue(m_rootLdapPath.GetBuffer(), LDAP_SCOPE_SUBTREE, pSearchFilter, pAttributeName, value);

		//FTW::CCMLog::Instance().LogMsg(FTW::CCMLog::Error, AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
		//	IDS_LOGERROR_OBJECTATTRIBUTE, pAttributeName, pObjectName, ldap_err2string(result));

		m_rootLdapPath.ReleaseBuffer();

		return returnValue;
	}

	ULONG CLdap::_GetAttributeValue(PCTSTR pSearchBase, ULONG scope, PCTSTR searchFilter, PCTSTR attributeName, CString& attributeValue)
	{
		TCHAR** ppAttr = nullptr;

		assert(m_ldap);

		ULONG returnvalue = _GetAttributeValue(pSearchBase, scope, searchFilter, attributeName, attributeValue, ppAttr);

		if (ppAttr != nullptr)
		{
			ldap_value_free(ppAttr);
			ppAttr = nullptr;
		}

		return returnvalue;
	}

	ULONG CLdap::_GetAttributeValue(PCTSTR pSearchBase, ULONG scope, PCTSTR searchFilter, PCTSTR attributeName, CString& attributeValue, TCHAR**& ppAttributeVal) 
	{
		assert(m_ldap);

		CString searchBase(pSearchBase);

		LPCTSTR attribs[2];
		attribs[0] = attributeName;
		attribs[1] = NULL;

		LDAPMessage* pLdapMsg;
		ULONG ldapResult = LDAP_SUCCESS;
		attributeValue.Empty();

		ldapResult = ldap_search_s(m_ldap, const_cast<PTSTR>(pSearchBase), scope, const_cast<PTSTR>(searchFilter), const_cast<PZPWSTR>(attribs), 0, &pLdapMsg);

		if (ldapResult == LDAP_SUCCESS)
		{
			LDAPMessage* pLdapEntry;

			pLdapEntry = ldap_first_entry(m_ldap, pLdapMsg);

			if (pLdapEntry)
			{
 				ppAttributeVal = ldap_get_values(m_ldap, pLdapEntry, const_cast<PTSTR>(attributeName));

				if (ppAttributeVal != NULL)
				{
					if (ldap_count_values(ppAttributeVal) > 0)
						attributeValue = ppAttributeVal[0];
				}
				else
					ldapResult = LdapGetLastError();
			}
		}

		if (pLdapMsg)
			ldap_msgfree(pLdapMsg);

		return ldapResult;
	}

	bool CLdap::IsValueinAttributeList(TCHAR**& ppAttributeList, PCTSTR pValue)
	{
		bool returnValue = false;

		if (ppAttributeList != NULL)
		{
			int attributeCount = ldap_count_values(ppAttributeList);
			CString attributeValue;

			for (int count = 0; count < attributeCount; count++)
			{
				attributeValue = ppAttributeList[count];

				if (attributeValue == pValue)
				{
					returnValue = true;
					break;
				}
			}
		}

		return returnValue;

	}
}