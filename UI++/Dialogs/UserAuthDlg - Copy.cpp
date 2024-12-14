// UserAuthDlg.cpp : implementation file
//

#include "stdafx.h"
#include "UI++.h"
#include "UserAuthDlg.h"
#include "afxdialogex.h"
#include "TSvar.h"
#include "Constants.h"

//#include "activeds.h"

// CUserAuthDlg dialog

IMPLEMENT_DYNAMIC(CUserAuthDlg, CDialog)

CUserAuthDlg::CUserAuthDlg(PCTSTR groupName, bool getGroups, bool showBack, bool disableCancel, 
	PCTSTR groupsForUser, PCTSTR attributesForUser, int maxRetries, CWnd* pParent /*=NULL*/)
: m_maxRetryCount(maxRetries), m_retryCount(0), m_getGroups(getGroups), m_authError(false), m_groupName(groupName), m_userAttributes(attributesForUser),
m_altUserName(groupsForUser), m_disableCancel(disableCancel), CInputDlg(CUserAuthDlg::IDD, CUIDlg::ShowButtons(true, showBack), pParent)
{
	m_dlgIconID = IDI_AUTH;
}

CUserAuthDlg::~CUserAuthDlg()
{
}

void CUserAuthDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TITLE, m_dlgTitle);
	DDX_Control(pDX, IDC_USERMSG, m_userMsg);
	DDX_Control(pDX, IDC_NEXT, m_next);
	DDX_Control(pDX, IDC_CANCEL, m_cancel);
	DDX_Control(pDX, IDC_BACK, m_back);
}


BEGIN_MESSAGE_MAP(CUserAuthDlg, CDialog)
	ON_BN_CLICKED(IDC_NEXT, &CUIDlg::OnBnClickedNextaction)
	ON_BN_CLICKED(IDC_CANCEL, &CUserAuthDlg::OnBnClickedCanceled)
	ON_BN_CLICKED(IDC_BACK, &CUIDlg::OnBnClickedBack)
	ON_BN_CLICKED(ID_SHOWTSVARDLG, &CUIDlg::OnBnClickedShowTSVarDLG)
	ON_BN_CLICKED(ID_DUMPTSVAR, &CUIDlg::OnBnClickedDumpTSVar)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_NCACTIVATE()
	ON_WM_NCPAINT()
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_WM_CTLCOLOR()
	ON_MESSAGE(WM_NCUAHDRAWCAPTION, OnNCUAHDrawCaption)
    ON_MESSAGE(WM_NCUAHDRAWFRAME, OnNCUAHDrawFrame)
	ON_COMMAND(IDCANCEL, &CUserAuthDlg::OnIdcancel)
	ON_REGISTERED_MESSAGE(CRegExEdit::UWM_VALUE_CHANGED, OnInputChange)
	ON_CONTROL_RANGE(CBN_SELCHANGE, IDC_INPUTCONTROLS, IDC_INPUTCONTROLS + 50, &CInputDlg::OnComboChange)
	ON_CONTROL_RANGE(CBN_EDITCHANGE, IDC_INPUTCONTROLS, IDC_INPUTCONTROLS + 50, &CInputDlg::OnComboChange)
END_MESSAGE_MAP()


// CUserAuthDlg message handlers

void CUserAuthDlg::OnIdcancel()
{
	// TODO: Add your command handler code here
}

BOOL CUserAuthDlg::OnInitDialog()
{
	CInputDlg::OnInitDialog();

	if (m_disableCancel)
		m_cancel.EnableWindow(false);

		return FALSE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CUserAuthDlg::OnBnClickedNextaction()
{
	//UIpp::CUserInput* pPasswordUI;

	if (m_authError)
	{
		EndDialog(ERROR_ACCESS_DENIED);
		return;
	}

	CString valueName, value;
	POSITION p = m_addedPrompts.GetHeadPosition();

	PTSTR pPass = new TCHAR[MAX_STRING_LENGTH];
	PTSTR pUser = new TCHAR[MAX_STRING_LENGTH];
	PTSTR pDom = new TCHAR[MAX_STRING_LENGTH];
	PTSTR pCount = new TCHAR[MAX_STRING_LENGTH];

	int count = 0;

	m_next.EnableWindow(false);
	SetMessage(IDS_MSGAUTHENTICATING);

	while (p != NULL)
	{
		UIpp::CUserItem* pui = m_addedPrompts.GetNext(p);

		ASSERT(pui);

		if (count == 0)
			_tcscpy_s(pUser, MAX_STRING_LENGTH, pui->GetValue(true));
		else if (count == 1)
			_tcscpy_s(pPass, MAX_STRING_LENGTH, pui->GetValue(true));
		else if (count == 2)
			_tcscpy_s(pDom, MAX_STRING_LENGTH, pui->GetValue(true));

		count++;
	}

	LDAP* ld = NULL;

	ULONG authResult = DoAuth(pUser, pPass, pDom, ld);
	SecureZeroMemory(pPass, sizeof(TCHAR) * (MAX_STRING_LENGTH - 1));

	_itot_s(m_retryCount + 1, pCount, MAX_STRING_LENGTH, 10);

	FTW::CCMLog::Instance().LogMsg(authResult == ERROR_SUCCESS ? FTW::CCMLog::Info : FTW::CCMLog::Error, AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
		IDS_LOGMSG_USERAUTH, pUser, pDom, ldap_err2string(authResult), pCount);

	if(authResult != LDAP_SUCCESS)
	{
		SetMessage(IDS_MSGTRYAGAIN, ldap_err2string(authResult), Error);

		if (m_maxRetryCount > 0 && ++m_retryCount >= m_maxRetryCount)
			DisplayFinalError(IDS_MSGTOOMANYTRIES);

	}
	else
	{
		bool authSuccess = false;

		if (GetRootNamingContext(ld, pDom))
		{
			TCHAR** ppAttributeVal = nullptr;

			if (m_groupName.GetLength() > 0 || m_getGroups)
			{
				ASSERT(ld);

				bool isUserMember = false;
				CString groupDistinguishedName;
				CString grpName;
				int pos = 0;

				if (GetRootNamingContext(ld, pDom) && GetGroupMembership(ld, pUser, ppAttributeVal))
				{
					grpName = m_groupName.Tokenize(_T(",;"), pos);

					grpName.Trim();

					while (grpName != _T("") && isUserMember == false)
					{
						//GetGroupDN(ld, grpName, groupDistinguishedName);
						GetLDAPAttributeValueFromObject(ld, _T("distinguishedName"), grpName, groupDistinguishedName, Group);

						isUserMember = isUserMember || groupDistinguishedName.GetLength() > 0 && ppAttributeVal != NULL && IsMember(ppAttributeVal, groupDistinguishedName);

						FTW::CCMLog::Instance().LogMsg(isUserMember == true ? FTW::CCMLog::Info : FTW::CCMLog::Error, AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
							isUserMember == true ? IDS_LOGMSG_USERISMEMBER : IDS_LOGERROR_USERISMEMBER, pUser, grpName);

						grpName = m_groupName.Tokenize(_T(",;"), pos);
						grpName.Trim();
					}

					authSuccess = isUserMember;

					if (!isUserMember && m_groupName.GetLength() > 0)
					{
						if (m_maxRetryCount > 0 && ++m_retryCount >= m_maxRetryCount)
							DisplayFinalError(IDS_MSGTOOMANYTRIES);

						else
							SetMessage(IDS_MSGUSERNOTMEMBER, nullptr, CUIDlg::Warning);

						//DisplayFinalError(IDS_MSGUSERNOTMEMBER);
					}
					else if (m_getGroups)
					{
						CString groups;

						if (ExtractGroups(ld, ppAttributeVal, groups))
							CTSEnv::Instance().Set(VAR_AUTHUSERGROUPS, groups);

						authSuccess = true;
					}
				}
				else
					DisplayFinalError(IDS_MSGADGROUPISSUE);

				if (ppAttributeVal != nullptr)
				{
					ldap_value_free(ppAttributeVal);
					ppAttributeVal = nullptr;
				}

			}
			else
				authSuccess = true;

			if (authSuccess)
			{
				if (!m_authError)
				{
					CTSEnv::Instance().Set(VAR_AUTHUSER, pUser);
					CTSEnv::Instance().Set(VAR_AUTHUSERDOMAIN, pDom);
				}

				if (m_userAttributes)
				{
					int pos = 0;

					CString attrValue, attrVariableName;
					CString attrName = m_userAttributes.Tokenize(_T(",;"), pos);

					attrName.Trim();

					while (attrName != _T(""))
					{
						//GetGroupDN(ld, grpName, groupDistinguishedName);
						if (GetLDAPAttributeValueFromObject(ld, attrName.GetBuffer(), pUser, attrValue))
						{
							if (!attrValue.IsEmpty())
							{
								attrVariableName = VAR_AUTHUSERATTR;
								attrVariableName += _T("_");
								attrVariableName += attrName;

								CTSEnv::Instance().Set(attrVariableName, attrValue);

								attrValue.Empty();
							}
						}

						attrName.ReleaseBuffer();

						attrName = m_userAttributes.Tokenize(_T(",;"), pos);
						attrName.Trim();
					}

				}

				if (m_altUserName.GetLength() > 0 && GetGroupMembership(ld, m_altUserName, ppAttributeVal))
				{
					CString groups;

					CTSEnv::Instance().Set(VAR_ALTUSER, m_altUserName);

					if (ExtractGroups(ld, ppAttributeVal, groups))
						CTSEnv::Instance().Set(VAR_ALTUSERGROUPS, groups);
				}

				if (ppAttributeVal != nullptr)
				{
					ldap_value_free(ppAttributeVal);
					ppAttributeVal = nullptr;
				}
			}

		}

		if (authSuccess && !m_authError)
			EndDialog(ERROR_SUCCESS);
	}

	ldap_unbind(ld);

	delete pPass;
	delete pUser;
	delete pDom;
	delete pCount;

		

}

bool CUserAuthDlg::ExtractGroups(LDAP* ld, TCHAR**& ppAttributeVal, CString& groupNameList)
{
	if (ppAttributeVal != NULL && ld != NULL)
	{
		int groupCount = ldap_count_values(ppAttributeVal);
		CString groupDN;
		CString groupName;

		for (int count = 0; count < groupCount; count++)
		{
			groupDN = ppAttributeVal[count];

			if (GetLDAPAttributeValue(ld, groupDN.GetBuffer(), LDAP_SCOPE_BASE, _T("(objectClass=*)"), _T("sAMAccountName"), groupName))
			{
				if (groupNameList.GetLength() > 0)
					groupNameList.AppendChar(',');

				groupNameList += groupName;
			}
		}

		return true;
	}

	return false;
}

void CUserAuthDlg::DisplayFinalError(DWORD msg)
{
	POSITION p = m_addedPrompts.GetHeadPosition();

	SetMessage(msg, 0, Error);
	//m_next.SetIcon(IDI_CANCEL, IDI_CANCELGREY, CXButtonXP::LEFT).EnableTheming(TRUE).SetDrawToolbar(TRUE);
	//m_next.EnableWindow(true);
	
	//m_cancel.ShowWindow(SW_HIDE);

	m_cancel.EnableWindow(true);

	FTW::CCMLog::Instance().LogMsg(FTW::CCMLog::Error, AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
		msg);

	p = m_addedPrompts.GetHeadPosition();

	while (p != NULL)
	{
		UIpp::CUserItem* pui = m_addedPrompts.GetNext(p);

		ASSERT(pui);

		pui->Disable();

	}

	m_authError = true;
}

bool CUserAuthDlg::GetRootNamingContext(LDAP* ld/*, PCTSTR pUsername*/, PCTSTR pDomain)
{
	if (m_rootLdapPath.GetLength() == 0)
	{
		try
		{
			if (GetLDAPAttributeValue(ld, _T(""), LDAP_SCOPE_BASE, _T("(objectClass=*)"), _T("defaultNamingContext"), m_rootLdapPath))
			{

				FTW::CCMLog::Instance().LogMsg(FTW::CCMLog::Info, AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
					IDS_LOGMSG_DEFAULTNC, pDomain, m_rootLdapPath);

			}
			else
			{
				FTW::CCMLog::Instance().LogMsg(FTW::CCMLog::Error, AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
					IDS_LOGERROR_DEFAULTNCNOTFOUND, pDomain);

				return false;
			}
		}
		catch (ULONG& ldapResult)
		{
			FTW::CCMLog::Instance().LogMsg(FTW::CCMLog::Error, AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
				IDS_LOGERROR_DEFAULTNC, pDomain, ldap_err2string(ldapResult));

			return false;
		}
	}

	return true;
}

bool CUserAuthDlg::GetGroupMembership(LDAP* ld, PCTSTR pUsername, TCHAR**& ppAttributeVal)
{
	TCHAR pSearchFilter[MAX_STRING_LENGTH];
	bool returnValue = false;

	try
	{
		_stprintf_s(pSearchFilter, MAX_STRING_LENGTH, _T("(&(objectCategory=Person)(sAMAccountName=%s))"), pUsername);

//		if (FindMembers(ld, m_rootLdapPath.GetBuffer(), pSearchFilter, ppAttributeVal) == ERROR_SUCCESS)

		if (GetLDAPAttributeValue(ld, m_rootLdapPath.GetBuffer(), LDAP_SCOPE_SUBTREE, pSearchFilter, _T("memberOf"), ppAttributeVal)  )
			{
			FTW::CCMLog::Instance().LogMsg(FTW::CCMLog::Info, AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
				IDS_LOGMSG_GROUPMEMBERSHIP, pUsername);

			returnValue = true;
		}

	}
	catch (ULONG& result)
	{
		FTW::CCMLog::Instance().LogMsg(FTW::CCMLog::Error, AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
			IDS_LOGERROR_GROUPMEMBERSHIP, pUsername, ldap_err2string(result));
	}

	m_rootLdapPath.ReleaseBuffer();

	return returnValue;
}

//bool CUserAuthDlg::GetGroupDN(LDAP* ld, PCTSTR pGroupname, CString& groupDN)
//{
//	TCHAR pSearchFilter[MAX_STRING_LENGTH];
//	bool returnValue = false;
//
//	_stprintf_s(pSearchFilter, MAX_STRING_LENGTH, _T("(&(objectCategory=Group)(sAMAccountName=%s))"), pGroupname);
//
//	try
//	{
//		if (GetLDAPAttributeValue(ld, m_rootLdapPath.GetBuffer(), LDAP_SCOPE_SUBTREE, pSearchFilter, _T("distinguishedName"), groupDN))
//		{
//			FTW::CCMLog::Instance().LogMsg(FTW::CCMLog::Info, AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
//				IDS_LOGMSG_GROUPDN, pGroupname, groupDN);
//
//			returnValue = true;
//		}
//	}
//	catch (ULONG& result)
//	{
//		FTW::CCMLog::Instance().LogMsg(FTW::CCMLog::Error, AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
//			IDS_LOGERROR_GROUPDN, pGroupname, ldap_err2string(result));
//	}
//
//	m_rootLdapPath.ReleaseBuffer();
//
//	return returnValue;
//}

bool CUserAuthDlg::GetLDAPAttributeValueFromObject(LDAP* ld, PTSTR pAttributeName, PCTSTR pObjectName, CString& value, LDAPObjectType objType)
{
	TCHAR pSearchFilter[MAX_STRING_LENGTH];
	bool returnValue = false;

	_stprintf_s(pSearchFilter, MAX_STRING_LENGTH, _T("(&(objectCategory=%s)(sAMAccountName=%s))"), (objType == Person ? _T("Person") : _T("Group")), pObjectName);

	try
	{
		if (GetLDAPAttributeValue(ld, m_rootLdapPath.GetBuffer(), LDAP_SCOPE_SUBTREE, pSearchFilter, pAttributeName, value))
		{
			FTW::CCMLog::Instance().LogMsg(FTW::CCMLog::Info, AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
				IDS_LOGMSG_OBJECTATTRIBUTE, pAttributeName, pObjectName, value);

			returnValue = true;
		}
	}
	catch (ULONG& result)
	{
		FTW::CCMLog::Instance().LogMsg(FTW::CCMLog::Error, AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
			IDS_LOGERROR_OBJECTATTRIBUTE, pAttributeName, pObjectName, ldap_err2string(result));
	}

	m_rootLdapPath.ReleaseBuffer();

	return returnValue;
}

//bool CUserAuthDlg::GetGroupMembership(LDAP* ld, PCTSTR pUsername, PCTSTR pDomain, PCTSTR pGroupname, TCHAR**& ppAttributeVal, CString& groupDN)
//{
//	TCHAR pSearchFilter[MAX_STRING_LENGTH];
//	bool returnValue = false;
//
//	_stprintf_s(pSearchFilter, MAX_STRING_LENGTH, _T("(&(objectCategory=Group)(sAMAccountName=%s))"), pGroupname);
//
//	try
//	{
//		if (GetLDAPAttributeValue(ld, m_rootLdapPath.GetBuffer(), LDAP_SCOPE_SUBTREE, pSearchFilter, _T("distinguishedName"), groupDN))
//		{
//			FTW::CCMLog::Instance().LogMsg(FTW::CCMLog::Info, AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
//				IDS_LOGMSG_GROUPDN, pGroupname, groupDN);
//
//			_stprintf_s(pSearchFilter, MAX_STRING_LENGTH, _T("(&(objectCategory=Person)(sAMAccountName=%s))"), pUsername);
//
//			returnValue = FindMembers(ld, m_rootLdapPath.GetBuffer(), pSearchFilter, ppAttributeVal);
//		}
//
//	}
//	catch (ULONG& result)
//	{
//		FTW::CCMLog::Instance().LogMsg(FTW::CCMLog::Error, AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
//			IDS_LOGERROR_GROUPDN, pGroupname, ldap_err2string(result));
//	}
//
//	m_rootLdapPath.ReleaseBuffer();
//
//	return returnValue;
//}

ULONG CUserAuthDlg::DoAuth(PTSTR pUsername, PTSTR pPassword, PTSTR pDomain, LDAP*& ld)
{
	ld = ldap_init(pDomain, LDAP_PORT);

	if (ld == NULL)
	{
		return LdapGetLastError();
	}

	unsigned ldapVersion = LDAP_VERSION3;
	ULONG result = ldap_set_option(ld, LDAP_OPT_VERSION, &ldapVersion);

	ASSERT(result == LDAP_SUCCESS);

	result = ldap_set_option(ld, LDAP_OPT_ENCRYPT, LDAP_OPT_ON);

	ASSERT(result == LDAP_SUCCESS);

	result = ldap_set_option(ld, LDAP_OPT_SIGN, LDAP_OPT_ON);

	ASSERT(result == LDAP_SUCCESS);

	result = ldap_connect(ld, NULL);

	if (result == LDAP_SUCCESS)
	{
		SEC_WINNT_AUTH_IDENTITY auth;
		ZeroMemory(&auth, sizeof(auth));

		auth.User = reinterpret_cast<unsigned short*> (pUsername);
		auth.UserLength = (unsigned long)_tcsnlen(pUsername, 36);
		auth.Domain = reinterpret_cast<unsigned short*> (pDomain);
		auth.DomainLength = (unsigned long)_tcsnlen(pDomain, 36);
		auth.Password = reinterpret_cast<unsigned short*> (pPassword);
		auth.PasswordLength = (unsigned long)_tcsnlen(pPassword, 36);
		auth.Flags = SEC_WINNT_AUTH_IDENTITY_UNICODE;

		result = ldap_bind_s(ld, NULL, (TCHAR*)(&auth), LDAP_AUTH_NEGOTIATE);

		SecureZeroMemory(&auth, sizeof(auth));

	}

	return result;
}

//bool CUserAuthDlg::IsMember(LDAP* ld, PTSTR pSearchBase, PTSTR searchFilter, PCTSTR pGroupDN)
//{
//	ASSERT(ld);
//
//	bool returnValue = false;
//	LPTSTR attribs[2];
//	attribs[0] = _T("memberOf");
//	attribs[1] = NULL;
//
//	LDAPMessage* pLdapMsg;
//
//	ULONG ldapResult = ldap_search_s(ld, pSearchBase, LDAP_SCOPE_SUBTREE, searchFilter, attribs, 0, &pLdapMsg);
//
//	if (ldapResult == LDAP_SUCCESS)
//	{
//		LDAPMessage* pLdapEntry;
//
//		pLdapEntry = ldap_first_entry(ld, pLdapMsg);
//
//		if (pLdapEntry)
//		{
//			TCHAR** ppAttributeVal = ldap_get_values(ld, pLdapEntry, _T("memberOf"));
//			
//			if (ppAttributeVal != NULL)
//			{
//				int groupCount = ldap_count_values(ppAttributeVal);
//				CString groupDN;
//
//				for (int count = 0; count < groupCount; count++)
//				{
//					groupDN = ppAttributeVal[count];
//
//					if (groupDN == pGroupDN)
//					{
//						returnValue = true;
//						break;
//					}
//				}
//
//				ldap_value_free(ppAttributeVal);
//			}
//
//			ldap_msgfree(pLdapMsg);
//
//		}
//	}
//
//	return returnValue;
//}

bool CUserAuthDlg::IsMember(TCHAR**& ppAttributeVal, PCTSTR pGroupDN)
{
	bool returnValue = false;

	if (ppAttributeVal != NULL)
	{
		int groupCount = ldap_count_values(ppAttributeVal);
		CString groupDN;

		for (int count = 0; count < groupCount; count++)
		{
			groupDN = ppAttributeVal[count];

			if (groupDN == pGroupDN)
			{
				returnValue = true;
				break;
			}
		}
	}

	return returnValue;

}

bool CUserAuthDlg::GetLDAPAttributeValue(LDAP* ld, PTSTR pSearchBase, ULONG scope, PTSTR searchFilter, PTSTR attributeName, CString& attributeValue)
{
	TCHAR** ppAttr = nullptr;

	bool returnvalue = _GetLDAPAttributeValue(ld, pSearchBase, scope, searchFilter, attributeName, attributeValue, ppAttr);

	if (ppAttr != nullptr)
	{
		ldap_value_free(ppAttr);
		ppAttr = nullptr;
	}

	return returnvalue;
}

bool CUserAuthDlg::GetLDAPAttributeValue(LDAP* ld, PTSTR pSearchBase, ULONG scope, PTSTR searchFilter, PTSTR attributeName, TCHAR**& ppAttributeVal)
{
	CString attrVal;

	return _GetLDAPAttributeValue(ld, pSearchBase, scope, searchFilter, attributeName, attrVal, ppAttributeVal);

}

bool CUserAuthDlg::_GetLDAPAttributeValue(LDAP* ld, PTSTR pSearchBase, ULONG scope, PTSTR searchFilter, PTSTR attributeName, CString& attributeValue, TCHAR**& ppAttributeVal)
{
	ASSERT(ld);

	LPTSTR attribs[2];
	attribs[0] = attributeName;
	attribs[1] = NULL;

	LDAPMessage* pLdapMsg;
	bool attrFound = false;
	attributeValue.Empty();

	ULONG ldapResult = ldap_search_s(ld, pSearchBase, scope, searchFilter, attribs, 0, &pLdapMsg);

	if (ldapResult != LDAP_SUCCESS)
		throw ldapResult;

	LDAPMessage* pLdapEntry;

	pLdapEntry = ldap_first_entry(ld, pLdapMsg);

	if (pLdapEntry)
	{
		ppAttributeVal = ldap_get_values(ld, pLdapEntry, attributeName);

		if (ppAttributeVal != NULL)
		{
			if (ldap_count_values(ppAttributeVal) > 0)
			{
				attributeValue = ppAttributeVal[0];
				attrFound = true;
			}

			//ldap_value_free(ppAttributeVal);
		}

		ldap_msgfree(pLdapMsg);

	}

	return attrFound;
}

//ULONG CUserAuthDlg::FindMembers(LDAP* ld, PTSTR pSearchBase, PTSTR searchFilter, TCHAR**& ppAttributeVal)
//{
//	ASSERT(ld);
//
//	LPTSTR attribs[2];
//	attribs[0] = _T("memberOf");
//	attribs[1] = NULL;
//
////	CString groupName;
////	CString groupNames = _T("");
//
//	LDAPMessage* pLdapMsg;
//
//	ULONG ldapResult = ldap_search_s(ld, pSearchBase, LDAP_SCOPE_SUBTREE, searchFilter, attribs, 0, &pLdapMsg);
//
//	if (ldapResult == LDAP_SUCCESS)
//	{
//		LDAPMessage* pLdapEntry;
//
//		pLdapEntry = ldap_first_entry(ld, pLdapMsg);
//
//		if (pLdapEntry)
//		{
//			ppAttributeVal = ldap_get_values(ld, pLdapEntry, _T("memberOf"));
//
//			//if (ppAttributeVal != NULL)
//			//	returnValue = true;
//
//			ldap_msgfree(pLdapMsg);
//		}
//	}
//	else throw(ldapResult);
//
//	return ldapResult;
//}

afx_msg void CUserAuthDlg::OnBnClickedCanceled()
{
	if (m_authError)
		EndDialog(ERROR_ACCESS_DENIED);
	else
		EndDialog(ERROR_CANCELLED);
}