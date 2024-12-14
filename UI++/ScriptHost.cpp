#include "StdAfx.h"
#include "ScriptHost.h"
#include "FTW\FTWError.h"

namespace FTW {

CScriptHost::CScriptHost(void) : m_activeScriptParse (0), m_activeScriptSite(0), m_activeScript(0)
{
	GUID CLSID_JScript  = {0xf414c260, 0x6ac0, 0x11cf, {0xb6, 0xd1, 0x00, 0xaa, 0x00, 0xbb, 0xbb, 0x58}};
	GUID CLSID_VBScript = {0xb54f3741, 0x5b07, 0x11cf, {0xa4, 0xb0, 0x0, 0xaa, 0x0, 0x4a, 0x55, 0xe8}};
	HRESULT hr = S_OK;

	hr = CoCreateInstance(CLSID_VBScript,
		NULL,
		CLSCTX_INPROC_SERVER,
		IID_IActiveScript,
		(void **)&m_activeScript);

	if (SUCCEEDED(hr))
	{
		hr = m_activeScript->QueryInterface(IID_IActiveScriptParse, (void **)&m_activeScriptParse);
	
		if (SUCCEEDED(hr))
		{
			m_activeScriptSite = new CScriptSite;
	
			if (m_activeScriptSite == NULL) 
				hr = E_OUTOFMEMORY;
			else
				hr = m_activeScript->SetScriptSite(m_activeScriptSite);
		}
	}

	if (FAILED(hr)) 
	{
		if (m_activeScriptParse)
			m_activeScriptParse->Release();
		if (m_activeScript)
			m_activeScript->Release();
		if (m_activeScriptSite)
		{
			m_activeScriptSite->Release();
			delete m_activeScriptSite;
		}

		throw FTWErrorCodeException(hr, FTWException::classification::Error);
	}
}


CScriptHost::~CScriptHost(void)
{
	if (m_activeScriptParse)
		m_activeScriptParse->Release();
	if (m_activeScript)
		m_activeScript->Release();
	if (m_activeScriptSite)
	{
		m_activeScriptSite->Release();
//		delete m_activeScriptSite;
	}
}

HRESULT __stdcall CScriptHost::Eval(const _TCHAR *source, VARIANT *result)
{
    ASSERT(source != NULL);

    if (source == NULL)
        return E_POINTER;

	setlocale(LC_ALL, "");

    return m_activeScriptParse->ParseScriptText(source, NULL, NULL, NULL, 0, 1, SCRIPTTEXT_ISEXPRESSION, result, NULL);
}

ULONG __stdcall CScriptSite::AddRef(void)
{
    return (ULONG)InterlockedIncrement(&_ref);
}

ULONG __stdcall CScriptSite::Release(void)
{
    LONG ref = InterlockedDecrement(&_ref);
    if (ref == 0)
        delete this;

    return ref;
}

HRESULT __stdcall CScriptSite::QueryInterface(REFIID iid, void **obj)
{
    *obj = NULL;
    if (IsEqualIID(iid, IID_IUnknown) == TRUE)
        *obj = (IUnknown *)this;
    else if (IsEqualIID(iid, IID_IActiveScriptSite) == TRUE)
        *obj = (IActiveScriptSite *)this;
    else
        return E_NOINTERFACE;

    AddRef();
    return S_OK;
}

}