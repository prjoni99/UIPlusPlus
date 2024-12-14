#include "StdAfx.h"
#include "Actions.h"
#include "resource.h"
#include "TSVar.h"

#include "Dialogs\DlgUserInput.h"

namespace UIpp
{
	INT_PTR CActionUserInput::Go(void)
	{
		CString text, variable, altVariable;
		CString hint, prompt, regex, defaultAnswer;
		CString option, altValue;
		bool required, password, autoComplete, hScroll;
		CString forceCase;
		CString adValidateField = _T("");
		CString adValidateType = _T("");;

		pugi::xpath_node_set choiceNodes, choiceListNodes;
		pugi::xml_node choiceNode, choiceListNode;
		//Values values;
		//COptionValueMap choices;
		CUserInputChoiceOptions choices;
		CString choiceValue;

		CString actionTitle = GetXMLAttribute(m_actionData.pActionNode, XML_ATTRIBUTE_TITLE, XML_ACTION_TYPE_USERINPUT);
		bool centerTitle = FTW::IsTrue(GetXMLAttribute(m_actionData.pActionNode, XML_ATTRIBUTE_CENTERTITLE, XML_ACTION_FALSE));
		CString actionSize = GetXMLAttribute(m_actionData.pActionNode, XML_ATTRIBUTE_SIZE, XML_ATTRIBUTE_SIZE_REGULAR);
		bool includeBack = FTW::IsTrue(GetXMLAttribute(m_actionData.pActionNode, XML_ATTRIBUTE_SHOWBACK, XML_ACTION_FALSE));
		bool includeCancel = FTW::IsTrue(GetXMLAttribute(m_actionData.pActionNode, XML_ATTRIBUTE_SHOWCANCEL, XML_ACTION_FALSE));
		bool noDefaultButton = FTW::IsTrue(GetXMLAttribute(m_actionData.pActionNode, XML_ATTRIBUTE_NODEFAULT, XML_ACTION_FALSE));

		CDlgBase::DlgSize mySize = CDlgBase::DlgSize::Regular;

		if (actionSize == XML_ATTRIBUTE_SIZE_TALL)
			mySize = CDlgBase::DlgSize::Tall;

		CDlgBase::DialogVisibilityFlags dlgFlags = CDlgBase::BuildDialogVisibilityFlags(
											includeCancel,
											includeBack, 
											true, 
											centerTitle,
											noDefaultButton);

		CDlgUserInput dlg(actionTitle, dlgFlags, m_actionData, mySize);

		for (pugi::xml_node xmlinput = m_actionData.pActionNode->first_child(); xmlinput; xmlinput = xmlinput.next_sibling())
		{

			if (_tcsicmp(xmlinput.name(), XML_ACTION_USERINPUT_TEXT) != 0 
				&& _tcsicmp(xmlinput.name(), XML_ACTION_USERINPUT_CHOICE) != 0 
				&& _tcsicmp(xmlinput.name(), XML_ACTION_USERINPUT_CHECKBOX) != 0
				&& _tcsicmp(xmlinput.name(), XML_ACTION_USERINPUT_INFO) != 0
				&& _tcsicmp(xmlinput.name(), XML_ACTION_USERINPUT_BROWSE) != 0
				&& _tcsicmp(xmlinput.name(), XML_ACTION_USERINPUT_TEXT_OLD) != 0
				&& _tcsicmp(xmlinput.name(), XML_ACTION_USERINPUT_CHOICE_OLD) != 0
				&& _tcsicmp(xmlinput.name(), XML_ACTION_USERINPUT_CHECKBOX_OLD) != 0)
				continue;

			text = GetXMLAttribute(&xmlinput, XML_ATTRIBUTE_QUESTION, XML_ATTRIBUTE_QUESTION_DEF);

			if (CActionHelper::EvalCondition(m_actionData.pCMLog, xmlinput.attribute(XML_ATTRIBUTE_CONDITION).value(), m_actionData.pScriptHost, xmlinput.name(), text))
			{
				variable = GetXMLAttribute(&xmlinput, XML_ATTRIBUTE_VARIABLE, XML_ATTRIBUTE_VARIABLE_DEF);

				defaultAnswer.Empty();

				if (!CTSEnv::Instance().Get(variable, defaultAnswer) || defaultAnswer.GetLength() == 0)
					defaultAnswer = GetXMLAttribute(&xmlinput, XML_ATTRIBUTE_DEFAULT);

				if (_tcsicmp(xmlinput.name(), XML_ACTION_USERINPUT_TEXT) == 0 || _tcsicmp(xmlinput.name(), XML_ACTION_USERINPUT_TEXT_OLD) == 0)
				{
					hint = GetXMLAttribute(&xmlinput, XML_ATTRIBUTE_HINT);
					prompt = GetXMLAttribute(&xmlinput, XML_ATTRIBUTE_PROMPT);
					regex = GetXMLAttribute(&xmlinput, XML_ATTRIBUTE_REGEX);
					required = FTW::IsTrue(GetXMLAttribute(&xmlinput, XML_ATTRIBUTE_REQUIRED, XML_ACTION_TRUE));
					password = FTW::IsTrue(GetXMLAttribute(&xmlinput, XML_ATTRIBUTE_PASSWORD, XML_ACTION_FALSE));
					hScroll = FTW::IsTrue(GetXMLAttribute(&xmlinput, XML_ATTRIBUTE_HSCROLL, XML_ACTION_FALSE));
					forceCase = GetXMLAttribute(&xmlinput, XML_ATTRIBUTE_FORCECASE);
					bool readonly = FTW::IsTrue(GetXMLAttribute(&xmlinput, XML_ATTRIBUTE_READONLY, XML_ACTION_FALSE));

					adValidateType = GetXMLAttribute(&xmlinput, XML_ATTRIBUTE_ADVALIDATE, XML_ACTION_FALSE);

					TRACE1("AD Validate Type: %s\n", adValidateType);

					if (FTW::IsTrue(adValidateType))
					{
						adValidateType = VALUE_COMPUTER;
					}

					if (_tcsicmp(adValidateType, VALUE_COMPUTER) == 0
						|| _tcsicmp(adValidateType, VALUE_COMPUTERWARNING) == 0
						|| _tcsicmp(adValidateType, VALUE_USER) == 0
						|| _tcsicmp(adValidateType, VALUE_USERWARNING) == 0)
					{
						adValidateField = variable;
					}

					m_actionData.pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info, 
						AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
						FTW::FormatResourceString(IDS_LOGMSG_ADDINPUT, XML_ACTION_USERINPUT_TEXT, text));

					dlg.AddTextInput(text, variable, m_actionData.globalDialogTraits.fontFace, hint, prompt, regex, required, defaultAnswer, password, forceCase, hScroll, readonly);
				}
				else if (_tcsicmp(xmlinput.name(), XML_ACTION_USERINPUT_CHECKBOX) == 0 || _tcsicmp(xmlinput.name(), XML_ACTION_USERINPUT_CHECKBOX_OLD) == 0)
				{
					CString checkedValue = GetXMLAttribute(&xmlinput, XML_ACTION_USERINPUT_CHECKED_VALUE);
					CString uncheckedValue = GetXMLAttribute(&xmlinput, XML_ACTION_USERINPUT_UNCHECKED_VALUE);
					
					//FTW::CCMLog::Instance().LogMsg(FTW::CCMLog::Info, AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
					//	IDS_LOGMSG_ADDINPUT, XML_ACTION_USERINPUT_TEXT, text);

					dlg.AddCheckboxInput(text, variable, m_actionData.globalDialogTraits.fontFace, checkedValue, uncheckedValue, defaultAnswer);

				}
				else if (_tcsicmp(xmlinput.name(), XML_ACTION_USERINPUT_INFO) == 0)
				{
					text = CTSEnv::Instance().VariableSubstitute(xmlinput.child_value());
					CString textColor = GetXMLAttribute(&xmlinput, XML_ATTRIBUTE_COLOR);
					int numLines = GetXMLIntAttribute(&xmlinput, XML_ATTRIBUTE_NUMBEROFLINES, 1);

					text.Replace(_T("\\t"), _T("\t"));
					text.Replace(_T("\\r"), _T("\r"));
					text.Replace(_T("\\n"), _T("\n"));

					_variant_t r;

					if (SUCCEEDED(m_actionData.pScriptHost->Eval(text, &r)) && r.vt > 0 && ((_bstr_t)r).length() > 0)
					{
						text = ((_bstr_t)r).GetBSTR();
					}

					m_actionData.pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info, 
						AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
						FTW::FormatResourceString(IDS_LOGMSG_ADDINFO, XML_ACTION_TYPE_USERINFO, text));

					dlg.AddInfo(text, m_actionData.globalDialogTraits.fontFace, numLines, FTW::HexToCOLORREF(textColor));
				}
				else if (_tcsicmp(xmlinput.name(), XML_ACTION_USERINPUT_CHOICE) == 0 || _tcsicmp(xmlinput.name(), XML_ACTION_USERINPUT_CHOICE_OLD) == 0)
				{
					choiceNodes = xmlinput.select_nodes(XML_ACTION_USERINPUT_CHOICE2);
					choiceListNodes = xmlinput.select_nodes(XML_ACTION_USERINPUT_CHOICELIST);
					choices.Clear();

					for (size_t choiceIndex = 0; choiceIndex < choiceNodes.size(); choiceIndex++)
					{
						choiceNode = choiceNodes[choiceIndex].node();

						option = GetXMLAttribute(&choiceNode, XML_ATTRIBUTE_OPTION);

						if (CActionHelper::EvalCondition(m_actionData.pCMLog, choiceNode.attribute(XML_ATTRIBUTE_CONDITION).value(), m_actionData.pScriptHost, choiceNode.name(), option))
						{
							//values.value1 = GetXMLAttribute(&choiceNode, XML_ATTRIBUTE_VALUE, option);
							//values.value2 = GetXMLAttribute(&choiceNode, XML_ATTRIBUTE_ALTVALUE);

							//std::pair<COptionValueMapIterator, bool> insertResult = choices.insert(COptionValuePair(option, values));

							choiceValue = GetXMLAttribute(&choiceNode, XML_ATTRIBUTE_VALUE, option);
							altValue = GetXMLAttribute(&choiceNode, XML_ATTRIBUTE_ALTVALUE);
							choices.AddItem(option, choiceValue, altValue);

							m_actionData.pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info, 
								AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
								FTW::FormatResourceString(IDS_LOGMSG_ADDCHOICE, option, choiceValue, altValue));
						}
						else
							m_actionData.pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info, 
								AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
							FTW::FormatResourceString(IDS_LOGMSG_SKIPCHOICE, option));

					}

					int optionIndex = 0;
					int valueIndex = 0;
					int altValueIndex = 0;
					CString optionList;
					CString valueList;
					CString altValueList;

					for (size_t choiceIndex = 0; choiceIndex < choiceListNodes.size(); choiceIndex++)
					{
						choiceListNode = choiceListNodes[choiceIndex].node();
						optionList = GetXMLAttribute(&choiceListNode, XML_ATTRIBUTE_OPTIONLIST);

						if (CActionHelper::EvalCondition(m_actionData.pCMLog, choiceListNode.attribute(XML_ATTRIBUTE_CONDITION).value(), m_actionData.pScriptHost, choiceListNode.name(), option))
						{
							valueList = GetXMLAttribute(&choiceListNode, XML_ATTRIBUTE_VALUELIST, optionList);
							altValueList = GetXMLAttribute(&choiceListNode, XML_ATTRIBUTE_ALTVALUELIST);

							m_actionData.pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info, 
								AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
								FTW::FormatResourceString(IDS_LOGMSG_ADDCHOICELIST, optionList));

							m_actionData.pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info, 
								AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
								FTW::FormatResourceString(IDS_LOGMSG_ADDCHOICEVALUELIST, valueList));

							m_actionData.pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info, 
								AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
								FTW::FormatResourceString(IDS_LOGMSG_ADDCHOICEALTVALUELIST, altValueList));

							option = optionList.Tokenize(_T(",;"), optionIndex);

							while (optionIndex != -1)
							{
								if (valueIndex != -1)
									choiceValue = valueList.Tokenize(_T(",;"), valueIndex);
								else
									choiceValue = _T("");

								if (altValueIndex != -1)
									altValue = altValueList.Tokenize(_T(",;"), altValueIndex);
								else
									altValue = _T("");

								m_actionData.pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info,
									AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
									FTW::FormatResourceString(IDS_LOGMSG_ADDCHOICE, option, choiceValue, altValue));

								choices.AddItem(option.Trim(), choiceValue.Trim(), altValue.Trim());

								option = optionList.Tokenize(_T(",;"), optionIndex);
							}
							
						}
						else
							m_actionData.pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info,
								AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
								FTW::FormatResourceString(IDS_LOGMSG_SKIPCHOICE, optionList));
					}

					if (!choices.IsEmpty())
					{
						required = FTW::IsTrue(GetXMLAttribute(&xmlinput, XML_ATTRIBUTE_REQUIRED, XML_ACTION_FALSE));
						altVariable = GetXMLAttribute(&xmlinput, XML_ATTRIBUTE_ALTVARIABLE);
						autoComplete = FTW::IsTrue(GetXMLAttribute(&xmlinput, XML_ATTRIBUTE_AUTOCOMPLETE, XML_ACTION_FALSE));
						bool sortChoices = FTW::IsTrue(GetXMLAttribute(&xmlinput, XML_ATTRIBUTE_SORT, XML_ACTION_TRUE));
						int dropListSize = GetXMLIntAttribute(&xmlinput, XML_ATTRIBUTE_DROPDOWNSIZE, 5);

						m_actionData.pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info,
							AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
							FTW::FormatResourceString(IDS_LOGMSG_ADDINPUT, XML_ACTION_USERINPUT_CHOICE, text));

						dlg.AddComboInput(text, variable, m_actionData.globalDialogTraits.fontFace, altVariable, choices, dropListSize, sortChoices, defaultAnswer, required, autoComplete);
					}
					else
					{
						m_actionData.pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info,
							AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
							FTW::FormatResourceString(IDS_LOGMSG_NOTADDINGINPUT_NOCHOICE, xmlinput.name(), text));
					}

				}
				//else if (_tcsicmp(xmlinput.name(), XML_ACTION_USERINPUT_BROWSE) == 0)
				//{
				//	dlg.AddBrowseInput(text, variable);
				//}
			}
			else
			{
				m_actionData.pCMLog->WriteMsg(FTWCMLOG::CCMLog::MsgType::Info, 
					AfxGetThread()->m_nThreadID, __TFILE__, __LINE__,
					FTW::FormatResourceString(IDS_LOGMSG_NOTADDINGINPUT, xmlinput.name(), text));

			}
		}

		if (!adValidateField.IsEmpty())
			dlg.SetFieldToADValidate(adValidateField, adValidateType);

		return dlg.DoModal();
	}
}