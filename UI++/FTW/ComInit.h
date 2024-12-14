#pragma once
namespace FTW
{

	class ComInit
	{
	public:
		ComInit(bool init = true) : m_hrInit(CO_E_NOTINITIALIZED) { if (init) Init(); }
		~ComInit()
		{
			if (m_hrInit == S_OK || m_hrInit == S_FALSE)
			{
				::CoUninitialize();
				TRACE("INFO: COM uninitizalized\n"); 
			}
		}

		static HRESULT InitSecurity(void) { return ::CoInitializeSecurity(NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_DEFAULT, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE, NULL); }

		static ComInit& Instance()
		{
			static ComInit _COM;
			return _COM;
		}

		HRESULT InitStatus(void) { return m_hrInit; }
		HRESULT Init(void)
		{
			if (m_hrInit == CO_E_NOTINITIALIZED)
				m_hrInit = ::CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

			if (m_hrInit == S_OK)
				TRACE("INFO: COM initizalized\n"); 

			return m_hrInit;
		}

	private:
		HRESULT m_hrInit;

	};
}