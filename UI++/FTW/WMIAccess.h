#pragma once

#ifdef _UNICODE
#ifndef UNICODE
#define UNICODE
#endif
#endif

#define WIN32_LEAN_AND_MEAN
#define _WIN32_DCOM
//#include <tchar.h>
//#include <windows.h>
#include <list>
#include <map>
#include <comdef.h>
#include <wbemidl.h>
#pragma comment(lib, "wbemuuid.lib")

_COM_SMARTPTR_TYPEDEF(IWbemLocator, __uuidof(IWbemLocator));
_COM_SMARTPTR_TYPEDEF(IWbemServices, __uuidof(IWbemServices));
_COM_SMARTPTR_TYPEDEF(IWbemClassObject, __uuidof(IWbemClassObject));
_COM_SMARTPTR_TYPEDEF(IEnumWbemClassObject, __uuidof(IEnumWbemClassObject));
_COM_SMARTPTR_TYPEDEF(IWbemQualifierSet, __uuidof(IWbemQualifierSet));

namespace FTW
{

struct CIMTypeMap : public std::map<CString, CIMTYPE_ENUMERATION>
{
	CIMTypeMap()
	{
		this->operator[](_T("CIM_EMPTY")) = CIM_EMPTY;
		this->operator[](_T("CIM_SINT8")) = CIM_SINT8;
		this->operator[](_T("CIM_UINT8")) = CIM_UINT8;

		this->operator[](_T("CIM_SINT16")) = CIM_SINT16;
		this->operator[](_T("CIM_UINT16")) = CIM_UINT16;
		this->operator[](_T("CIM_SINT32")) = CIM_SINT32;

		this->operator[](_T("CIM_UINT32")) = CIM_UINT32;
		this->operator[](_T("CIM_SINT64")) = CIM_SINT64;
		this->operator[](_T("CIM_UINT64")) = CIM_UINT64;

		this->operator[](_T("CIM_REAL32")) = CIM_REAL32;
		this->operator[](_T("CIM_REAL64")) = CIM_REAL64;
		this->operator[](_T("CIM_BOOLEAN")) = CIM_BOOLEAN;

		this->operator[](_T("CIM_STRING")) = CIM_STRING;
		this->operator[](_T("CIM_DATETIME")) = CIM_DATETIME;
		this->operator[](_T("CIM_REFERENCE")) = CIM_REFERENCE;

		this->operator[](_T("CIM_CHAR16")) =  CIM_CHAR16;

	};
	~CIMTypeMap(){};
};

struct WMIProperties
{
	CString propertyName;
	CString propertyValue;
	CString	propertyType;
	bool isPropertyKey = false;
};

typedef std::list<WMIProperties> WMIPropertiesList;
typedef std::list<WMIProperties>::iterator WMIPropertiesListIterator;
	
class CWMIAccess
{
public:
	CWMIAccess(PCTSTR pNamespace = 0);
	CWMIAccess(const CWMIAccess& w);
	CWMIAccess& operator=(const CWMIAccess& w);

	~CWMIAccess(void);

	PCTSTR Namespace() const
	{
		return (PCTSTR)m_namespaceName;
	}

	PCTSTR Class() const
	{
		return (PCTSTR)m_className;
	}

	bool Connect(PCTSTR pNamespaceName = 0, bool doCreate = false);
	void Disconnect(void);
	bool Open(PCTSTR pClassName, bool doCreate = false, WMIPropertiesList* pProps = 0);
	void Close(void);

	bool GetPropertyFromSingleInstance(PCTSTR pPropertyName, CString& propertyValue, PCTSTR pObjectKey = nullptr);
	bool GetPropertyFromAllInstances(PCTSTR pPropertyName, CString& propertyValues);
	bool GetPropertyFromAllInstances(PCTSTR pPropertyName, CString& propertyValues, IEnumWbemClassObjectPtr& pEnum);
	bool GetPropertyFromQuery(PCTSTR pQuery, PCTSTR pPropertyName, CString & propertyValues);
	int InstanceCount(void);
	bool CreateInstance(WMIPropertiesList& props);
	
	//bool CreateProperty(PCTSTR pPropertyName, PCTSTR propertyType, bool isKey = false);

	bool ExecMethod(PCTSTR pMethodName, PCTSTR pReturnPropertyName, _variant_t& returnValue, bool isClassMethod = false);

protected:
	_bstr_t m_namespaceName;
	_bstr_t m_className;
	//_bstr_t m_key;

	//CComPtr<IWbemLocator> m_pLocator;
    //CComPtr<IWbemServices> m_pService;
    //CComPtr<IWbemClassObject> m_pInstance;

	IWbemLocatorPtr m_pLocator;
	IWbemServicesPtr m_pService;
	//IWbemClassObjectPtr m_pInstance;
	//IEnumWbemClassObjectPtr m_pEnum;
	IWbemClassObjectPtr m_pClass;

	bool _GetProperty(PCTSTR pPropertyName, _variant_t& propertyValue, IWbemClassObjectPtr& pCurrentInstance);
	bool GetPropertyFromInstance(IWbemClassObjectPtr & pInstance, PCTSTR pPropertyName, CString & propertyValue);
	bool GetFirstInstance(IWbemClassObjectPtr& pInstance);
	bool GetNextInstance(IWbemClassObjectPtr& pCurrentInstance, IEnumWbemClassObjectPtr& pEnum);

	bool CreateNamespace(PCTSTR pNamespaceName);
	bool CreateClass(PCTSTR pClassName, WMIPropertiesList* pProps);
	bool CreateProperties(WMIPropertiesList* pProps);

	CIMTYPE_ENUMERATION  GetCIMTYPEFromString(PCTSTR cimType = 0);

};

}