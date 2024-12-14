#include "StdAfx.h"
#include "Actions.h"
#include "resource.h"
#include "TSVar.h"
#include "FTW\utils.h"
#include "FTW\TSProgress.h"

#include "Dialogs\DlgPreflight.h"
#include "Dialogs\DlgAppTree.h"
#include "Dialogs\DlgUserInfo.h"
#include "Dialogs\DlgUserInfoFullScreen.h"
#include "Dialogs\DlgUserAuth.h"

#include <tlhelp32.h>

namespace UIpp
{
	INT_PTR CActionUserInfo::Go(void)
	{
		//pugi::xml_attribute attrTitle = m_myAction.attribute(XML_ATTRIBUTE_TITLE);

		CString actionTitle = GetXMLAttribute(m_actionData.pActionNode, XML_ATTRIBUTE_TITLE, XML_ACTION_TYPE_USERINFO);
		bool centerTitle = FTW::IsTrue(GetXMLAttribute(m_actionData.pActionNode, XML_ATTRIBUTE_CENTERTITLE, XML_ACTION_FALSE));
		CString actionText = CTSEnv::Instance().VariableSubstitute(m_actionData.pActionNode->child_value());
		CString includeCancel = GetXMLAttribute(m_actionData.pActionNode, XML_ATTRIBUTE_SHOWCANCEL, XML_ACTION_FALSE);
		CString includeBack = GetXMLAttribute(m_actionData.pActionNode, XML_ATTRIBUTE_SHOWBACK, XML_ACTION_FALSE);
		CString bannerImagePath = GetXMLAttribute(m_actionData.pActionNode, XML_ATTRIBUTE_IMAGE);
		CString infoImagePath = GetXMLAttribute(m_actionData.pActionNode, XML_ATTRIBUTE_INFOIMAGE);
		int timeout = GetXMLIntAttribute(m_actionData.pActionNode, XML_ATTRIBUTE_TIMEOUT);

		CString timeoutMsg = _T("");
		DWORD timeoutAction = ERROR_SUCCESS;
		CString timeoutActionString = _T("");

		if (timeout > 0)
		{
			//timeoutMsg = GetXMLAttribute(m_actionData.pActionNode, XML_ATTRIBUTE_TIMEOUTMSG, XML_ATTRIBUTE_TIMEOUTMSG_DEF);
			timeoutActionString = GetXMLAttribute(m_actionData.pActionNode, XML_ATTRIBUTE_TIMEOUTACTION, XML_ATTRIBUTE_TIMEOUTACTION_DEF);
		}

		CDlgBase::DialogVisibilityFlags dlgFlags = CDlgBase::BuildDialogVisibilityFlags(FTW::IsTrue(includeCancel),
			FTW::IsTrue(includeBack),
			true,
			centerTitle);

		CDlgUserInfo dlg(actionTitle, dlgFlags, m_actionData, 
			actionText,
			bannerImagePath,
			infoImagePath);

		dlg.SetTimeoutInfo(CDlgBase::CDlgTimeoutInfo(timeout, timeoutMsg, timeoutActionString));

		return dlg.DoModal();
	}

	INT_PTR CActionUserInfoFullScreen::Go(void)
	{
		CString actionText = CTSEnv::Instance().VariableSubstitute(m_actionData.pActionNode->child_value());
		COLORREF textColor = FTW::HexToCOLORREF(GetXMLAttribute(m_actionData.pActionNode, XML_ATTRIBUTE_TEXTCOLOR, XML_ATTRIBUTE_FULLSCREENTEXTCOLOR_DEF));
		COLORREF backgroundColor = FTW::HexToCOLORREF(GetXMLAttribute(m_actionData.pActionNode, XML_ATTRIBUTE_COLOR, XML_ATTRIBUTE_ACCENTCOLOR_DEF));
		CString brandingImagePath = GetXMLAttribute(m_actionData.pActionNode, XML_ATTRIBUTE_IMAGE);

		CDlgUserInfoFullScreen dlg(backgroundColor, m_actionData.pCMLog, brandingImagePath, m_actionData.globalDialogTraits.fontFace);

		dlg.SetText(_T(""), actionText, textColor);
		dlg.SetupDialog(_T(""), m_actionData.globalDialogTraits);

		return dlg.DoModal();

	}

	INT_PTR CActionErrorInfo::Go(void)
	{
		//pugi::xml_attribute attrTitle = m_myAction.attribute(XML_ATTRIBUTE_TITLE);

		CString actionTitle = GetXMLAttribute(m_actionData.pActionNode, XML_ATTRIBUTE_TITLE, XML_ACTION_TYPE_USERINFO);
		bool centerTitle = FTW::IsTrue(GetXMLAttribute(m_actionData.pActionNode, XML_ATTRIBUTE_CENTERTITLE, XML_ACTION_FALSE));
		CString actionText = CTSEnv::Instance().VariableSubstitute(m_actionData.pActionNode->child_value());
		CString bannerImagePath = GetXMLAttribute(m_actionData.pActionNode, XML_ATTRIBUTE_IMAGE);
		CString infoImagePath = GetXMLAttribute(m_actionData.pActionNode, XML_ATTRIBUTE_INFOIMAGE);
		bool includeBack = FTW::IsTrue(GetXMLAttribute(m_actionData.pActionNode, XML_ATTRIBUTE_SHOWBACK, XML_ACTION_FALSE));
		bool includeCancel = FTW::IsTrue(GetXMLAttribute(m_actionData.pActionNode, XML_ATTRIBUTE_SHOWCANCEL, XML_ACTION_FALSE));

		CDlgBase::DialogVisibilityFlags dlgFlags = CDlgBase::BuildDialogVisibilityFlags(
			true,
			includeBack,
			false,
			centerTitle);

		CDlgUserInfo dlg(actionTitle,
			dlgFlags,
			m_actionData,
			actionText,
			bannerImagePath,
			infoImagePath);

		if (!includeCancel && m_actionData.inWinPE)
			dlg.ShowRestartButton();

		INT_PTR result = dlg.DoModal();

		if (!includeCancel && result == ERROR_CANCELLED && m_actionData.inWinPE)
		{
			DWORD lastError = ERROR_SUCCESS;
			PROCESSENTRY32 entry;
			entry.dwSize = sizeof(PROCESSENTRY32);

			HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

			if (snapshot != INVALID_HANDLE_VALUE && Process32First(snapshot, &entry) == TRUE)
			{
				while (Process32Next(snapshot, &entry) == TRUE)
				{
					if (_tcsicmp(entry.szExeFile, _T("winpeshl.exe")) == 0)
					{
						HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, entry.th32ProcessID);

						if (hProcess != NULL)
						{
							if (!TerminateProcess(hProcess, 0))
							{
								lastError = GetLastError();
							}
							else
							{
								m_actionData.pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info,
									AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
									FTW::FormatResourceString(IDS_LOGMSG_USERERRORREBOOTING));
							}

							CloseHandle(hProcess);
						}
						else
						{
							lastError = GetLastError();
						}
					}
				}

				CloseHandle(snapshot);
			}
			else
			{
				lastError = GetLastError();
			}
			
			if (lastError != ERROR_SUCCESS)
			{
				m_actionData.pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Error,
					AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
					FTW::FormatResourceString(IDS_LOGERROR_SHUTDOWN_FAIL, lastError, FTW::FormatHRString(lastError).c_str()));
			}

		}

		return result;
	}
	
	INT_PTR CActionPreflight::Go(void)
	{
		CString actionTitle = GetXMLAttribute(m_actionData.pActionNode, XML_ATTRIBUTE_TITLE, XML_ACTION_TYPE_USERINPUT);
		bool centerTitle = FTW::IsTrue(GetXMLAttribute(m_actionData.pActionNode, XML_ATTRIBUTE_CENTERTITLE, XML_ACTION_FALSE));
		bool includeBack = FTW::IsTrue(GetXMLAttribute(m_actionData.pActionNode, XML_ATTRIBUTE_SHOWBACK, XML_ACTION_FALSE));
		bool includeCancel = FTW::IsTrue(GetXMLAttribute(m_actionData.pActionNode, XML_ATTRIBUTE_SHOWCANCEL, XML_ACTION_FALSE));
		bool showOnFailOnly = FTW::IsTrue(GetXMLAttribute(m_actionData.pActionNode, XML_ATTRIBUTE_SHOWONFAILONLY, XML_ACTION_FALSE));
		CString actionSize = GetXMLAttribute(m_actionData.pActionNode, XML_ATTRIBUTE_SIZE, XML_ATTRIBUTE_SIZE_REGULAR);
		int timeout = GetXMLIntAttribute(m_actionData.pActionNode, XML_ATTRIBUTE_TIMEOUT);

		CString timeoutMsg = _T("");
		DWORD timeoutAction = ERROR_SUCCESS;
		CString timeoutActionString = _T("");

		if (timeout > 0)
		{
			//timeoutMsg = GetXMLAttribute(m_actionData.pActionNode, XML_ATTRIBUTE_TIMEOUTMSG, XML_ATTRIBUTE_TIMEOUTMSG_DEF);
			timeoutActionString = GetXMLAttribute(m_actionData.pActionNode, XML_ATTRIBUTE_TIMEOUTACTION, XML_ATTRIBUTE_TIMEOUTACTION_DEF);
		}

		CString text, check, description, errorDescription, warnDescription;

		bool checkSuccess = false;
		bool warnSuccess = false;
		bool allChecksPassed = true;
		bool anyChecksWithWarnings = false;
		UIpp::CUserInputBase::StatusType itemStatus = UIpp::CUserInputBase::StatusType::Failed;

		CDlgBase::DlgSize mySize = CDlgBase::DlgSize::Regular;

		if (actionSize == XML_ATTRIBUTE_SIZE_TALL)
			mySize = CDlgBase::DlgSize::Tall;

		CDlgBase::DialogVisibilityFlags dlgFlags = CDlgBase::BuildDialogVisibilityFlags(includeCancel,
			includeBack,
			true,
			centerTitle);

		CDlgPreflight dlg(actionTitle, dlgFlags, m_actionData, mySize);

		dlg.SetTimeoutInfo(CDlgBase::CDlgTimeoutInfo(timeout, timeoutMsg, timeoutActionString));
		dlg.ShowRefreshButton(FTW::CheckForFlag(m_actionData.globalDialogTraits.flags, FTW::DialogTraits::ALLOW_REFRESH));

		for (pugi::xml_node xmlinput = m_actionData.pActionNode->first_child(); xmlinput; xmlinput = xmlinput.next_sibling())
		{
			checkSuccess = false;
			warnSuccess = true;
			itemStatus = UIpp::CUserInputBase::StatusType::Failed;

			if (_tcsicmp(xmlinput.name(), XML_ACTION_PREFLIGHT_CHECK) != 0)
				continue;

			text = GetXMLAttribute(&xmlinput, XML_ATTRIBUTE_TEXT, XML_ATTRIBUTE_TEXT_DEF);
			description = GetXMLAttribute(&xmlinput, XML_ATTRIBUTE_DESCRIPTION);
			errorDescription = GetXMLAttribute(&xmlinput, XML_ATTRIBUTE_ERRORDESCRIPTION);
			warnDescription = GetXMLAttribute(&xmlinput, XML_ATTRIBUTE_WARNDESCRIPTION);

			if (CActionHelper::EvalCondition(m_actionData.pCMLog, xmlinput.attribute(XML_ATTRIBUTE_CONDITION).value(), m_actionData.pScriptHost, xmlinput.name(), text))
			{
				m_actionData.pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info,
					AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
					FTW::FormatResourceString(IDS_LOGMSG_ADDINFO, XML_ACTION_PREFLIGHT_CHECK, text));

				check = GetXMLAttribute(&xmlinput, XML_ATTRIBUTE_CHECKCONDITION, nullptr, nullptr, true);

				checkSuccess = CActionHelper::EvalCondition(m_actionData.pCMLog, check, m_actionData.pScriptHost, XML_ATTRIBUTE_CHECKCONDITION, text);

				allChecksPassed &= checkSuccess;

				if (checkSuccess)
				{
					check = GetXMLAttribute(&xmlinput, XML_ATTRIBUTE_WARNCONDITION, nullptr, nullptr, true);
					warnSuccess = CActionHelper::EvalCondition(m_actionData.pCMLog, check, m_actionData.pScriptHost, XML_ATTRIBUTE_WARNCONDITION, text);

					if (warnSuccess)
						itemStatus = UIpp::CUserInputBase::StatusType::Success;
					else
					{
						itemStatus = UIpp::CUserInputBase::StatusType::Warning;
						anyChecksWithWarnings = true;
					}
				}

				dlg.AddCheck(text, m_actionData.globalDialogTraits.fontFace, itemStatus, description, itemStatus == UIpp::CUserInputBase::StatusType::Failed ? errorDescription : warnDescription);
			}
			else
				m_actionData.pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info,
					AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
					FTW::FormatResourceString(IDS_LOGMSG_NOTADDINGINPUT, XML_ACTION_PREFLIGHT_CHECK, text));
		}

		if (!showOnFailOnly || (showOnFailOnly && !allChecksPassed))
		{
			dlg.ChecksWithWarnings(anyChecksWithWarnings);
			return dlg.DoModal();
		}

		else return ERROR_SUCCESS;
	}

	INT_PTR CActionUserAuth::Go(void)
	{
		CString domain, readonly;
		CString domainList;
		pugi::xml_node fieldNode;

		CString actionTitle = GetXMLAttribute(m_actionData.pActionNode, XML_ATTRIBUTE_TITLE, XML_ACTION_TYPE_USERAUTH);
		CString maxRetryCount = GetXMLAttribute(m_actionData.pActionNode, XML_ATTRIBUTE_MAXRETRY, XML_ATTRIBUTE_MAXRETRY_DEF);
		CString group = GetXMLAttribute(m_actionData.pActionNode, XML_ATTRIBUTE_GROUP);
		//CString groupsForUser = GetXMLAttribute(m_actionData.pActionNode, XML_ATTRIBUTE_GETGROUPSFORUSER);
		CString userAttributes = GetXMLAttribute(m_actionData.pActionNode, XML_ATTRIBUTE_ATTRIBUTES);
		bool getGroups = FTW::IsTrue(GetXMLAttribute(m_actionData.pActionNode, XML_ATTRIBUTE_GETGROUPS, XML_ACTION_FALSE));
		bool includeBack = FTW::IsTrue(GetXMLAttribute(m_actionData.pActionNode, XML_ATTRIBUTE_SHOWBACK, XML_ACTION_FALSE));
		bool disableCancel = FTW::IsTrue(GetXMLAttribute(m_actionData.pActionNode, XML_ATTRIBUTE_DISABLECANCEL, XML_ACTION_FALSE));

		if (!CTSEnv::Instance().Get(VAR_AUTHUSERDOMAIN, domain) || domain.GetLength() == 0)
			domain = GetXMLAttribute(m_actionData.pActionNode, XML_ATTRIBUTE_DOMAIN, XML_ATTRIBUTE_DOMAIN_DEF);

		CString domainController = GetXMLAttribute(m_actionData.pActionNode, XML_ATTRIBUTE_DOMAINCONTROLLER);
		bool doNotFallBack = FTW::IsTrue(GetXMLAttribute(m_actionData.pActionNode, XML_ATTRIBUTE_DONOTFALLBACK, XML_ACTION_FALSE));

		CDlgBase::DialogVisibilityFlags dlgFlags = CDlgBase::BuildDialogVisibilityFlags(true,
			includeBack,
			true,
			FTW::CheckForFlag(m_actionData.globalDialogTraits.flags, FTW::DialogTraits::SHOW_SIDEBAR));

		DlgUserAuthData authdata(group,
			userAttributes,
			domainController,
			getGroups,
			disableCancel,
			doNotFallBack,
			_tstoi(maxRetryCount));

		CDlgUserAuth dlg(actionTitle, dlgFlags, m_actionData, &authdata);

		fieldNode = m_actionData.pActionNode->find_child_by_attribute(XML_ACTION_TYPE_USERAUTHFIELD, XML_ATTRIBUTE_NAME, XML_ATTRIBUTE_USERNAME);

		CString question = GetXMLAttribute(&fieldNode, XML_ATTRIBUTE_QUESTION, IDS_PROMPT_USERNAMEQUESTION);
		CString hint = GetXMLAttribute(&fieldNode, XML_ATTRIBUTE_HINT, IDS_PROMPT_USERNAMEHINT);
		CString prompt = GetXMLAttribute(&fieldNode, XML_ATTRIBUTE_PROMPT, IDS_PROMPT_USERNAME);
		CString regex = GetXMLAttribute(&fieldNode, XML_ATTRIBUTE_REGEX, DEFAULT_USERNAMEREGEX);

		dlg.AddTextInput(question, VAR_AUTHUSER, m_actionData.globalDialogTraits.fontFace, hint, prompt, regex, true);

		fieldNode = m_actionData.pActionNode->find_child_by_attribute(XML_ACTION_TYPE_USERAUTHFIELD, XML_ATTRIBUTE_NAME, XML_ATTRIBUTE_PASSWORD);

		question = GetXMLAttribute(&fieldNode, XML_ATTRIBUTE_QUESTION, IDS_PROMPT_PASSWORDQUESTION);
		hint = GetXMLAttribute(&fieldNode, XML_ATTRIBUTE_HINT, IDS_PROMPT_PASSWORDHINT);
		prompt = GetXMLAttribute(&fieldNode, XML_ATTRIBUTE_PROMPT, IDS_PROMPT_PASSWORD);
		regex = GetXMLAttribute(&fieldNode, XML_ATTRIBUTE_REGEX, DEFAULT_PASSWORDREGEX);

		dlg.AddTextInput(question, 0, m_actionData.globalDialogTraits.fontFace, hint, prompt, regex, true, L"", true);

		fieldNode = m_actionData.pActionNode->find_child_by_attribute(XML_ACTION_TYPE_USERAUTHFIELD, XML_ATTRIBUTE_NAME, XML_ATTRIBUTE_DOMAIN);

		question = GetXMLAttribute(&fieldNode, XML_ATTRIBUTE_QUESTION, IDS_PROMPT_DOMAINQUESTION);
		hint = GetXMLAttribute(&fieldNode, XML_ATTRIBUTE_HINT, IDS_PROMPT_DOMAINHINT);
		prompt = GetXMLAttribute(&fieldNode, XML_ATTRIBUTE_PROMPT, IDS_PROMPT_DOMAIN);
		regex = GetXMLAttribute(&fieldNode, XML_ATTRIBUTE_REGEX, DEFAULT_DOMAINREGEX);
		readonly = GetXMLAttribute(&fieldNode, XML_ATTRIBUTE_READONLY, XML_ACTION_FALSE);
		domainList = GetXMLAttribute(&fieldNode, XML_ATTRIBUTE_LIST);

		if (domainList.IsEmpty())
			dlg.AddTextInput(question, VAR_AUTHUSERDOMAIN, m_actionData.globalDialogTraits.fontFace, hint, prompt, regex, true, domain, false, nullptr, true, FTW::IsTrue(readonly));
		else
		{
			CUserInputChoiceOptions domains;
			int domainIndex = 0;
			CString domainName;

			bool autoComplete = FTW::IsTrue(GetXMLAttribute(&fieldNode, XML_ATTRIBUTE_AUTOCOMPLETE, XML_ACTION_FALSE));

			domainName = domainList.Tokenize(_T(",;"), domainIndex);

			while (domainIndex != -1)
			{
				domainName = domainName.Trim();

				domains.AddItem(domainName, domainName, domainName);

				domainName = domainList.Tokenize(_T(",;"), domainIndex);
			}

			dlg.AddComboInput(question, VAR_AUTHUSERDOMAIN, m_actionData.globalDialogTraits.fontFace, nullptr, domains, 5, false, domain, true, autoComplete);
		}

		return dlg.DoModal();
	}

	int CActionAppTree::GetIDsFromVariableList(PCTSTR baseVariable, CStringList& selection, CActionAppTree::ItemType itemType)
	{
		CString variable, value, formatString;
		int count = 1;
		BOOL foundValue = false;

		if (itemType == CActionAppTree::ItemType::Application)
			formatString = _T("%s%02d");
		else
			formatString = _T("%s%03d");
		do
		{
			variable.Format(formatString, baseVariable, count);

			foundValue = CTSEnv::Instance().Get(variable, value);

			if (foundValue)
			{
				selection.AddTail(value);
				count++;
			}

		} while (foundValue);

		return (count - 1);
	}

	INT_PTR CActionAppTree::Go(void)
	{
		CString nodeNameAttr;
		CString actionTitle = GetXMLAttribute(m_actionData.pActionNode, XML_ATTRIBUTE_TITLE, XML_ACTION_TYPE_APPTREE);
		bool centerTitle = FTW::IsTrue(GetXMLAttribute(m_actionData.pActionNode, XML_ATTRIBUTE_CENTERTITLE, XML_ACTION_FALSE));
		CString pkgVarBase = GetXMLAttribute(m_actionData.pActionNode, XML_ATTRIBUTE_PACKAGEVAR, XML_ATTRIBUTE_PACKAGEVAR_DEF);
		CString appVarBase = GetXMLAttribute(m_actionData.pActionNode, XML_ATTRIBUTE_APPVAR, XML_ATTRIBUTE_APPVAR_DEF);
		CString actionSize = GetXMLAttribute(m_actionData.pActionNode, XML_ATTRIBUTE_SIZE, XML_ATTRIBUTE_SIZE_REGULAR);
		bool treeExpanded = FTW::IsTrue(GetXMLAttribute(m_actionData.pActionNode, XML_ATTRIBUTE_EXPANDED, XML_ACTION_TRUE));
		bool includeBack = FTW::IsTrue(GetXMLAttribute(m_actionData.pActionNode, XML_ATTRIBUTE_SHOWBACK, XML_ACTION_FALSE));
		bool includeCancel = FTW::IsTrue(GetXMLAttribute(m_actionData.pActionNode, XML_ATTRIBUTE_SHOWCANCEL, XML_ACTION_FALSE));
		bool noDefaultButton = FTW::IsTrue(GetXMLAttribute(m_actionData.pActionNode, XML_ATTRIBUTE_NODEFAULT, XML_ACTION_FALSE));

		CDlgBase::DlgSize mySize = CDlgBase::DlgSize::Regular;

		if (actionSize == XML_ATTRIBUTE_SIZE_TALL)
			mySize = CDlgBase::DlgSize::Tall;
		else if (actionSize == XML_ATTRIBUTE_SIZE_EXTRATALL)
			mySize = CDlgBase::DlgSize::ExtraTall;

		GetIDsFromVariableList(pkgVarBase, m_itemSelection, CActionAppTree::ItemType::Package);
		GetIDsFromVariableList(appVarBase, m_itemSelection, CActionAppTree::ItemType::Application);
		GetAppSets();

		CDlgBase::DialogVisibilityFlags dlgFlags = CDlgBase::BuildDialogVisibilityFlags(
			includeCancel,
			includeBack,
			true,
			centerTitle,
			noDefaultButton);

		CDlgAppTree dlg(actionTitle, dlgFlags, m_actionData, &m_treeItems, mySize, treeExpanded);

		INT_PTR retVal = dlg.DoModal();

		if (retVal == 0)
		{
			OrderedSoftwareMap selectedSoftware;
			CString variableName;
			int appCount = 0, pkgCount = 0;

			std::for_each(m_treeItems.begin(), m_treeItems.end(), [&](FTW::CFTWTreeItemPtr& pTI)
			{
				if (pTI->Type() == FTW::CFTWTreeItem::ItemType::LEAF && pTI->IsChecked())
				{
					SoftwareMap::const_iterator it = m_actionData.pSoftware->find(pTI->GetID());

					selectedSoftware.insert(OrderedSoftwareMap::value_type(it->second->GetIndex(), it->second.get()));
				}
			});

			std::for_each(selectedSoftware.begin(), selectedSoftware.end(), [&](std::pair<const int, CSoftware*>& pTI2)
			{
				if (pTI2.second->Type() == CSoftware::SoftwareType::APPLICATION)
					variableName.Format(_T("%s%02d"), appVarBase.GetString(), ++appCount);

				else if (pTI2.second->Type() == CSoftware::SoftwareType::PACKAGE)
					variableName.Format(_T("%s%03d"), pkgVarBase.GetString(), ++pkgCount);

				CTSEnv::Instance().Set(m_actionData.pCMLog, variableName, pTI2.second->GetVariableValue());
			});

			variableName.Format(_T("%s%02d"), appVarBase.GetString(), ++appCount);
			CTSEnv::Instance().Set(m_actionData.pCMLog, variableName, _T(""));
			variableName.Format(_T("%s%03d"), pkgVarBase.GetString(), ++pkgCount);
			CTSEnv::Instance().Set(m_actionData.pCMLog, variableName, _T(""));


		}

		return retVal;
	}

	void CActionAppTree::GetAppSets(void)
	{
		CString nodeNameAttr;
		pugi::xml_node xmlSoftwareSet;

		pugi::xpath_node_set xmlSets = m_actionData.pActionNode->child(XML_ACTION_APPTREE_SOFTWARESETS).select_nodes(XML_ACTION_APPTREE_SET);

		for (size_t nodeIndex = 0; nodeIndex < xmlSets.size(); nodeIndex++)
		{
			xmlSoftwareSet = xmlSets[nodeIndex].node();

			nodeNameAttr = GetXMLAttribute(&xmlSoftwareSet, XML_ATTRIBUTE_NAME);

			m_actionData.pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info,
				AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
				FTW::FormatResourceString(IDS_LOGMSG_NODEFOUND, xmlSoftwareSet.name(), nodeNameAttr));

			if (!CActionHelper::EvalCondition(m_actionData.pCMLog, xmlSoftwareSet.attribute(XML_ATTRIBUTE_CONDITION).value(), m_actionData.pScriptHost, xmlSoftwareSet.name(), _T("")))
				continue;

			GetTreeItems(xmlSoftwareSet);
		}
	}

	void CActionAppTree::GetTreeItems(pugi::xml_node& parentNode, FTW::CFTWTreeGroupItem* pParentItem)
	{
		CString id, label;
		SoftwareMap::const_iterator it;
		CString requiredItem, defaultItem, hiddenItem;
		bool isItemDefault = false;

		for (pugi::xml_node childNode = parentNode.first_child(); childNode; childNode = childNode.next_sibling())
		{
			if (_tcsicmp(childNode.name(), XML_ACTION_APPTREE_SOFTWAREREF) != 0 && _tcsicmp(childNode.name(), XML_ACTION_APPTREE_SOFTWAREGROUP) != 0)
				continue;

			id = GetXMLAttribute(&childNode, XML_ATTRIBUTE_ID);

			if (id.GetLength() == 0)
			{
				m_actionData.pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Warning,
					AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
					FTW::FormatResourceString(IDS_LOGMSG_SOFTWAREFOUNDNOID, childNode.name()));
				continue;
			}

			if (!CActionHelper::EvalCondition(m_actionData.pCMLog, childNode.attribute(XML_ATTRIBUTE_CONDITION).value(), m_actionData.pScriptHost, childNode.name(), _T("")))
				continue;

			requiredItem = GetXMLAttribute(&childNode, XML_ATTRIBUTE_REQUIRED, XML_ACTION_FALSE);
			defaultItem = GetXMLAttribute(&childNode, XML_ATTRIBUTE_DEFAULT, XML_ACTION_FALSE);

			if (_tcsicmp(childNode.name(), XML_ACTION_APPTREE_SOFTWAREREF) == 0)
			{
				hiddenItem = GetXMLAttribute(&childNode, XML_ATTRIBUTE_HIDDEN, XML_ACTION_FALSE);

				it = m_actionData.pSoftware->find(id);

				if (it != m_actionData.pSoftware->end())
				{
					if (m_itemSelection.GetCount() > 0)
						if (m_itemSelection.Find(it->second->GetVariableValue()) != NULL)
							isItemDefault = true;
						else
							isItemDefault = false;
					else
						isItemDefault = !pParentItem ? FTW::IsTrue(defaultItem) : FTW::IsTrue(defaultItem) | pParentItem->IsDefault();

					FTW::CFTWTreeLeafItem* pItem = new FTW::CFTWTreeLeafItem(it->second->GetLabel(), id, pParentItem,
						isItemDefault,
						!pParentItem ? FTW::IsTrue(requiredItem) : FTW::IsTrue(requiredItem) | pParentItem->IsRequired(),
						!pParentItem ? FTW::IsTrue(hiddenItem) : FTW::IsTrue(hiddenItem) | pParentItem->IsHidden(),
						it->second->GetIncludes());

					m_treeItems.insert(m_treeItems.end(), FTW::CFTWTreeItemPtr(pItem));

					if (pParentItem)
						pParentItem->AddChildItem(pItem);

					m_actionData.pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info,
						AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
						FTW::FormatResourceString(IDS_LOGMSG_ADDSOFTWARETOTREE, id, pParentItem == 0 ? _T("none") : pParentItem->GetLabel()));
				}
			}
			else if (_tcsicmp(childNode.name(), XML_ACTION_APPTREE_SOFTWAREGROUP) == 0)
			{
				label = GetXMLAttribute(&childNode, XML_ATTRIBUTE_LABEL, XML_ATTRIBUTE_LABEL_DEF);

				FTW::CFTWTreeGroupItem* pGroup = new FTW::CFTWTreeGroupItem(label, id, pParentItem,
					!pParentItem ? FTW::IsTrue(defaultItem) : FTW::IsTrue(defaultItem) | pParentItem->IsDefault(),
					!pParentItem ? FTW::IsTrue(requiredItem) : FTW::IsTrue(requiredItem) | pParentItem->IsRequired());

				m_treeItems.insert(m_treeItems.end(), FTW::CFTWTreeItemPtr(pGroup));

				if (pParentItem)
					pParentItem->AddChildItem(pGroup);

				m_actionData.pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info,
					AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
					FTW::FormatResourceString(IDS_LOGMSG_ADDGROUPTOTREE, id, pParentItem == 0 ? _T("none") : pParentItem->GetLabel()));

				GetTreeItems(childNode, pGroup);
			}
		}
	}

	CActionAppTree::~CActionAppTree(void)
	{

		bool success = m_treeItems.empty();

	}

}