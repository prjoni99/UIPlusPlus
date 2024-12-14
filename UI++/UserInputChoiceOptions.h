#pragma once

namespace UIpp {

	struct ChoiceValues
	{
		ChoiceValues() {}

		ChoiceValues(PCTSTR val1, PCTSTR val2)
			: value1(val1), value2(val2) {}

		//ChoiceValues(ChoiceValues& vals)
		//	: value1(vals.value1), value2(vals.value2) {}

		ChoiceValues(const ChoiceValues& vals)
			: value1(vals.value1), value2(vals.value2) {}

		ChoiceValues(ChoiceValues&& vals) noexcept
			: value1(vals.value1), value2(vals.value2) {}

		ChoiceValues& operator=(ChoiceValues& rval)
		{
			value1 = rval.value1;
			value2 = rval.value2;

			return *this;
		}

		CString value1;
		CString value2;
	};

	typedef std::pair<CString, ChoiceValues> COptionValuePair;

	typedef std::map<CString, ChoiceValues> COptionValueMap;
	typedef std::map<CString, ChoiceValues>::const_iterator COptionValueMapIterator;

	typedef std::forward_list<CString> COptionValueList;
	typedef std::forward_list<CString>::const_iterator COptionValueListIterator;

	class CUserInputChoiceOptions
	{
	public:
		CUserInputChoiceOptions() {};
		~CUserInputChoiceOptions() {};

		bool AddItem(CString optionName, CString value1, CString value2)
		{
			ChoiceValues values(value1, value2);
			//values.value1 = value1;
			//values.value2 = value2;

			std::pair<COptionValueMapIterator, bool> insertResult = m_optionMap.insert(COptionValuePair(optionName, values));

			if (insertResult.second == true)
			{
				m_optionList.push_front(optionName);

				return true;
			}

			return false;
		}

		void Clear(void)
		{
			m_optionList.clear();
			m_optionMap.clear();
		};

		bool IsEmpty(void)
		{
			return m_optionMap.empty();
		};

		CString Find(PCTSTR optionName, int valueNumber)
		{
			CString value;

			if (valueNumber == 2)
				value = m_optionMap.find(optionName)->second.value2;
			else
				value = m_optionMap.find(optionName)->second.value1;

			return value;
		}

		COptionValueListIterator Begin()
		{
			if (!m_reversed)
			{
				m_optionList.reverse();

				m_reversed = true;
			}

			return m_optionList.begin();
		}

		COptionValueListIterator End()
		{
			return m_optionList.end();
		}

		COptionValueMapIterator BeginSorted()
		{
			return m_optionMap.begin();
		}

		COptionValueMapIterator EndSorted()
		{
			return m_optionMap.end();
		}

	private:
		COptionValueMap m_optionMap;
		COptionValueList m_optionList;

		bool m_reversed = false;

	};
}