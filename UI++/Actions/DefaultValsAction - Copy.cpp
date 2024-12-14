#include "StdAfx.h"
#include "Action.h"
#include "resource.h"
#include "TSVar.h"
#include "Helper.h"
#include "UIProgressDlg.h"
#include "WMIAccess.h"
#include "regkey.h"
#include "Security.h"
#include "Windowsdefender.h"
#include "DefaultValsAction.h"

#include <lm.h>
#include <wlanapi.h>
#include <windot11.h>

typedef DWORD(WINAPI* WlanOpenHandleFunc) (DWORD, PVOID, PDWORD, PHANDLE);
typedef DWORD(WINAPI* WlanEnumInterfacesFunc) (HANDLE, PVOID, PWLAN_INTERFACE_INFO_LIST*);
typedef DWORD(WINAPI* WlanQueryInterfaceFunc) (HANDLE, CONST GUID *, WLAN_INTF_OPCODE, PVOID, PDWORD, PVOID *, PWLAN_OPCODE_VALUE_TYPE);
typedef DWORD(WINAPI* WlanCloseHandleFunc) (HANDLE, PVOID);

namespace UIpp {

	typedef std::unordered_map<CString, CString*> ProgressTextMap;

	struct DefaultValueThreadData
	{
		DefaultValueThreadData(CString* pValueT, int count, HWND hDlg, ProgressTextMap* pStrings, bool initCOM = false)
			: pValueTypes(pValueT), valueTypeCount(count),
			hProgressDialog(hDlg), pProgressStrings(pStrings), comInit(initCOM)
		{}

		CString* pValueTypes;
		ProgressTextMap* pProgressStrings;
		int valueTypeCount;
		HWND hProgressDialog;
		bool comInit;
	};

	void SendProgress(HWND hWnd, ProgressTextMap* pProgressStrings, int& count, LPCTSTR stage)
	{
		if (hWnd != NULL && pProgressStrings != nullptr && stage != nullptr) //pThreadData->hProgressDialog
		{
			auto it = pProgressStrings->find(stage);
			if (it != pProgressStrings->end())
			{
				::PostMessage(hWnd, WM_UPDATEPROGRESSBAR, (WPARAM)count++, (LPARAM)it->second);
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

		delete buffer;

		return returnValue;
	}

	void GetBatteryInfo(FTW::CWMIAccess& wmi)
	{
		bool onBattery = false;
		CString temp;
		CString systemDrive = GetEnvVar(_T("SystemDrive"));

		if(systemDrive.GetLength() > 0 && systemDrive != _T("X:") && FTW::FileExists(_T("X:\\Windows\\Inf\\Battery.inf")))
		{
				FTW::CCMLog::Instance().LogMsg(FTW::CCMLog::Info, AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
					IDS_LOGMSG_LOADINGBATTERYDRIVER);

				LPTSTR commandLine = _T("X:\\Windows\\System32\\drvload.exe X:\\Windows\\Inf\\Battery.inf");

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

		if (GetWMIPropertyFromFirstInstance(wmi, _T("Win32_Battery"), NULL, _T("Name"), temp))
		{
			if (GetWMIPropertyFromFirstInstance(wmi, _T("Win32_Battery"), NULL, _T("BatteryStatus"), temp) && temp != _T("2"))
				onBattery = true;
		}

		CTSEnv::Instance().Set(VAR_ONBATTERY, FTW::BoolString(onBattery));
	}

	void GetHardwareInfo(FTW::CWMIAccess& wmi)
	{
		CString temp;

		if (GetWMIPropertyFromFirstInstance(wmi, _T("Win32_SystemEnclosure"), NULL, _T("ChassisTypes"), temp))
		{
			if (temp == _T("8") || temp == _T("9") || temp == _T("10") || temp == _T("11") || temp == _T("12")
				|| temp == _T("14") || temp == _T("18") || temp == _T("21")
				|| temp == _T("30") || temp == _T("31") || temp == _T("32"))
				temp = VALUE_CHASSIS_LAPTOP;
			else if (temp == _T("3") || temp == _T("4") || temp == _T("5") || temp == _T("6") || temp == _T("7") || temp == _T("15") || temp == _T("16"))
				temp = VALUE_CHASSIS_DESKTOP;
			else if (temp == _T("23"))
				temp = VALUE_CHASSIS_SERVER;
			else
				temp = VALUE_UNKNOWN;

			CTSEnv::Instance().Set(VAR_CHASSISTYPE, temp);
		}

		if (GetWMIPropertyFromFirstInstance(wmi, _T("Win32_SystemEnclosure"), NULL, _T("SMBIOSAssetTag"), temp))
			CTSEnv::Instance().Set(VAR_ASSETTAG, temp);

		if (GetWMIPropertyFromFirstInstance(wmi, _T("Win32_BIOS"), NULL, _T("SerialNumber"), temp))
			CTSEnv::Instance().Set(VAR_SERIALNUMBER, temp);

		if (GetWMIPropertyFromFirstInstance(wmi, _T("Win32_ComputerSystem"), NULL, _T("Manufacturer"), temp))
			CTSEnv::Instance().Set(VAR_MANUFACTURER, temp);

		if (GetWMIPropertyFromFirstInstance(wmi, _T("Win32_ComputerSystem"), NULL, _T("Model"), temp))
			CTSEnv::Instance().Set(VAR_MODEL, temp);

		if (GetWMIPropertyFromFirstInstance(wmi, _T("Win32_BaseBoard"), NULL, _T("Product"), temp))
			CTSEnv::Instance().Set(VAR_PRODUCT, temp);

		if (GetWMIPropertyFromFirstInstance(wmi, _T("Win32_ComputerSystem"), NULL, _T("TotalPhysicalMemory"), temp))
		{
			unsigned long long val = _tstoi64(temp) / 1024 / 1024;

			CTSEnv::Instance().Set(VAR_MEMORY, (unsigned long)val);
		}

		if (GetWMIPropertyFromFirstInstance(wmi, _T("Win32_ComputerSystemProduct"), NULL, _T("UUID"), temp))
			CTSEnv::Instance().Set(VAR_UUID, temp);

		if (GetWMIPropertyFromFirstInstance(wmi, _T("Win32_Processor"), NULL, _T("DataWidth"), temp))
		{
			if (temp == _T("64"))
				CTSEnv::Instance().Set(VAR_PROCARCH, VALUE_ARCHITECTUREX64);
			else if (temp == _T("32"))
				CTSEnv::Instance().Set(VAR_PROCARCH, VALUE_ARCHITECTUREX86);
			else
				CTSEnv::Instance().Set(VAR_PROCARCH, VALUE_ARCHITECTUREOTHER);
		}
	}

	void GetProcessorInfo()
	{
		CString temp;
		SYSTEM_INFO si;
		ZeroMemory(&si, sizeof(SYSTEM_INFO));

		GetNativeSystemInfo(&si);

		switch (si.wProcessorArchitecture)
		{
		case PROCESSOR_ARCHITECTURE_AMD64:
			CTSEnv::Instance().Set(VAR_OSARCH, VALUE_ARCHITECTUREX64);
			break;
		case PROCESSOR_ARCHITECTURE_INTEL:
			CTSEnv::Instance().Set(VAR_OSARCH, VALUE_ARCHITECTUREX86);
			break;
		default:
			CTSEnv::Instance().Set(VAR_OSARCH, VALUE_ARCHITECTUREOTHER);
			break;
		}

		int cpuid_results[4];
		char cpuString[0x20];
		unsigned logical;

		try
		{
			FTW::CCMLog::Instance().LogMsg(FTW::CCMLog::Info, AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
				IDS_LOGMSG_GETTINGCPUINFO);

			ZeroMemory(cpuString, sizeof(cpuString));
			__cpuid(cpuid_results, 0);

			*((int*)cpuString) = cpuid_results[1];
			*((int*)(cpuString + 4)) = cpuid_results[3];
			*((int*)(cpuString + 8)) = cpuid_results[2];

			temp = cpuString;
			CTSEnv::Instance().Set(VAR_PROCVENDOR, temp);

			__cpuid(cpuid_results, 1);

			logical = (cpuid_results[1] >> 16) & 0xff;
			CTSEnv::Instance().Set(VAR_PROCLOGICALCOUNT, logical);

			//			__cpuid(cpuid_results, 0x80000001);

			//			if ((cpuid_results[3] & 29) == 29)
			//				CTSEnv::Instance().Set(VAR_PROCARCH, VALUE_ARCHITECTUREX64);
			//			else
			//				CTSEnv::Instance().Set(VAR_PROCARCH, VALUE_ARCHITECTUREX86);

			__cpuid(cpuid_results, 1);

			if ((cpuid_results[3] & 6) == 6)
				CTSEnv::Instance().Set(VAR_PROCPAE, TRUE_STRING);
			else
				CTSEnv::Instance().Set(VAR_PROCPAE, FALSE_STRING);

			if ((cpuid_results[3] & 20) == 20)
				CTSEnv::Instance().Set(VAR_PROCNX, TRUE_STRING);
			else
				CTSEnv::Instance().Set(VAR_PROCNX, FALSE_STRING);

			if ((cpuid_results[3] & 26) == 26)
				CTSEnv::Instance().Set(VAR_PROCSSE2, TRUE_STRING);
			else
				CTSEnv::Instance().Set(VAR_PROCSSE2, FALSE_STRING);
		}
		catch (...)
		{

		}
	}

	void GetSystemInfo(bool isPE/*FTW::CWMIAccess& wmi*/)
	{
		PVOID pBuffer = nullptr;
		DWORD size = 0;
		CString temp, query;

		DWORD result = GetFirmwareEnvironmentVariable(_T(""), _T("{00000000-0000-0000-0000-000000000000}"), pBuffer, size);

		if (result == ERROR_INVALID_FUNCTION)
			CTSEnv::Instance().Set(VAR_ISUEFI, FALSE_STRING);
		else
			CTSEnv::Instance().Set(VAR_ISUEFI, TRUE_STRING);

		CodeProject::RegKey hklm(HKEY_LOCAL_MACHINE);

		if (hklm.Open(_T("SYSTEM\\CurrentControlSet\\Control\\SecureBoot\\State"), KEY_READ) == ERROR_SUCCESS)
		{
			CString dn;
			CodeProject::RegValue dnVal = hklm[_T("UEFISecureBootEnabled")];

			if (dnVal.Type == REG_DWORD)
				CTSEnv::Instance().Set(VAR_ISSECUREBOOT, FTW::BoolString((DWORD)dnVal));
			else
				CTSEnv::Instance().Set(VAR_ISSECUREBOOT, VALUE_UNKNOWN);

			hklm.Close();
		}
		else
			CTSEnv::Instance().Set(VAR_ISSECUREBOOT, VALUE_UNKNOWN);

		if (!isPE)
		{
			FTW::CWMIAccess wmi(_T("root\\cimv2\\security\\MicrosoftVolumeEncryption"));

			CString systemDrive = GetEnvVar(_T("SystemDrive"));

			query = _T("SELECT ProtectionStatus FROM Win32_EncryptableVolume WHERE DriveLetter='");
			query += systemDrive;
			query += _T("'");

			if (QueryWMI(wmi, _T("ProtectionStatus"), query, temp))
				CTSEnv::Instance().Set(VAR_SYSTEMDRIVEBITLOCKERPROTECTED, temp);
		}

		//if (GetWMIPropertyFromFirstInstance(wmi, _T("Win32_EncryptableVolume"), NULL, _T("ProtectionStatus"), temp))
			
	}

	void GetAssetInfo(FTW::CWMIAccess& wmi, bool isPE)
	{
		CString temp;

		FTW::CCMLog::Instance().LogMsg(FTW::CCMLog::Info, AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
			IDS_LOGMSG_GETTINGASSETINFO);

		GetHardwareInfo(wmi);

		GetProcessorInfo();

		GetSystemInfo(isPE);

		GetBatteryInfo(wmi);
	}

	void GetTPMInfo(void)
	{
		CString temp;

		try
		{
			FTW::CWMIAccess wmi(_T("root\\cimv2\\Security\\MicrosoftTpm"));

			FTW::CCMLog::Instance().LogMsg(FTW::CCMLog::Info, AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
				IDS_LOGMSG_GETTINGTPMINFO);

			variant_t result;

			if (wmi.Open(_T("Win32_Tpm")))
			{
				if (wmi.InstanceCount() > 0)
				{
					CTSEnv::Instance().Set(VAR_TPMAVAILABLE, FTW::BoolString(true));

					if (wmi.ExecMethod(_T("IsEnabled"), _T("IsEnabled"), result))
						CTSEnv::Instance().Set(VAR_TPMENABLED, FTW::BoolString((bool)result == true));

					if (wmi.ExecMethod(_T("IsActivated"), _T("IsActivated"), result))
						CTSEnv::Instance().Set(VAR_TPMACTIVATED, FTW::BoolString((bool)result == true));

					if (wmi.ExecMethod(_T("IsOwned"), _T("IsOwned"), result))
						CTSEnv::Instance().Set(VAR_TPMOWNED, FTW::BoolString((bool)result == true));

					if (GetWMIPropertyFromFirstInstance(wmi, _T("Win32_TPM"), NULL, _T("SpecVersion"), temp))
						CTSEnv::Instance().Set(VAR_TPMSPECVERSION, temp);
				}
				else
				{
					CTSEnv::Instance().Set(VAR_TPMAVAILABLE, FTW::BoolString(false));
				}
			}

			wmi.Close();

		}
		catch (FTW::FTWException& i)
		{
			FTW::CCMLog::Instance().LogMsg(FTW::CCMLog::Error, AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
				IDS_LOGERROR_WMINAMESPACEOPEN, _T("root\\cimv2\\Security\\MicrosoftTpm"), i.Message());

		}

	}

	void GetWirelessNetworkInfo(void)
	{
		FTW::CCMLog::Instance().LogMsg(FTW::CCMLog::Info, AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
			IDS_LOGMSG_GETTINGWNETINFO);

		try
		{
			FTW::CCMLog::Instance().LogMsg(FTW::CCMLog::Info, AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
				IDS_LOGMSG_LOADINGWLANDLLS);

			HINSTANCE hWlanDll = LoadLibrary(_T("WlanApi.dll"));

			if (hWlanDll == NULL)
			{
				FTW::CCMLog::Instance().LogMsg(FTW::CCMLog::Info, AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
					IDS_LOGMSG_WLANNOGO, FTW::FormatHRString(HRESULT_FROM_WIN32(GetLastError())));

				CTSEnv::Instance().Set(VAR_WLANDISCONNECTED, XML_ACTION_TRUE);

				return;
			}

			WlanOpenHandleFunc WlanOpenHandle = (WlanOpenHandleFunc)GetProcAddress(hWlanDll, "WlanOpenHandle");
			WlanEnumInterfacesFunc WlanEnumInterfaces = (WlanEnumInterfacesFunc)GetProcAddress(hWlanDll, "WlanEnumInterfaces");
			WlanCloseHandleFunc WlanCloseHandle = (WlanCloseHandleFunc)GetProcAddress(hWlanDll, "WlanCloseHandle");
			WlanQueryInterfaceFunc WlanQueryInterface = (WlanQueryInterfaceFunc)GetProcAddress(hWlanDll, "WlanQueryInterface");

			if (!WlanOpenHandle || !WlanEnumInterfaces || !WlanCloseHandle || !WlanQueryInterface)
			{
				FTW::CCMLog::Instance().LogMsg(FTW::CCMLog::Info, AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
					IDS_LOGMSG_WLANNOGO, FTW::FormatHRString(HRESULT_FROM_WIN32(GetLastError())));

				CTSEnv::Instance().Set(VAR_WLANDISCONNECTED, XML_ACTION_TRUE);

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

				FTW::CCMLog::Instance().LogMsg(FTW::CCMLog::Info, AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
					IDS_LOGMSG_LOADINGWLANINFO);

				DWORD result = (*WlanOpenHandle)(maxClientVer, NULL, &currentClientVersion, &clientHandle);

				if (result == ERROR_SUCCESS)
				{
					result = (*WlanEnumInterfaces)(clientHandle, NULL, &pInterfaceInfoList);

					if (result == ERROR_SUCCESS)
					{
						for (int interfaceIndex = 0; interfaceIndex < (int)pInterfaceInfoList->dwNumberOfItems; interfaceIndex++)
						{
							pInterfaceInfo = (PWLAN_INTERFACE_INFO)&pInterfaceInfoList->InterfaceInfo[interfaceIndex];

							switch (pInterfaceInfo->isState)
							{
							case wlan_interface_state_not_ready:
								wlanState.LoadString(IDS_WLANSTATE_NOTREADY);
								break;
							case wlan_interface_state_connected:
								wlanState.LoadString(IDS_WLANSTATE_CONNECTED);
								break;
							case wlan_interface_state_ad_hoc_network_formed:
								wlanState.LoadString(IDS_WLANSTATE_ADHOC);
								break;
							case wlan_interface_state_disconnecting:
								wlanState.LoadString(IDS_WLANSTATE_DISCONNECTING);
								break;
							case wlan_interface_state_disconnected:
								wlanState.LoadString(IDS_WLANSTATE_DISCONNECTED);
								break;
							case wlan_interface_state_associating:
								wlanState.LoadString(IDS_WLANSTATE_ASSOCIATING);
								break;
							case wlan_interface_state_discovering:
								wlanState.LoadString(IDS_WLANSTATE_DISCOVERING);
								break;
							case wlan_interface_state_authenticating:
								wlanState.LoadString(IDS_WLANSTATE_AUTHENTICATING);
								break;
							default:
								wlanState.LoadString(IDS_WLANSTATE_UNKNOWN);
								break;
							}

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

							FTW::CCMLog::Instance().LogMsg(FTW::CCMLog::Info, AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
								IDS_LOGMSG_FOUNDWLAN_INTERFACE, pInterfaceInfo->strInterfaceDescription, wlanState);
						}
					}

				}

				result = (*WlanCloseHandle)(clientHandle, NULL);

				if (wlanDisconnected == true)
				{
					CTSEnv::Instance().Set(VAR_WLANDISCONNECTED, XML_ACTION_TRUE);
				}
				else
				{
					CTSEnv::Instance().Set(VAR_WLANDISCONNECTED, XML_ACTION_FALSE);
					CTSEnv::Instance().Set(VAR_WLANSSID, SSID);

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

	void GetNetworkInfo(FTW::CWMIAccess& wmi)
	{
		GetWirelessNetworkInfo();

		FTW::CCMLog::Instance().LogMsg(FTW::CCMLog::Info, AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
			IDS_LOGMSG_GETTINGNETINFO);

		try
		{
			variant_t result;
			CString temp;

			if (QueryWMI(wmi, _T("DefaultIPGateway"), _T("SELECT DefaultIPGateway FROM Win32_NetworkAdapterConfiguration WHERE IPEnabled='true'"), temp))
				CTSEnv::Instance().Set(VAR_IPGATEWAY, temp);

			if (QueryWMI(wmi, _T("IPAddress"), _T("SELECT IPAddress FROM Win32_NetworkAdapterConfiguration WHERE IPEnabled='true'"), temp))
				CTSEnv::Instance().Set(VAR_IPADDRESS, temp);

			if (QueryWMI(wmi, _T("IPSubnet"), _T("SELECT IPSubnet FROM Win32_NetworkAdapterConfiguration WHERE IPEnabled='true'"), temp))
				CTSEnv::Instance().Set(VAR_IPSUBNETMASK, temp);

			if (QueryWMI(wmi, _T("MACAddress"), _T("SELECT MACAddress FROM Win32_NetworkAdapterConfiguration WHERE IPEnabled='true'"), temp))
				CTSEnv::Instance().Set(VAR_MACADDRESS, temp);

			if (QueryWMI(wmi, _T("Name"), _T("SELECT Name FROM Win32_NetworkAdapter WHERE NetConnectionStatus=2 And NetEnabled='True' And AdapterType='Ethernet 802.3' And PhysicalAdapter='True'"), temp))
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
						query.Format(_T("SELECT InstanceName FROM MSNdis_PhysicalMediumType WHERE InstanceName='%s' And NDisPhysicalMediumType=0"), name);

						if(QueryWMI(wmi2, _T("InstanceName"), query, wiredNICName))
						{
							wiredLANConnected = true;

							FTW::CCMLog::Instance().LogMsg(FTW::CCMLog::Info, AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
								IDS_LOGMSG_FOUNDWIREDLAN_INTERFACE, wiredNICName);

							break;
						}
					}

					name = temp.Tokenize(_T(","), pos);
				}

				CTSEnv::Instance().Set(VAR_WIREDLANCONNECTED, wiredLANConnected ? XML_ACTION_TRUE : XML_ACTION_FALSE);
			}

		}
		catch (FTW::FTWException& i)
		{
			FTW::CCMLog::Instance().LogMsg(FTW::CCMLog::Error, AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
				IDS_LOGERROR_WMINAMESPACEOPEN, _T("root\\cimv2\\Win32_NetworkAdapterConfiguration"), i.Message());

		}
	}

	void GetUserInfo(void)
	{
		LPTSTR pName = nullptr;
		ULONG size = 0;

		if (GetUserNameExW(NameDisplay, pName, &size) == 0 && GetLastError() == ERROR_MORE_DATA)
		{
			pName = new TCHAR[size + 1];

			if (GetUserNameExW(NameDisplay, pName, &size) != 0)
			{
				CTSEnv::Instance().Set(VAR_USERDISPLAYNAME, pName);
			}

			delete pName;
		}

		if (GetUserNameExW(NameSamCompatible, pName, &size) == 0 && GetLastError() == ERROR_MORE_DATA)
		{
			pName = new TCHAR[size + 1];

			if (GetUserNameExW(NameSamCompatible, pName, &size) != 0)
			{
				CTSEnv::Instance().Set(VAR_USERSAMACCOUNTNAME, pName);
			}
			
			delete pName;
		}

		if (GetUserNameExW(NameSamCompatible, pName, &size) == 0 && GetLastError() == ERROR_MORE_DATA)
		{
			pName = new TCHAR[size + 1];

			if (GetUserNameExW(NameUserPrincipal, pName, &size) != 0)
			{
				CTSEnv::Instance().Set(VAR_USERPRINCIPALNAME, pName);
			}

			delete pName;
		}

	}
	
	void GetPendingRebootInfo(void)
	{
		CodeProject::RegKey hklm(HKEY_LOCAL_MACHINE);
		bool cbsPendingReboot = false;
		bool wuaPendingReboot = false;
		bool pfro = false;
		bool renamePendingReboot = false;
		bool ccmRebootPending = false;
		bool rebootPending = false;

		FTW::CCMLog::Instance().LogMsg(FTW::CCMLog::Info, AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
			IDS_LOGMSG_GETTINGREBOOTPENDINGINFO);

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

				CString computerName = regCompterName;
				
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

				CTSEnv::Instance().Set(VAR_CCMREBOOTPENDING, FTW::BoolString(ccmRebootPending));
			}

			wmi.Close();

		}
		catch (FTW::FTWException& i)
		{
		}

		rebootPending = cbsPendingReboot || wuaPendingReboot || pfro || renamePendingReboot;

		CTSEnv::Instance().Set(VAR_OSCBSREBOOTPENDING, FTW::BoolString(cbsPendingReboot));
		CTSEnv::Instance().Set(VAR_OSWUAREBOOTPENDING, FTW::BoolString(wuaPendingReboot));
		CTSEnv::Instance().Set(VAR_OSPFRO, FTW::BoolString(pfro));
		CTSEnv::Instance().Set(VAR_OSPENDINGRENAME, FTW::BoolString(renamePendingReboot));
		CTSEnv::Instance().Set(VAR_REBOOTPENDING, FTW::BoolString(rebootPending));

	}

	void GetOSInfo(FTW::CWMIAccess& wmi, bool isPE)
	{
		CString temp;
		bool serverCoreOS = false;
		//DWORD productType;
		TCHAR path[MAX_STRING_LENGTH];

		//OSVERSIONINFOEX osInfo;

		FTW::CCMLog::Instance().LogMsg(FTW::CCMLog::Info, AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
			IDS_LOGMSG_GETTINGOSINFO);

		if (GetWMIPropertyFromFirstInstance(wmi, _T("Win32_OperatingSystem"), NULL, _T("BuildNumber"), temp))
			CTSEnv::Instance().Set(VAR_OSBUILD, temp);

		if (GetWMIPropertyFromFirstInstance(wmi, _T("Win32_OperatingSystem"), NULL, _T("Version"), temp))
			CTSEnv::Instance().Set(VAR_OSVERSION, temp);

		if (GetWMIPropertyFromFirstInstance(wmi, _T("Win32_OperatingSystem"), NULL, _T("ServicePackMajorVersion"), temp))
			CTSEnv::Instance().Set(VAR_OSSERVICEPACK, temp);

		if (GetWMIPropertyFromFirstInstance(wmi, _T("Win32_OperatingSystem"), NULL, _T("ProductType"), temp))
		{
			int prodType = _ttoi(temp);

			switch (prodType)
			{
			case VER_NT_WORKSTATION:
				CTSEnv::Instance().Set(VAR_OSPRODUCT, VALUE_OSWORKSTATION);
				break;
			case VER_NT_SERVER:
				CTSEnv::Instance().Set(VAR_OSPRODUCT, VALUE_OSSERVER);
				break;
			case VER_NT_DOMAIN_CONTROLLER:
				CTSEnv::Instance().Set(VAR_OSPRODUCT, VALUE_OSSDOMAINCONTROLLER);
				break;
			default:
				CTSEnv::Instance().Set(VAR_OSPRODUCT, VALUE_UNKNOWN);
			}
		}

		CTSEnv::Instance().Set(VAR_OSSYSTEMDRIVE, GetEnvVar(_T("SystemDrive")));

		/*ZeroMemory(&osInfo, sizeof(OSVERSIONINFOEX));

		osInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

		if (GetVersionEx((LPOSVERSIONINFO)(&osInfo)))
		{
			temp.Format(_T("%u.%u"), osInfo.dwMajorVersion, osInfo.dwMinorVersion);
			CTSEnv::Instance().Set(VAR_OSVERSION, temp);

			temp.Format(_T("%u"), osInfo.dwBuildNumber);
			CTSEnv::Instance().Set(VAR_OSBUILD, temp);

			temp.Format(_T("%u"), osInfo.wServicePackMajor);
			CTSEnv::Instance().Set(VAR_OSSERVICEPACK, temp);

			switch (osInfo.wProductType)
			{
			case VER_NT_WORKSTATION:
				CTSEnv::Instance().Set(VAR_OSPRODUCT, VALUE_OSWORKSTATION);
				break;
			case VER_NT_SERVER:
				CTSEnv::Instance().Set(VAR_OSPRODUCT, VALUE_OSSERVER);
				break;
			case VER_NT_DOMAIN_CONTROLLER:
				CTSEnv::Instance().Set(VAR_OSPRODUCT, VALUE_OSSDOMAINCONTROLLER);
				break;
			default:
				CTSEnv::Instance().Set(VAR_OSPRODUCT, VALUE_UNKNOWN);
			}

		} */

		if (!isPE && ExpandEnvironmentStrings(_T("%windir%\\Explorer.exe"), path, MAX_STRING_LENGTH) > 0 && !FTW::FileExists(path))
		{
			serverCoreOS = true;
		}

		CTSEnv::Instance().Set(VAR_ISSERVERCOREOS, FTW::BoolString(serverCoreOS));
		CTSEnv::Instance().Set(VAR_ISWINPE, FTW::BoolString(isPE));

		if (!isPE)
		{
			GetPendingRebootInfo();
			GetUserInfo();
		}

	}

	void GetSecurityInfo(bool isPE)
	{
		FTW::CCMLog::Instance().LogMsg(FTW::CCMLog::Info, AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
			IDS_LOGMSG_GETTINGSECURITYINFO);
	}

	void GetDomainInfo(FTW::CWMIAccess& wmi)
	{
		CString temp;

		DWORD level = 100;
		LPWKSTA_INFO_100 pBuf = NULL;

		FTW::CCMLog::Instance().LogMsg(FTW::CCMLog::Info, AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
			IDS_LOGMSG_GETTINGDOMAININFO);

		NET_API_STATUS status = NetWkstaGetInfo(NULL, level, (LPBYTE*)&pBuf);

		if (status == NERR_Success)
		{
			temp.Format(_T("%s"), pBuf->wki100_langroup);
			CTSEnv::Instance().Set(VAR_CURRENTCOMPUTERDOMAIN, temp);

			temp.Format(_T("%s"), pBuf->wki100_computername);
			CTSEnv::Instance().Set(VAR_CURRENTCOMPUTERNAME, temp);
		}

		if (pBuf != NULL)
			NetApiBufferFree(pBuf);

		if (GetWMIPropertyFromFirstInstance(wmi, _T("Win32_ComputerSystem"), NULL, _T("PartOfDomain"), temp))
			CTSEnv::Instance().Set(VAR_CURRENTCOMPUTERJOINEDTODOMAIN, temp);

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
					CTSEnv::Instance().Set(VAR_CURRENTCOMPUTEROUDN, dn);
			}

			hklm.Close();
		}

	}

	void GetVirtualizationInfo(FTW::CWMIAccess& wmi)
	{
		FTW::CCMLog::Instance().LogMsg(FTW::CCMLog::Info, AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
			IDS_LOGMSG_GETTINGVIRTUALINFO);

		CString model = CTSEnv::Instance().Get(VAR_MODEL);
		CString make = CTSEnv::Instance().Get(VAR_MANUFACTURER);

		if (model == VALUE_MODEL_VM)
		{
			CTSEnv::Instance().Set(VAR_CHASSISTYPE, VALUE_CHASSIS_VIRTUAL);

			CString biosVersion;

			if (GetWMIPropertyFromFirstInstance(wmi, _T("Win32_BIOS"), NULL, _T("Version"), biosVersion))
			{
				if (biosVersion.Find(VALUE_BIOS_VERSION) == 0)
					CTSEnv::Instance().Set(VAR_VMTYPE, VALUE_VMTYPE_HYPERV);
				else
					CTSEnv::Instance().Set(VAR_VMTYPE, VALUE_VMTYPE_OTHERMS);
			}
			else
				CTSEnv::Instance().Set(VAR_VMTYPE, VALUE_VMTYPE_OTHERMS);
		}
		else if (model == VALUE_MODEL_VMWAREESX)
		{
			CTSEnv::Instance().Set(VAR_CHASSISTYPE, VALUE_CHASSIS_VIRTUAL);
			CTSEnv::Instance().Set(VAR_VMTYPE, VALUE_VMTYPE_VMWAREESX);
		}
		else if (model.Find(VALUE_MODEL_VMWARE) == 0)
		{
			CTSEnv::Instance().Set(VAR_CHASSISTYPE, VALUE_CHASSIS_VIRTUAL);
			CTSEnv::Instance().Set(VAR_VMTYPE, VALUE_VMTYPE_VMWARE);
		}
		else if (model == VALUE_MODEL_VIRTUALBOX)
		{
			CTSEnv::Instance().Set(VAR_CHASSISTYPE, VALUE_CHASSIS_VIRTUAL);
			CTSEnv::Instance().Set(VAR_VMTYPE, VALUE_VMTYPE_VIRTUALBOX);
		}
		else if (make == VALUE_MAKE_XEN)
		{
			CTSEnv::Instance().Set(VAR_CHASSISTYPE, VALUE_CHASSIS_VIRTUAL);
			CTSEnv::Instance().Set(VAR_VMTYPE, VALUE_VMTYPE_XEN);
		}
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

			try
			{
				int valueCount = 0;
				CString progressText;
				bool isPE = false;

				TRACE0("Starting default values thread\n");

				CodeProject::RegKey hklm(HKEY_LOCAL_MACHINE);

				if (hklm.Open(_T("System\\CurrentControlSet\\Control\\MiniNT"), KEY_READ) == ERROR_SUCCESS)
				{
					isPE = true;

					hklm.Close();
				}

				FTW::CWMIAccess wmi(_T("root\\cimv2"));

				bool getAll = (pThreadData->pValueTypes->Find(XML_ATTRIBUTE_DEFAULTVALUE_ALL) > -1);

				if (getAll || pThreadData->pValueTypes->Find(XML_ATTRIBUTE_DEFAULTVALUE_OS) > -1)
				{
					TRACE0("Getting OS Info\n");

					SendProgress(pThreadData->hProgressDialog,
						pThreadData->pProgressStrings, valueCount, XML_ATTRIBUTE_DEFAULTVALUE_OS);

					GetOSInfo(wmi, isPE);
				}

				if (getAll || pThreadData->pValueTypes->Find(XML_ATTRIBUTE_DEFAULTVALUE_ASSET) > -1)
				{
					TRACE0("Getting Asset Info\n");

					SendProgress(pThreadData->hProgressDialog,
						pThreadData->pProgressStrings, valueCount, XML_ATTRIBUTE_DEFAULTVALUE_ASSET);

					GetAssetInfo(wmi, isPE);

					TRACE0("Getting Virtualization Info\n");

					SendProgress(pThreadData->hProgressDialog,
						pThreadData->pProgressStrings, valueCount, XML_ATTRIBUTE_DEFAULTVALUE_VM);

					GetVirtualizationInfo(wmi);
				}

				if (getAll || pThreadData->pValueTypes->Find(XML_ATTRIBUTE_DEFAULTVALUE_DOM) > -1)
				{
					TRACE0("Getting Domain Info\n");

					SendProgress(pThreadData->hProgressDialog,
						pThreadData->pProgressStrings, valueCount, XML_ATTRIBUTE_DEFAULTVALUE_DOM);

					GetDomainInfo(wmi);
				}

				if (getAll || pThreadData->pValueTypes->Find(XML_ATTRIBUTE_DEFAULTVALUE_NET) > -1)
				{
					TRACE0("Getting Network Info\n");

					SendProgress(pThreadData->hProgressDialog,
						pThreadData->pProgressStrings, valueCount, XML_ATTRIBUTE_DEFAULTVALUE_NET);

					GetNetworkInfo(wmi);
				}

				if (getAll || pThreadData->pValueTypes->Find(XML_ATTRIBUTE_DEFAULTVALUE_TPM) > -1)
				{
					TRACE0("Getting TPM Info\n");

					SendProgress(pThreadData->hProgressDialog,
						pThreadData->pProgressStrings, valueCount, XML_ATTRIBUTE_DEFAULTVALUE_TPM);

					GetTPMInfo();
				}

				if (getAll || pThreadData->pValueTypes->Find(XML_ATTRIBUTE_DEFAULTVALUE_SECURITY) > -1)
				{
					TRACE0("Getting Security Info\n");

					SendProgress(pThreadData->hProgressDialog,
						pThreadData->pProgressStrings, valueCount, XML_ATTRIBUTE_DEFAULTVALUE_SECURITY);

					GetSecurityInfo(isPE);
				}

				::PostMessage(pThreadData->hProgressDialog, WM_UPDATEPROGRESSBAR, (WPARAM)valueCount, (LPARAM)0);

				wmi.Close();

			}
			catch (FTW::FTWException& i)
			{
				FTW::CCMLog::Instance().LogMsg(FTW::CCMLog::Error, AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
					IDS_LOGERROR_WMINAMESPACEOPEN, _T("root\\cimv2"), i.Message());

			}
		}
		else if(pThreadData->comInit == true)
		{
			FTW::CCMLog::Instance().LogMsg(FTW::CCMLog::Error,
				AfxGetThread()->m_nThreadID,
				__TFILE__,
				__LINE__,
				IDS_LOGERROR_COM);
		}
		::PostMessage(pThreadData->hProgressDialog, WM_CLOSEPROGRESSBAR, (WPARAM)0, (LPARAM)0);

		return ERROR_SUCCESS;
	}

	INT_PTR CDefaultValsAction::Go(void)
	{

		FTW::CCMLog::Instance().LogMsg(FTW::CCMLog::Info, AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
			IDS_LOGMSG_POPULATEDEFAULT);

		CString valueTypes = GetXMLAttribute(m_actionData.pActionNode, XML_ATTRIBUTE_DEFAULTVALUE_TYPES, XML_ATTRIBUTE_DEFAULTVALUE_ALL);
		CString titleText = GetXMLAttribute(m_actionData.pActionNode, XML_ATTRIBUTE_TITLE, XML_ATTRIBUTE_TITLE_PROGRESS_DEF);
		bool showProgress = FTW::IsTrue(GetXMLAttribute(m_actionData.pActionNode, XML_ATTRIBUTE_DEFAULTVALUE_SHOWPROGRESS, XML_ACTION_TRUE));
		ProgressTextMap progressStrings;

		if (showProgress)
		{
			CString* pValue = nullptr;
			CString type;

			progressStrings.insert(ProgressTextMap::value_type(XML_ATTRIBUTE_DEFAULTVALUE_OS, new CString(XML_DEFAULTVALUE_OS_PROGRESSTEXT)));
			progressStrings.insert(ProgressTextMap::value_type(XML_ATTRIBUTE_DEFAULTVALUE_TPM, new CString(XML_DEFAULTVALUE_TPM_PROGRESSTEXT)));
			progressStrings.insert(ProgressTextMap::value_type(XML_ATTRIBUTE_DEFAULTVALUE_NET, new CString(XML_DEFAULTVALUE_NET_PROGRESSTEXT)));
			progressStrings.insert(ProgressTextMap::value_type(XML_ATTRIBUTE_DEFAULTVALUE_VM, new CString(XML_DEFAULTVALUE_VM_PROGRESSTEXT)));
			progressStrings.insert(ProgressTextMap::value_type(XML_ATTRIBUTE_DEFAULTVALUE_DOM, new CString(XML_DEFAULTVALUE_DOM_PROGRESSTEXT)));
			progressStrings.insert(ProgressTextMap::value_type(XML_ATTRIBUTE_DEFAULTVALUE_ASSET, new CString(XML_DEFAULTVALUE_ASSET_PROGRESSTEXT)));
			progressStrings.insert(ProgressTextMap::value_type(XML_ATTRIBUTE_DEFAULTVALUE_SECURITY, new CString(XML_DEFAULTVALUE_SECURITY_PROGRESSTEXT)));

			for (pugi::xml_node xmlinput = m_actionData.pActionNode->first_child(); xmlinput; xmlinput = xmlinput.next_sibling())
			{

				if (_tcsicmp(xmlinput.name(), XML_ACTION_DEFAULT_TEXT) != 0)
					continue;

				type = GetXMLAttribute(&xmlinput, XML_ATTRIBUTE_TYPE);

				if (EvalCondition(xmlinput.attribute(XML_ATTRIBUTE_CONDITION).value(), m_actionData.pScriptHost, xmlinput.name(), type))
				{
					if (type == XML_ATTRIBUTE_DEFAULTVALUE_OS)
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
						pValue = new CString(GetXMLAttribute(&xmlinput, XML_ATTRIBUTE_VALUE, XML_DEFAULTVALUE_SECURITY_PROGRESSTEXT));

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

			int valueTypeCount = 0;

			if (valueTypes.Find(XML_ATTRIBUTE_DEFAULTVALUE_ALL) > -1)
				valueTypeCount = 7;

			else
			{
				if (valueTypes.Find(XML_ATTRIBUTE_DEFAULTVALUE_OS) > -1)
					valueTypeCount++;

				if (valueTypes.Find(XML_ATTRIBUTE_DEFAULTVALUE_ASSET) > -1)
					valueTypeCount += 2;

				if (valueTypes.Find(XML_ATTRIBUTE_DEFAULTVALUE_DOM) > -1)
					valueTypeCount++;

				if (valueTypes.Find(XML_ATTRIBUTE_DEFAULTVALUE_NET) > -1)
					valueTypeCount++;

				if (valueTypes.Find(XML_ATTRIBUTE_DEFAULTVALUE_TPM) > -1)
					valueTypeCount++;

				if (valueTypes.Find(XML_ATTRIBUTE_DEFAULTVALUE_SECURITY) > -1)
					valueTypeCount++;
			}

			CUIProgressDlg* pDlg = new CUIProgressDlg(valueTypeCount, titleText);

			pDlg->Create(IDD_UIPROGRESSDLG);

			pDlg->ShowWindow(SW_SHOW);

			DefaultValueThreadData threadData(&valueTypes, valueTypeCount, pDlg->GetSafeHwnd(), &progressStrings, true);

			AfxBeginThread(GetDefaulValues, static_cast<LPVOID>(&threadData));
			//GetDefaulValues((LPVOID)&threadData);

			int returnValue = pDlg->RunModalLoop();

			delete pDlg;

			for (auto it = progressStrings.begin(); it != progressStrings.end(); ++it)
			{
				if (it->second != nullptr)
					delete it->second;
			}

			return ERROR_SUCCESS;// returnValue;
		}
		else
		{
			DefaultValueThreadData threadData(&valueTypes, 0, NULL, nullptr);
			return GetDefaulValues((LPVOID)&threadData);
		}


	}

	INT_PTR CTPMAction::Go(void)
	{
		FTW::CCMLog::Instance().LogMsg(FTW::CCMLog::Info, AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
			IDS_LOGMSG_INTIATETPM);

		int request = GetXMLIntAttribute(m_actionData.pActionNode, XML_ATTRIBUTE_TPM_REQUEST, -1);

		if (request == -1)
			FTW::CCMLog::Instance().LogMsg(FTW::CCMLog::Info, AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
				IDS_LOGMSG_TPM_NOREQUEST);
		else
		{
			FTW::CCMLog::Instance().LogMsg(FTW::CCMLog::Info, AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
				IDS_LOGMSG_TPM_REQUEST, request);


		}

		return ERROR_SUCCESS;
	}

	CDefaultValues::CDefaultValues()
	{

	}
	void CDefaultValues::operator()(const wchar_t* valueType)
	{

	}
}