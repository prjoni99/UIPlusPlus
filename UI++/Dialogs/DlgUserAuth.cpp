// UserAuthDlg.cpp : implementation file
//

#include "stdafx.h"
#include "UI++.h"
#include "DlgUserAuth.h"
#include "afxdialogex.h"
#include "TSvar.h"
#include "Constants.h"

//#include "activeds.h"

// CUserAuthDlg dialog

IMPLEMENT_DYNAMIC(CDlgUserAuth, CDlgUserInput)

CDlgUserAuth::CDlgUserAuth(PCTSTR dlgTitle, 
	const DialogVisibilityFlags flags,
	const UIpp::ActionData& data, 
	PCDlgUserAuthData pAuthData, 
	CWnd* pParent /*=NULL*/)
	: m_pData (pAuthData),
      CDlgUserInput(dlgTitle, flags, data, CDlgUserAuth::IDD, pParent)
{
}

CDlgUserAuth::~CDlgUserAuth()
{
}

void CDlgUserAuth::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	CDlgBase::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDlgUserAuth, CDlgUserInput)
	//ON_REGISTERED_MESSAGE(FTW::CRegExEdit::UWM_VALUE_CHANGED, OnInputChange)
	//ON_CONTROL_RANGE(CBN_SELCHANGE, IDC_INPUTCONTROLS, IDC_INPUTCONTROLS + 50, &CDlgUserInput::OnComboChange)
	//ON_CONTROL_RANGE(CBN_EDITCHANGE, IDC_INPUTCONTROLS, IDC_INPUTCONTROLS + 50, &CDlgUserInput::OnComboChange)
END_MESSAGE_MAP()


// CUserAuthDlg message handlers

BOOL CDlgUserAuth::OnInitDialog()
{
	CDlgUserInput::OnInitDialog();

	if (m_pData->m_disableCancel)
		m_cancel.EnableWindow(false);

		return FALSE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgUserAuth::OnBnClickedNextaction()
{
	//UIpp::CUserInput* pPasswordUI;

	if (m_fatalAuthError)
	{
		EndDialog(ERROR_ACCESS_DENIED);
		return;
	}

//	CString valueName, value;
	bool groupMembershipOK = true;

	PTSTR pPass = new TCHAR[MAX_STRING_LENGTH];
	PTSTR pUser = new TCHAR[MAX_STRING_LENGTH];
	PTSTR pDom = new TCHAR[MAX_STRING_LENGTH];
	PTSTR pCount = new TCHAR[MAX_STRING_LENGTH];

	int count = 0;

	m_next.EnableWindow(false);
	
	SETMESSAGE(IDS_MSGAUTHENTICATING, nullptr, CDlgBase::MsgType::Info);

	BeginWaitCursor();

	for(auto pInput : m_allInputs)
	{
		ASSERT(pInput);

		if (count == 0)
			_tcscpy_s(pUser, MAX_STRING_LENGTH, pInput->GetValue(true));
		else if (count == 1)
			_tcscpy_s(pPass, MAX_STRING_LENGTH, pInput->GetValue(true));
		else if (count == 2)
			_tcscpy_s(pDom, MAX_STRING_LENGTH, pInput->GetValue(true));

		count++;
	}

	try
	{
		CString nc = _T("");
		
		_itot_s(m_retryCount + 1, pCount, MAX_STRING_LENGTH, 10);
		//FTWLDAP::ILdapPtr pLdap(FTWLDAP::GetLdap(pUser, pPass, pDom), std::mem_fn(&FTWLDAP::ILdap::Release));
		//FTWLDAP::ILdapPtr pLdap(FTWLDAP::GetLdap(), std::mem_fn(&FTWLDAP::ILdap::Release));

		PCTSTR pDC = nullptr;
		
		if(m_pData->m_domainController.GetLength() > 0)
			pDC	= m_pData->m_domainController.GetString();

		m_actionData.pLdap->Authenticate(pUser, pPass, pDom, pDC, m_pData->m_doNotFallback);

		SecureZeroMemory(pPass, sizeof(TCHAR) * (MAX_STRING_LENGTH - 1));

		m_actionData.pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info,
			AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
			FTW::FormatResourceString(IDS_LOGMSG_USERAUTH, pUser, pDom, ldap_err2string(LDAP_SUCCESS), pCount));

		ULONG result = m_actionData.pLdap->GetRootNamingContext(nc);

		if (result == LDAP_SUCCESS && nc.GetLength() > 0)
		{
			m_actionData.pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info,
				AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
				FTW::FormatResourceString(IDS_LOGMSG_DEFAULTNC, pDom, nc));

			if (m_pData->m_authorizedUserGroups.GetLength() > 0 || m_pData->m_getGroups)
			{
				TCHAR** ppAttributeVal = nullptr;

				result = m_actionData.pLdap->GetGroupMembership(pUser, ppAttributeVal);

				if (result == LDAP_SUCCESS)
				{
					m_actionData.pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info,
						AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
						FTW::FormatResourceString(IDS_LOGMSG_GROUPMEMBERSHIP, pUser));
							
					if(m_pData->m_authorizedUserGroups.GetLength() > 0)
					{
						if (!IsUserMemberOfGroup(m_actionData.pLdap, ppAttributeVal, pUser))
						{
							if (m_pData->m_maxRetries > 0 && ++m_retryCount >= m_pData->m_maxRetries)
								DisplayFinalError(IDS_MSGTOOMANYTRIES);
							else
								SETMESSAGE(IDS_MSGUSERNOTMEMBER, nullptr, CDlgBase::MsgType::Warning);

							groupMembershipOK = false;
						}
					}
								
					if (groupMembershipOK && m_pData->m_getGroups)
						CTSEnv::Instance().Set(m_actionData.pCMLog, VAR_AUTHUSERGROUPS, m_actionData.pLdap->ExtractGroupNamesFromAttributeList(ppAttributeVal));

				}
				else
				{
					m_actionData.pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Error,
						AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
						FTW::FormatResourceString(IDS_LOGERROR_GROUPMEMBERSHIP, pUser, ldap_err2string(result)));

					DisplayFinalError(IDS_MSGADGROUPISSUE);
				}

				if (ppAttributeVal != nullptr)
				{
					ldap_value_free(ppAttributeVal);
					ppAttributeVal = nullptr;
				}

			}

			if (groupMembershipOK && !m_fatalAuthError)
			{
				CTSEnv::Instance().Set(m_actionData.pCMLog, VAR_AUTHUSER, pUser);
				CTSEnv::Instance().Set(m_actionData.pCMLog, VAR_AUTHUSERDOMAIN, pDom);

	//			GetOUs(ld, m_rootLdapPath.GetBuffer());

				if (m_pData->m_userAttributes.GetLength() > 0)
					GetObjectAttributes(m_actionData.pLdap, pUser, FTWLDAP::ILdap::LDAPObjectCategory::Person);

				CString attrValue;

				if (m_actionData.pLdap->GetAttributeValueFromObject(L"displayName", pUser, FTWLDAP::ILdap::LDAPObjectCategory::Person, attrValue) == LDAP_SUCCESS &&
					!attrValue.IsEmpty())
				{
						CTSEnv::Instance().Set(m_actionData.pCMLog, VAR_AUTHUSERDISPLAYNAME, attrValue);
				}	
			}

			if (groupMembershipOK && !m_fatalAuthError)
			{
				EndWaitCursor();
				EndDialog(ERROR_SUCCESS);
			}
		}
		else
		{
			if (result == LDAP_SUCCESS)
				m_actionData.pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Error,
					AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
					FTW::FormatResourceString(IDS_LOGERROR_DEFAULTNCNOTFOUND, pDom));
			else
				m_actionData.pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Error,
					AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
					FTW::FormatResourceString(IDS_LOGERROR_DEFAULTNC, pDom, ldap_err2string(result)));

			DisplayFinalError(IDS_MSGDOMAINNCISSUE);
		}


	}
	catch (ULONG& u)
	{
		SecureZeroMemory(pPass, sizeof(TCHAR) * (MAX_STRING_LENGTH - 1)); 
		
		m_actionData.pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Error,
			AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
			FTW::FormatResourceString(IDS_LOGMSG_USERAUTH, pUser, pDom, ldap_err2string(u), pCount));
		
		SETMESSAGE(IDS_MSGTRYAGAIN, ldap_err2string(u), CDlgBase::MsgType::Error);

		if (m_pData->m_maxRetries > 0 && ++m_retryCount >= m_pData->m_maxRetries)
			DisplayFinalError(IDS_MSGTOOMANYTRIES);
	}
	
	//ldap_unbind(ld);

	EndWaitCursor();

	delete[] pPass;
	delete[] pUser;
	delete[] pDom;
	delete[] pCount;

}

void CDlgUserAuth::DisplayFinalError(DWORD msg)
{
	SETMESSAGE(msg, nullptr, CDlgBase::MsgType::Error);
	//m_next.SetIcon(IDI_CANCEL, IDI_CANCELGREY, CXButtonXP::LEFT).EnableTheming(TRUE).SetDrawToolbar(TRUE);
	//m_next.EnableWindow(true);
	
	//m_cancel.ShowWindow(SW_HIDE);

	m_cancel.EnableWindow(true);

	m_actionData.pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Error,
		AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
		FTW::FormatResourceString(msg));

	for(auto pInput : m_allInputs)
	{
		ASSERT(pInput);

		pInput->Disable();

	}

	m_fatalAuthError = true;
}

//bool CUserAuthDlg::GetOUs(LDAP* ld, PTSTR pSearchBase)
//{
//	ASSERT(ld);
//
//	LPTSTR attribs[3];
//	attribs[0] = _T("distinguishedName");
//	attribs[1] = _T("name");
//	attribs[2] = NULL;
//	
//	LDAPMessage* pLdapMsg;
//	PTCHAR* attributeValue;
//	
//	ULONG ldapResult = ldap_search_s(ld, pSearchBase, LDAP_SCOPE_ONELEVEL, _T("(objectCategory=organizationalUnit)"), attribs, 0, &pLdapMsg);
//
//	if (ldapResult != LDAP_SUCCESS)
//	{
//		if (pLdapMsg)
//			ldap_msgfree(pLdapMsg);
//
//		throw ldapResult;
//	}
//
//	LDAPMessage* pLdapEntry = ldap_first_entry(ld, pLdapMsg);
//
//	while (pLdapEntry)
//	{
//		attributeValue = ldap_get_values(ld, pLdapEntry, attribs[0]);
//
//		if (attributeValue != NULL && ldap_count_values(attributeValue) > 0)
//		{
//			//attributeValue = ppAttributeVal[0];
//			//attrFound = true;
//
//			TRACE1("Found OU: %s\n", attributeValue[0]);
//
//			ldap_value_free(attributeValue);
//		}
//			
//		pLdapEntry = ldap_next_entry(ld, pLdapEntry);
//
//	}
//
//	if(pLdapMsg)
//		ldap_msgfree(pLdapMsg);
//
//}

void CDlgUserAuth::GetObjectAttributes(FTWLDAP::ILdapPtr& ld, PCTSTR pObject, FTWLDAP::ILdap::LDAPObjectCategory objectCategory)
{
	int pos = 0;

	CString attrValue, attrVariableName;
	CString attrName = m_pData->m_userAttributes.Tokenize(_T(",;"), pos);
	
	attrName.Trim();

	while (attrName != _T(""))
	{
		if (ld->GetAttributeValueFromObject(attrName.GetBuffer(), pObject, objectCategory, attrValue) == LDAP_SUCCESS)
		{
			if (!attrValue.IsEmpty())
			{
				attrVariableName = VAR_AUTHUSERATTR;
				attrVariableName += _T("_");
				attrVariableName += attrName;

				CTSEnv::Instance().Set(m_actionData.pCMLog, attrVariableName, attrValue);

				attrValue.Empty();
			}
		}

		attrName.ReleaseBuffer();

		attrName = m_pData->m_userAttributes.Tokenize(_T(",;"), pos);
		attrName.Trim();
	}

}

bool CDlgUserAuth::IsUserMemberOfGroup(FTWLDAP::ILdapPtr& ld, TCHAR**& ppUserGroupList, PTSTR pUser)
{
	bool isUserMember = false;

	if (ppUserGroupList != NULL)
	{
		CString groupDistinguishedName;
		int pos = 0;
		ULONG result = LDAP_SUCCESS;

		CString groupName = m_pData->m_authorizedUserGroups.Tokenize(_T(",;"), pos);

		groupName.Trim();

		while (groupName != _T("") && isUserMember == false)
		{
			
			result = ld->GetAttributeValueFromObject(_T("distinguishedName"), groupName, FTWLDAP::ILdap::LDAPObjectCategory::Group, groupDistinguishedName) &&
				groupDistinguishedName.GetLength() > 0;

			if (result == LDAP_SUCCESS)
			{
				isUserMember = isUserMember || ld->IsValueinAttributeList(ppUserGroupList, groupDistinguishedName);

				m_actionData.pCMLog->WriteMsg(isUserMember == true ? FTWCMLOG::CCMLog::MsgType::Info : FTWCMLOG::CCMLog::MsgType::Error,
					AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
					FTW::FormatResourceString(isUserMember == true ? IDS_LOGMSG_USERISMEMBER : IDS_LOGERROR_USERISMEMBER, pUser, groupName));
			}
			else
			{
				m_actionData.pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Error,
					AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
					FTW::FormatResourceString(IDS_LOGERROR_OBJECTATTRIBUTE, _T("distinguishedName"), groupName, ldap_err2string(result)));
			}

			groupName = m_pData->m_authorizedUserGroups.Tokenize(_T(",;"), pos);
			groupName.Trim();
		}
	}

	return isUserMember;
}

afx_msg void CDlgUserAuth::OnBnClickedCanceled()
{
	if (m_fatalAuthError)
		EndDialog(ERROR_ACCESS_DENIED);
	else
		EndDialog(ERROR_CANCELLED);
}
