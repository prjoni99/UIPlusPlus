#pragma once

#include "DlgUserInput.h"
#include "winldap.h"
#include "CLdap.h"

// CUserAuthDlg dialog

struct DlgUserAuthData
{
	DlgUserAuthData(PCTSTR pAuthorizedUserGroups, 
		PCTSTR pUserattributes,
		PCTSTR pDomainController,	
		bool getGroups = false,
		bool disableCancel = false, 
		bool doNotFallback = false,
		int maxRetries = 0)
		:	m_authorizedUserGroups(pAuthorizedUserGroups),
			m_userAttributes(pUserattributes),
			m_domainController(pDomainController),
			m_getGroups(getGroups),
			m_disableCancel(disableCancel),
			m_doNotFallback(doNotFallback),
			m_maxRetries(maxRetries)
	{};

	CString m_authorizedUserGroups;
	CString m_userAttributes;
	CString m_domainController;
	int m_maxRetries;
	bool m_getGroups;
	bool m_disableCancel;
	bool m_doNotFallback;
};

typedef DlgUserAuthData* PDlgUserAuthData;
typedef const DlgUserAuthData* PCDlgUserAuthData;

class CDlgUserAuth : public CDlgUserInput
{
	DECLARE_DYNAMIC(CDlgUserAuth)

public:
	CDlgUserAuth(PCTSTR dlgTitle, 
		const DialogVisibilityFlags flags,
		const UIpp::ActionData& data, 
		PCDlgUserAuthData pAuthData, 
		CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgUserAuth();

// Dialog Data
	enum { IDD = IDD_USERAUTHDLG };

	virtual BOOL OnInitDialog();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	afx_msg void OnBnClickedNextaction();

	DECLARE_MESSAGE_MAP()

	void Dummy(void) {};

	//bool GetOUs(LDAP * ld, PTSTR pSearchBase);
	void GetObjectAttributes(FTWLDAP::ILdapPtr& ld, PCTSTR pUser, FTWLDAP::ILdap::LDAPObjectCategory objectCategory);

	bool IsUserMemberOfGroup(FTWLDAP::ILdapPtr& ld, TCHAR**& ppUserGroupList, PTSTR pUser);
	void DisplayFinalError(DWORD msg);

	int m_retryCount = 0;
	bool m_fatalAuthError = false;

	CString m_rootLdapPath;

	PCDlgUserAuthData m_pData;

	//CString m_groupName;
	//CString m_altUserName;
	//CString m_userAttributes;
	//CString m_domainController;
	//bool m_doNotFallback;
	//bool m_getGroups;
	//bool m_disableCancel;
	//int m_maxRetryCount;


public:
	virtual afx_msg void OnBnClickedCanceled();

};
