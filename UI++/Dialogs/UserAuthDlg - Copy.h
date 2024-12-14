#pragma once

#include "InputDlg.h"
#include "winldap.h"

// CUserAuthDlg dialog

class CUserAuthDlg : public CInputDlg
{
	DECLARE_DYNAMIC(CUserAuthDlg)

public:
	CUserAuthDlg(PCTSTR groupName, bool getGroups, bool showBack = false, bool disableCancel = false, 
		PCTSTR groupsForUser = _T(""), PCTSTR attributesForUser = _T(""), int maxRetries = 0, CWnd* pParent = NULL);   // standard constructor
	virtual ~CUserAuthDlg();

// Dialog Data
	enum { IDD = IDD_USERAUTHDLG };

	enum LDAPObjectType { Person = 0, Group };

	virtual BOOL OnInitDialog();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	afx_msg void OnBnClickedNextaction();

	DECLARE_MESSAGE_MAP()

	void Dummy(void) {};

	ULONG DoAuth(PTSTR pUsername, PTSTR pPassword, PTSTR pDomain, LDAP*& ld);

	bool _GetLDAPAttributeValue(LDAP* ld, PTSTR pSearchBase, ULONG scope, PTSTR searchFilter, PTSTR attributeName, CString& attributeValue, TCHAR**& ppAttributeVal);
	bool GetLDAPAttributeValue(LDAP* ld, PTSTR pSearchBase, ULONG scope, PTSTR searchFilter, PTSTR attributeName, CString& attributeValue);
	bool GetLDAPAttributeValue(LDAP* ld, PTSTR pSearchBase, ULONG scope, PTSTR searchFilter, PTSTR attributeName, TCHAR**& ppAttributeVal);
	bool GetLDAPAttributeValueFromObject(LDAP * ld, PTSTR pAttributeName, PCTSTR pObjectName, CString& value, LDAPObjectType objType = Person);
	bool GetRootNamingContext(LDAP* ld/*, PCTSTR pUsername*/, PCTSTR pDomain);
	bool IsMember(TCHAR**& ppAttributeVal, PCTSTR pGroupDN);
//	bool GetGroupMembership(LDAP* ld, PCTSTR pUsername, PCTSTR pDomain, PCTSTR pGroupname, TCHAR**& ppAttributeVal, CString& groupDN);
	bool GetGroupMembership(LDAP* ld, PCTSTR pUsername, TCHAR**& ppAttributeVal);
	bool ExtractGroups(LDAP* ld, TCHAR**& ppAttributeVal, CString& groupNames);
//	bool GetGroupDN(LDAP* ld, PCTSTR pGroupname, CString& groupDN);
//	ULONG FindMembers(LDAP* ld, PTSTR pSearchBase, PTSTR searchFilter, TCHAR**& ppAttributeVal);

	void DisplayFinalError(DWORD msg);

	int m_maxRetryCount;
	int m_retryCount;
	bool m_authError;
	bool m_getGroups;
	bool m_disableCancel;
	CString m_groupName;
	CString m_rootLdapPath;
	CString m_altUserName;
	CString m_userAttributes;

public:
	afx_msg void OnIdcancel();

	virtual afx_msg void OnBnClickedCanceled();

};
