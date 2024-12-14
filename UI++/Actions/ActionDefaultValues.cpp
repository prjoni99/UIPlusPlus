#include "StdAfx.h"
#include "resource.h"
#include "TSVar.h"
#include "Actions.h"
#include "Dialogs\DlgProgress.h"
#include "FTW\WMIAccess.h"
#include "FTW\ComInit.h"
#include "FTW\TSProgress.h"
#include "CodeProject\regkey.h"
#include "FTW\Utils.h"

#include "Security.h"
#include "Windowsdefender.h"
#include "ActionDefaultValues.h"

#include <netfw.h>
#include <Wuapi.h>
#include <lm.h>
#include <wlanapi.h>
#include <windot11.h>

typedef DWORD(WINAPI* WlanOpenHandleFunc) (DWORD, PVOID, PDWORD, PHANDLE);
typedef DWORD(WINAPI* WlanEnumInterfacesFunc) (HANDLE, PVOID, PWLAN_INTERFACE_INFO_LIST*);
typedef DWORD(WINAPI* WlanQueryInterfaceFunc) (HANDLE, CONST GUID *, WLAN_INTF_OPCODE, PVOID, PDWORD, PVOID *, PWLAN_OPCODE_VALUE_TYPE);
typedef DWORD(WINAPI* WlanCloseHandleFunc) (HANDLE, PVOID);

namespace UIpp 
{

	void SendProgress(HWND hWnd, ProgressTextMap* pProgressStrings, int count, LPCTSTR stage)
	{
		if (hWnd != NULL && pProgressStrings != nullptr && stage != nullptr) //pThreadData->hProgressDialog
		{
			auto it = pProgressStrings->find(stage);
			if (it != pProgressStrings->end())
			{
				::PostMessage(hWnd, WM_UPDATEPROGRESSBAR, (WPARAM)count, (LPARAM)it->second);
				Sleep(1000);
			}
		}
	}

	CString GetEnvVar(PCTSTR pEnvVarName)
	{
		LPTSTR buffer = 0;
		DWORD bufferSize = GetEnvironmentVariable(pEnvVarName, buffer, 0) + 1;
		CString returnValue = _T("");

		if (bufferSize > 0)
		{
			buffer = new TCHAR[bufferSize];

			if (GetEnvironmentVariable(pEnvVarName, buffer, bufferSize))
			{
				returnValue = buffer;
			}
		}

		delete[] buffer;

		return returnValue;
	}

	UINT GetDefaulValues(LPVOID pData)
	{
		DefaultValueThreadData* pThreadData = static_cast<DefaultValueThreadData*>(pData);
		bool comOK = false;
		FTW::ComInit com(false);
		//CString* actionTypes = (CString*)pData;

		if (pThreadData->comInit == true)
		{
			comOK = com.Init() == S_OK;
		}
		else
		{
			comOK = true;
		}
		if (comOK)
		{
			if (pThreadData->pValueTypes == nullptr)
				return ERROR_INVALID_PARAMETER;

			int valueCount = 0;
			CString progressText;

			TRACE0("Starting default values thread\n");

			setlocale(LC_ALL, "");

			bool getAll = (pThreadData->pValueTypes->Find(XML_ATTRIBUTE_DEFAULTVALUE_ALL) > -1);

			CDefaultValues defVals(pThreadData, getAll);

			//std::for_each(pThreadData->pProgressStrings->begin(), pThreadData->pProgressStrings->end(), defVals.Execute());

			for (auto it = pThreadData->pProgressStrings->begin(); it != pThreadData->pProgressStrings->end(); ++it)
			{
				defVals.Execute(*it);
			}

			if (pThreadData->hProgressDialog)
				::PostMessage(pThreadData->hProgressDialog, WM_UPDATEPROGRESSBAR, (WPARAM)valueCount, (LPARAM)0);
		}
		else if (pThreadData->comInit == true)
		{
			pThreadData->pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Error,
				AfxGetThread()->m_nThreadID, __TFILE__,	__LINE__,
				FTW::FormatResourceString(IDS_LOGERROR_COM));
		}

		if (pThreadData->hProgressDialog)
			::PostMessage(pThreadData->hProgressDialog, WM_CLOSEPROGRESSBAR, (WPARAM)0, (LPARAM)0);

		return ERROR_SUCCESS;
	}

	CDefaultValues::CDefaultValues(DefaultValueThreadData* pThreadData, bool all)
		: m_isPE(false), m_allValues(all), m_count(0), m_pThreadData(pThreadData)
	{
		ASSERT(m_pThreadData);

		CodeProject::RegKey hklm(HKEY_LOCAL_MACHINE);

		if (hklm.Open(_T("System\\CurrentControlSet\\Control\\MiniNT"), KEY_READ) == ERROR_SUCCESS)
		{
			m_isPE = true;

			hklm.Close();
		}

		m_wmi.Connect(_T("root\\cimv2"));

		functionDispatcher.insert(DefaultValuesFunctions::value_type(XML_ATTRIBUTE_DEFAULTVALUE_USER, &CDefaultValues::GetUserInfo));
		functionDispatcher.insert(DefaultValuesFunctions::value_type(XML_ATTRIBUTE_DEFAULTVALUE_OS, &CDefaultValues::GetOSInfo));
		functionDispatcher.insert(DefaultValuesFunctions::value_type(XML_ATTRIBUTE_DEFAULTVALUE_ASSET, &CDefaultValues::GetAssetInfo));
		functionDispatcher.insert(DefaultValuesFunctions::value_type(XML_ATTRIBUTE_DEFAULTVALUE_VM, &CDefaultValues::GetVirtualizationInfo));
		functionDispatcher.insert(DefaultValuesFunctions::value_type(XML_ATTRIBUTE_DEFAULTVALUE_DOM, &CDefaultValues::GetDomainInfo));
		functionDispatcher.insert(DefaultValuesFunctions::value_type(XML_ATTRIBUTE_DEFAULTVALUE_NET, &CDefaultValues::GetNetworkInfo));
		functionDispatcher.insert(DefaultValuesFunctions::value_type(XML_ATTRIBUTE_DEFAULTVALUE_TPM, &CDefaultValues::GetTPMInfo));
		functionDispatcher.insert(DefaultValuesFunctions::value_type(XML_ATTRIBUTE_DEFAULTVALUE_SECURITY, &CDefaultValues::GetSecurityInfo));
		functionDispatcher.insert(DefaultValuesFunctions::value_type(XML_ATTRIBUTE_DEFAULTVALUE_MGMT, &CDefaultValues::GetMgmtInfo));
	}

	bool CDefaultValues::GetServiceInfo(PCTSTR pServiceName)
	{
		bool serviceExists = false;

		if (pServiceName != nullptr && _tcslen(pServiceName) > 0)
		{
			CString serviceDisplayName, temp, variable, key;

			key.Format(_T("Name=\"%s\""), pServiceName);

			try
			{
				if (CActionHelper::GetWMIPropertyFromFirstInstance(m_pThreadData->pCMLog, m_wmi, _T("Win32_Service"), key, _T("DisplayName"), serviceDisplayName))
				{
					serviceExists = true;
					
					serviceDisplayName.Replace(_T(" "), _T(""));

					variable = VAR_SERVICESTATE;

					if (CActionHelper::GetWMIPropertyFromFirstInstance(m_pThreadData->pCMLog, m_wmi, _T("Win32_Service"), key, _T("State"), temp))
						CTSEnv::Instance().Set(m_pThreadData->pCMLog, variable + serviceDisplayName, temp);

					variable = VAR_SERVICESTARTMODE;

					if (CActionHelper::GetWMIPropertyFromFirstInstance(m_pThreadData->pCMLog, m_wmi, _T("Win32_Service"), key, _T("StartMode"), temp))
						CTSEnv::Instance().Set(m_pThreadData->pCMLog, variable + serviceDisplayName, temp);
				}
			}
			catch (FTW::FTWException& i)
			{
				m_pThreadData->pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Error, 
					AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
					FTW::FormatResourceString(IDS_LOGERROR_DEFAULTVALUERETRIEVAL, i.Message()));
			}
		}

		return serviceExists;
	}

	void CDefaultValues::GetBatteryInfo(void)
	{
		bool onBattery = false;
		CString temp;
		CString systemDrive = GetEnvVar(_T("SystemDrive"));

		if(systemDrive.GetLength() > 0 && systemDrive != _T("X:") && FTW::FileExists(_T("X:\\Windows\\Inf\\Battery.inf")))
		{
				m_pThreadData->pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info, 
					AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
					FTW::FormatResourceString(IDS_LOGMSG_LOADINGBATTERYDRIVER));

				TCHAR commandLine[256];
				_tcscpy_s(commandLine, 256, _T("X:\\Windows\\System32\\drvload.exe X:\\Windows\\Inf\\Battery.inf"));

				LPSTARTUPINFO lpStartupInfo;
				LPPROCESS_INFORMATION lpProcessInfo;

				ZeroMemory(&lpStartupInfo, sizeof(LPSTARTUPINFO));
				ZeroMemory(&lpProcessInfo, sizeof(LPPROCESS_INFORMATION));

				/* Create the process */
				CreateProcess(NULL, commandLine,
					NULL, NULL,
					NULL, NULL, NULL, NULL,
					lpStartupInfo,
					lpProcessInfo
				);

		}

		if (CActionHelper::GetWMIPropertyFromFirstInstance(m_pThreadData->pCMLog, m_wmi, _T("Win32_Battery"), NULL, _T("Name"), temp))
		{
			if (CActionHelper::GetWMIPropertyFromFirstInstance(m_pThreadData->pCMLog, m_wmi, _T("Win32_Battery"), NULL, _T("BatteryStatus"), temp) && temp != _T("2"))
				onBattery = true;
		}

		CTSEnv::Instance().Set(m_pThreadData->pCMLog, VAR_ONBATTERY, FTW::BoolString(onBattery));
	}

	void CDefaultValues::GetHardwareInfo(void)
	{
		CString temp;

		if (m_chassisTypeSettoVM == false && CActionHelper::GetWMIPropertyFromFirstInstance(m_pThreadData->pCMLog, m_wmi, _T("Win32_SystemEnclosure"), NULL, _T("ChassisTypes"), temp))
		{
			if (temp == _T("8") || temp == _T("9") || temp == _T("10") || temp == _T("11") || temp == _T("12")
				|| temp == _T("14") || temp == _T("18") || temp == _T("21")
				|| temp == _T("32"))
				temp = VALUE_CHASSIS_LAPTOP;
			else if (temp == _T("3") || temp == _T("4") || temp == _T("5") || temp == _T("6") || temp == _T("7") 
				|| temp == _T("15") || temp == _T("16") || temp == _T("35") || temp == _T("36"))
				temp = VALUE_CHASSIS_DESKTOP;
			else if (temp == _T("30") || temp == _T("31"))
				temp = VALUE_CHASSIS_TABLET;
			else if (temp == _T("23"))
				temp = VALUE_CHASSIS_SERVER;
			else
				temp = VALUE_UNKNOWN;

			CTSEnv::Instance().Set(m_pThreadData->pCMLog, VAR_CHASSISTYPE, temp);
		}

		if (CActionHelper::GetWMIPropertyFromFirstInstance(m_pThreadData->pCMLog, m_wmi, _T("Win32_SystemEnclosure"), NULL, _T("SMBIOSAssetTag"), temp))
			CTSEnv::Instance().Set(m_pThreadData->pCMLog, VAR_ASSETTAG, temp);

		if (CActionHelper::GetWMIPropertyFromFirstInstance(m_pThreadData->pCMLog, m_wmi, _T("Win32_BIOS"), NULL, _T("SerialNumber"), temp))
			CTSEnv::Instance().Set(m_pThreadData->pCMLog, VAR_SERIALNUMBER, temp);

		if (CActionHelper::GetWMIPropertyFromFirstInstance(m_pThreadData->pCMLog, m_wmi, _T("Win32_ComputerSystem"), NULL, _T("Manufacturer"), temp))
			CTSEnv::Instance().Set(m_pThreadData->pCMLog, VAR_MANUFACTURER, temp);

		if (CActionHelper::GetWMIPropertyFromFirstInstance(m_pThreadData->pCMLog, m_wmi, _T("Win32_ComputerSystem"), NULL, _T("Model"), temp))
			CTSEnv::Instance().Set(m_pThreadData->pCMLog, VAR_MODEL, temp);

		if (CActionHelper::GetWMIPropertyFromFirstInstance(m_pThreadData->pCMLog, m_wmi, _T("Win32_ComputerSystemProduct"), NULL, _T("Version"), temp))
			CTSEnv::Instance().Set(m_pThreadData->pCMLog, VAR_LENOVOMODEL, temp);

		if (CActionHelper::GetWMIPropertyFromFirstInstance(m_pThreadData->pCMLog, m_wmi, _T("Win32_BaseBoard"), NULL, _T("Product"), temp))
			CTSEnv::Instance().Set(m_pThreadData->pCMLog, VAR_PRODUCT, temp);

		if (CActionHelper::GetWMIPropertyFromFirstInstance(m_pThreadData->pCMLog, m_wmi, _T("Win32_ComputerSystem"), NULL, _T("TotalPhysicalMemory"), temp))
		{
			unsigned long long val = _tstoi64(temp) / 1024 / 1024;

			CTSEnv::Instance().Set(m_pThreadData->pCMLog, VAR_MEMORY, (unsigned long)val);
		}

		if (CActionHelper::GetWMIPropertyFromFirstInstance(m_pThreadData->pCMLog, m_wmi, _T("Win32_ComputerSystemProduct"), NULL, _T("UUID"), temp))
			CTSEnv::Instance().Set(m_pThreadData->pCMLog, VAR_UUID, temp);

		if (CActionHelper::GetWMIPropertyFromFirstInstance(m_pThreadData->pCMLog, m_wmi, _T("Win32_Processor"), NULL, _T("DataWidth"), temp))
		{
			if (temp == _T("64"))
				CTSEnv::Instance().Set(m_pThreadData->pCMLog, VAR_PROCARCH, VALUE_ARCHITECTUREX64);
			else if (temp == _T("32"))
				CTSEnv::Instance().Set(m_pThreadData->pCMLog, VAR_PROCARCH, VALUE_ARCHITECTUREX86);
			else
				CTSEnv::Instance().Set(m_pThreadData->pCMLog, VAR_PROCARCH, VALUE_ARCHITECTUREOTHER);
		}
	}

	void CDefaultValues::GetDiskInfo(void)
	{
		ULARGE_INTEGER bytesAvailabletoCaller, totalBytes, freeBytes;
		double bytesperGB = 1073741824;
	
		CString sysDrive = GetEnvVar(_T("SystemDrive"));
		sysDrive += _T("\\");

		if (GetDiskFreeSpaceEx(sysDrive, &bytesAvailabletoCaller, &totalBytes, &freeBytes))
		{
			CString tmp;
			
			tmp.Format(_T("%.2f"), totalBytes.QuadPart / bytesperGB);
			CTSEnv::Instance().Set(m_pThreadData->pCMLog, VAR_SYSTEMDISKTOTALSIZE, tmp);

			tmp.Format(_T("%.2f"), freeBytes.QuadPart / bytesperGB);
			CTSEnv::Instance().Set(m_pThreadData->pCMLog, VAR_SYSTEMDISKFREESPACE, tmp);
		}
	}

	void CDefaultValues::GetProcessorInfo(void)
	{
		CString temp;
		SYSTEM_INFO si;
		ZeroMemory(&si, sizeof(SYSTEM_INFO));

		GetNativeSystemInfo(&si);

		switch (si.wProcessorArchitecture)
		{
		case PROCESSOR_ARCHITECTURE_AMD64:
			CTSEnv::Instance().Set(m_pThreadData->pCMLog, VAR_OSARCH, VALUE_ARCHITECTUREX64);
			break;
		case PROCESSOR_ARCHITECTURE_INTEL:
			CTSEnv::Instance().Set(m_pThreadData->pCMLog, VAR_OSARCH, VALUE_ARCHITECTUREX86);
			break;
		default:
			CTSEnv::Instance().Set(m_pThreadData->pCMLog, VAR_OSARCH, VALUE_ARCHITECTUREOTHER);
			break;
		}

		int cpuid_results[4];
		char cpuString[0x20];
		unsigned logical;

		try
		{
			m_pThreadData->pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info, 
				AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
				FTW::FormatResourceString(IDS_LOGMSG_GETTINGCPUINFO));

			ZeroMemory(cpuString, sizeof(cpuString));
			__cpuid(cpuid_results, 0);

			*((int*)cpuString) = cpuid_results[1];
			*((int*)(cpuString + 4)) = cpuid_results[3];
			*((int*)(cpuString + 8)) = cpuid_results[2];

			temp = cpuString;
			CTSEnv::Instance().Set(m_pThreadData->pCMLog, VAR_PROCVENDOR, temp);

			__cpuid(cpuid_results, 1);

			logical = (cpuid_results[1] >> 16) & 0xff;
			CTSEnv::Instance().Set(m_pThreadData->pCMLog, VAR_PROCLOGICALCOUNT, logical);

			//			__cpuid(cpuid_results, 0x80000001);

			//			if ((cpuid_results[3] & 29) == 29)
			//				CTSEnv::Instance().Set(m_pThreadData->pCMLog, VAR_PROCARCH, VALUE_ARCHITECTUREX64);
			//			else
			//				CTSEnv::Instance().Set(m_pThreadData->pCMLog, VAR_PROCARCH, VALUE_ARCHITECTUREX86);

			__cpuid(cpuid_results, 1);

			if ((cpuid_results[3] & 6) == 6)
				CTSEnv::Instance().Set(m_pThreadData->pCMLog, VAR_PROCPAE, TRUE_STRING);
			else
				CTSEnv::Instance().Set(m_pThreadData->pCMLog, VAR_PROCPAE, FALSE_STRING);

			if ((cpuid_results[3] & 20) == 20)
				CTSEnv::Instance().Set(m_pThreadData->pCMLog, VAR_PROCNX, TRUE_STRING);
			else
				CTSEnv::Instance().Set(m_pThreadData->pCMLog, VAR_PROCNX, FALSE_STRING);

			if ((cpuid_results[3] & 26) == 26)
				CTSEnv::Instance().Set(m_pThreadData->pCMLog, VAR_PROCSSE2, TRUE_STRING);
			else
				CTSEnv::Instance().Set(m_pThreadData->pCMLog, VAR_PROCSSE2, FALSE_STRING);
		}
		catch (...)
		{

		}
	}

	void CDefaultValues::GetSystemInfo(void)
	{
		BYTE pBuffer[256] = { '\0' };
		CString temp, query;

		if (GetFirmwareEnvironmentVariable(_T(""), _T("{00000000-0000-0000-0000-000000000000}"), 
			(PVOID)pBuffer, (DWORD) sizeof(pBuffer)) != 0 || GetLastError() != ERROR_INVALID_FUNCTION)
			CTSEnv::Instance().Set(m_pThreadData->pCMLog, VAR_ISUEFI, TRUE_STRING);
		else
			CTSEnv::Instance().Set(m_pThreadData->pCMLog, VAR_ISUEFI, FALSE_STRING);

		CodeProject::RegKey hklm(HKEY_LOCAL_MACHINE);

		if (hklm.Open(_T("SYSTEM\\CurrentControlSet\\Control\\SecureBoot\\State"), KEY_READ) == ERROR_SUCCESS)
		{
			CString dn;
			CodeProject::RegValue dnVal = hklm[_T("UEFISecureBootEnabled")];

			if (dnVal.Type == REG_DWORD)
				CTSEnv::Instance().Set(m_pThreadData->pCMLog, VAR_ISSECUREBOOT, FTW::BoolString((DWORD)dnVal != 0));
			else
				CTSEnv::Instance().Set(m_pThreadData->pCMLog, VAR_ISSECUREBOOT, VALUE_UNKNOWN);

			hklm.Close();
		}
		else
			CTSEnv::Instance().Set(m_pThreadData->pCMLog, VAR_ISSECUREBOOT, VALUE_UNKNOWN);
			
	}

	void CDefaultValues::GetAssetInfo(void)
	{
		CString temp;

		m_pThreadData->pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info, 
			AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
			FTW::FormatResourceString(IDS_LOGMSG_GETTINGASSETINFO));

		GetHardwareInfo();

		GetProcessorInfo();

		GetSystemInfo();

		GetDiskInfo();

		GetBatteryInfo();
	}

	void CDefaultValues::GetTPMInfo(void)
	{
		CString temp;

		try
		{
			FTW::CWMIAccess wmi(_T("root\\cimv2\\Security\\MicrosoftTpm"));

			m_pThreadData->pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info, 
				AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
				FTW::FormatResourceString(IDS_LOGMSG_GETTINGTPMINFO));

			variant_t result;

			if (wmi.Open(_T("Win32_Tpm")))
			{
				if (wmi.InstanceCount() > 0)
				{
					CTSEnv::Instance().Set(m_pThreadData->pCMLog, VAR_TPMAVAILABLE, FTW::BoolString(true));

					if (wmi.ExecMethod(_T("IsEnabled"), _T("IsEnabled"), result))
						CTSEnv::Instance().Set(m_pThreadData->pCMLog, VAR_TPMENABLED, FTW::BoolString((bool)result == true));

					if (wmi.ExecMethod(_T("IsActivated"), _T("IsActivated"), result))
						CTSEnv::Instance().Set(m_pThreadData->pCMLog, VAR_TPMACTIVATED, FTW::BoolString((bool)result == true));

					if (wmi.ExecMethod(_T("IsOwned"), _T("IsOwned"), result))
						CTSEnv::Instance().Set(m_pThreadData->pCMLog, VAR_TPMOWNED, FTW::BoolString((bool)result == true));

					if (CActionHelper::GetWMIPropertyFromFirstInstance(m_pThreadData->pCMLog, wmi, _T("Win32_TPM"), NULL, _T("SpecVersion"), temp))
						CTSEnv::Instance().Set(m_pThreadData->pCMLog, VAR_TPMSPECVERSION, temp);
				}
				else
				{
					CTSEnv::Instance().Set(m_pThreadData->pCMLog, VAR_TPMAVAILABLE, FTW::BoolString(false));
				}
			}

			wmi.Close();

		}
		catch (FTW::FTWException& i)
		{
			m_pThreadData->pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Error, 
				AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
				FTW::FormatResourceString(IDS_LOGERROR_WMINAMESPACEOPEN, _T("root\\cimv2\\Security\\MicrosoftTpm"), i.Message()));

		}

	}

	void CDefaultValues::GetWirelessNetworkInfo(void)
	{
		m_pThreadData->pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info, 
			AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
			FTW::FormatResourceString(IDS_LOGMSG_GETTINGWNETINFO));

		try
		{
			m_pThreadData->pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info, 
				AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
				FTW::FormatResourceString(IDS_LOGMSG_LOADINGWLANDLLS));

			HINSTANCE hWlanDll = LoadLibrary(_T("WlanApi.dll"));

			if (hWlanDll == NULL)
			{
				std::wstring errormsg = FTW::FormatHRString(HRESULT_FROM_WIN32(GetLastError()));

				m_pThreadData->pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info, 
					AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
					FTW::FormatResourceString(IDS_LOGMSG_WLANNOGO, errormsg.c_str()));

				CTSEnv::Instance().Set(m_pThreadData->pCMLog, VAR_WLANDISCONNECTED, XML_ACTION_TRUE);

				return;
			}

			WlanOpenHandleFunc WlanOpenHandle = (WlanOpenHandleFunc)GetProcAddress(hWlanDll, "WlanOpenHandle");
			WlanEnumInterfacesFunc WlanEnumInterfaces = (WlanEnumInterfacesFunc)GetProcAddress(hWlanDll, "WlanEnumInterfaces");
			WlanCloseHandleFunc WlanCloseHandle = (WlanCloseHandleFunc)GetProcAddress(hWlanDll, "WlanCloseHandle");
			WlanQueryInterfaceFunc WlanQueryInterface = (WlanQueryInterfaceFunc)GetProcAddress(hWlanDll, "WlanQueryInterface");

			if (!WlanOpenHandle || !WlanEnumInterfaces || !WlanCloseHandle || !WlanQueryInterface)
			{
				std::wstring errormsg = FTW::FormatHRString(HRESULT_FROM_WIN32(GetLastError()));

				m_pThreadData->pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info,
					AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
					FTW::FormatResourceString(IDS_LOGMSG_WLANNOGO, errormsg.c_str()));

				CTSEnv::Instance().Set(m_pThreadData->pCMLog, VAR_WLANDISCONNECTED, XML_ACTION_TRUE);

				return;
			}

			try
			{
				DWORD maxClientVer = 2;
				DWORD currentClientVersion = 0;
				HANDLE clientHandle;

				PWLAN_INTERFACE_INFO pInterfaceInfo = NULL;
				PWLAN_INTERFACE_INFO_LIST pInterfaceInfoList = NULL;

				PWLAN_CONNECTION_ATTRIBUTES pConnectionInfo = NULL;
				DWORD connectionInfoSize = sizeof(WLAN_CONNECTION_ATTRIBUTES);
				WLAN_OPCODE_VALUE_TYPE opCodeType = wlan_opcode_value_type_invalid;

				CString SSID = _T(""), tmpSSID = _T("");
				CString wlanState = _T("");
				bool wlanDisconnected = true;

				m_pThreadData->pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info, 
					AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
					FTW::FormatResourceString(IDS_LOGMSG_LOADINGWLANINFO));

				DWORD result = (*WlanOpenHandle)(maxClientVer, NULL, &currentClientVersion, &clientHandle);

				if (result == ERROR_SUCCESS)
				{
					result = (*WlanEnumInterfaces)(clientHandle, NULL, &pInterfaceInfoList);

					if (result == ERROR_SUCCESS)
					{
						for (int interfaceIndex = 0; interfaceIndex < (int)pInterfaceInfoList->dwNumberOfItems; interfaceIndex++)
						{
							pInterfaceInfo = (PWLAN_INTERFACE_INFO)&pInterfaceInfoList->InterfaceInfo[interfaceIndex];
							DWORD wlanStateStringId = 0;

							switch (pInterfaceInfo->isState)
							{
							case wlan_interface_state_not_ready:
								wlanStateStringId = IDS_WLANSTATE_NOTREADY;
								break;
							case wlan_interface_state_connected:
								wlanStateStringId = IDS_WLANSTATE_CONNECTED;
								break;
							case wlan_interface_state_ad_hoc_network_formed:
								wlanStateStringId = IDS_WLANSTATE_ADHOC;
								break;
							case wlan_interface_state_disconnecting:
								wlanStateStringId = IDS_WLANSTATE_DISCONNECTING;
								break;
							case wlan_interface_state_disconnected:
								wlanStateStringId = IDS_WLANSTATE_DISCONNECTED;
								break;
							case wlan_interface_state_associating:
								wlanStateStringId = IDS_WLANSTATE_ASSOCIATING;
								break;
							case wlan_interface_state_discovering:
								wlanStateStringId = IDS_WLANSTATE_DISCOVERING;
								break;
							case wlan_interface_state_authenticating:
								wlanStateStringId = IDS_WLANSTATE_AUTHENTICATING;
								break;
							default:
								wlanStateStringId = IDS_WLANSTATE_UNKNOWN;
								break;
							}

							if (!wlanState.LoadString(wlanStateStringId))
								wlanState = _T("");


							if (pInterfaceInfo->isState != wlan_interface_state_disconnected)
								wlanDisconnected = false;

							if (pInterfaceInfo->isState == wlan_interface_state_connected)
							{
								result = (*WlanQueryInterface)(clientHandle, &pInterfaceInfo->InterfaceGuid, wlan_intf_opcode_current_connection,
									NULL, &connectionInfoSize, (PVOID *)&pConnectionInfo, &opCodeType);

								if (result == ERROR_SUCCESS && pConnectionInfo->wlanAssociationAttributes.dot11Ssid.uSSIDLength > 0)
								{
									if (SSID.GetLength() > 0)
										SSID.AppendChar(L';');

									tmpSSID = pConnectionInfo->wlanAssociationAttributes.dot11Ssid.ucSSID;

									wlanState.Append(L" (");
									wlanState.Append(tmpSSID);
									wlanState.Append(L")");

									SSID.Append(tmpSSID);
								}
							}

							m_pThreadData->pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info, 
								AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
								FTW::FormatResourceString(IDS_LOGMSG_FOUNDWLAN_INTERFACE, pInterfaceInfo->strInterfaceDescription, wlanState));
						}
					}

				}

				result = (*WlanCloseHandle)(clientHandle, NULL);

				if (wlanDisconnected == true)
				{
					CTSEnv::Instance().Set(m_pThreadData->pCMLog, VAR_WLANDISCONNECTED, XML_ACTION_TRUE);
				}
				else
				{
					CTSEnv::Instance().Set(m_pThreadData->pCMLog, VAR_WLANDISCONNECTED, XML_ACTION_FALSE);
					CTSEnv::Instance().Set(m_pThreadData->pCMLog, VAR_WLANSSID, SSID);

				}
			}
			catch (...)
			{
			}

			FreeLibrary(hWlanDll);

		}
		catch (...)
		{
		}

	}

	void CDefaultValues::GetNetworkInfo(void)
	{
		GetWirelessNetworkInfo();

		m_pThreadData->pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info, 
			AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
			FTW::FormatResourceString(IDS_LOGMSG_GETTINGNETINFO));

		try
		{
			variant_t result;
			CString temp;

			if (CActionHelper::QueryWMI(m_pThreadData->pCMLog, m_wmi, _T("DefaultIPGateway"), _T("SELECT DefaultIPGateway FROM Win32_NetworkAdapterConfiguration WHERE IPEnabled='true'"), temp))
				CTSEnv::Instance().Set(m_pThreadData->pCMLog, VAR_IPGATEWAY, temp);

			if (CActionHelper::QueryWMI(m_pThreadData->pCMLog, m_wmi, _T("IPAddress"), _T("SELECT IPAddress FROM Win32_NetworkAdapterConfiguration WHERE IPEnabled='true'"), temp))
				CTSEnv::Instance().Set(m_pThreadData->pCMLog, VAR_IPADDRESS, temp);

			if (CActionHelper::QueryWMI(m_pThreadData->pCMLog, m_wmi, _T("IPSubnet"), _T("SELECT IPSubnet FROM Win32_NetworkAdapterConfiguration WHERE IPEnabled='true'"), temp))
				CTSEnv::Instance().Set(m_pThreadData->pCMLog, VAR_IPSUBNETMASK, temp);

			if (CActionHelper::QueryWMI(m_pThreadData->pCMLog, m_wmi, _T("MACAddress"), _T("SELECT MACAddress FROM Win32_NetworkAdapterConfiguration WHERE IPEnabled='true'"), temp))
				CTSEnv::Instance().Set(m_pThreadData->pCMLog, VAR_MACADDRESS, temp);

			if (CActionHelper::QueryWMI(m_pThreadData->pCMLog, m_wmi, _T("Name"), _T("SELECT Name FROM Win32_NetworkAdapter WHERE NetConnectionStatus=2 And NetEnabled='True' And AdapterTypeID=0 And PhysicalAdapter='True'"), temp))
			{
				FTW::CWMIAccess wmi2(_T("root\\wmi"));

				int pos = 0;
				CString name = temp.Tokenize(_T(","), pos);
				CString query;
				CString wiredNICName;
				bool wiredLANConnected = false;

				while (pos != -1)
				{
					if (name != _T(""))
					{
						query.Format(_T("SELECT InstanceName FROM MSNdis_PhysicalMediumType WHERE InstanceName='%s' And NDisPhysicalMediumType=0"), name.GetString());

						if(CActionHelper::QueryWMI(m_pThreadData->pCMLog, wmi2, _T("InstanceName"), query, wiredNICName))
						{
							wiredLANConnected = true;

							m_pThreadData->pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info, 
								AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
								FTW::FormatResourceString(IDS_LOGMSG_FOUNDWIREDLAN_INTERFACE, wiredNICName));

							break;
						}
					}

					name = temp.Tokenize(_T(","), pos);
				}

				CTSEnv::Instance().Set(m_pThreadData->pCMLog, VAR_WIREDLANCONNECTED, wiredLANConnected ? XML_ACTION_TRUE : XML_ACTION_FALSE);
			}
			else
				CTSEnv::Instance().Set(m_pThreadData->pCMLog, VAR_WIREDLANCONNECTED, XML_ACTION_FALSE);

		}
		catch (FTW::FTWException& i)
		{
			m_pThreadData->pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Error, 
				AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
				FTW::FormatResourceString(IDS_LOGERROR_WMINAMESPACEOPEN, _T("root\\cimv2\\Win32_NetworkAdapterConfiguration"), i.Message()));

		}
	}

	void CDefaultValues::GetUserInfo(void)
	{
		m_pThreadData->pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info, 
			AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
			FTW::FormatResourceString(IDS_LOGMSG_GETTINGUSERINFO));
		
		ULONG size = 256;
		LPTSTR pName = new TCHAR[256];
		//CString temp;

		DWORD level = 1;
		LPUSER_INFO_1 pBuf = NULL;

		if (GetUserNameExW(NameDisplay, pName, &size))
		{
			CTSEnv::Instance().Set(m_pThreadData->pCMLog, VAR_USERDISPLAYNAME, pName);
			wmemset(pName, 0, size);
		}

		if (GetUserNameExW(NameSamCompatible, pName, &size))
		{
			CTSEnv::Instance().Set(m_pThreadData->pCMLog, VAR_USERSAMACCOUNTNAME, pName);
			wmemset(pName, 0, size);
		}

		if (GetUserNameExW(NameUserPrincipal, pName, &size))
		{
			CTSEnv::Instance().Set(m_pThreadData->pCMLog, VAR_USERPRINCIPALNAME, pName);
			wmemset(pName, 0, size);
		}

		delete [] pName;
		pName = nullptr;

		//if (!username.IsEmpty())
		//{
		//	NET_API_STATUS status = NetUserGetInfo(NULL, username, level, (LPBYTE*)&pBuf);

		//	if (status == NERR_Success)
		//	{
		//		CTSEnv::Instance().Set(m_pThreadData->pCMLog, VAR_USERISLOCALADMIN, FTW::BoolString(pBuf->usri1_priv == USER_PRIV_ADMIN));
		//	}

		//	if (pBuf != NULL)
		//		NetApiBufferFree(pBuf);
		//}

		PSID pSid;
		DWORD sidSize = SECURITY_MAX_SID_SIZE;;
		BOOL isAdmin = FALSE;

		pSid = LocalAlloc(LMEM_FIXED, sidSize);

		if (CreateWellKnownSid(WinBuiltinAdministratorsSid, NULL, pSid, &sidSize))
		{
			if(pSid && CheckTokenMembership(NULL, pSid, &isAdmin))
				CTSEnv::Instance().Set(m_pThreadData->pCMLog, VAR_USERISLOCALADMIN, FTW::BoolString(isAdmin));
		}

		LocalFree(pSid);

	}

	void CDefaultValues::GetMgmtInfo(void)
	{
		if (!m_isPE)
		{
			m_pThreadData->pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info, 
				AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
				FTW::FormatResourceString(IDS_LOGMSG_GETTINGMGMTINFO));

			if (GetServiceInfo(_T("ccmexec")))
			{
				try
				{
					FTW::CWMIAccess wmi(_T("root\\ccm"));

					variant_t result;
					CString version, mp;

					if (wmi.Open(_T("SMS_Client")))
					{
						if (wmi.GetPropertyFromSingleInstance(_T("ClientVersion"), version))
							CTSEnv::Instance().Set(m_pThreadData->pCMLog, VAR_CONFIGMGRAGENTVERSION, version);

						if (wmi.ExecMethod(_T("GetAssignedSite"), _T("sSiteCode"), result, true))
							CTSEnv::Instance().Set(m_pThreadData->pCMLog, VAR_CONFIGMGRSITECODE, CString(result));

					}

					if (wmi.Open(_T("SMS_Authority")))
					{
						if (wmi.GetPropertyFromSingleInstance(_T("CurrentManagementPoint"), mp))
							CTSEnv::Instance().Set(m_pThreadData->pCMLog, VAR_CONFIGMGRCURRENTMP, mp);
					}

					wmi.Close();

				}
				catch (FTW::FTWException& i)
				{
					m_pThreadData->pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info, 
						AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
						FTW::FormatResourceString(IDS_LOGERROR_DEFAULTVALUERETRIEVAL, i.Message()));
				}
			}

			CTSEnv::Instance().Set(m_pThreadData->pCMLog, VAR_INTUNECLIENTAGENT, FTW::BoolString(GetServiceInfo(_T("OmcSvc"))));
		
			FTW::CWMIAccess wmi(_T("root\\cimv2\\mdm"));
			CString authorityName;

			if (CActionHelper::GetWMIPropertyFromFirstInstance(m_pThreadData->pCMLog, wmi, _T("MDM_MgmtAuthority"), NULL, _T("AuthorityName"), authorityName))
				CTSEnv::Instance().Set(m_pThreadData->pCMLog, VAR_MDMAUTHORITYNAME, authorityName);
		}
	}

	void CDefaultValues::GetFirewallInfo(void)
	{
		HRESULT hr = S_OK;
		//INetFwPolicy2 *pNetFwPolicy2 = NULL;
		CComPtr<INetFwPolicy2> pNetFwPolicy2;
		CString temp = _T("");

		struct ProfileMapElement
		{
			NET_FW_PROFILE_TYPE2 Id = NET_FW_PROFILE2_DOMAIN;
			CString Name;
		};
		ProfileMapElement profileMap[3];
		profileMap[0].Id = NET_FW_PROFILE2_DOMAIN;
		profileMap[0].Name = VAR_FIREWALLDOMAIN;
		profileMap[1].Id = NET_FW_PROFILE2_PRIVATE;
		profileMap[1].Name = VAR_FIREWALLPRIVATE;
		profileMap[2].Id = NET_FW_PROFILE2_PUBLIC;
		profileMap[2].Name = VAR_FIREWALLPUBLIC;

		CString firewallProfiles[3]{ VAR_FIREWALLBLOCK, VAR_FIREWALLALLOW, VAR_FIREWALLMAX };
		CString firewallActions[3]{ VAR_FIREWALLBLOCK, VAR_FIREWALLALLOW, VAR_FIREWALLMAX };

		GetServiceInfo(_T("MpsSvc"));
		
		hr = CoCreateInstance(
			__uuidof(NetFwPolicy2),
			NULL,
			CLSCTX_INPROC_SERVER,
			__uuidof(INetFwPolicy2),
			reinterpret_cast<LPVOID*> (&pNetFwPolicy2));

		if (FAILED(hr))
		{
			m_pThreadData->pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Error, 
				AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
				FTW::FormatResourceString(IDS_LOGERROR_DEFAULTVALUERETRIEVAL, FTW::FTWErrorCodeException(hr, XML_ATTRIBUTE_DEFAULTVALUE_FIREWALL).Message()));
		}
		else
		{
			long currentProfilesBitMask = 0;
			hr = pNetFwPolicy2->get_CurrentProfileTypes(&currentProfilesBitMask);

			if (SUCCEEDED(hr))
			{
				VARIANT_BOOL isEnabled = FALSE;
				NET_FW_ACTION fwAction;

				int activeCount = 0;
				CString variableName;
				temp.Empty();

				for (int i = 0; i < 3; i++)
				{
					if (currentProfilesBitMask & profileMap[i].Id)
					{
						if (activeCount > 0)
							temp += _T(",");

						temp += profileMap[i].Name;
						activeCount++;
					}

					hr = pNetFwPolicy2->get_FirewallEnabled(profileMap[i].Id, &isEnabled);

					if (SUCCEEDED(hr))
					{
						variableName = VAR_FIREWALLENABLED;
						variableName += profileMap[i].Name;

						CTSEnv::Instance().Set(m_pThreadData->pCMLog, variableName, FTW::BoolString(isEnabled));
					}

					hr = pNetFwPolicy2->get_DefaultInboundAction(profileMap[i].Id, &fwAction);

					if (SUCCEEDED(hr))
					{
						variableName = VAR_FIREWALLINBOUND;
						variableName += profileMap[i].Name;

						CTSEnv::Instance().Set(m_pThreadData->pCMLog, variableName, firewallActions[fwAction]);
					}

					hr = pNetFwPolicy2->get_DefaultOutboundAction(profileMap[i].Id, &fwAction);

					if (SUCCEEDED(hr))
					{
						variableName = VAR_FIREWALLOUTBOUND;
						variableName += profileMap[i].Name;

						CTSEnv::Instance().Set(m_pThreadData->pCMLog, variableName, firewallActions[fwAction]);
					}
				}

				CTSEnv::Instance().Set(m_pThreadData->pCMLog, VAR_FIREWALLCURRENTPROFILES, temp);

			}
		}
	}

	void CDefaultValues::GetWindowsUpdateInfo(void)
	{
		GetServiceInfo(_T("wuauserv"));

		CComPtr<IAutomaticUpdates> pAutoUpdates;
		CComPtr<IUpdateSession3> pUpdateSession;

		HRESULT hr = CoCreateInstance(
			CLSID_AutomaticUpdates,
			NULL,
			CLSCTX_INPROC_SERVER,
			IID_IAutomaticUpdates,
			(void**)&pAutoUpdates);

		if (FAILED(hr))
		{
			m_pThreadData->pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Error, 
				AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
				FTW::FormatResourceString(IDS_LOGERROR_DEFAULTVALUERETRIEVAL, FTW::FTWErrorCodeException(hr, XML_ATTRIBUTE_DEFAULTVALUE_WINDOWSUPDATE).Message()));
		}
		else
		{
			VARIANT_BOOL enabled = VARIANT_FALSE;

			hr = pAutoUpdates->get_ServiceEnabled(&enabled);

			if(SUCCEEDED(hr))
				CTSEnv::Instance().Set(m_pThreadData->pCMLog, VAR_WINDOWSUPDATESENABLED, FTW::BoolString(enabled));
		}

		hr = CoCreateInstance(
			CLSID_UpdateSession,
			NULL,
			CLSCTX_INPROC_SERVER,
			IID_IUpdateSession,
			reinterpret_cast<LPVOID*> (&pUpdateSession));

		if (SUCCEEDED(hr))
		{
			CComPtr<IUpdateServiceManager2> pUpdateServiceManager;

			hr = pUpdateSession->CreateUpdateServiceManager(&pUpdateServiceManager);

			if (SUCCEEDED(hr))
			{
				CComPtr<IUpdateServiceCollection> pUpdateServiceCollection;

				hr = pUpdateServiceManager->get_Services(&pUpdateServiceCollection);

				if(SUCCEEDED(hr))
				{
					long count = 0;
					CComBSTR serviceURL;
					CComBSTR serviceName;
					VARIANT_BOOL defaultService;

					hr = pUpdateServiceCollection->get_Count(&count);

					for (long i = 0; SUCCEEDED(hr) && i < count; i++)
					{
						CComQIPtr<IUpdateService> pUpdateService;
						hr = pUpdateServiceCollection->get_Item(i, &pUpdateService);

						if (SUCCEEDED(hr))
						{
							hr = pUpdateService->get_Name(&serviceName);
							
							if (SUCCEEDED(hr))
							{
								CComQIPtr<IUpdateService2> pUpdateService2(pUpdateService);
								hr = pUpdateService2->get_IsDefaultAUService(&defaultService);

								if (SUCCEEDED(hr))
								{
									if (defaultService == VARIANT_TRUE)
									{
										CTSEnv::Instance().Set(m_pThreadData->pCMLog, VAR_WINDOWSUPDATEDEFAULTSERVICE, serviceName);

										if (serviceName == "Windows Server Update Service")
										{
											CodeProject::RegKey hklm(HKEY_LOCAL_MACHINE);

											if (hklm.Open(_T("SOFTWARE\\Policies\\Microsoft\\Windows\\WindowsUpdate"), KEY_READ | KEY_WOW64_64KEY) == ERROR_SUCCESS)
											{
												//CString server;
												CodeProject::RegValue server = hklm[_T("WUServer")];

												CTSEnv::Instance().Set(m_pThreadData->pCMLog, VAR_WINDOWSUPDATESERVER, (LPCWSTR)server);

												hklm.Close();
											}

										}
									}
								}
							}
						}
					}
				}
			}
		}

		if(FAILED(hr))
		{
			m_pThreadData->pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Error, 
				AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
				FTW::FormatResourceString(IDS_LOGERROR_DEFAULTVALUERETRIEVAL, FTW::FTWErrorCodeException(hr, XML_ATTRIBUTE_DEFAULTVALUE_WINDOWSUPDATE).Message()));
		}
	}

	void CDefaultValues::GetWindowsDefenderInfo(void)
	{
		if (!m_isPE)
		{
			m_pThreadData->pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info, 
				AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
				FTW::FormatResourceString(IDS_LOGMSG_GETTINGDEFENDERINFO));

			if (GetServiceInfo(_T("WinDefend")))
			{
				try
				{
					FTW::CWMIAccess wmi(_T("root\\Microsoft\\Windows\\Defender"));

					variant_t result;
					CString version, mp;

					if (wmi.Open(_T("MSFT_MpComputerStatus")))
					{
						if (wmi.GetPropertyFromSingleInstance(_T("AntivirusEnabled"), version))
							CTSEnv::Instance().Set(m_pThreadData->pCMLog, VAR_DEFENDERAVENABLED, version);

						if (wmi.GetPropertyFromSingleInstance(_T("AntispywareEnabled"), version))
							CTSEnv::Instance().Set(m_pThreadData->pCMLog, VAR_DEFENDERASENABLED, version);

						if (wmi.GetPropertyFromSingleInstance(_T("NISEnabled"), version))
							CTSEnv::Instance().Set(m_pThreadData->pCMLog, VAR_DEFENDERNISENABLED, version);

						if (wmi.GetPropertyFromSingleInstance(_T("FullScanAge"), version))
							CTSEnv::Instance().Set(m_pThreadData->pCMLog, VAR_DEFENDERFULLSCANAGE, version);

						if (wmi.GetPropertyFromSingleInstance(_T("AMEngineVersion"), version))
							CTSEnv::Instance().Set(m_pThreadData->pCMLog, VAR_DEFENDERENGINEVERSION, version);

						if (wmi.GetPropertyFromSingleInstance(_T("AntivirusSignatureAge"), version))
							CTSEnv::Instance().Set(m_pThreadData->pCMLog, VAR_DEFENDERAVSIGAGE, version);
					}

					wmi.Close();

				}
				catch (FTW::FTWException& i)
				{
					m_pThreadData->pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info, 
						AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
						FTW::FormatResourceString(IDS_LOGERROR_DEFAULTVALUERETRIEVAL, i.Message()));
				}
			}
		}
	}
	
	void CDefaultValues::GetPendingRebootInfo(void)
	{
		CodeProject::RegKey hklm(HKEY_LOCAL_MACHINE);
		bool cbsPendingReboot = false;
		bool wuaPendingReboot = false;
		bool pfro = false;
		bool renamePendingReboot = false;
		bool ccmRebootPending = false;
		bool rebootPending = false;

		m_pThreadData->pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info, 
			AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
			FTW::FormatResourceString(IDS_LOGMSG_GETTINGREBOOTPENDINGINFO));

		if (hklm.Open(_T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Component Based Servicing\\RebootPending"), KEY_READ) == ERROR_SUCCESS)
		{
			cbsPendingReboot = true;

			hklm.Close();
		}

		if (hklm.Open(_T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\WindowsUpdate\\Auto Update\\RebootRequired"), KEY_READ) == ERROR_SUCCESS)
		{
			wuaPendingReboot = true;

			hklm.Close();
		}

		if (hklm.Open(_T("SYSTEM\\CurrentControlSet\\Control\\Session Manager"), KEY_READ) == ERROR_SUCCESS)
		{
			CodeProject::RegValue regVal = hklm[_T("PendingFileRenameOperations")];

			if (regVal.Type == REG_SZ)
			{
				//CString pfroVal = regVal;
				//if (pfroVal.GetLength() > 0)
				pfro = true;
			}

			hklm.Close();
		}

		if (hklm.Open(_T("SYSTEM\\CurrentControlSet\\Control\\ComputerName\\ActiveComputerName"), KEY_READ) == ERROR_SUCCESS)
		{
			CodeProject::RegValue regActiveCompterName = hklm[_T("ComputerName")];
			CString activeComputerName;

			if (regActiveCompterName.Type == REG_SZ)
			{
				activeComputerName = regActiveCompterName;
			}

			hklm.Close();

			if (hklm.Open(_T("SYSTEM\\CurrentControlSet\\Control\\ComputerName\\ComputerName"), KEY_READ) == ERROR_SUCCESS)
			{
				CodeProject::RegValue regCompterName = hklm[_T("ComputerName")];

				CString computerName = (LPCTSTR)regCompterName;
				
				if (computerName != activeComputerName)
					renamePendingReboot = true;

				hklm.Close();
			}
		}

		try
		{
			FTW::CWMIAccess wmi(_T("root\\ccm\\ClientSDK"));

			variant_t result;

			if (wmi.Open(_T("CCM_ClientUtilities")))
			{
				if (wmi.ExecMethod(_T("DetermineIfRebootPending"), _T("RebootPending"), result))
					if ((bool)result == true)
						ccmRebootPending = true;

				if (wmi.ExecMethod(_T("DetermineIfRebootPending"), _T("IsHardRebootPending"), result))
					if ((bool)result == true)
						ccmRebootPending = true;

				CTSEnv::Instance().Set(m_pThreadData->pCMLog, VAR_CCMREBOOTPENDING, FTW::BoolString(ccmRebootPending));
			}

			wmi.Close();

		}
		catch (FTW::FTWException& i)
		{
		}

		rebootPending = cbsPendingReboot || wuaPendingReboot || pfro || renamePendingReboot;

		CTSEnv::Instance().Set(m_pThreadData->pCMLog, VAR_OSCBSREBOOTPENDING, FTW::BoolString(cbsPendingReboot));
		CTSEnv::Instance().Set(m_pThreadData->pCMLog, VAR_OSWUAREBOOTPENDING, FTW::BoolString(wuaPendingReboot));
		CTSEnv::Instance().Set(m_pThreadData->pCMLog, VAR_OSPFRO, FTW::BoolString(pfro));
		CTSEnv::Instance().Set(m_pThreadData->pCMLog, VAR_OSPENDINGRENAME, FTW::BoolString(renamePendingReboot));
		CTSEnv::Instance().Set(m_pThreadData->pCMLog, VAR_REBOOTPENDING, FTW::BoolString(rebootPending));

	}

	void CDefaultValues::GetOSInfo(void)
	{
		CString temp;
		bool serverCoreOS = false;
		//DWORD productType;
		TCHAR path[MAX_STRING_LENGTH];

		//OSVERSIONINFOEX osInfo;

		m_pThreadData->pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info, 
			AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
			FTW::FormatResourceString(IDS_LOGMSG_GETTINGOSINFO));

		if (CActionHelper::GetWMIPropertyFromFirstInstance(m_pThreadData->pCMLog, m_wmi, _T("Win32_OperatingSystem"), NULL, _T("BuildNumber"), temp))
			CTSEnv::Instance().Set(m_pThreadData->pCMLog, VAR_OSBUILD, temp);

		if (CActionHelper::GetWMIPropertyFromFirstInstance(m_pThreadData->pCMLog, m_wmi, _T("Win32_OperatingSystem"), NULL, _T("Version"), temp))
			CTSEnv::Instance().Set(m_pThreadData->pCMLog, VAR_OSVERSION, temp);

		if (CActionHelper::GetWMIPropertyFromFirstInstance(m_pThreadData->pCMLog, m_wmi, _T("Win32_OperatingSystem"), NULL, _T("ServicePackMajorVersion"), temp))
			CTSEnv::Instance().Set(m_pThreadData->pCMLog, VAR_OSSERVICEPACK, temp);

		if (CActionHelper::GetWMIPropertyFromFirstInstance(m_pThreadData->pCMLog, m_wmi, _T("Win32_OperatingSystem"), NULL, _T("ProductType"), temp))
		{
			int prodType = _ttoi(temp);

			switch (prodType)
			{
			case VER_NT_WORKSTATION:
				CTSEnv::Instance().Set(m_pThreadData->pCMLog, VAR_OSPRODUCT, VALUE_OSWORKSTATION);
				break;
			case VER_NT_SERVER:
				CTSEnv::Instance().Set(m_pThreadData->pCMLog, VAR_OSPRODUCT, VALUE_OSSERVER);
				break;
			case VER_NT_DOMAIN_CONTROLLER:
				CTSEnv::Instance().Set(m_pThreadData->pCMLog, VAR_OSPRODUCT, VALUE_OSSDOMAINCONTROLLER);
				break;
			default:
				CTSEnv::Instance().Set(m_pThreadData->pCMLog, VAR_OSPRODUCT, VALUE_UNKNOWN);
			}
		}

		CTSEnv::Instance().Set(m_pThreadData->pCMLog, VAR_OSSYSTEMDRIVE, GetEnvVar(_T("SystemDrive")));

		/*ZeroMemory(&osInfo, sizeof(OSVERSIONINFOEX));

		osInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

		if (GetVersionEx((LPOSVERSIONINFO)(&osInfo)))
		{
			temp.Format(_T("%u.%u"), osInfo.dwMajorVersion, osInfo.dwMinorVersion);
			CTSEnv::Instance().Set(m_pThreadData->pCMLog, VAR_OSVERSION, temp);

			temp.Format(_T("%u"), osInfo.dwBuildNumber);
			CTSEnv::Instance().Set(m_pThreadData->pCMLog, VAR_OSBUILD, temp);

			temp.Format(_T("%u"), osInfo.wServicePackMajor);
			CTSEnv::Instance().Set(m_pThreadData->pCMLog, VAR_OSSERVICEPACK, temp);

			switch (osInfo.wProductType)
			{
			case VER_NT_WORKSTATION:
				CTSEnv::Instance().Set(m_pThreadData->pCMLog, VAR_OSPRODUCT, VALUE_OSWORKSTATION);
				break;
			case VER_NT_SERVER:
				CTSEnv::Instance().Set(m_pThreadData->pCMLog, VAR_OSPRODUCT, VALUE_OSSERVER);
				break;
			case VER_NT_DOMAIN_CONTROLLER:
				CTSEnv::Instance().Set(m_pThreadData->pCMLog, VAR_OSPRODUCT, VALUE_OSSDOMAINCONTROLLER);
				break;
			default:
				CTSEnv::Instance().Set(m_pThreadData->pCMLog, VAR_OSPRODUCT, VALUE_UNKNOWN);
			}

		} */

		if (!m_isPE && ExpandEnvironmentStrings(_T("%windir%\\Explorer.exe"), path, MAX_STRING_LENGTH) > 0 && !FTW::FileExists(path))
		{
			serverCoreOS = true;
		}

		CTSEnv::Instance().Set(m_pThreadData->pCMLog, VAR_ISSERVERCOREOS, FTW::BoolString(serverCoreOS));
		CTSEnv::Instance().Set(m_pThreadData->pCMLog, VAR_ISWINPE, FTW::BoolString(m_isPE));

		if (!m_isPE)
		{
			GetPendingRebootInfo();
		}

	}

	void CDefaultValues::GetSecurityInfo(void)
	{
		if (!m_isPE)
		{
			CString temp;

			m_pThreadData->pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info, 
				AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
				FTW::FormatResourceString(IDS_LOGMSG_GETTINGSECURITYINFO));

			try
			{
				FTW::CWMIAccess wmi(_T("root\\cimv2\\security\\MicrosoftVolumeEncryption"));

				CString systemDrive = GetEnvVar(_T("SystemDrive"));

				CString query = _T("SELECT ProtectionStatus FROM Win32_EncryptableVolume WHERE DriveLetter='");
				query += systemDrive;
				query += _T("'");

				if (CActionHelper::QueryWMI(m_pThreadData->pCMLog, wmi, _T("ProtectionStatus"), query, temp))
					CTSEnv::Instance().Set(m_pThreadData->pCMLog, VAR_SYSTEMDRIVEBITLOCKERPROTECTED, temp);
			}
			catch (FTW::FTWException& i)
			{
				m_pThreadData->pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Error, 
					AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
					FTW::FormatResourceString(IDS_LOGERROR_DEFAULTVALUERETRIEVAL, i.Message()));

			}

			GetFirewallInfo();
			GetWindowsUpdateInfo();
			GetWindowsDefenderInfo();
		}
	}

	bool CDefaultValues::GetAzureADInfo(bool aadJoin)
	{
		CString keyName, keyName2, aadName, tenantId, userEmail;
		TCHAR subkeyName[MAX_KEY_LENGTH];
		DWORD nameLen = MAX_KEY_LENGTH;
		
		bool inAzureAD = false;
		CString azureRegKey;

		CodeProject::RegKey hk;
		
		if (aadJoin)
		{
			hk.Connect(HKEY_LOCAL_MACHINE);
			azureRegKey = _T("SYSTEM\\CurrentControlSet\\Control\\CloudDomainJoin");
		}
		else
		{
			hk.Connect(HKEY_CURRENT_USER);
			azureRegKey = _T("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\WorkplaceJoin");
		}

		keyName.Format(_T("%s\\%s"), azureRegKey.GetString(), _T("JoinInfo"));

		if (hk.Open(keyName, KEY_READ) == ERROR_SUCCESS)
		{
			if (hk.EnumKey(0, subkeyName, &nameLen) == ERROR_SUCCESS)
			{
				hk.Close();

				keyName2.Format(_T("%s\\%s"), keyName.GetString(), subkeyName);

				if (hk.Open(keyName2, KEY_READ) == ERROR_SUCCESS)
				{
					CodeProject::RegValue rVal = hk[_T("TenantId")];

					if (rVal.Type == REG_SZ)
					{
						tenantId = rVal;

						if (tenantId.GetLength() > 0)
							CTSEnv::Instance().Set(m_pThreadData->pCMLog, VAR_COMPUTERAZURETENANTID, tenantId);
					}

					rVal = hk[_T("UserEmail")];

					if (rVal.Type == REG_SZ)
					{
						userEmail = rVal;

						if (userEmail.GetLength() > 0)
							CTSEnv::Instance().Set(m_pThreadData->pCMLog, VAR_COMPUTERAZUREUSER, userEmail);
					}
				}
			}

			keyName.Format(_T("%s\\%s\\%s"), azureRegKey.GetString(), _T("TenantInfo"), tenantId.GetString());

			hk.Close();

			if (tenantId.GetLength() > 0 && hk.Open(keyName, KEY_READ) == ERROR_SUCCESS)
			{
				inAzureAD = true;

				CodeProject::RegValue dnVal = hk[_T("DisplayName")];

				if (dnVal.Type == REG_SZ)
				{
					aadName = dnVal;

					if (aadName.GetLength() > 0)
						CTSEnv::Instance().Set(m_pThreadData->pCMLog, VAR_COMPUTERAZUREDOMAIN, aadName);
				}
			}
		}

		hk.Close();

		if(aadJoin)
			CTSEnv::Instance().Set(m_pThreadData->pCMLog, VAR_COMPUTERAZUREDOMAINJOINED, inAzureAD ? TRUE_STRING : FALSE_STRING);
		else
			CTSEnv::Instance().Set(m_pThreadData->pCMLog, VAR_COMPUTERAZUREDOMAINREG, inAzureAD ? TRUE_STRING : FALSE_STRING);

		return inAzureAD;
	}

	void CDefaultValues::GetDomainInfo(void)
	{
		CString temp;

		DWORD level = 100;
		LPWKSTA_INFO_100 pBuf = NULL;

		m_pThreadData->pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info, 
			AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
			FTW::FormatResourceString(IDS_LOGMSG_GETTINGDOMAININFO));

		NET_API_STATUS status = NetWkstaGetInfo(NULL, level, (LPBYTE*)&pBuf);

		if (status == NERR_Success)
		{
			temp.Format(_T("%s"), pBuf->wki100_langroup);
			CTSEnv::Instance().Set(m_pThreadData->pCMLog, VAR_CURRENTCOMPUTERDOMAIN, temp);

			temp.Format(_T("%s"), pBuf->wki100_computername);
			CTSEnv::Instance().Set(m_pThreadData->pCMLog, VAR_CURRENTCOMPUTERNAME, temp);
		}

		if (pBuf != NULL)
			NetApiBufferFree(pBuf);

		if (CActionHelper::GetWMIPropertyFromFirstInstance(m_pThreadData->pCMLog, m_wmi, _T("Win32_ComputerSystem"), NULL, _T("PartOfDomain"), temp))
			CTSEnv::Instance().Set(m_pThreadData->pCMLog, VAR_CURRENTCOMPUTERJOINEDTODOMAIN, temp);

		CodeProject::RegKey hklm(HKEY_LOCAL_MACHINE);

		if (hklm.Open(_T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Group Policy\\State\\Machine"), KEY_READ) == ERROR_SUCCESS)
		{
			CString dn;
			CodeProject::RegValue dnVal = hklm[_T("Distinguished-Name")];

			if (dnVal.Type == REG_SZ)
			{
				dn = dnVal;
				int i = dn.Find(_T(","));
				dn = dn.Mid(i + 1);

				if (i > 0 && dn.GetLength() > 0)
					CTSEnv::Instance().Set(m_pThreadData->pCMLog, VAR_CURRENTCOMPUTEROUDN, dn);
			}
		}
		
		hklm.Close();
		
		if (!GetAzureADInfo(true))
		{
			GetAzureADInfo(false);
		}
		else
		{
			CTSEnv::Instance().Set(m_pThreadData->pCMLog, VAR_COMPUTERAZUREDOMAINREG, FALSE_STRING);
		}
	}

	void CDefaultValues::GetVirtualizationInfo(void)
	{
		m_pThreadData->pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info, 
			AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
			FTW::FormatResourceString(IDS_LOGMSG_GETTINGVIRTUALINFO));

		CString model;
		CString make;

		CActionHelper::GetWMIPropertyFromFirstInstance(m_pThreadData->pCMLog, m_wmi, _T("Win32_ComputerSystem"), NULL, _T("Model"), model);
		CActionHelper::GetWMIPropertyFromFirstInstance(m_pThreadData->pCMLog, m_wmi, _T("Win32_ComputerSystem"), NULL, _T("Manufacturer"), make);

		if (model == VALUE_MODEL_VM)
		{
			CTSEnv::Instance().Set(m_pThreadData->pCMLog, VAR_CHASSISTYPE, VALUE_CHASSIS_VIRTUAL);
			m_chassisTypeSettoVM = true;

			CString biosVersion;

			if (CActionHelper::GetWMIPropertyFromFirstInstance(m_pThreadData->pCMLog, m_wmi, _T("Win32_BIOS"), NULL, _T("Version"), biosVersion))
			{
				if (biosVersion.Find(VALUE_BIOS_VERSION) == 0)
					CTSEnv::Instance().Set(m_pThreadData->pCMLog, VAR_VMTYPE, VALUE_VMTYPE_HYPERV);
				else
					CTSEnv::Instance().Set(m_pThreadData->pCMLog, VAR_VMTYPE, VALUE_VMTYPE_OTHERMS);
			}
			else
				CTSEnv::Instance().Set(m_pThreadData->pCMLog, VAR_VMTYPE, VALUE_VMTYPE_OTHERMS);
		}
		else if (model == VALUE_MODEL_VMWAREESX)
		{
			CTSEnv::Instance().Set(m_pThreadData->pCMLog, VAR_CHASSISTYPE, VALUE_CHASSIS_VIRTUAL);
			m_chassisTypeSettoVM = true;

			CTSEnv::Instance().Set(m_pThreadData->pCMLog, VAR_VMTYPE, VALUE_VMTYPE_VMWAREESX);
		}
		else if (model.Find(VALUE_MODEL_VMWARE) == 0)
		{
			CTSEnv::Instance().Set(m_pThreadData->pCMLog, VAR_CHASSISTYPE, VALUE_CHASSIS_VIRTUAL);
			m_chassisTypeSettoVM = true;

			CTSEnv::Instance().Set(m_pThreadData->pCMLog, VAR_VMTYPE, VALUE_VMTYPE_VMWARE);
		}
		else if (model == VALUE_MODEL_VIRTUALBOX)
		{
			CTSEnv::Instance().Set(m_pThreadData->pCMLog, VAR_CHASSISTYPE, VALUE_CHASSIS_VIRTUAL);
			m_chassisTypeSettoVM = true;

			CTSEnv::Instance().Set(m_pThreadData->pCMLog, VAR_VMTYPE, VALUE_VMTYPE_VIRTUALBOX);
		}
		else if (make == VALUE_MAKE_XEN)
		{
			CTSEnv::Instance().Set(m_pThreadData->pCMLog, VAR_CHASSISTYPE, VALUE_CHASSIS_VIRTUAL);
			m_chassisTypeSettoVM = true;

			CTSEnv::Instance().Set(m_pThreadData->pCMLog, VAR_VMTYPE, VALUE_VMTYPE_XEN);
		}
	}

	void CDefaultValues::Execute(ProgressTextMapPair text)
	{
		ASSERT(m_pThreadData); 
		
		if (m_allValues || (m_pThreadData != nullptr && m_pThreadData->pValueTypes->Find(text.first) > -1))
		{
			TRACE1("Getting %s Info\n", text.first);

			SendProgress(m_pThreadData->hProgressDialog,
				m_pThreadData->pProgressStrings, m_count++, text.first);

			DefaultValuesFunctionsIterator it = functionDispatcher.find(text.first);

			if (it != functionDispatcher.end())
			{
				try
				{
					DefaultValuesFunc valueFunc = it->second;
					(this->*valueFunc)();
				}
				catch (FTW::FTWException& i)
				{
					m_pThreadData->pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Error, AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
						FTW::FormatResourceString(IDS_LOGERROR_DEFAULTVALUERETRIEVAL, i.Message()));

				}
			}
		}
	}

	INT_PTR CActionDefaultValues::Go(void)
	{

		m_actionData.pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info,
			AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
			FTW::FormatResourceString(IDS_LOGMSG_POPULATEDEFAULT));

		CString valueTypes = GetXMLAttribute(m_actionData.pActionNode, XML_ATTRIBUTE_DEFAULTVALUE_TYPES, XML_ATTRIBUTE_DEFAULTVALUE_ALL);
		CString titleText = GetXMLAttribute(m_actionData.pActionNode, XML_ATTRIBUTE_TITLE, XML_ATTRIBUTE_TITLE_PROGRESS_DEF);
		bool showProgress = FTW::IsTrue(GetXMLAttribute(m_actionData.pActionNode, XML_ATTRIBUTE_DEFAULTVALUE_SHOWPROGRESS, XML_ACTION_TRUE));
		INT_PTR functionReturnValue = 0;
		ProgressTextMap progressStrings;

		progressStrings.insert(ProgressTextMap::value_type(XML_ATTRIBUTE_DEFAULTVALUE_USER, new CString(XML_DEFAULTVALUE_USER_PROGRESSTEXT)));
		progressStrings.insert(ProgressTextMap::value_type(XML_ATTRIBUTE_DEFAULTVALUE_OS, new CString(XML_DEFAULTVALUE_OS_PROGRESSTEXT)));
		progressStrings.insert(ProgressTextMap::value_type(XML_ATTRIBUTE_DEFAULTVALUE_TPM, new CString(XML_DEFAULTVALUE_TPM_PROGRESSTEXT)));
		progressStrings.insert(ProgressTextMap::value_type(XML_ATTRIBUTE_DEFAULTVALUE_NET, new CString(XML_DEFAULTVALUE_NET_PROGRESSTEXT)));
		progressStrings.insert(ProgressTextMap::value_type(XML_ATTRIBUTE_DEFAULTVALUE_DOM, new CString(XML_DEFAULTVALUE_DOM_PROGRESSTEXT)));
		progressStrings.insert(ProgressTextMap::value_type(XML_ATTRIBUTE_DEFAULTVALUE_ASSET, new CString(XML_DEFAULTVALUE_ASSET_PROGRESSTEXT)));
		progressStrings.insert(ProgressTextMap::value_type(XML_ATTRIBUTE_DEFAULTVALUE_VM, new CString(XML_DEFAULTVALUE_VM_PROGRESSTEXT)));
		progressStrings.insert(ProgressTextMap::value_type(XML_ATTRIBUTE_DEFAULTVALUE_SECURITY, new CString(XML_DEFAULTVALUE_SECURITY_PROGRESSTEXT)));
		progressStrings.insert(ProgressTextMap::value_type(XML_ATTRIBUTE_DEFAULTVALUE_MGMT, new CString(XML_DEFAULTVALUE_MGMT_PROGRESSTEXT)));

		if (showProgress)
		{
			CString* pValue = nullptr;
			CString type;

			for (pugi::xml_node xmlinput = m_actionData.pActionNode->first_child(); xmlinput; xmlinput = xmlinput.next_sibling())
			{
				if (_tcsicmp(xmlinput.name(), XML_ACTION_DEFAULT_TEXT) != 0)
					continue;

				type = GetXMLAttribute(xmlinput, XML_ATTRIBUTE_TYPE);

				if (CActionHelper::EvalCondition(m_actionData.pCMLog, xmlinput.attribute(XML_ATTRIBUTE_CONDITION).value(), m_actionData.pScriptHost, xmlinput.name(), type))
				{
					ProgressTextMapIterator it = progressStrings.find(type);

					if (it != progressStrings.end())
					{
						pValue = new CString(GetXMLAttribute(xmlinput, XML_ATTRIBUTE_VALUE, *(it->second)));

						/*if (type == XML_ATTRIBUTE_DEFAULTVALUE_OS)
							pValue = new CString(GetXMLAttribute(&xmlinput, XML_ATTRIBUTE_VALUE, XML_DEFAULTVALUE_OS_PROGRESSTEXT));
						else if (type == XML_ATTRIBUTE_DEFAULTVALUE_TPM)
							pValue = new CString(GetXMLAttribute(&xmlinput, XML_ATTRIBUTE_VALUE, XML_DEFAULTVALUE_TPM_PROGRESSTEXT));
						else if (type == XML_ATTRIBUTE_DEFAULTVALUE_NET)
							pValue = new CString(GetXMLAttribute(&xmlinput, XML_ATTRIBUTE_VALUE, XML_DEFAULTVALUE_NET_PROGRESSTEXT));
						else if (type == XML_ATTRIBUTE_DEFAULTVALUE_VM)
							pValue = new CString(GetXMLAttribute(&xmlinput, XML_ATTRIBUTE_VALUE, XML_DEFAULTVALUE_VM_PROGRESSTEXT));
						else if (type == XML_ATTRIBUTE_DEFAULTVALUE_DOM)
							pValue = new CString(GetXMLAttribute(&xmlinput, XML_ATTRIBUTE_VALUE, XML_DEFAULTVALUE_DOM_PROGRESSTEXT));
						else if (type == XML_ATTRIBUTE_DEFAULTVALUE_ASSET)
							pValue = new CString(GetXMLAttribute(&xmlinput, XML_ATTRIBUTE_VALUE, XML_DEFAULTVALUE_ASSET_PROGRESSTEXT));
						else if (type == XML_ATTRIBUTE_DEFAULTVALUE_SECURITY)
							pValue = new CString(GetXMLAttribute(&xmlinput, XML_ATTRIBUTE_VALUE, XML_DEFAULTVALUE_SECURITY_PROGRESSTEXT));*/

						if (pValue != nullptr && pValue->GetLength() > 0)
						{
							auto it = progressStrings.find(type);

							if (it != progressStrings.end())
							{
								if (it->second != nullptr)
									delete it->second;

								progressStrings.erase(it);
							}

							progressStrings.insert(ProgressTextMap::value_type(type, pValue));
						}
						else if (pValue != nullptr)
							delete pValue;

						pValue = nullptr;
					}
				}
			}

			int valueTypeCount = 0;

			if (valueTypes.Find(XML_ATTRIBUTE_DEFAULTVALUE_ALL) > -1)
				valueTypeCount = (int)(progressStrings.size());

			else
			{
				std::for_each(progressStrings.begin(), progressStrings.end(),
					[&](ProgressTextMapPair text) {
					if (valueTypes.Find(text.first) > -1)
						valueTypeCount++;
				});
			}

			if(CTSEnv::Instance().InTS())
				CTSProgress::Instance().Close();

			CDlgUIProgress* pDlg = new CDlgUIProgress(valueTypeCount, titleText, m_actionData.globalDialogTraits.fontFace);

			pDlg->Create(IDD_UIPROGRESSDLG);

			pDlg->ShowWindow(SW_SHOW);

			DefaultValueThreadData threadData(&valueTypes, valueTypeCount, pDlg->GetSafeHwnd(), &progressStrings, m_actionData.pCMLog, true);

			AfxBeginThread(GetDefaulValues, static_cast<LPVOID>(&threadData));
			//GetDefaulValues((LPVOID)&threadData);

			int returnValue = pDlg->RunModalLoop();

			delete pDlg;

			functionReturnValue = ERROR_SUCCESS;// returnValue;
		}
		else
		{
			DefaultValueThreadData threadData(&valueTypes, 0, NULL, &progressStrings, m_actionData.pCMLog);
			functionReturnValue = GetDefaulValues((LPVOID)&threadData);
		}

		for (auto it = progressStrings.begin(); it != progressStrings.end(); ++it)
		{
			if (it->second != nullptr)
				delete it->second;
		}

		return functionReturnValue;
	}
}