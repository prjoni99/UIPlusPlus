#include "StdAfx.h"
#include "Actions.h"
#include "resource.h"
#include "TSVar.h"
#include "CodeProject\regkey.h"
#include "FTW\utils.h"
#include "FTW\FTWVersion.h"

#include <algorithm>
#include <filesystem>
#include <random>

namespace UIpp
{

	INT_PTR CActionWMIRead::Go(void)
	{
		CString wmiNamespace = GetXMLAttribute(m_actionData.pActionNode, XML_ATTRIBUTE_NAMESPACE, XML_ATTRIBUTE_NAMESPACE_DEF);
		CString wmiClass = GetXMLAttribute(m_actionData.pActionNode, XML_ATTRIBUTE_CLASS);
		CString wmiProperty = GetXMLAttribute(m_actionData.pActionNode, XML_ATTRIBUTE_PROPERTY);
		CString wmiKey = GetXMLAttribute(m_actionData.pActionNode, XML_ATTRIBUTE_KEYQUALIFIER);
		CString wmiVariable = GetXMLAttribute(m_actionData.pActionNode, XML_ATTRIBUTE_VARIABLE, XML_ATTRIBUTE_VARIABLE_DEF);
		CString defaultValue = GetXMLAttribute(m_actionData.pActionNode, XML_ATTRIBUTE_DEFAULT);
		CString wmiQuery = GetXMLAttribute(m_actionData.pActionNode, XML_ATTRIBUTE_QUERY);

		CString wmiValue = _T("");
		bool valueFound = false;

		FTW::CWMIAccess wmi(wmiNamespace);

		if (wmiClass.GetLength() > 0)
		{
			if (wmiProperty.GetLength() > 0 && CActionHelper::GetWMIPropertyFromFirstInstance(m_actionData.pCMLog, wmi, wmiClass, wmiKey, wmiProperty, wmiValue))
			{
				m_actionData.pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info,
					AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
					FTW::FormatResourceString(IDS_LOGMSG_WMIVALUE, wmiNamespace, wmiClass, wmiProperty, wmiKey, wmiValue));

				if (wmiValue != _T(""))
					valueFound = true;
			}
		}
		else if(wmiQuery.GetLength() > 0)
		{
			if(wmiProperty.GetLength() && CActionHelper::QueryWMI(m_actionData.pCMLog, wmi, wmiProperty, wmiQuery, wmiValue))
			{
				m_actionData.pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info,
					AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
					FTW::FormatResourceString(IDS_LOGMSG_WMIQUERYVALUE, wmiQuery, wmiNamespace, wmiValue));

				if(wmiValue != _T(""))
					valueFound = true;
			}
		}
		
		if (defaultValue.GetLength() != 0 && !valueFound)
		{
			m_actionData.pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info,
				AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
				FTW::FormatResourceString(IDS_LOGMSG_USING_DEFAULTVALUE, defaultValue));

			wmiValue = defaultValue;
			valueFound = true;
		}

		if(valueFound)
			CTSEnv::Instance().Set(m_actionData.pCMLog, wmiVariable, wmiValue);

		return ERROR_SUCCESS;
	}

	INT_PTR CActionWMIWrite::Go(void)
	{
		CString wmiNamespace = GetXMLAttribute(m_actionData.pActionNode, XML_ATTRIBUTE_NAMESPACE, XML_ATTRIBUTE_NAMESPACE_DEF);
		CString wmiClass = GetXMLAttribute(m_actionData.pActionNode, XML_ATTRIBUTE_CLASS);

		FTW::CWMIAccess wmi;

		try
		{
			wmi.Connect(wmiNamespace, false);
		}
		catch (FTW::FTWException& j)
		{
			m_actionData.pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Warning,
				AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
				FTW::FormatResourceString(IDS_LOGERROR_WMINAMESPACEOPEN, wmiNamespace, j.Message()));

			try
			{
				m_actionData.pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info,
					AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
					FTW::FormatResourceString(IDS_LOGMSG_WMINAMESPACE_CREATE, wmiNamespace));

				wmi.Connect(wmiNamespace, true);

				m_actionData.pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info,
					AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
					FTW::FormatResourceString(IDS_LOGMSG_WMINAMESPACE_CREATESUCCESS));
			}
			catch (FTW::FTWException& e)
			{
				m_actionData.pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Error,
					AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
					FTW::FormatResourceString(IDS_LOGERROR_WMINAMESPACE_CREATEFAILED, e.Message()));

				return ERROR_SUCCESS;
			}

		}

		if (wmiClass.GetLength())
		{

			FTW::WMIPropertiesList propList;

			for (pugi::xml_node wmiProperty = m_actionData.pActionNode->first_child(); wmiProperty; wmiProperty = wmiProperty.next_sibling())
			{
				FTW::WMIProperties newProps;

				newProps.propertyName = GetXMLAttribute(&wmiProperty, XML_ATTRIBUTE_NAME);
				newProps.propertyType = GetXMLAttribute(&wmiProperty, XML_ATTRIBUTE_TYPE, XML_ATTRIBUTE_CIMTYPE_DEF);
				newProps.isPropertyKey = FTW::IsTrue(GetXMLAttribute(&wmiProperty, XML_ATTRIBUTE_KEY, XML_ACTION_FALSE));
				newProps.propertyValue = GetXMLAttribute(&wmiProperty, XML_ATTRIBUTE_VALUE);

				if (_tcsnlen(newProps.propertyName, MAX_STRING_LENGTH) > 0 ||
					_tcsnlen(newProps.propertyValue, MAX_STRING_LENGTH) > 0 ||
					_tcsnlen(newProps.propertyType, MAX_STRING_LENGTH) > 0)
				{
					propList.push_back(newProps);
				}

			}

			if (propList.empty())
				return ERROR_SUCCESS;

			try
			{
				wmi.Open(wmiClass);
			}
			catch (FTW::FTWException& j)
			{
				m_actionData.pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Warning,
					AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
					FTW::FormatResourceString(IDS_LOGERROR_WMICLASSOPEN, wmiClass, j.Message()));

				try
				{
					m_actionData.pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info,
						AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
						FTW::FormatResourceString(IDS_LOGMSG_WMICLASS_CREATE, wmiClass));

					if (wmi.Open(wmiClass, true, &propList))
					{
						m_actionData.pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info,
							AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
							FTW::FormatResourceString(IDS_LOGMSG_WMICLASS_CREATESUCCESS));
					}

				}
				catch (FTW::FTWException& j)
				{
					m_actionData.pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Error,
						AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
						FTW::FormatResourceString(IDS_LOGERROR_WMICLASS_CREATEFAILED, j.Message()));

					return ERROR_SUCCESS;
				}

			}

			try
			{
				m_actionData.pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info,
					AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
					FTW::FormatResourceString(IDS_LOGMSG_WMIINSTANCE_CREATE, wmiClass));

				if (wmi.CreateInstance(propList))
					m_actionData.pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info,
						AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
						FTW::FormatResourceString(IDS_LOGMSG_WMIINSTANCE_CREATESUCCESS));
			}
			catch (FTW::FTWException& j)
			{
				m_actionData.pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Error,
					AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
					FTW::FormatResourceString(IDS_LOGERROR_WMIINSTANCE_CREATEFAILED, j.Message()));

				return ERROR_SUCCESS;
			}

		}

		return ERROR_SUCCESS;
	}

	INT_PTR CActionRegistry::Go(void)
	{
		CString hive = GetXMLAttribute(m_actionData.pActionNode, XML_ATTRIBUTE_HIVE, XML_ATTRIBUTE_HIVE_HKLM);
		CString key = GetXMLAttribute(m_actionData.pActionNode, XML_ATTRIBUTE_KEY);
		CString valueName = GetXMLAttribute(m_actionData.pActionNode, XML_ATTRIBUTE_VALUE);
		CString variable = GetXMLAttribute(m_actionData.pActionNode, XML_ATTRIBUTE_VARIABLE, XML_ATTRIBUTE_VARIABLE_DEF);
		CString reg64 = GetXMLAttribute(m_actionData.pActionNode, XML_ATTRIBUTE_REG64, XML_ACTION_TRUE);
		CString defaultValue = GetXMLAttribute(m_actionData.pActionNode, XML_ATTRIBUTE_DEFAULT);

		CString value;
		CodeProject::RegValue regVal;

		HKEY hHive;

		if (key.GetLength() > 0 && valueName.GetLength() > 0)
		{
			if (hive == XML_ATTRIBUTE_HIVE_HKLM)
				hHive = HKEY_LOCAL_MACHINE;
			else if (hive == XML_ATTRIBUTE_HIVE_HKCU)
				hHive = HKEY_CURRENT_USER;
			else
			{
				m_actionData.pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Error,
					AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
					FTW::FormatResourceString(IDS_LOGERROR_UNKNOWNHIVETYPE, hive));
				return ERROR_SUCCESS;
			}

			CodeProject::RegKey reg(hHive);

			LONG ret = reg.Open(key, FTW::IsTrue(reg64) ? KEY_READ | KEY_WOW64_64KEY : KEY_READ);

			if (ret == ERROR_SUCCESS)
			{
				regVal = reg[valueName];

				if (regVal.Type == REG_SZ || regVal.Type == REG_EXPAND_SZ || regVal.Type == REG_BINARY)
					value = regVal;
				else if (regVal.Type == REG_DWORD || regVal.Type == REG_QWORD)
					value.Format(_T("%d"), regVal);
				else
					value = VALUE_UNSUPPORTED;

				m_actionData.pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info,
					AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
					FTW::FormatResourceString(IDS_LOGMSG_REGVALUE, hive, key, valueName, value));

				if (defaultValue.GetLength() != 0 && value == _T(""))
				{
					m_actionData.pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info,
						AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
						FTW::FormatResourceString(IDS_LOGMSG_USING_DEFAULTVALUE, defaultValue));

					value = defaultValue;
				}
				
				CTSEnv::Instance().Set(m_actionData.pCMLog, variable, value);
			}
			else
			{
				FTW::FTWErrorCodeException err(ret);

				m_actionData.pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Error,
					AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
					FTW::FormatResourceString(IDS_LOGERROR_READREGVALUE, hive, key, valueName, err.Message()));

				if (defaultValue.GetLength() != 0)
				{
					m_actionData.pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info,
						AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
						FTW::FormatResourceString(IDS_LOGMSG_USING_DEFAULTVALUE, defaultValue));

					CTSEnv::Instance().Set(m_actionData.pCMLog, variable, defaultValue);
				}
			}
		}

		return ERROR_SUCCESS;
	}

	INT_PTR CActionRegistryWrite::Go(void)
	{
		CString hive = GetXMLAttribute(m_actionData.pActionNode, XML_ATTRIBUTE_HIVE, XML_ATTRIBUTE_HIVE_HKLM);
		CString key = GetXMLAttribute(m_actionData.pActionNode, XML_ATTRIBUTE_KEY);
		CString valueName = GetXMLAttribute(m_actionData.pActionNode, XML_ATTRIBUTE_VALUE);
		CString valueType = GetXMLAttribute(m_actionData.pActionNode, XML_ATTRIBUTE_REGVALUE_TYPE, XML_ATTRIBUTE_REGVALUE_TYPE_DEF);
		CString reg64 = GetXMLAttribute(m_actionData.pActionNode, XML_ATTRIBUTE_REG64, XML_ACTION_TRUE);

		CString regValue = CTSEnv::Instance().VariableSubstitute(m_actionData.pActionNode->child_value());

		//CodeProject::RegValue regVal;

		HKEY hHive;

		if (key.GetLength() > 0 && valueName.GetLength() > 0)
		{
			if (hive == XML_ATTRIBUTE_HIVE_HKLM)
				hHive = HKEY_LOCAL_MACHINE;
			else if (hive == XML_ATTRIBUTE_HIVE_HKCU)
				hHive = HKEY_CURRENT_USER;
			else
			{
				m_actionData.pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Error,
					AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
					FTW::FormatResourceString(IDS_LOGERROR_UNKNOWNHIVETYPE, hive));
				return ERROR_SUCCESS;
			}

			m_actionData.pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info,
				AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
				FTW::FormatResourceString(IDS_LOGMSG_REGWRITE_PREP, regValue, hive, valueName, key));

			CodeProject::RegKey reg(hHive);

			LONG ret = reg.Open(key, FTW::IsTrue(reg64) ? KEY_WRITE | KEY_WOW64_64KEY : KEY_WRITE);

			if (ret == ERROR_SUCCESS || ret == ERROR_FILE_NOT_FOUND)
			{
				if (ret == ERROR_FILE_NOT_FOUND)
				{
					m_actionData.pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info,
						AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
						FTW::FormatResourceString(IDS_LOGMSG_KEYNOTFOUND, hive, key));

					if (reg.Create(key, FTW::IsTrue(reg64) ? KEY_WRITE | KEY_WOW64_64KEY : KEY_WRITE) == ERROR_SUCCESS)
					{
						m_actionData.pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info,
							AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
							FTW::FormatResourceString(IDS_LOGMSG_KEYCREATED));
					}
					else
					{
						FTW::FTWErrorCodeException e(ret);

						m_actionData.pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Error,
							AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
							FTW::FormatResourceString(IDS_LOGERROR_KEYCREATEFAIL, e.Message()));

						return ERROR_SUCCESS;
					}
				}

				try
				{
					if (valueType == _T("REG_SZ"))
						reg[valueName] = regValue.GetBuffer();
					else if (valueType == _T("REG_DWORD"))
						reg[valueName] = (DWORD)_tstol(regValue);
					else
					{
						m_actionData.pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Warning,
							AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
							FTW::FormatResourceString(IDS_LOGMSG_UNSUPPORTEDREGISTRYTYPE, valueType));
						return ERROR_SUCCESS;
					}

					m_actionData.pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info,
						AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
						FTW::FormatResourceString(IDS_LOGMSG_REGWRITESUCCESS));
				}

				catch (LONG& err)
				{
					FTW::FTWErrorCodeException e(err);

					m_actionData.pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info,
						AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
						FTW::FormatResourceString(IDS_LOGERROR_REGWRITEFAIL, e.Message()));
				}

				regValue.ReleaseBuffer();
			}
			else
			{
				FTW::FTWErrorCodeException err(ret);

				m_actionData.pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Error,
					AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
					FTW::FormatResourceString(IDS_LOGERROR_REGKEYOPEN, hive, key, err.Message()));
			}

		}
		return ERROR_SUCCESS;
	}

	INT_PTR CActionTSVar::Go(void)
	{
		CString variableName = GetXMLAttribute(m_actionData.pActionNode, XML_ATTRIBUTE_VARIABLE);
		if(variableName.GetLength() == 0)
			variableName = GetXMLAttribute(m_actionData.pActionNode, XML_ATTRIBUTE_NAME, XML_ATTRIBUTE_VARIABLE_DEF);
		CString variableValue = CTSEnv::Instance().VariableSubstitute(m_actionData.pActionNode->child_value());
		bool dontEval = FTW::IsTrue(GetXMLAttribute(m_actionData.pActionNode, XML_ATTRIBUTE_DONTEVAL, XML_ACTION_FALSE));

		_variant_t r;

		if (!dontEval && SUCCEEDED(m_actionData.pScriptHost->Eval(variableValue, &r)) && r.vt > 0 && ((_bstr_t)r).length() > 0)
		{
			variableValue = ((_bstr_t)r).GetBSTR();
		}

		if (variableName.GetLength() > 0)
			CTSEnv::Instance().Set(m_actionData.pCMLog, variableName, variableValue);

		return ERROR_SUCCESS;
	}

	INT_PTR CActionVars::Go(void)
	{
		CString direction = GetXMLAttribute(m_actionData.pActionNode, XML_ATTRIBUTE_DIRECTION, XML_ATTRIBUTE_DIRECTION_SAVE);
		CString filename = GetXMLAttribute(m_actionData.pActionNode, XML_ATTRIBUTE_FILENAME, XML_ATTRIBUTE_FILENAME_DEF);

		if (direction.Compare(XML_ATTRIBUTE_DIRECTION_SAVE) == 0)
			CTSEnv::Instance().SaveToFile(m_actionData.pCMLog, filename);
		else if (direction.Compare(XML_ATTRIBUTE_DIRECTION_LOAD) == 0)
			CTSEnv::Instance().LoadFromFile(m_actionData.pCMLog, filename);

		return ERROR_SUCCESS;
	}

	INT_PTR CActionSoftwareDiscovery::Go(void)
	{
		softwareinfoset uninstallSoftware;
		CString displayName;
		CString variableName;
		CString version;
		CString versionOperator;
		pugi::xpath_node_set softwareNodes;
		pugi::xml_node softNode;
		std::wregex regex;

		OrderedSoftwareMap softwareToAdd;

		bool matchFound = false;

		bool getSystemComponents = FTW::IsTrue(GetXMLAttribute(m_actionData.pActionNode, XML_ATTRIBUTE_SYSTEMCOMPONENTS, XML_ACTION_FALSE));

		GetSoftware(uninstallSoftware, false, getSystemComponents);

		if (FTW::Is64BitOS())
			GetSoftware(uninstallSoftware, true, getSystemComponents);

		m_actionData.pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info, 
			AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
			FTW::FormatResourceString(IDS_LOGMSG_SCANNINGSOFTWARE));

		softwareNodes = m_actionData.pActionNode->select_nodes(XML_ACTION_TYPE_MATCH);

		for (size_t softwareIndex = 0; softwareIndex < softwareNodes.size(); softwareIndex++)
		{
			matchFound = false;
			
			softNode = softwareNodes[softwareIndex].node();

			displayName = GetXMLAttribute(&softNode, XML_ATTRIBUTE_DISPLAYNAME);
			variableName = GetXMLAttribute(&softNode, XML_ATTRIBUTE_VARIABLE);
			version = GetXMLAttribute(&softNode, XML_ATTRIBUTE_VERSION);
			versionOperator = GetXMLAttribute(&softNode, XML_ATTRIBUTE_VERSIONOPERATOR, XML_ACTION_OPR_EQUAL);

			if (displayName.GetLength() > 0 && variableName.GetLength() > 0)
			{
				regex.assign(displayName, std::regex_constants::icase);

				//std::for_each(uninstallSoftware.begin(), uninstallSoftware.end(), [&](const SoftwareInfo& softwareInstance)
				for (auto softwareInstance = uninstallSoftware.begin(); softwareInstance != uninstallSoftware.end(); softwareInstance++)
				{
					//TRACE3("%s -> %s (%s)\n", displayName, uninstallSoftware.first, uninstallSoftware.second);

					if (std::regex_match(softwareInstance->first.GetString(), regex))
					{
						m_actionData.pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info, 
							AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
							FTW::FormatResourceString(IDS_LOGMSG_SOFTWAREMATCH, displayName, softwareInstance->first));

						if (version.GetLength() > 0)
						{
							matchFound = FTW::VersionCheck(softwareInstance->second, version, versionOperator);

							m_actionData.pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info, 
								AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
								FTW::FormatResourceString(IDS_LOGMSG_SOFTWAREVERSIONMATCH, softwareInstance->second, versionOperator, version, FTW::BoolString(matchFound)));

						}
						else
							matchFound = true;
					}

					if (matchFound)
					{
						CTSEnv::Instance().Set(m_actionData.pCMLog, variableName, TRUE_STRING);
						break;
					}
				}
			}

			if (!matchFound && displayName.GetLength() > 0)
			{
				m_actionData.pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info, 
					AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
					FTW::FormatResourceString(IDS_LOGMSG_SOFTWARENOMATCH, displayName));

				if (variableName.GetLength() > 0)
					CTSEnv::Instance().Set(m_actionData.pCMLog, variableName, FALSE_STRING);
			}
		}

		return ERROR_SUCCESS;
	}

	void CActionSoftwareDiscovery::GetSoftware(softwareinfoset& softwareList, bool hive64, bool includeSystemComponents)
	{
		HKEY hHive = HKEY_LOCAL_MACHINE;
		CodeProject::RegKey uninstallRegKey(hHive);
		CodeProject::RegKey uninstallRegSubkey(hHive);
		CString keyName(_T("Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall"));
		CString subkeyName(_T("Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall"));
		CString displayName(_T("DisplayName"));
		CString displayVersion(_T("DisplayVersion"));
		CString systemComponent(_T("SystemComponent"));
		CString displayNameValue, displayVersionValue;

		LONG ret = uninstallRegKey.Open(keyName, KEY_READ | (hive64 ? KEY_WOW64_64KEY : KEY_WOW64_32KEY));
		CodeProject::RegValue regDisplayNameValue, regDisplayVersionValue, regSystemComponentValue;

		if (ret == ERROR_SUCCESS)
		{
			m_actionData.pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info, 
				AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
				FTW::FormatResourceString(IDS_LOGMSG_LOADINGUNINSTALL, hive64 ? VALUE_64 : VALUE_32));

			CodeProject::RegKeyForwardIterator i(uninstallRegKey), e;

			for (; i != e; ++i)
			{
				LPCTSTR name = (*i);

				subkeyName.Format(_T("%s\\%s"), keyName, name);

				LONG ret = uninstallRegSubkey.Open(subkeyName, KEY_READ | (hive64 ? KEY_WOW64_64KEY : KEY_WOW64_32KEY));

				if (ret == ERROR_SUCCESS)
				{
					regSystemComponentValue = uninstallRegSubkey[systemComponent];

					if ((includeSystemComponents && regSystemComponentValue.Type == REG_DWORD && (DWORD)regSystemComponentValue == 1) || regSystemComponentValue.Type == REG_NONE)
					{

						regDisplayNameValue = uninstallRegSubkey[displayName];

						if (regDisplayNameValue.Type == REG_SZ)
						{
							displayNameValue = regDisplayNameValue;

							regDisplayVersionValue = uninstallRegSubkey[displayVersion];

							if (regDisplayVersionValue.Type == REG_SZ)
								displayVersionValue = regDisplayVersionValue;
							else
								displayVersionValue = L"";

							m_actionData.pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info, 
								AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
								FTW::FormatResourceString(IDS_LOGMSG_FOUNDUNINSTALL, displayNameValue, displayVersionValue));

							softwareList.insert(SoftwareInfo(displayNameValue, displayVersionValue));
						}
					}
				}
			}
		}
	}
	
	INT_PTR CActionTSVarList::Go(void)
	{
		CString actionTitle = GetXMLAttribute(m_actionData.pActionNode, XML_ATTRIBUTE_TITLE, XML_ACTION_TYPE_APPTREE);
		CString pkgVarBase = GetXMLAttribute(m_actionData.pActionNode, XML_ATTRIBUTE_PACKAGEVAR);
		CString appVarBase = GetXMLAttribute(m_actionData.pActionNode, XML_ATTRIBUTE_APPVAR);

		CString id;
		SoftwareMap::const_iterator it;
		OrderedSoftwareMap softwareMap;

		CString variableName;
		int appCount = 0, pkgCount = 0;

		if (pkgVarBase == _T("") && appVarBase == _T(""))
		{
			m_actionData.pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Warning,
				AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
				FTW::FormatResourceString(IDS_LOGMSG_NOBASEVARIABLES));

			return ERROR_SUCCESS;
		}

		if (pkgVarBase == _T(""))
			pkgCount = -1;
		
		if (appVarBase == _T(""))
			appCount = -1;

		for (pugi::xml_node childNode = m_actionData.pActionNode->first_child(); childNode; childNode = childNode.next_sibling())
		{
			if (_tcsicmp(childNode.name(), XML_ACTION_TSVARLIST_SOFTWAREREF) != 0)
				continue;

			id = GetXMLAttribute(&childNode, XML_ATTRIBUTE_ID);

			if (id.GetLength() == 0)
			{
				m_actionData.pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Warning,
					AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
					FTW::FormatResourceString(IDS_LOGMSG_SOFTWAREFOUNDNOID, childNode.name()));
				continue;
			}

			m_actionData.pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info,
				AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
				FTW::FormatResourceString(IDS_LOGMSG_FOUNDSOFTWAREREF, id));

			if (!CActionHelper::EvalCondition(m_actionData.pCMLog, childNode.attribute(XML_ATTRIBUTE_CONDITION).value(), m_actionData.pScriptHost, childNode.name(), _T("")))
				continue;

			it = m_actionData.pSoftware->find(id);

			if (it != m_actionData.pSoftware->end())
			{
				CSoftware* softwareItem = it->second.get();

				m_actionData.pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info,
					AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
					FTW::FormatResourceString(IDS_LOGMSG_FOUNDSOFTWAREID, softwareItem->GetLabel()));

				softwareMap.insert(OrderedSoftwareMap::value_type(it->second->GetIndex(), softwareItem));
			}
		}

		std::for_each(softwareMap.begin(), softwareMap.end(), [&](std::pair<const int, CSoftware*>& pTI2)
		{
			if (appCount > -1 && pTI2.second->Type() == CSoftware::SoftwareType::APPLICATION)
				variableName.Format(_T("%s%02d"), appVarBase, ++appCount);

			else if (pkgCount > -1 && pTI2.second->Type() == CSoftware::SoftwareType::PACKAGE)
				variableName.Format(_T("%s%03d"), pkgVarBase, ++pkgCount);

			CTSEnv::Instance().Set(m_actionData.pCMLog, variableName, pTI2.second->GetVariableValue());
		});

		return ERROR_SUCCESS;
	}

	INT_PTR CActionFileRead::Go(void)
	{
		CString filename = GetXMLAttribute(m_actionData.pActionNode, XML_ATTRIBUTE_FILENAME);
		bool deleteLine = FTW::IsTrue(GetXMLAttribute(m_actionData.pActionNode, XML_ATTRIBUTE_DELETELINE, XML_ACTION_TRUE));
		CString variableName = GetXMLAttribute(m_actionData.pActionNode, XML_ATTRIBUTE_VARIABLE, XML_ATTRIBUTE_VARIABLE_DEF);

		try
		{
			// Open the file for reading
			CStdioFile dataFileIn(filename, CFile::modeRead | CFile::typeText | CFile::shareDenyNone);
			CStdioFile dataFileOut;

			if (deleteLine)
			{
				// Open the file a second time for writing so the line can be deleted
				dataFileOut.Open(filename, CFile::modeWrite | CFile::typeText | CFile::shareDenyWrite);

				// Make sure we're at the beginning of the file because we will be overwriting it
				dataFileOut.SeekToBegin();
			}

			m_actionData.pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info,
				AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
				FTW::FormatResourceString(IDS_LOGMSG_LOADFILE, filename));

			CString dataLine = _T("");
			CString variableValue = _T("");

			// Loop through the file reading in all of the lines
			while (dataFileIn.ReadString(dataLine))
			{
				// Set the variableValue only if we haven't done so before and the data read in is not blank
				if (variableValue.GetLength() == 0 && dataLine.Trim().GetLength() > 0)
					variableValue = dataLine.Trim();

				// If not deleting the line, simply exit, no need to re-write the file
				if (!deleteLine || dataFileOut.m_hFile == CFile::hFileNull)
					break;

				// Write out all lines except the first non-blank line
				if (variableValue != dataLine.Trim() || dataLine.Trim().GetLength() == 0)
				{
					dataLine += "\n";
					dataFileOut.WriteString(dataLine);
				}
			}

			if(variableValue.GetLength() > 0)
				CTSEnv::Instance().Set(m_actionData.pCMLog, variableName, variableValue);

			// Close the input stream/file
			dataFileIn.Close();

			// Truncate and close the output stream/file
			if (deleteLine && dataFileOut.m_hFile != CFile::hFileNull)
			{
				dataFileOut.SetLength(dataFileOut.GetPosition());
				dataFileOut.Close();
			}
		}

		catch(CFileException* pe)
		{
			PTSTR pErrorMsg = new TCHAR[MAX_STRING_LENGTH];

			pe->GetErrorMessage(pErrorMsg, MAX_STRING_LENGTH);

			m_actionData.pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Error,
				AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
				FTW::FormatResourceString(IDS_LOGERROR_LOADFILE_FAILED, filename, pe->m_lOsError, pErrorMsg));

			delete[] pErrorMsg;

			pe->Delete();
		}

		return ERROR_SUCCESS;
	}
	
	INT_PTR CActionSwitch::Go(void)
	{
		CString onValue = GetXMLAttribute(m_actionData.pActionNode, XML_ATTRIBUTE_ONVALUE);
		bool dontEvalSwitchValue = FTW::IsTrue(GetXMLAttribute(m_actionData.pActionNode, XML_ATTRIBUTE_DONTEVAL, XML_ACTION_TRUE));

		CString regex, switchOn;
		bool dontEval;
		bool matchFound = false;

		std::wregex regularExp;

		if (dontEvalSwitchValue == false)
		{
			_variant_t r;

			if (SUCCEEDED(m_actionData.pScriptHost->Eval(onValue, &r)) && r.vt > 0 && ((_bstr_t)r).length() > 0)
			{
				switchOn = ((_bstr_t)r).GetBSTR();

				m_actionData.pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info, 
					AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
					FTW::FormatResourceString(IDS_LOGMSG_EXPRESSIONEVALUATION, onValue, switchOn));
			}
			else
			{
				//m_actionData.pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Error, AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
				//	IDS_LOGERROR_EXPRESSIONEVALUATION, onValue, evalResult);

				switchOn = onValue;
			}
		}
		else
		{
			switchOn = onValue;
		}

		m_actionData.pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info, 
			AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
			FTW::FormatResourceString(IDS_LOGMSG_SWICTHONVALUE, switchOn));

		for (pugi::xml_node childNode = m_actionData.pActionNode->first_child(); !matchFound && childNode; childNode = childNode.next_sibling())
		{
			if (_tcsicmp(childNode.name(), XML_ACTION_SWITCH_CASE) != 0 && _tcsicmp(childNode.name(), XML_ACTION_SWITCH_DEFAULT) != 0)
				continue;

			if (_tcsicmp(childNode.name(), XML_ACTION_SWITCH_CASE) == 0)
			{
				regex = GetXMLAttribute(&childNode, XML_ATTRIBUTE_REGEX, DEFAULT_SWITCHREGEX);
				bool caseInsensitive = FTW::IsTrue(GetXMLAttribute(&childNode, XML_ACTION_USERINPUT_CASEINSENSITIVE, XML_ACTION_FALSE));

				if (CActionHelper::EvalCondition(m_actionData.pCMLog, childNode.attribute(XML_ATTRIBUTE_CONDITION).value(), m_actionData.pScriptHost, childNode.name(), regex))
				{
					m_actionData.pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info, 
						AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
						FTW::FormatResourceString(IDS_LOGMS_FOUNDCASE_REGEX, regex));

					try
					{
						try
						{
							if (caseInsensitive)
								regularExp.assign(regex, std::regex_constants::icase);
							else regularExp.assign(regex);

						}
						catch (std::regex_error e)
						{
							regularExp.assign(_T(".*"), std::regex_constants::icase);
						}

//						if (std::regex_match(switchOn.GetString(), regularExp, std::regex_constants::match_continuous))
						if (std::regex_search(switchOn.GetString(), regularExp))
						{
							matchFound = true;

							m_actionData.pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info, 
								AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
								FTW::FormatResourceString(IDS_LOGMS_REGEX_MATCHED));
						}
						else
						{
							m_actionData.pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info, 
								AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
								FTW::FormatResourceString(IDS_LOGMS_REGEX_NOTMATCHED));
						}
					}
					catch (std::regex_error& e)
					{
						m_actionData.pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Error, 
							AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
							FTW::FormatResourceString(IDS_LOGERROR_BADREGEX, regex, CString(e.what())));
					}
				}
			}
			else
			{
				m_actionData.pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info, 
					AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
					FTW::FormatResourceString(IDS_LOGMS_USINGDEFAULTCASE));
				
				matchFound = true;
			}

			if (matchFound)
			{
				CString variableName, variableValue;
				_variant_t r;

				for (pugi::xml_node varNode = childNode.first_child(); varNode; varNode = varNode.next_sibling())
				{
					if (_tcsicmp(varNode.name(), XML_ACTION_SWITCH_VARIABLE) != 0)
						continue;

					variableName = GetXMLAttribute(&varNode, XML_ATTRIBUTE_NAME, XML_ATTRIBUTE_VARIABLE_DEF);
					
					if (CActionHelper::EvalCondition(m_actionData.pCMLog, varNode.attribute(XML_ATTRIBUTE_CONDITION).value(), m_actionData.pScriptHost, varNode.name(), variableName))
					{
						variableValue = CTSEnv::Instance().VariableSubstitute(varNode.child_value());
						dontEval = FTW::IsTrue(GetXMLAttribute(&varNode, XML_ATTRIBUTE_DONTEVAL, XML_ACTION_FALSE));

						if (!dontEval && SUCCEEDED(m_actionData.pScriptHost->Eval(variableValue, &r)) && r.vt > 0 && ((_bstr_t)r).length() > 0)
						{
							variableValue = ((_bstr_t)r).GetBSTR();
						}

						CTSEnv::Instance().Set(m_actionData.pCMLog, variableName, variableValue);
					}
				}

			}
		}

		return ERROR_SUCCESS;
	}

//	INT_PTR CRestCallAction::Go(FTWLDAP::ILdapPtr& pLdap)
//	{
//		CString url = GetXMLAttribute(m_actionData.pActionNode, XML_ATTRIBUTE_URL);
//
//		if (url.GetLength() == 0 || (url.Left(7).MakeLower() != _T("http://") && url.Left(8).MakeLower() != _T("https://")))
//		{
//			m_actionData.pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Warning, AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
//				IDS_LOGMSG_NOVALIDURL);
//		}
//		else
//		{
//			CString json = _T("{");
//			CString attr, value;
//			pugi::xml_node attrNode;
//			bool dontEval;
//			size_t attributeCount = 0;
//
//			pugi::xpath_node_set jsonAttributes = m_actionData.pActionNode->child(XML_ACTION_REST_JSON).select_nodes(XML_ACTION_REST_JSONATTRIBUTE);
//
//			for (size_t nodeIndex = 0; nodeIndex < jsonAttributes.size(); nodeIndex++)
//			{
//				attrNode = jsonAttributes[nodeIndex].node();
//
//				attr = GetXMLAttribute(&attrNode, XML_ATTRIBUTE_NAME);
//				value = CTSEnv::Instance().VariableSubstitute(attrNode.child_value());
//				dontEval = FTW::IsTrue(GetXMLAttribute(&attrNode, XML_ATTRIBUTE_DONTEVAL, XML_ACTION_FALSE));
//
//				_variant_t r;
//
//				if (!dontEval && SUCCEEDED(m_actionData.pScriptHost->Eval(value, &r)) && r.vt > 0 && ((_bstr_t)r).length() > 0)
//				{
//					value = ((_bstr_t)r).GetBSTR();
//				}
//
//				if (attr.GetLength() > 0)
//				{
//					if(attributeCount)
//						json += _T(",");
//
//					json += _T("\"");
//					json += attr;
//					json += _T("\":\"");
//					
//					if (value.GetLength() == 0)
//						value = " ";
//
//					json += value;
//					json += _T("\"");
//
//					attributeCount++;
//				}
//			}
//
//			json += _T("}");
//
//			m_actionData.pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info, AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
//				IDS_LOGMSG_RESTCALL, url, json);
//
//			try
//			{
//				if (CHTTPOperation::PostJsonData((PCTSTR)url, (PCTSTR)json))
//				{
//					m_actionData.pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info, AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
//						IDS_LOGMSG_RESTSUCCESS);
//				}
//			}
//			catch (HTTPCURLException& e)
//			{
//				m_actionData.pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Error, AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
//					IDS_LOGERROR_RESTFAIL, CString(e.what()));
//			}
//		}
//
////		CHTTPOperation::PostJsonData("https://kndkpwv331.execute-api.us-east-2.amazonaws.com/prod/dataDemoFunction",
////		"{\"device\":\"ui++\",\"lat\":\"31.06\",\"lon\":\"22.22\"}");
//
//		return ERROR_SUCCESS;
//	}

	INT_PTR CActionTPM::Go(void)
	{
		m_actionData.pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info, 
			AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
			FTW::FormatResourceString(IDS_LOGMSG_INTIATETPM));

		int request = GetXMLIntAttribute(m_actionData.pActionNode, XML_ATTRIBUTE_TPM_REQUEST, -1);

		if (request == -1)
			m_actionData.pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info, 
				AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
				FTW::FormatResourceString(IDS_LOGMSG_TPM_NOREQUEST));
		else
		{
			m_actionData.pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info, 
				AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
				FTW::FormatResourceString(IDS_LOGMSG_TPM_REQUEST, request));


		}

		return ERROR_SUCCESS;
	}

	INT_PTR CActionSaveItems::Go(void)
	{
		CString path = GetXMLAttribute(m_actionData.pActionNode, XML_ATTRIBUTE_PATH);
		CString itemsToSave = GetXMLAttribute(m_actionData.pActionNode, XML_ATTRIBUTE_ITEMS);
		CString saveMethod = GetXMLAttribute(m_actionData.pActionNode, XML_ATTRIBUTE_METHOD, XML_ATTRIBUTE_METHOD_UNC);

		if(_tcsicmp(saveMethod, XML_ATTRIBUTE_METHOD_UNC) == 0)
		{
			if (!std::filesystem::exists((PCTSTR)path))
			{
				m_actionData.pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Error,
					AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
					FTW::FormatResourceString(IDS_LOGERROR_PATHNOTFOUND, path));

				return ERROR_SUCCESS;
			}
		}

		int counter = 0;
		CString item = itemsToSave.Tokenize(L",;", counter);

		while (counter != -1)
		{
			item = item.Trim();

			if (_tcsicmp(item, XML_ATTRIBUTE_ITEMS_UILOG) == 0)
			{
				m_actionData.pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info,
					AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
					FTW::FormatResourceString(IDS_LOGMSG_SAVINGITEMS, _T("UI++.log")));
				
				//UICopyFiles(FTW::CCMLog::Instance().GetLogFileName(), FTW::CCMLog::Instance().GetLocation(), path);
				CActionHelper::UICopyFiles(m_actionData.pCMLog, m_actionData.pCMLog->Filename().c_str(), m_actionData.pCMLog->Path().c_str(), path);
			}
			else if (_tcsicmp(item, XML_ATTRIBUTE_ITEMS_SMSTSLOG) == 0)
			{
				m_actionData.pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info,
					AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
					FTW::FormatResourceString(IDS_LOGMSG_SAVINGITEMS, _T("smsts.log")));

				//UICopyFiles(_T("\\smsts*.log"), FTW::CCMLog::Instance().GetLocation(), path);
				CActionHelper::UICopyFiles(m_actionData.pCMLog, _T("\\smsts*.log"), m_actionData.pCMLog->Path().c_str(), path);
			}
			else if (_tcsicmp(item.Left((int)_tcsclen(XML_ATTRIBUTE_ITEMS_TSVARS)), XML_ATTRIBUTE_ITEMS_TSVARS) == 0)
			{
				m_actionData.pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info,
					AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
					FTW::FormatResourceString(IDS_LOGMSG_SAVINGITEMS, _T("task sequence variables")));
				
				CString file = _T("UI++ Variable Dump.txt");

				int seperator = item.Find(_T(':'));

				if (seperator != -1)
				{
					CString tmp = item.Right(item.GetLength() - (seperator + 1));

					if(!tmp.IsEmpty())
						file = tmp;
				}

				CTSEnv::Instance().DumpToFile(m_actionData.pCMLog, FTW::JoinPath(path.GetBuffer(), file.GetBuffer()).c_str());
			}
			else
			{
				CString msg = _T("files matching ");
				msg += item;

				m_actionData.pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info,
					AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
					FTW::FormatResourceString(IDS_LOGMSG_SAVINGITEMS, msg));
				
				int lastSlash = item.ReverseFind(_T('\\'));

				if (lastSlash != -1)
				{
					CString sPath = item.Left(lastSlash);

					PTSTR filePath = nullptr;
					DWORD bufferLength = 0;
					bufferLength = ExpandEnvironmentStrings((PCTSTR)sPath, filePath, bufferLength);

					if (bufferLength > 0)
					{
						filePath = new TCHAR[bufferLength + 2];

						bufferLength = ExpandEnvironmentStrings((PCTSTR)sPath, filePath, bufferLength);

						if (bufferLength > 0)
						{
							sPath = filePath;
						}

						delete[] filePath;
					}

					CString filename = item.Right(item.GetLength() - (lastSlash + 1));

					CActionHelper::UICopyFiles(m_actionData.pCMLog, filename, sPath, path);
				}
				else
				{
					CActionHelper::UICopyFiles(m_actionData.pCMLog, item, m_actionData.pCMLog->Path().c_str(), path);
				}
			}

			item = itemsToSave.Tokenize(L",;", counter);
		}

		return ERROR_SUCCESS;
	}

	INT_PTR CActionRandomString::Go(void)
	{
		CString allowedChars = GetXMLAttribute(m_actionData.pActionNode, XML_ATTRIBUTE_ALLOWEDCHARS, XML_ATTRIBUTE_ALLOWEDCHARS_DEFAULT);
		size_t desiredLength = _tstoi(GetXMLAttribute(m_actionData.pActionNode, XML_ATTRIBUTE_LENGTH, XML_ATTRIBUTE_LENGTH_DEFAULT));
		CString variableName = GetXMLAttribute(m_actionData.pActionNode, XML_ATTRIBUTE_VARIABLE, XML_ATTRIBUTE_VARIABLE_DEF);

		if (desiredLength < 1 || desiredLength > 36)
			desiredLength = _tstoi(XML_ATTRIBUTE_LENGTH_DEFAULT);

		m_actionData.pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info,
			AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
			FTW::FormatResourceString(IDS_LOGMSG_CREATERANDOMSTRING, desiredLength, allowedChars));
		
		std::mt19937_64 gen{ std::random_device()() };

		size_t allowCharsLength = allowedChars.GetLength() - 1;

		std::uniform_int_distribution<size_t> dist{ 0, allowCharsLength };

		std::wstring ret;

		std::generate_n(std::back_inserter(ret), desiredLength, [&] { return allowedChars[dist(gen)]; });

		CTSEnv::Instance().Set(m_actionData.pCMLog, _T("Random"), ret.c_str());

		return ERROR_SUCCESS;
	}

	static ActionMaker<CActionUserInfo> maker(XML_ACTION_TYPE_USERINFO);
	static ActionMaker<CActionErrorInfo> maker1(XML_ACTION_TYPE_ERRORINFO);
	static ActionMaker<CActionUserInfoFullScreen> maker2(XML_ACTION_TYPE_USERINFOFULLSCREEN);
	static ActionMaker<CActionRegistryWrite> maker3(XML_ACTION_TYPE_REGISTRY_WRITE);
	static ActionMaker<CActionRegistry> maker4(XML_ACTION_TYPE_REGISTRY);
	static ActionMaker<CActionUserInput> maker5(XML_ACTION_TYPE_USERINPUT);
	static ActionMaker<CActionAppTree> maker6(XML_ACTION_TYPE_APPTREE);
	static ActionMaker<CActionWMIRead> maker8(XML_ACTION_TYPE_WMI);
	static ActionMaker<CActionWMIWrite> maker9(XML_ACTION_TYPE_WMI_WRITE);
	static ActionMaker<CActionTSVar> maker10(XML_ACTION_TYPE_TSVAR);
	static ActionMaker<CActionDefaultValues> maker11(XML_ACTION_TYPE_DEFAULTVALS);
	static ActionMaker<CActionUserAuth> maker12(XML_ACTION_TYPE_USERAUTH);
	static ActionMaker<CActionPreflight> maker13(XML_ACTION_TYPE_PREFLIGHT);
	static ActionMaker<CActionVars> maker14(XML_ACTION_TYPE_VARS);
	static ActionMaker<CActionSoftwareDiscovery> maker15(XML_ACTION_TYPE_SOFTWAREDISC);
	static ActionMaker<CActionTSVarList> maker16(XML_ACTION_TYPE_TSVARLIST);
	static ActionMaker<CActionFileRead> maker17(XML_ACTION_TYPE_FILEREAD);
	static ActionMaker<CActionExternalCall> maker18(XML_ACTION_TYPE_EXTERNALCALL);
	static ActionMaker<CActionSwitch> maker19(XML_ACTION_TYPE_SWITCH);
	static ActionMaker<CActionSaveItems> maker20(XML_ACTION_TYPE_SAVEITEMS);
	static ActionMaker<CActionRandomString> maker21(XML_ACTION_TYPE_RANDOMSTRING);
}

