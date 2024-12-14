#pragma once

#include "FTWCMLog.h"
#include "TSVar.h"
#include "pugi\pugixml.hpp"
#include "ScriptHost.h"
#include "Software.h"
#include "Dialogs\DlgData.h"
#include "CLdap.h"

#include <memory>
#include <map>
#include <functional>

#define MAX_ATTRIBUTE_LENGTH 1024

#define REGISTER_ACTION(T,T1) static UIpp::CActionMaker<T> maker(T1);

namespace UIpp {

	CString GetXMLAttribute(const pugi::xml_node& node, PCTSTR attributeName, PCTSTR defaultValue = _T(""), CTSEnv* ptsEnv = nullptr, bool raw = false);
	CString GetXMLAttribute(const pugi::xml_node* const node, PCTSTR attributeName, PCTSTR defaultValue = _T(""), CTSEnv* ptsEnv = nullptr, bool raw = false);
	CString GetXMLAttribute(const pugi::xml_node& node, PCTSTR attributeName, DWORD defaultValue, CTSEnv* ptsEnv = nullptr, bool raw = false);
	CString GetXMLAttribute(const pugi::xml_node* const node, PCTSTR attributeName, DWORD defaultValue, CTSEnv* ptsEnv = nullptr, bool raw = false);
	int GetXMLIntAttribute(const pugi::xml_node& node, PCTSTR attributeName, int defaultValue = 0, CTSEnv* ptsEnv = nullptr);
	int GetXMLIntAttribute(const pugi::xml_node* const node, PCTSTR attributeName, int defaultValue = 0, CTSEnv* ptsEnv = nullptr);

	struct ActionData
	{
		ActionData(const ActionData& data)
			: pActionNode(data.pActionNode),
			pScriptHost(data.pScriptHost),
			inTS(data.inTS),
			inWinPE(data.inWinPE),
			pSoftware(data.pSoftware),
			pCMLog(data.pCMLog),
			pTSEnv(data.pTSEnv),
			pLdap(data.pLdap),
			messages(data.messages),
			globalDialogTraits(data.globalDialogTraits.title,
				data.globalDialogTraits.subtitle,
				data.globalDialogTraits.fontFace,
				data.globalDialogTraits.hIcon,
				data.globalDialogTraits.accentColor,
				data.globalDialogTraits.textColor,
				data.globalDialogTraits.screenScale,
				data.globalDialogTraits.flags)
		{
			TRACE("Copy");

		}

		ActionData(const pugi::xml_node* pan,
			FTW::CScriptHost* const psh,
			bool ts,
			bool pe,
			const SoftwareMap* const ps,
			PCTSTR ttl,
			PCTSTR subttl,
			PCTSTR fface,
			const HICON icon,
			const COLORREF aColor,
			const COLORREF tColor,
			FTWCMLOG::ICMLogPtr plog,
			CTSEnv* pEnv,
			FTW::Scale s,
			FTWLDAP::ILdapPtr& pl,
			pugi::xml_node msg,
			FTW::DialogTraitFlags flags = FTW::DialogTraits::DEFAULT_FLAGS)
				: pActionNode(pan),
				pScriptHost(psh),
				inTS(ts),
				inWinPE(pe),
				pSoftware(ps),
				pCMLog(plog),
				pTSEnv(pEnv),
				pLdap(pl),
				messages(msg),
				globalDialogTraits(ttl,
					subttl,
					fface,
					icon,
					aColor,
					tColor,
					s,
					flags)
		{
			TRACE("Create");
		}

		const pugi::xml_node* pActionNode;
		FTW::CScriptHost* const pScriptHost;
		bool inTS;
		bool inWinPE;
		const SoftwareMap* const pSoftware;
		FTWCMLOG::ICMLogPtr pCMLog;
		CTSEnv* pTSEnv;
		FTWLDAP::ILdapPtr& pLdap;
		pugi::xml_node messages;
		FTW::DialogTraits globalDialogTraits;
	};

	class IAction
	{
	public:
		//	IAction() {};
		IAction(const ActionData& data) : m_actionData(data) {};
		virtual ~IAction(void) {};
		//virtual NextActionType Go(PCTSTR pTitle, PCTSTR pSubtitle, HICON hIcon, FTW::CScriptHost& scriptHost) { return Finish; };
		virtual INT_PTR Go(void) = 0;//{ return ERROR_SUCCESS; };
		virtual bool IsGUIAction(void) { return false; };

	protected:
		const ActionData& m_actionData;
		CString m_actionTypeName;
	};

	class IActionMaker
	{
	public:
		virtual IAction* Create(ActionData& actionData) const = 0;
		virtual ~IActionMaker() {}
	};

	class CActionFactory
	{
	public:
		static CActionFactory& Instance();
		void RegisterMaker(const std::wstring key, IActionMaker* maker);
		IAction* Create(const std::wstring, ActionData& actionData) const;

	private:
		CActionFactory() {}

		CActionFactory(const CActionFactory& other);
		CActionFactory& operator=(const CActionFactory& other);

		std::map<std::wstring, IActionMaker*> _makers;
	};

	/// Helper template to simplify the process of generating action-maker
	template<typename T>
	class ActionMaker : public IActionMaker
	{
	public:
		/// When created, the action maker will automaticly register itself with the factory
		/// Note - you are discouraged from using ActionMaker outside REGISTER_ACTION macro
		/// For example, creating ActionMaker on the stack will end up badly
		ActionMaker(const std::wstring key)
		{
			CActionFactory::Instance().RegisterMaker(key, this);
		}

		virtual IAction* Create(ActionData& actionData) const
		{
			// Create new instance of T using constructor from XML
			// Assumes T has a constructor that accepts ActionData&
			return new T(actionData);
		}
	};
}

	//typedef bool(__stdcall *RegisterActionFunction)(UIpp::typeActionFactory* pActiontypes, FTWCMLOG::ICMLogPtr pLog);
