#pragma once

#import "TSProgressUI.exe" named_guids

class CTSProgress
{
public:
	CTSProgress(void);
	~CTSProgress(void);

	inline static CTSProgress& Instance()
	{
		static CTSProgress _TSProgress;
		return _TSProgress;
	}

	void Close(void)
	{
		if(m_pTSProgress)
			m_pTSProgress->CloseProgressDialog();
	}

private:
	CTSProgress operator=(CTSProgress const&);
	CTSProgress (CTSProgress const&);

	ProgressUILib::IProgressUIPtr m_pTSProgress;
};

