#pragma once

#include <activscp.h>

namespace FTW {
//class IActiveScriptHost : public IUnknown {
//
//public:
//
//	// IUnknown
//	virtual ULONG __stdcall AddRef(void) = 0;
//	virtual ULONG __stdcall Release(void) = 0;
//	virtual HRESULT __stdcall QueryInterface(REFIID iid, void **obj) = 0;
//
//	// IActiveScriptHost
//	//virtual HRESULT __stdcall Eval(const _TCHAR *source, VARIANT *result) = 0;
//	//virtual HRESULT __stdcall Inject(const _TCHAR *name, IUnknown *unkn) = 0;
//
//};

class CScriptSite : public IActiveScriptSite
{
private:

	LONG _ref;

public:
	CScriptSite(void) : _ref(1) {};
	virtual ~CScriptSite(void) {};

	// IUnknown
	virtual ULONG __stdcall AddRef(void);
	virtual ULONG __stdcall Release(void);
	virtual HRESULT __stdcall QueryInterface(REFIID iid, void **obj);

	// IActiveScriptSite
	virtual HRESULT __stdcall GetLCID(LCID *lcid) { return E_NOTIMPL;} ;
	virtual HRESULT __stdcall GetItemInfo(LPCOLESTR name,
		DWORD returnMask,
		IUnknown **item,
		ITypeInfo **typeInfo) { return E_NOTIMPL;} ;
	virtual HRESULT __stdcall GetDocVersionString(BSTR *versionString) { return E_NOTIMPL;} ;
	virtual HRESULT __stdcall OnScriptTerminate(const VARIANT *result,
		const EXCEPINFO *exceptionInfo) { return E_NOTIMPL;} ;
	virtual HRESULT __stdcall OnStateChange(SCRIPTSTATE state) { return E_NOTIMPL;} ;
	virtual HRESULT __stdcall OnEnterScript(void) { return E_NOTIMPL;} ;
	virtual HRESULT __stdcall OnLeaveScript(void) { return E_NOTIMPL;} ;
	virtual HRESULT __stdcall OnScriptError(IActiveScriptError *error) { return S_OK;} ;
};


class CScriptHost
{
public:
	CScriptHost(void);
	~CScriptHost(void);

	HRESULT __stdcall Eval(const _TCHAR *source, VARIANT *result);
	
	IActiveScript* m_activeScript;
	IActiveScriptParse* m_activeScriptParse;
	IActiveScriptSite* m_activeScriptSite;

};

}