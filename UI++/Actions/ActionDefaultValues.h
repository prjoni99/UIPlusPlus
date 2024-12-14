#pragma once

#define MAX_KEY_LENGTH 255

namespace UIpp 
{
	typedef std::unordered_map<CString, CString*> ProgressTextMap;
	typedef std::unordered_map<CString, CString*>::iterator ProgressTextMapIterator;
	typedef std::pair<CString, CString*> ProgressTextMapPair;

	class CDefaultValues;

	typedef void (CDefaultValues::*DefaultValuesFunc)(void);
	typedef std::unordered_map<CString, DefaultValuesFunc> DefaultValuesFunctions;
	typedef std::unordered_map<CString, DefaultValuesFunc>::iterator DefaultValuesFunctionsIterator;

	struct DefaultValueThreadData
	{
		DefaultValueThreadData(CString* pValueT, int count, HWND hDlg, ProgressTextMap* pStrings, FTWCMLOG::ICMLogPtr pLog, bool initCOM = false)
			: pValueTypes(pValueT), valueTypeCount(count),
			hProgressDialog(hDlg), pProgressStrings(pStrings), 
			pCMLog(pLog), comInit(initCOM)
		{}

		CString* pValueTypes;
		ProgressTextMap* pProgressStrings;
		int valueTypeCount;
		HWND hProgressDialog;
		bool comInit;
		FTWCMLOG::ICMLogPtr pCMLog;
	};

	class CDefaultValues
	{
	public:

		CDefaultValues(DefaultValueThreadData* pThreadData, bool all);
		//void operator()(ProgressTextMapPair text);
		void Execute(ProgressTextMapPair text);

	private:
		void GetOSInfo(void);
		void GetAssetInfo(void);
		void GetVirtualizationInfo(void);
		void GetDomainInfo(void);
		void GetNetworkInfo(void);
		void GetTPMInfo(void);
		void GetSecurityInfo(void);
		bool GetAzureADInfo(bool aadJoin);
		void GetUserInfo(void);
		void GetMgmtInfo(void);
		void GetDiskInfo(void);

		void GetPendingRebootInfo(void);
		void GetBatteryInfo(void);
		void GetHardwareInfo(void);
		void GetProcessorInfo(void);
		void GetSystemInfo(void);
		void GetWirelessNetworkInfo(void);
		void GetFirewallInfo(void);
		void GetWindowsUpdateInfo(void);
		void GetWindowsDefenderInfo(void);

		bool GetServiceInfo(PCTSTR pServiceName);

		bool m_isPE;
		bool m_allValues;
		bool m_chassisTypeSettoVM = false;
		int m_count;
		FTW::CWMIAccess m_wmi;
		DefaultValueThreadData* m_pThreadData;
		DefaultValuesFunctions functionDispatcher;
	};

}