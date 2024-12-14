#include "stdafx.h"
#include "WMIAccess.h"
#include "FTWError.h"

#define MAX_WMI_STRING_LENGTH 256

namespace FTW
{

CWMIAccess::CWMIAccess(PCTSTR pNamespaceName) :
	m_pLocator(0), 
	m_pService(0), 
	//m_pInstance(0),
	m_pClass(0),
	//m_pEnum(0),
	m_namespaceName(_T("")), 
	m_className(_T("")),
	m_key(_T(""))
{
	if (pNamespaceName && _tcsnlen(pNamespaceName, MAX_WMI_STRING_LENGTH) > 0)
	{
		Connect(pNamespaceName);
	}
}

CWMIAccess::CWMIAccess(const CWMIAccess& w)
{
	m_namespaceName = w.m_namespaceName; 
	m_className = w.m_className;
}

CWMIAccess& CWMIAccess::operator=(const CWMIAccess& w)
{
	if (this != &w)
	{
		m_namespaceName = w.m_namespaceName; 
		m_className = w.m_className;
	}

	return *this;
}

CWMIAccess::~CWMIAccess(void)
{
	Close();
	Disconnect();
}

bool CWMIAccess::Connect(PCTSTR pNamespaceName, bool doCreate)
{
	if ((pNamespaceName && _tcsnlen(pNamespaceName, MAX_STRING_LENGTH) == 0))
		return false;
	else if (pNamespaceName)
	{
		m_namespaceName = pNamespaceName; 
	}

	Disconnect ();

    HRESULT hr = ERROR_SUCCESS;

    if (!m_pLocator)
    {
        hr = m_pLocator.CreateInstance(CLSID_WbemLocator);

        if (FAILED(hr))
        {
			throw FTW::FTWErrorCodeException(hr);
        }
    }

	hr = m_pLocator->ConnectServer (m_namespaceName, NULL, NULL, 0, NULL, 0, 0, &m_pService);

	if (doCreate && hr == WBEM_E_INVALID_NAMESPACE)
	{
		CreateNamespace(m_namespaceName);

		hr = m_pLocator->ConnectServer (m_namespaceName, NULL, NULL, 0, NULL, 0, 0, &m_pService);
	}
	
	if (FAILED (hr))
	{
		Disconnect();
		throw FTWErrorCodeException(hr, m_namespaceName);
	}

	hr = CoSetProxyBlanket(m_pService, 
                        RPC_C_AUTHN_DEFAULT,
                        RPC_C_AUTHZ_NONE,
                        NULL,
                        RPC_C_AUTHN_LEVEL_CALL,
                        RPC_C_IMP_LEVEL_IMPERSONATE, 
                        NULL, 
                        EOAC_NONE);

	if (FAILED (hr))
	{
		Disconnect();
		throw FTWErrorCodeException(hr);
	}

	return false;
}



bool CWMIAccess::CreateNamespace(PCTSTR pNamespaceName)
{
	IWbemClassObjectPtr pRootNamespace;
	IWbemClassObjectPtr pNewNamespace;

	if (!pNamespaceName || (pNamespaceName && _tcsnlen(pNamespaceName, MAX_STRING_LENGTH) == 0))
		return false;
	
	CString namespaceName = pNamespaceName;

	int firstSlash = namespaceName.Find(_T("\\"));

	if (namespaceName.Left(firstSlash).CompareNoCase(_T("root")) != 0 || namespaceName.Find(_T("\\"), firstSlash + 1) > -1)
	{
		Disconnect();
		throw FTWErrorCodeException(WBEM_E_INVALID_NAMESPACE, pNamespaceName);
	}

	_variant_t newNamespaceName(namespaceName.Mid(firstSlash + 1));

	HRESULT hr = m_pLocator->ConnectServer (_T("root"), NULL, NULL, 0, NULL, 0, 0, &m_pService);

	if (FAILED (hr))
	{
		Disconnect();
		throw FTWErrorCodeException(hr, _T("root"));
	}

	hr = CoSetProxyBlanket(m_pService, 
                        RPC_C_AUTHN_DEFAULT,
                        RPC_C_AUTHZ_NONE,
                        NULL,
                        RPC_C_AUTHN_LEVEL_CALL,
                        RPC_C_IMP_LEVEL_IMPERSONATE, 
                        NULL, 
                        EOAC_NONE);

	if (FAILED (hr))
	{
		Disconnect();
		throw FTWErrorCodeException(hr);
	}

	hr = m_pService->GetObject(_T("__Namespace"), WBEM_FLAG_RETURN_WBEM_COMPLETE, NULL, &pRootNamespace, NULL);

	if (FAILED (hr))
	{
		Disconnect();
		throw FTWErrorCodeException(hr, _T("__Namespace"));
	}

	hr = pRootNamespace->SpawnInstance(0, &pNewNamespace);
	
	if (FAILED (hr))
	{
		Disconnect();
		throw FTWErrorCodeException(hr, pNamespaceName);
	}

	hr = pNewNamespace->Put(_T("Name"), 0, &newNamespaceName, 0);

	if (FAILED (hr))
	{
		Disconnect();
		throw FTWErrorCodeException(hr, pNamespaceName);
	}

	hr = m_pService->PutInstance(pNewNamespace, WBEM_FLAG_CREATE_OR_UPDATE, NULL, NULL);

	if (FAILED (hr))
	{
		Disconnect();
		throw FTWErrorCodeException(hr, pNamespaceName);
	}

	return true;
}

void CWMIAccess::Disconnect()
{

	Close();
	
	if (m_pService)
	{
		//m_pService->Release();
		m_pService = nullptr;
	}

	if (m_pLocator)
	{
		//m_pLocator->Release();
		m_pLocator = nullptr;
	}

}

void CWMIAccess::Close()
{
//	m_pInstance = 0;
	
	if (m_pClass)
	{
		m_pClass.Release();
		m_pClass = 0;
	}

}

//bool CWMIAccess::NextInstance()
//{
//	if (m_key.length() > 0 || m_pInstance == NULL)
//		return false;
//
//	return GetNextInstance();
//}

bool CWMIAccess::GetFirstInstance(IWbemClassObjectPtr& pInstance)
{
	IEnumWbemClassObjectPtr pEnum;
	ULONG instanceCount = 0;
	
	HRESULT hr = m_pService->CreateInstanceEnum(m_className, WBEM_FLAG_SHALLOW, NULL, &pEnum);

	if (FAILED(hr))
		throw FTWErrorCodeException(hr);

	hr = pEnum->Next(10000, 1, &pInstance, &instanceCount);
	
	if(FAILED(hr))
		throw FTWErrorCodeException(hr);

	if (pInstance == NULL || instanceCount == 0)
		return false;

	return true;
}

//bool CWMIAccess::GetNextInstance(void)
//{
//	ULONG instanceCount = 0;
//
//	//ASSERT(m_pInstance);
//	
//	HRESULT hr = m_pEnum->Next(10000, 1, &m_pInstance, &instanceCount);
//
//	if(FAILED(hr))
//		throw FTWErrorCodeException(hr);
//
//	if (m_pInstance == NULL || instanceCount == 0)
//		return false;
//
//	return true;
//}

bool CWMIAccess::Open(PCTSTR pClassName, bool doCreate, WMIPropertiesList* pProps)
{
	if (!pClassName || (pClassName && _tcsnlen(pClassName, MAX_STRING_LENGTH) == 0))
		return false;

	m_className = pClassName; 

	Close();
	
	HRESULT hr = ERROR_SUCCESS;

	//IWbemClassObjectPtr pClassTest;

	hr = m_pService->GetObject(m_className, WBEM_FLAG_RETURN_WBEM_COMPLETE, NULL, &m_pClass, NULL);

	if(doCreate && hr == WBEM_E_NOT_FOUND)
	{
		return CreateClass(pClassName, pProps);
	}

	if(FAILED(hr))
		throw FTWErrorCodeException(hr);

	return true;

}

bool CWMIAccess::CreateClass(PCTSTR pClassName, WMIPropertiesList* pProps)
{
	ASSERT(pProps);
	
	if (!pClassName || (pClassName && _tcsnlen(pClassName, MAX_STRING_LENGTH) == 0))
		return false;

	HRESULT hr = m_pService->GetObject(0, 0, NULL, &m_pClass, NULL);

	if(FAILED(hr))
		throw FTWErrorCodeException(hr, _T("Getting blank object"));

	_variant_t v (pClassName);

	hr = m_pClass->Put(_T("__CLASS"), 0, &v, 0);

	if(FAILED(hr))
		throw FTWErrorCodeException(hr, _T("Setting __CLASS property"));

	CreateProperties(pProps);

	hr = m_pService->PutClass(m_pClass, 0, 0, 0);

	if(FAILED(hr))
		throw FTWErrorCodeException(hr, _T("Putting new class"));

	Close();
	
	return true;

}

bool CWMIAccess::CreateInstance(WMIPropertiesList& props)
{
	HRESULT hr = S_OK;
	IWbemClassObjectPtr pInstance;

	ASSERT (m_pService);
//	ASSERT (!m_pInstance);

	if (!m_pClass)
	{
		hr = m_pService->GetObject(m_className, WBEM_FLAG_RETURN_WBEM_COMPLETE, NULL, &m_pClass, NULL);

		if(FAILED(hr))
			throw FTWErrorCodeException(hr, _T("Getting Class"));
	}

	hr = m_pClass->SpawnInstance(0, &pInstance);

	if(FAILED(hr))
		throw FTWErrorCodeException(hr, _T("Spawning new class instance"));
	
	for(WMIPropertiesListIterator iter = props.begin(); iter != props.end(); iter++)
	{
		if (_tcsnlen(iter->propertyName, MAX_STRING_LENGTH) == 0 || 
			_tcsnlen(iter->propertyValue, MAX_STRING_LENGTH) == 0 ||
			_tcsnlen(iter->propertyType, MAX_STRING_LENGTH) == 0)
			continue;

		_variant_t value(iter->propertyValue);

		hr = pInstance->Put((LPCTSTR)(iter->propertyName), 0, &value, GetCIMTYPEFromString(iter->propertyType));
	
		if(FAILED(hr))
			throw FTWErrorCodeException(hr, _T("Putting value"));
	}

	hr = m_pService->PutInstance(pInstance, WBEM_FLAG_CREATE_OR_UPDATE, 0, 0);

	if(FAILED(hr))
		throw FTWErrorCodeException(hr, _T("Writing instance"));

	return true;
}

bool CWMIAccess::CreateProperties(WMIPropertiesList* pProps)
{
	HRESULT hr = S_OK;

	ASSERT (m_pService);
	ASSERT (pProps);
	ASSERT (m_pClass);

	for(WMIPropertiesListIterator iter = pProps->begin(); iter != pProps->end(); iter++)
	{
		if (_tcsnlen(iter->propertyName, MAX_STRING_LENGTH) == 0 || 
			_tcsnlen(iter->propertyValue, MAX_STRING_LENGTH) == 0 ||
			_tcsnlen(iter->propertyType, MAX_STRING_LENGTH) == 0)
			continue;

		hr = m_pClass->Put(iter->propertyName, 0, 0, GetCIMTYPEFromString(iter->propertyType));

		if(FAILED(hr))
			throw FTWErrorCodeException(hr, _T("Adding property to new class"));

		if (iter->isPropertyKey)
		{
			IWbemQualifierSetPtr pQual;
			hr = m_pClass->GetPropertyQualifierSet(iter->propertyName, &pQual);

			if(FAILED(hr))
				throw FTWErrorCodeException(hr);

			_variant_t v (true);

			hr = pQual->Put(_T("Key"), &v, 0);
		
			if(FAILED(hr))
				throw FTWErrorCodeException(hr);

		}

		//hr = m_pService->PutClass(m_pClass, 0, 0, 0);

		//if(FAILED(hr))
		//	throw JHResultException(hr, _T("Adding key qualifier to new class"));

	}

	return true;
}

bool CWMIAccess::ExecMethod(PCTSTR pMethodName, PCTSTR pReturnPropertyName, _variant_t & returnValue)
{
	//IWbemClassObjectPtr pOutParams = NULL;
	IWbemClassObject* pOutParams = NULL;
	IWbemClassObjectPtr pInstance;

	variant_t objectPath;
	_bstr_t methodName(pMethodName);
	_bstr_t propertyName(pReturnPropertyName);

	ASSERT(m_pClass);

	if (GetFirstInstance(pInstance))
	{
		HRESULT hr = pInstance->Get(_T("__PATH"), 0, &objectPath, NULL, NULL);

		if (FAILED(hr))
			throw FTWErrorCodeException(hr);

		hr = m_pService->ExecMethod(_bstr_t(objectPath), methodName, 0, NULL, NULL, &pOutParams, NULL);

		if (FAILED(hr))
			throw FTWErrorCodeException(hr);

		hr = pOutParams->Get(propertyName, 0, &returnValue, NULL, 0);

		if (FAILED(hr))
			throw FTWErrorCodeException(hr);

		return true;
	}

	return false;
}


bool CWMIAccess::GetProperty(PCTSTR pPropertyName, _variant_t& propertyValue, PCTSTR pObjectKey)
{
	_variant_t temp;
	CIMTYPE ct;
	IWbemClassObjectPtr pInstance = nullptr;

	TRACE3("Get WMI Property: <%s>, <%s>, <%s>", this->Class(), pPropertyName, pObjectKey);
	
	if(!pPropertyName || _tcsnlen(pPropertyName, MAX_WMI_STRING_LENGTH) == 0)
		return false;

	//m_key = pObjectKey;

	//if(!m_pInstance)
	//{
	//	if (pObjectKey && _tcsnlen(pObjectKey, MAX_STRING_LENGTH) > 0)
	//	{
	//		_bstr_t path(m_className);
	//		path += _T(".");
	//		path += pObjectKey;

	//		HRESULT hr = m_pService->GetObject(path, WBEM_FLAG_RETURN_WBEM_COMPLETE, NULL, &m_pInstance, NULL);
	//	
	//		if(FAILED(hr))
	//			throw FTWErrorCodeException(hr);

	//	}
	//	else
	//	{
	//	}
	//}

	if (pObjectKey && _tcsnlen(pObjectKey, MAX_STRING_LENGTH) > 0)
	{
		_bstr_t path(m_className);
		path += _T(".");
		path += pObjectKey;

		HRESULT hr = m_pService->GetObject(path, WBEM_FLAG_RETURN_WBEM_COMPLETE, NULL, &pInstance, NULL);

		if (FAILED(hr))
			throw FTWErrorCodeException(hr);
	}
	else
		GetFirstInstance(pInstance);
	
	//ASSERT(pInstance);

	HRESULT hr = pInstance->Get(pPropertyName, 0, &temp, &ct, NULL);

	if(FAILED(hr))
	{
		throw FTWErrorCodeException(hr);
	}

	if ((temp.vt & VT_ARRAY) == VT_ARRAY)
	{
		if (SafeArrayGetDim(temp.parray) == 1)
		{
			long index;
			VARTYPE vt = temp.vt ^ VT_ARRAY;

			if (SafeArrayGetLBound(temp.parray, 1, &index) == S_OK)
			{
				long lTemp;
				BSTR tmp;
				bool b;

				if ((vt == VT_I4 || vt == VT_I2) 
					&& SafeArrayGetElement(temp.parray, &index, &lTemp) == S_OK)
					temp = lTemp;
				else if ((vt == VT_BSTR) 
					&& SafeArrayGetElement(temp.parray, &index, &tmp) == S_OK)
				{
					temp = tmp;

					::SysFreeString(tmp);
				}
				else if ((vt == VT_BOOL) 
					&& SafeArrayGetElement(temp.parray, &index, &b) == S_OK)
				{
					if (b)
						temp = TRUE_STRING;
					else
						temp = FALSE_STRING;
				}
			}
		}

	}

	propertyValue = temp;

	return true;
}

//CString CWMIAccess::operator[](PCTSTR pPropertyName)
//{
//	CString temp;
//	GetProperty(pPropertyName, temp);
//	return temp;
//}

bool CWMIAccess::GetProperty(PCTSTR pPropertyName, CString& propertyValue, PCTSTR pObjectKey)
{
	_variant_t temp;

	if (GetProperty(pPropertyName, temp, pObjectKey))
	{
		if (temp.vt != VT_BSTR)
			temp.ChangeType(VT_BSTR, NULL);

		if (temp.vt == VT_BSTR && ((_bstr_t)temp).length() > 0)
		{
			propertyValue = ((_bstr_t)temp).GetBSTR();
			return true;
		}
	}

	return false;
}

bool CWMIAccess::GetPropertyAllValues(PCTSTR pPropertyName, CString& propertyValue)
{
	_variant_t temp;

	if (GetPropertyAllValues(pPropertyName, temp))
	{
		if (temp.vt != VT_BSTR)
			temp.ChangeType(VT_BSTR, NULL);

		if (temp.vt == VT_BSTR && ((_bstr_t)temp).length() > 0)
		{
			propertyValue = ((_bstr_t)temp).GetBSTR();
			return true;
		}
	}

	return false;
}

CIMTYPE_ENUMERATION CWMIAccess::GetCIMTYPEFromString(PCTSTR cimType)
{
	if (!cimType || _tcsnlen(cimType, MAX_STRING_LENGTH) == 0)
		return CIM_STRING;

	else if (_tcsnlen(cimType, MAX_STRING_LENGTH) < 5)
		return CIM_ILLEGAL;

	else
	{
		CIMTypeMap map;
		return map[cimType];
	}

	return CIM_ILLEGAL;
}

}