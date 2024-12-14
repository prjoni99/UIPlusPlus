#pragma once

#include "IAction.h"
#include "Constants.h"
#include "ActionHelper.h"
#include "FTW\FTWTreeCtrl.h"
#include <unordered_set>

typedef std::pair<CString, CString> SoftwareInfo;

namespace std
{
	template<>
	class hash<SoftwareInfo>
	{
	public:
		size_t operator()(const SoftwareInfo& info) const
		{
			return hash<CString>() (info.first) ^ hash<CString>() (info.second);
		}
	};
};

namespace UIpp {

typedef std::vector<CString> stringlist;
typedef std::unordered_set<SoftwareInfo> softwareinfoset;

class CActionUserInfo : public IAction
{
public:
	CActionUserInfo(const ActionData& data) : IAction(data) { m_actionTypeName = XML_ACTION_TYPE_USERINFO; };
	~CActionUserInfo(void) {};
	INT_PTR Go(void);
	bool IsGUIAction(void) { return true; };

};

class CActionUserInfoFullScreen : public IAction
{
public:
	CActionUserInfoFullScreen(const ActionData& data) : IAction(data) { m_actionTypeName = XML_ACTION_TYPE_USERINFOFULLSCREEN; };
	~CActionUserInfoFullScreen(void) {};
	INT_PTR Go(void);
	bool IsGUIAction(void) { return true; };

};

class CActionErrorInfo : public IAction
{
public:
	CActionErrorInfo(const ActionData& data) : IAction(data) { m_actionTypeName = XML_ACTION_TYPE_ERRORINFO; };
	~CActionErrorInfo(void) {};

	INT_PTR Go(void);
	// While this is a GUI action, we should never get past it and therefore can never come back to it anyway
	bool IsGUIAction(void) { return false; };

};

class CActionSoftwareDiscovery : public IAction
{
public:
	CActionSoftwareDiscovery(const ActionData& data) : IAction(data) { m_actionTypeName = XML_ACTION_TYPE_SOFTWAREDISC; };
	~CActionSoftwareDiscovery(void) {};

	INT_PTR Go(void);
	void GetSoftware(softwareinfoset& softwareList, bool hive64, bool includeSystemComponents = false);
};

class CActionRegistry : public IAction
{
public:
	CActionRegistry(const ActionData& data) : IAction(data) { m_actionTypeName = XML_ACTION_TYPE_REGISTRY; };
	~CActionRegistry(void) {};

	INT_PTR Go(void);

};

class CActionRegistryWrite : public IAction
{
public:
	CActionRegistryWrite(const ActionData& data) : IAction(data) { m_actionTypeName = XML_ACTION_TYPE_REGISTRY_WRITE; };
	~CActionRegistryWrite(void) {};

	INT_PTR Go(void);

};

class CActionUserInput : public IAction
{
public:
	CActionUserInput(const ActionData& data) : IAction(data) { m_actionTypeName = XML_ACTION_TYPE_USERINPUT; };
	~CActionUserInput(void) {};

	INT_PTR Go(void);
	bool IsGUIAction(void) { return true; };

};

class CActionTPM : public IAction
{
public:
	CActionTPM(const ActionData& data) : IAction(data) { m_actionTypeName = XML_ACTION_TYPE_TPM; };
	~CActionTPM(void) {};

	INT_PTR Go(void);

};

class CActionAppTree : public IAction
{
public:
	CActionAppTree(const ActionData& data) : IAction(data) { m_actionTypeName = XML_ACTION_TYPE_APPTREE; };
	~CActionAppTree(void);

	INT_PTR Go(void);
	bool IsGUIAction(void) { return true; };

protected:
	enum class ItemType { Package = 1, Application };
	
	void GetAppSets(void);
	void GetTreeItems(pugi::xml_node& parentNode, FTW::CFTWTreeGroupItem* pParentItem = 0);

	int GetIDsFromVariableList(PCTSTR baseVariable, CStringList& selection, CActionAppTree::ItemType itemType);


	FTW::FTWTreeItemList m_treeItems;
	CStringList m_itemSelection;
};

//class CActionAD : public IAction
//{
//public:
//	CActionAD(const ActionData& data) : IAction(data) {  };
//	~CActionAD(void) {};
//};

class CActionWMIRead : public IAction
{
public:
	CActionWMIRead(const ActionData& data) : IAction(data) { m_actionTypeName = XML_ACTION_TYPE_WMI; };
	~CActionWMIRead(void) {};

	INT_PTR Go(void);
};

class CActionWMIWrite : public IAction
{
public:
	CActionWMIWrite(const ActionData& data) : IAction(data) { m_actionTypeName = XML_ACTION_TYPE_WMI_WRITE; };
	~CActionWMIWrite(void) {};

	INT_PTR Go(void);
};

class CActionTSVar : public IAction
{
public:
	CActionTSVar(const ActionData& data) : IAction(data) { m_actionTypeName = XML_ACTION_TYPE_TSVAR; };
	~CActionTSVar(void) {};

	INT_PTR Go(void);

};

class CActionVars : public IAction
{
public:
	CActionVars(const ActionData& data) : IAction(data) { m_actionTypeName = XML_ACTION_TYPE_VARS; };
	~CActionVars(void) {};

	INT_PTR Go(void);

};

class CActionDefaultValues : public IAction
{
public:
	CActionDefaultValues(const ActionData& data) : IAction(data) { m_actionTypeName = XML_ACTION_TYPE_DEFAULTVALS; };
	~CActionDefaultValues(void) {};

	INT_PTR Go(void);

};

class CActionUserAuth : public IAction
{
public:
	CActionUserAuth(const ActionData& data) : IAction(data) { m_actionTypeName = XML_ACTION_TYPE_USERAUTH; };
	~CActionUserAuth(void) {};

	INT_PTR Go(void);
	bool IsGUIAction(void) { return true; };

};

class CActionPreflight : public IAction
{
public:
	CActionPreflight(const ActionData& data) : IAction(data) { m_actionTypeName = XML_ACTION_TYPE_PREFLIGHT; };
	~CActionPreflight(void) {};

	INT_PTR Go(void);
	bool IsGUIAction(void) { return true; };

};

class CActionTSVarList : public IAction
{
public:
	CActionTSVarList(const ActionData& data) : IAction(data) { m_actionTypeName = XML_ACTION_TYPE_TSVARLIST; };
	~CActionTSVarList(void) {};

	INT_PTR Go(void);

};

class CActionFileRead : public IAction
{
public:
	CActionFileRead(const ActionData& data) : IAction(data) { m_actionTypeName = XML_ACTION_TYPE_FILEREAD; };
	~CActionFileRead(void) {};

	INT_PTR Go(void);

};

class CActionExternalCall : public IAction
{
public:
	CActionExternalCall(const ActionData& data) : IAction(data) { m_actionTypeName = XML_ACTION_TYPE_EXTERNALCALL; };
	~CActionExternalCall(void) {};

	INT_PTR Go(void);

};

class CActionSwitch : public IAction
{
public:
	CActionSwitch(const ActionData& data) : IAction(data) { m_actionTypeName = XML_ACTION_TYPE_SWITCH; };
	~CActionSwitch(void) {};

	INT_PTR Go(void);

};

class CActionSaveItems : public IAction
{
public:
	CActionSaveItems(const ActionData& data) : IAction(data) { m_actionTypeName = XML_ACTION_TYPE_SAVEITEMS; };
	~CActionSaveItems(void) {};

	INT_PTR Go(void);
};

class CActionRandomString : public IAction
{
public:
	CActionRandomString(const ActionData& data) : IAction(data) { m_actionTypeName = XML_ACTION_TYPE_RANDOMSTRING; };
	~CActionRandomString(void) {};

	INT_PTR Go(void);
};

//class CRestCallAction : public CAction
//{
//public:
//	CRestCallAction(const ActionData& data) : IAction(data) { m_actionTypeName = XML_ACTION_TYPE_SWITCH; };
//	~CRestCallAction(void) {};
//
//	INT_PTR Go(FTWLDAP::ILdapPtr& pLdap);
//
//};

}