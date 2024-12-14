#include "StdAfx.h"
#include "IAction.h"

namespace UIpp
{

	CString GetXMLAttribute(const pugi::xml_node& node, PCTSTR attributeName, DWORD defaultValue, CTSEnv* ptsEnv, bool raw)
	{
		CString defaultString = _T("");

		defaultString.LoadString(defaultValue);

		return GetXMLAttribute(node, attributeName, defaultString, ptsEnv, raw);
	}
	
	CString GetXMLAttribute(const pugi::xml_node* const node, PCTSTR attributeName, DWORD defaultValue, CTSEnv* ptsEnv, bool raw)
	{
		return GetXMLAttribute(*node, attributeName, defaultValue, ptsEnv, raw);
	}

	CString GetXMLAttribute(const pugi::xml_node& node, PCTSTR attributeName, PCTSTR defaultValue, CTSEnv* ptsEnv, bool raw)
	{
		CString attributeValue = _T("");
		CString attrName = attributeName;

		attrName.Trim();

		if (attrName.GetLength() == 0)
			return _T("");

		if (node == nullptr)
			return defaultValue;

		attributeValue = node.attribute(attrName).value();

		if (attributeValue.GetLength() > 0 && raw == false)
			if (ptsEnv == nullptr)
				return CTSEnv::Instance().VariableSubstitute(attributeValue);
			else
				return ptsEnv->VariableSubstitute(attributeValue);
		else if (attributeValue.GetLength() > 0)
			return attributeValue;
		else if (defaultValue != NULL && _tcsnlen(defaultValue, MAX_ATTRIBUTE_LENGTH) > 0)
			return defaultValue;
		else
			return _T("");
	}

	CString GetXMLAttribute(const pugi::xml_node* const node, PCTSTR attributeName, PCTSTR defaultValue, CTSEnv* ptsEnv, bool raw)
	{
		return GetXMLAttribute(*node, attributeName, defaultValue, ptsEnv, raw);
	}

	int GetXMLIntAttribute(const pugi::xml_node& node, PCTSTR attributeName, int defaultValue, CTSEnv* ptsEnv)
	{
		CString attributeValue = _T("");
		CString attrName = attributeName;
		PTSTR pStop;

		attrName.Trim();

		if (attrName.GetLength() == 0)
			return 0;

		attributeValue = node.attribute(attrName).value();

		if (attributeValue.GetLength() > 0)
			return _tcstol(attributeValue, &pStop, 10);
		else
			return defaultValue;
	}

	int GetXMLIntAttribute(const pugi::xml_node* const node, PCTSTR attributeName, int defaultValue, CTSEnv* ptsEnv)
	{
		return GetXMLIntAttribute(*node, attributeName, defaultValue, ptsEnv);
	}

	CActionFactory& CActionFactory::Instance()
	{
		// So called Meyers Singleton implementation,
		// In C++ 11 it is in fact thread-safe
		// In older versions you should ensure thread-safety here
		static CActionFactory factory;
		return factory;
	}

	void CActionFactory::RegisterMaker(const std::wstring key, IActionMaker* maker)
	{
		// Validate uniquness and add to the map
		if (_makers.find(key) != _makers.end())
		{
			throw new std::exception("Multiple makers for given key!");
		}
		_makers[key] = maker;
	}

	IAction* CActionFactory::Create(const std::wstring key, ActionData& actionData) const
	{
		//// If XML node relies on some external library,
		//// load it before continuing
		//rapidxml::xml_attribute<>* pAttr = node->first_attribute("LibraryName");
		//if (pAttr != NULL)
		//{
		//	std::string libraryName(pAttr->value());
		//	// Note: Unicode has been disabled to avoid extra conversions
		//	// (This is a toy example)
		//	LoadLibrary(libraryName.c_str());
		//}

		// Look up the maker by nodes name
		//std::string key(node->name());

		auto i = _makers.find(key);

		if (i == _makers.end())
		{
			throw new std::exception("Unrecognized object type!");
		}

		IActionMaker* maker = i->second;
		// Invoke create polymorphiclly
		return maker->Create(actionData);
	}
}

//CString GetXMLAttributeFromChildElement(const pugi::xml_node* const node, 
//	PCTSTR elementName, 
//	PCTSTR attributeNametoMatch,
//	PCTSTR attributeValuetoMatch,
//	PCTSTR attributeName,
//	PCTSTR defaultValue, bool raw)
//{
//	CString attributeValue = _T("");
//	CString attrName = attributeName;
//	CString elemName = elementName;
//	CString attrMatchName = attributeNametoMatch;
//	CString attrMatchValue = attributeValuetoMatch;
//
//	attrName.Trim();
//	elemName.Trim();
//	attrMatchName.Trim();
//	attrMatchValue.Trim();
//
//	if (attrName.GetLength() == 0 || elemName.GetLength() == 0
//		|| attrMatchName.GetLength() == 0 || attrMatchValue.GetLength() == 0)
//		return _T("");
//
//	if (node == nullptr)
//		return defaultValue;
//
//	pugi::xml_node childNode = node->find_child_by_attribute(elemName, attrMatchName, attrMatchValue);
//
//	if (childNode == NULL)
//		return defaultValue;
//
//	else 
//		return GetXMLAttribute(&childNode, attrName, defaultValue);
//}
