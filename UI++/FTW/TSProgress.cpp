#include "stdafx.h"
#include "TSProgress.h"


CTSProgress::CTSProgress(void)
	: m_pTSProgress(0)
{

	try
	{
		//Try to retrieve an instance of the Microsoft.SMS.TSEnvironment class
		HRESULT hr = m_pTSProgress.CreateInstance (ProgressUILib::CLSID_ProgressUI);

		if (hr == S_OK)
		{
			
			TRACE("INFO: Created Microsoft.SMS.TsProgressUI\n");

		}
		else
		{
			TRACE("INFO: Unable to create Microsoft.SMS.TsProgressUI\n");
			m_pTSProgress = 0;
		}

	}
	catch (_com_error ce)
	{
		TRACE("INFO: Unable to create Microsoft.SMS.TsProgressUI\n");
		m_pTSProgress = 0;
	}

}


CTSProgress::~CTSProgress(void)
{
}
